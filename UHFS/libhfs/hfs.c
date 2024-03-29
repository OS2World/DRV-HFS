/*
 * hfsutils - tools for reading and writing Macintosh HFS volumes
 * Copyright (C) 1996, 1997 Robert Leslie
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifdef UHFS

# define INCL_NOPMAPI
# include <os2.h>

# include <stdlib.h>
# include <stdio.h>
# include <errno.h>
# include <string.h>
# include <time.h>
# include <ctype.h>

#include "../misc.h"

#else

# include <stdlib.h>
# include <stdio.h>
# include <unistd.h>
# include <fcntl.h>
# include <sys/stat.h>
#endif

# include "internal.h"
# include "data.h"
# include "block.h"
# include "low.h"
# include "file.h"
# include "btree.h"
# include "node.h"
# include "record.h"
# include "volume.h"

char *hfs_error = "no error";	/* static error string */

hfsvol *hfs_mounts;		/* linked list of mounted volumes */
hfsvol *hfs_curvol;		/* current volume */

/* High-Level Volume Routines ============================================== */

/*
 * NAME:	hfs->flush()
 * DESCRIPTION:	flush all pending changes to an HFS volume
 */
int hfs_flush(hfsvol *vol)
{
  hfsfile *file;

  if (v_getvol(&vol) < 0)
    return -1;

  for (file = vol->files; file; file = file->next)
    {
      if (f_flush(file) < 0)
	return -1;
    }

  if (v_flush(vol, 0) < 0)
    return -1;

  return 0;
}

/*
 * NAME:	hfs->flushall()
 * DESCRIPTION:	flush all pending changes to all mounted HFS volumes
 */
void hfs_flushall(void)
{
  hfsvol *vol;

  for (vol = hfs_mounts; vol; vol = vol->next)
    hfs_flush(vol);
}

/*
 * NAME:	hfs->getvol()
 * DESCRIPTION:	return a pointer to a mounted volume
 */
hfsvol *hfs_getvol(char *name)
{
  hfsvol *vol;

  if (name == 0)
    return hfs_curvol;

  for (vol = hfs_mounts; vol; vol = vol->next)
    {
      if (d_relstring(name, vol->mdb.drVN) == 0)
	return vol;
    }

  return 0;
}

/*
 * NAME:	hfs->setvol()
 * DESCRIPTION:	change the current volume
 */
void hfs_setvol(hfsvol *vol)
{
  hfs_curvol = vol;
}

/*
 * NAME:	hfs->vstat()
 * DESCRIPTION:	return volume statistics
 */
int hfs_vstat(hfsvol *vol, hfsvolent *ent)
{
  if (v_getvol(&vol) < 0)
    return -1;

  strcpy(ent->name, vol->mdb.drVN);

  ent->flags     = (vol->flags & HFS_READONLY) ? HFS_ISLOCKED : 0;
  ent->totbytes  = vol->mdb.drNmAlBlks * vol->mdb.drAlBlkSiz;
  ent->freebytes = vol->mdb.drFreeBks  * vol->mdb.drAlBlkSiz;
  ent->crdate    = d_toutime(vol->mdb.drCrDate);
  ent->mddate    = d_toutime(vol->mdb.drLsMod);

  return 0;
}

/*
 * NAME:	hfs->format()
 * DESCRIPTION:	write a new filesystem
 */
int hfs_format(unsigned hf, unsigned long vlen, int pnum, char *vname)
{
  hfsvol vol;
  btree *ext = &vol.ext;
  btree *cat = &vol.cat;
  unsigned int vbmsz;
  int i, result = 0;
  block vbm[16];
  char *map;

  if (strchr(vname, ':'))
    {
      ERROR(EINVAL, "volume name may not contain colons");
      return -1;
    }

  i = strlen(vname);
  if (i < 1 || i > HFS_MAX_VLEN)
    {
      ERROR(EINVAL, "volume name must be 1-27 chars");
      return -1;
    }

  vol.flags  = 0;
  vol.pnum   = pnum;
  vol.vstart = 0;
  vol.vlen   = 0;
  vol.lpa    = 0;
  vol.vbm    = vbm;
  vol.cwd    = HFS_CNID_ROOTDIR;

  vol.refs   = 0;
  vol.files  = 0;
  vol.dirs   = 0;
  vol.prev   = 0;
  vol.next   = 0;

  vol.fd = hf;

  if (pnum > 0)
    {
      if (l_readpm(&vol) < 0)
	{
	  /*	  close(vol.fd);*/
	  return -1;
	}
    }
  else
    vol.vlen = vlen;

  /* initialize volume geometry */

  vol.lpa = 1 + vol.vlen / 65536;

  vbmsz = (vol.vlen / vol.lpa + 4095) / 4096;

  vol.mdb.drSigWord  = 0x4244;
#ifdef UHFS
  vol.mdb.drCrDate   = 0;     /* time() doesn't work. Need to fix. */
#else
  vol.mdb.drCrDate   = d_tomtime(time(0));
#endif
  vol.mdb.drLsMod    = vol.mdb.drCrDate;
  vol.mdb.drAtrb     = 0;
  vol.mdb.drNmFls    = 0;
  vol.mdb.drVBMSt    = 3;
  vol.mdb.drAllocPtr = 0;
  vol.mdb.drNmAlBlks = (vol.vlen - 5 - vbmsz) / vol.lpa;
  vol.mdb.drAlBlkSiz = vol.lpa * HFS_BLOCKSZ;
  vol.mdb.drClpSiz   = vol.mdb.drAlBlkSiz * 4;
  vol.mdb.drAlBlSt   = 3 + vbmsz;
  vol.mdb.drNxtCNID  = HFS_CNID_ROOTDIR;  /* modified later */
  vol.mdb.drFreeBks  = vol.mdb.drNmAlBlks;

  strcpy(vol.mdb.drVN, vname);

  vol.mdb.drVolBkUp  = 0;
  vol.mdb.drVSeqNum  = 0;
  vol.mdb.drWrCnt    = 0;
  vol.mdb.drXTClpSiz = vol.mdb.drNmAlBlks / 128 * vol.mdb.drAlBlkSiz;
  vol.mdb.drCTClpSiz = vol.mdb.drXTClpSiz;
  vol.mdb.drNmRtDirs = 0;
  vol.mdb.drFilCnt   = 0;
  vol.mdb.drDirCnt   = -1;  /* incremented when root folder is created */

  for (i = 0; i < 8; ++i)
    vol.mdb.drFndrInfo[i] = 0;

  vol.mdb.drVCSize   = 0;
  vol.mdb.drVBMCSize = 0;
  vol.mdb.drCtlCSize = 0;

  vol.mdb.drXTFlSize = 0;
  vol.mdb.drCTFlSize = 0;

  for (i = 0; i < 3; ++i)
    {
      vol.mdb.drXTExtRec[i].xdrStABN    = 0;
      vol.mdb.drXTExtRec[i].xdrNumABlks = 0;

      vol.mdb.drCTExtRec[i].xdrStABN    = 0;
      vol.mdb.drCTExtRec[i].xdrNumABlks = 0;
    }

  /* initialize volume bitmap */

  memset(vol.vbm, 0, sizeof(vbm));

  /* create extents overflow file */

  ext->f.vol   = &vol;
  ext->f.parid = 0;
  strcpy(ext->f.name, "extents overflow");

  ext->f.cat.cdrType          = cdrFilRec;
  /* ext->f.cat.cdrResrv2 */
  ext->f.cat.u.fil.filFlags   = 0;
  ext->f.cat.u.fil.filTyp     = 0;
  /* ext->f.cat.u.fil.filUsrWds */
  ext->f.cat.u.fil.filFlNum   = HFS_CNID_EXT;
  ext->f.cat.u.fil.filStBlk   = 0;
  ext->f.cat.u.fil.filLgLen   = 0;
  ext->f.cat.u.fil.filPyLen   = 0;
  ext->f.cat.u.fil.filRStBlk  = 0;
  ext->f.cat.u.fil.filRLgLen  = 0;
  ext->f.cat.u.fil.filRPyLen  = 0;
  ext->f.cat.u.fil.filCrDat   = vol.mdb.drCrDate;
  ext->f.cat.u.fil.filMdDat   = vol.mdb.drLsMod;
  ext->f.cat.u.fil.filBkDat   = 0;
  /* ext->f.cat.u.fil.filFndrInfo */
  ext->f.cat.u.fil.filClpSize = 0;

  for (i = 0; i < 3; ++i)
    {
      ext->f.cat.u.fil.filExtRec[i].xdrStABN     = 0;
      ext->f.cat.u.fil.filExtRec[i].xdrNumABlks  = 0;

      ext->f.cat.u.fil.filRExtRec[i].xdrStABN    = 0;
      ext->f.cat.u.fil.filRExtRec[i].xdrNumABlks = 0;
    }
  /* ext->f.cat.u.fil.filResrv */
  f_selectfork(&ext->f, 0);

  ext->f.clump = vol.mdb.drXTClpSiz;
  ext->f.flags = 0;

  ext->f.prev = ext->f.next = 0;

  n_init(&ext->hdrnd, ext, ndHdrNode, 0);

  ext->hdrnd.nnum       = 0;
  ext->hdrnd.nd.ndNRecs = 3;
  ext->hdrnd.roff[1]    = 0x078;
  ext->hdrnd.roff[2]    = 0x0f8;
  ext->hdrnd.roff[3]    = 0x1f8;

  memset(HFS_NODEREC(ext->hdrnd, 1), 0, 128);

  ext->hdr.bthDepth    = 0;
  ext->hdr.bthRoot     = 0;
  ext->hdr.bthNRecs    = 0;
  ext->hdr.bthFNode    = 0;
  ext->hdr.bthLNode    = 0;
  ext->hdr.bthNodeSize = HFS_BLOCKSZ;
  ext->hdr.bthKeyLen   = 0x07;
  ext->hdr.bthNNodes   = 0;
  ext->hdr.bthFree     = 0;
  for (i = 0; i < 76; ++i)
    ext->hdr.bthResv[i] = 0;

  map = ALLOC(char, HFS_MAP1SZ);
  if (map == 0)
    {
      if (result == 0)
	{
	  ERROR(ENOMEM, 0);
	  result = -1;
	}
    }
  else
    {
      memset(map, 0, HFS_MAP1SZ);
      BMSET(map, 0);
    }

  ext->map     = map;
  ext->mapsz   = HFS_MAP1SZ;
  ext->flags   = HFS_UPDATE_BTHDR;
  ext->compare = r_compareextkeys;

  if (result == 0 && bt_space(ext, 1) < 0)
    result = -1;

  --ext->hdr.bthFree;

  /* create catalog file */

  cat->f.vol   = &vol;
  cat->f.parid = 0;
  strcpy(cat->f.name, "catalog");

  cat->f.cat.cdrType          = cdrFilRec;
  /* cat->f.cat.cdrResrv2 */
  cat->f.cat.u.fil.filFlags   = 0;
  cat->f.cat.u.fil.filTyp     = 0;
  /* cat->f.cat.u.fil.filUsrWds */
  cat->f.cat.u.fil.filFlNum   = HFS_CNID_CAT;
  cat->f.cat.u.fil.filStBlk   = 0;
  cat->f.cat.u.fil.filLgLen   = 0;
  cat->f.cat.u.fil.filPyLen   = 0;
  cat->f.cat.u.fil.filRStBlk  = 0;
  cat->f.cat.u.fil.filRLgLen  = 0;
  cat->f.cat.u.fil.filRPyLen  = 0;
  cat->f.cat.u.fil.filCrDat   = vol.mdb.drCrDate;
  cat->f.cat.u.fil.filMdDat   = vol.mdb.drLsMod;
  cat->f.cat.u.fil.filBkDat   = 0;
  /* cat->f.cat.u.fil.filFndrInfo */
  cat->f.cat.u.fil.filClpSize = 0;

  for (i = 0; i < 3; ++i)
    {
      cat->f.cat.u.fil.filExtRec[i].xdrStABN     = 0;
      cat->f.cat.u.fil.filExtRec[i].xdrNumABlks  = 0;

      cat->f.cat.u.fil.filRExtRec[i].xdrStABN    = 0;
      cat->f.cat.u.fil.filRExtRec[i].xdrNumABlks = 0;
    }
  /* cat->f.cat.u.fil.filResrv */
  f_selectfork(&cat->f, 0);

  cat->f.clump = vol.mdb.drCTClpSiz;
  cat->f.flags = 0;

  cat->f.prev = cat->f.next = 0;

  n_init(&cat->hdrnd, cat, ndHdrNode, 0);

  cat->hdrnd.nnum       = 0;
  cat->hdrnd.nd.ndNRecs = 3;
  cat->hdrnd.roff[1]    = 0x078;
  cat->hdrnd.roff[2]    = 0x0f8;
  cat->hdrnd.roff[3]    = 0x1f8;

  memset(HFS_NODEREC(cat->hdrnd, 1), 0, 128);

  cat->hdr.bthDepth    = 0;
  cat->hdr.bthRoot     = 0;
  cat->hdr.bthNRecs    = 0;
  cat->hdr.bthFNode    = 0;
  cat->hdr.bthLNode    = 0;
  cat->hdr.bthNodeSize = HFS_BLOCKSZ;
  cat->hdr.bthKeyLen   = 0x25;
  cat->hdr.bthNNodes   = 0;
  cat->hdr.bthFree     = 0;
  for (i = 0; i < 76; ++i)
    cat->hdr.bthResv[i] = 0;

  map = ALLOC(char, HFS_MAP1SZ);
  if (map == 0)
    {
      if (result == 0)
	{
	  ERROR(ENOMEM, 0);
	  result = -1;
	}
    }
  else
    {
      memset(map, 0, HFS_MAP1SZ);
      BMSET(map, 0);
    }

  cat->map     = map;
  cat->mapsz   = HFS_MAP1SZ;
  cat->flags   = HFS_UPDATE_BTHDR;
  cat->compare = r_comparecatkeys;

  if (result == 0 && bt_space(cat, 1) < 0)
    result = -1;

  --cat->hdr.bthFree;

  /* create root folder */

  if (result == 0 && v_newfolder(&vol, HFS_CNID_ROOTPAR, vname) < 0)
    result = -1;

  vol.mdb.drNxtCNID = 16;

  /* finish up */

  if (result == 0)
    {
      block b;

      /* write boot blocks */

      memset(&b, 0, sizeof(b));
      b_writelb(&vol, 0, &b);
      b_writelb(&vol, 1, &b);

      /* flush other disk state */

      vol.flags |= HFS_UPDATE_MDB | HFS_UPDATE_ALTMDB | HFS_UPDATE_VBM;

      if (v_flush(&vol, 1) < 0)
	result = -1;
    }

  FREE(vol.ext.map);
  FREE(vol.cat.map);

#ifdef UHFS
  print("\r\n%lu bytes total disk space.\r\n",
	(unsigned long)(vol.vlen - 5 - vbmsz) * HFS_BLOCKSZ);
  print("%lu bytes free.\r\n",
	(unsigned long)vol.mdb.drFreeBks * vol.lpa * HFS_BLOCKSZ);
  print("\r\n%lu bytes in each allocation block.\r\n", 
	(unsigned long)vol.lpa * HFS_BLOCKSZ);
  print("%hu available allocation blocks on volume.\r\n", 
	(vol.vlen - 5 - vbmsz) / vol.lpa);
#endif

  return result;
}

/* High-Level Directory Routines =========================================== */

/*
 * NAME:	hfs->chdir()
 * DESCRIPTION:	change current HFS directory
 */
int hfs_chdir(hfsvol *vol, char *path)
{
  CatDataRec data;

  if (v_getvol(&vol) < 0 ||
      v_resolve(&vol, path, &data, 0, 0, 0) <= 0)
    return -1;

  if (data.cdrType != cdrDirRec)
    {
      ERROR(ENOTDIR, 0);
      return -1;
    }

  vol->cwd = data.u.dir.dirDirID;

  return 0;
}

/*
 * NAME:	hfs->getcwd()
 * DESCRIPTION:	return the current working directory ID
 */
long hfs_getcwd(hfsvol *vol)
{
  if (v_getvol(&vol) < 0)
    return 0;

  return vol->cwd;
}

/*
 * NAME:	hfs->setcwd()
 * DESCRIPTION:	set the current working directory ID
 */
int hfs_setcwd(hfsvol *vol, long id)
{
  if (v_getvol(&vol) < 0)
    return -1;

  if (id == vol->cwd)
    return 0;

  /* make sure the directory exists */

  if (v_getdthread(vol, id, 0, 0) <= 0)
    return -1;

  vol->cwd = id;

  return 0;
}

/*
 * NAME:	hfs->dirinfo()
 * DESCRIPTION:	given a directory ID, return its (name and) parent ID
 */
int hfs_dirinfo(hfsvol *vol, long *id, char *name)
{
  CatDataRec thread;

  if (v_getvol(&vol) < 0 ||
      v_getdthread(vol, *id, &thread, 0) <= 0)
    return -1;

  *id = thread.u.dthd.thdParID;

  if (name)
    strcpy(name, thread.u.dthd.thdCName);

  return 0;
}

/*
 * NAME:	hfs->opendir()
 * DESCRIPTION:	prepare to read the contents of a directory
 */
hfsdir *hfs_opendir(hfsvol *vol, char *path)
{
  hfsdir *dir;
  CatKeyRec key;
  CatDataRec data;
  unsigned char pkey[HFS_CATKEYLEN];

  if (v_getvol(&vol) < 0)
    return 0;

  dir = ALLOC(hfsdir, 1);
  if (dir == 0)
    {
      ERROR(ENOMEM, 0);
      return 0;
    }

  dir->vol = vol;

  if (*path == 0)
    {
      /* meta-directory containing root dirs from all mounted volumes */

      dir->dirid = 0;
      dir->vptr  = hfs_mounts;
    }
  else
    {
      if (v_resolve(&vol, path, &data, 0, 0, 0) <= 0)
	{
	  FREE(dir);
	  return 0;
	}

      if (data.cdrType != cdrDirRec)
	{
	  FREE(dir);
	  ERROR(ENOTDIR, 0);
	  return 0;
	}

      dir->dirid = data.u.dir.dirDirID;
      dir->vptr  = 0;

      r_makecatkey(&key, dir->dirid, "");
      r_packcatkey(&key, pkey, 0);

      if (bt_search(&vol->cat, pkey, &dir->n) <= 0)
	{
	  FREE(dir);
	  return 0;
	}
    }

  dir->prev = 0;
  dir->next = vol->dirs;

  if (vol->dirs)
    vol->dirs->prev = dir;

  vol->dirs = dir;

  return dir;
}

/*
 * NAME:	hfs->readdir()
 * DESCRIPTION:	return the next entry in the directory
 */
int hfs_readdir(hfsdir *dir, hfsdirent *ent)
{
  CatKeyRec key;
  CatDataRec data;
  unsigned char *ptr;

  if (dir->dirid == 0)
    {
      hfsvol *vol;
      char cname[HFS_MAX_FLEN + 1];

      for (vol = hfs_mounts; vol; vol = vol->next)
	{
	  if (vol == dir->vptr)
	    break;
	}

      if (vol == 0)
	{
	  ERROR(ENOENT, "no more entries");
	  return -1;
	}

      if (v_getdthread(vol, HFS_CNID_ROOTDIR, &data, 0) <= 0 ||
	  v_catsearch(vol, HFS_CNID_ROOTPAR, data.u.dthd.thdCName,
		      &data, cname, 0) < 0)
	return -1;

      r_unpackdirent(HFS_CNID_ROOTPAR, cname, &data, ent);

      dir->vptr = vol->next;

      return 0;
    }

  if (dir->n.rnum == -1)
    {
      ERROR(ENOENT, "no more entries");
      return -1;
    }

  while (1)
    {
      ++dir->n.rnum;

      while (dir->n.rnum >= dir->n.nd.ndNRecs)
	{
	  dir->n.nnum = dir->n.nd.ndFLink;
	  if (dir->n.nnum == 0)
	    {
	      dir->n.rnum = -1;
	      ERROR(ENOENT, "no more entries");
	      return -1;
	    }

	  if (bt_getnode(&dir->n) < 0)
	    {
	      dir->n.rnum = -1;
	      return -1;
	    }

	  dir->n.rnum = 0;
	}

      ptr = HFS_NODEREC(dir->n, dir->n.rnum);

      r_unpackcatkey(ptr, &key);

      if (key.ckrParID != dir->dirid)
	{
	  dir->n.rnum = -1;
	  ERROR(ENOENT, "no more entries");
	  return -1;
	}

      r_unpackcatdata(HFS_RECDATA(ptr), &data);

      switch (data.cdrType)
	{
	case cdrDirRec:
	case cdrFilRec:
	  r_unpackdirent(key.ckrParID, key.ckrCName, &data, ent);
	  return 0;

	case cdrThdRec:
	case cdrFThdRec:
	  break;

	default:
	  dir->n.rnum = -1;

	  ERROR(EIO, "unexpected directory entry found");
	  return -1;
	}
    }
}

/*
 * NAME:	hfs->closedir()
 * DESCRIPTION:	stop reading a directory
 */
int hfs_closedir(hfsdir *dir)
{
  hfsvol *vol = dir->vol;

  if (dir->prev)
    dir->prev->next = dir->next;
  if (dir->next)
    dir->next->prev = dir->prev;
  if (dir == vol->dirs)
    vol->dirs = dir->next;

  FREE(dir);

  return 0;
}

/* High-Level File Routines ================================================ */

/*
 * NAME:	hfs->open()
 * DESCRIPTION:	prepare a file for I/O
 */
hfsfile *hfs_open(hfsvol *vol, char *path)
{
  hfsfile *file;

  if (v_getvol(&vol) < 0)
    return 0;

  file = ALLOC(hfsfile, 1);
  if (file == 0)
    {
      ERROR(ENOMEM, 0);
      return 0;
    }

  if (v_resolve(&vol, path, &file->cat, &file->parid, file->name, 0) <= 0)
    {
      FREE(file);
      return 0;
    }

  if (file->cat.cdrType != cdrFilRec)
    {
      FREE(file);
      ERROR(EISDIR, 0);
      return 0;
    }

  file->vol   = vol;
  file->clump = file->cat.u.fil.filClpSize;
  file->flags = 0;

  f_selectfork(file, 0);

  file->prev = 0;
  file->next = vol->files;

  if (vol->files)
    vol->files->prev = file;

  vol->files = file;

  return file;
}

/*
 * NAME:	hfs->setfork()
 * DESCRIPTION:	select file fork for I/O operations
 */
int hfs_setfork(hfsfile *file, int fork)
{
  int result = 0;

  if (! (file->vol->flags & HFS_READONLY) &&
      f_trunc(file) < 0)
    result = -1;

  f_selectfork(file, fork);

  return result;
}

/*
 * NAME:	hfs->getfork()
 * DESCRIPTION:	return the current fork for I/O operations
 */
int hfs_getfork(hfsfile *file)
{
  return file->fork != fkData;
}

/*
 * NAME:	hfs->read()
 * DESCRIPTION:	read from an open file
 */
long hfs_read(hfsfile *file, void *buf, unsigned long len)
{
  unsigned long *lglen, count;
  unsigned char *ptr = buf;

  f_getptrs(file, &lglen, 0, 0);

  if (file->pos + len > *lglen)
    len = *lglen - file->pos;

  count = len;
  while (count)
    {
      block b;
      unsigned long bnum, offs, chunk;

      bnum  = file->pos / HFS_BLOCKSZ;
      offs  = file->pos % HFS_BLOCKSZ;

      chunk = HFS_BLOCKSZ - offs;
      if (chunk > count)
	chunk = count;

      if (f_getblock(file, bnum, &b) < 0)
	return -1;

      memcpy(ptr, b + offs, chunk);
      ptr += chunk;

      file->pos += chunk;
      count     -= chunk;
    }

  return len;
}

/*
 * NAME:	hfs->write()
 * DESCRIPTION:	write to an open file
 */
long hfs_write(hfsfile *file, void *buf, unsigned long len)
{
  unsigned long *lglen, *pylen, count;
  unsigned char *ptr = buf;

  if (file->vol->flags & HFS_READONLY)
    {
      ERROR(EROFS, 0);
      return -1;
    }

  f_getptrs(file, &lglen, &pylen, 0);

  count = len;

  /* set flag to update (at least) the modification time */

  if (count)
    {
      file->cat.u.fil.filMdDat = d_tomtime(time(0));
      file->flags |= HFS_UPDATE_CATREC;
    }

  while (count)
    {
      block b;
      unsigned long bnum, offs, chunk;

      bnum  = file->pos / HFS_BLOCKSZ;
      offs  = file->pos % HFS_BLOCKSZ;

      chunk = HFS_BLOCKSZ - offs;
      if (chunk > count)
	chunk = count;

      if (file->pos + chunk > *pylen)
	{
	  if (bt_space(&file->vol->ext, 1) < 0 ||
	      f_alloc(file) < 0)
	    return -1;
	}

      if (offs > 0 || chunk < HFS_BLOCKSZ)
	{
	  if (f_getblock(file, bnum, &b) < 0)
	    return -1;
	}

      memcpy(b + offs, ptr, chunk);
      ptr += chunk;

      if (f_putblock(file, bnum, &b) < 0)
	return -1;

      file->pos += chunk;
      count     -= chunk;

      if (file->pos > *lglen)
	*lglen = file->pos;
    }

  return len;
}

/*
 * NAME:	hfs->truncate()
 * DESCRIPTION:	truncate an open file
 */
int hfs_truncate(hfsfile *file, unsigned long len)
{
  unsigned long *lglen;

  f_getptrs(file, &lglen, 0, 0);

  if (*lglen > len)
    {
      if (file->vol->flags & HFS_READONLY)
	{
	  ERROR(EROFS, 0);
	  return -1;
	}

      *lglen = len;

      file->cat.u.fil.filMdDat = d_tomtime(time(0));
      file->flags |= HFS_UPDATE_CATREC;

      if (file->pos > len)
	file->pos = len;
    }

  return 0;
}

/*
 * NAME:	hfs->lseek()
 * DESCRIPTION:	change file seek pointer
 */
long hfs_lseek(hfsfile *file, long offset, int from)
{
  unsigned long *lglen;
  long newpos;

  f_getptrs(file, &lglen, 0, 0);

  switch (from)
    {
    case SEEK_SET:
      newpos = offset;
      break;

    case SEEK_CUR:
      newpos = file->pos + offset;
      break;

    case SEEK_END:
      newpos = *lglen + offset;
      break;

    default:
      ERROR(EINVAL, 0);
      return -1;
    }

  if (newpos < 0)
    newpos = 0;
  else if (newpos > *lglen)
    newpos = *lglen;

  file->pos = newpos;

  return newpos;
}

/*
 * NAME:	hfs->close()
 * DESCRIPTION:	close a file
 */
int hfs_close(hfsfile *file)
{
  hfsvol *vol = file->vol;
  int result = 0;

  if (f_trunc(file) < 0 ||
      f_flush(file) < 0)
    result = -1;

  if (file->prev)
    file->prev->next = file->next;
  if (file->next)
    file->next->prev = file->prev;
  if (file == vol->files)
    vol->files = file->next;

  FREE(file);

  return result;
}

/* High-Level Catalog Routines ============================================= */

/*
 * NAME:	hfs->stat()
 * DESCRIPTION:	return catalog information for an arbitrary path
 */
int hfs_stat(hfsvol *vol, char *path, hfsdirent *ent)
{
  CatDataRec data;
  long parid;
  char name[HFS_MAX_FLEN + 1];

  if (v_getvol(&vol) < 0 ||
      v_resolve(&vol, path, &data, &parid, name, 0) <= 0)
    return -1;

  r_unpackdirent(parid, name, &data, ent);

  return 0;
}

/*
 * NAME:	hfs->fstat()
 * DESCRIPTION:	return catalog information for an open file
 */
int hfs_fstat(hfsfile *file, hfsdirent *ent)
{
  r_unpackdirent(file->parid, file->name, &file->cat, ent);

  return 0;
}

/*
 * NAME:	hfs->setattr()
 * DESCRIPTION:	change a file's attributes
 */
int hfs_setattr(hfsvol *vol, char *path, hfsdirent *ent)
{
  CatDataRec data;
  node n;

  if (v_getvol(&vol) < 0 ||
      v_resolve(&vol, path, &data, 0, 0, &n) <= 0)
    return -1;

  if (vol->flags & HFS_READONLY)
    {
      ERROR(EROFS, 0);
      return -1;
    }

  r_packdirent(&data, ent);

  if (v_putcatrec(&data, &n) < 0)
    return -1;

  return 0;
}

/*
 * NAME:	hfs->fsetattr()
 * DESCRIPTION:	change an open file's attributes
 */
int hfs_fsetattr(hfsfile *file, hfsdirent *ent)
{
  if (file->vol->flags & HFS_READONLY)
    {
      ERROR(EROFS, 0);
      return -1;
    }

  r_packdirent(&file->cat, ent);

  file->flags |= HFS_UPDATE_CATREC;

  return 0;
}

/*
 * NAME:	hfs->mkdir()
 * DESCRIPTION:	create a new directory
 */
int hfs_mkdir(hfsvol *vol, char *path)
{
  CatDataRec data;
  long parid;
  char name[HFS_MAX_FLEN + 1];
  int found;

  if (v_getvol(&vol) < 0)
    return -1;

  found = v_resolve(&vol, path, &data, &parid, name, 0);
  if (found < 0 || parid == 0)
    return -1;
  else if (found)
    {
      ERROR(EEXIST, 0);
      return -1;
    }

  if (parid == HFS_CNID_ROOTPAR)
    {
      ERROR(EINVAL, 0);
      return -1;
    }

  if (vol->flags & HFS_READONLY)
    {
      ERROR(EROFS, 0);
      return -1;
    }

  if (v_newfolder(vol, parid, name) < 0)
    return -1;

  return 0;
}

/*
 * NAME:	hfs->rmdir()
 * DESCRIPTION:	delete an empty directory
 */
int hfs_rmdir(hfsvol *vol, char *path)
{
  CatKeyRec key;
  CatDataRec data;
  long parid;
  char name[HFS_MAX_FLEN + 1];
  unsigned char pkey[HFS_CATKEYLEN];

  if (v_getvol(&vol) < 0 ||
      v_resolve(&vol, path, &data, &parid, name, 0) <= 0)
    return -1;

  if (data.cdrType != cdrDirRec)
    {
      ERROR(ENOTDIR, 0);
      return -1;
    }

  if (data.u.dir.dirVal != 0)
    {
      ERROR(ENOTEMPTY, 0);
      return -1;
    }

  if (parid == HFS_CNID_ROOTPAR)
    {
      ERROR(EINVAL, 0);
      return -1;
    }

  if (vol->flags & HFS_READONLY)
    {
      ERROR(EROFS, 0);
      return -1;
    }

  /* delete directory record */

  r_makecatkey(&key, parid, name);
  r_packcatkey(&key, pkey, 0);

  if (bt_delete(&vol->cat, pkey) < 0)
    return -1;

  /* delete thread record */

  r_makecatkey(&key, data.u.dir.dirDirID, "");
  r_packcatkey(&key, pkey, 0);

  if (bt_delete(&vol->cat, pkey) < 0 ||
      v_adjvalence(vol, parid, 1, -1) < 0)
    return -1;

  return 0;
}

/*
 * NAME:	hfs->create()
 * DESCRIPTION:	create a new file
 */
int hfs_create(hfsvol *vol, char *path, char *type, char *creator)
{
  CatKeyRec key;
  CatDataRec data;
  long id, parid;
  char name[HFS_MAX_FLEN + 1];
  unsigned char record[HFS_CATRECMAXLEN];
  int found, i, reclen;

  if (v_getvol(&vol) < 0)
    return -1;

  found = v_resolve(&vol, path, &data, &parid, name, 0);
  if (found < 0 || parid == 0)
    return -1;
  else if (found)
    {
      ERROR(EEXIST, 0);
      return -1;
    }

  if (parid == HFS_CNID_ROOTPAR)
    {
      ERROR(EINVAL, 0);
      return -1;
    }

  if (vol->flags & HFS_READONLY)
    {
      ERROR(EROFS, 0);
      return -1;
    }

  /* create file `name' in parent `parid' */

  if (bt_space(&vol->cat, 1) < 0)
    return -1;

  id = vol->mdb.drNxtCNID++;
  vol->flags |= HFS_UPDATE_MDB;

  /* create file record */

  data.cdrType   = cdrFilRec;
  data.cdrResrv2 = 0;

  data.u.fil.filFlags = 0;
  data.u.fil.filTyp   = 0;

  memset(&data.u.fil.filUsrWds, 0, sizeof(data.u.fil.filUsrWds));

  data.u.fil.filUsrWds.fdType    = d_getl((unsigned char *) type);
  data.u.fil.filUsrWds.fdCreator = d_getl((unsigned char *) creator);

  data.u.fil.filFlNum  = id;
  data.u.fil.filStBlk  = 0;
  data.u.fil.filLgLen  = 0;
  data.u.fil.filPyLen  = 0;
  data.u.fil.filRStBlk = 0;
  data.u.fil.filRLgLen = 0;
  data.u.fil.filRPyLen = 0;
  data.u.fil.filCrDat  = d_tomtime(time(0));
  data.u.fil.filMdDat  = data.u.fil.filCrDat;
  data.u.fil.filBkDat  = 0;

  memset(&data.u.fil.filFndrInfo, 0, sizeof(data.u.fil.filFndrInfo));

  data.u.fil.filClpSize = 0;

  for (i = 0; i < 3; ++i)
    {
      data.u.fil.filExtRec[i].xdrStABN     = 0;
      data.u.fil.filExtRec[i].xdrNumABlks  = 0;

      data.u.fil.filRExtRec[i].xdrStABN    = 0;
      data.u.fil.filRExtRec[i].xdrNumABlks = 0;
    }

  data.u.fil.filResrv = 0;

  r_makecatkey(&key, parid, name);
  r_packcatkey(&key, record, &reclen);
  r_packcatdata(&data, HFS_RECDATA(record), &reclen);

  if (bt_insert(&vol->cat, record, reclen) < 0 ||
      v_adjvalence(vol, parid, 0, 1) < 0)
    return -1;

  return 0;
}

/*
 * NAME:	hfs->delete()
 * DESCRIPTION:	remove both forks of a file
 */
int hfs_delete(hfsvol *vol, char *path)
{
  hfsfile file;
  CatKeyRec key;
  unsigned char pkey[HFS_CATKEYLEN];
  int found;

  if (v_getvol(&vol) < 0 ||
      v_resolve(&vol, path, &file.cat, &file.parid, file.name, 0) <= 0)
    return -1;

  if (file.cat.cdrType != cdrFilRec)
    {
      ERROR(EISDIR, 0);
      return -1;
    }

  if (file.parid == HFS_CNID_ROOTPAR)
    {
      ERROR(EINVAL, 0);
      return -1;
    }

  if (vol->flags & HFS_READONLY)
    {
      ERROR(EROFS, 0);
      return -1;
    }

  /* free disk blocks */

  file.vol   = vol;
  file.flags = 0;

  file.cat.u.fil.filLgLen  = 0;
  file.cat.u.fil.filRLgLen = 0;

  f_selectfork(&file, 0);
  if (f_trunc(&file) < 0)
    return -1;

  f_selectfork(&file, 1);
  if (f_trunc(&file) < 0)
    return -1;

  /* delete file record */

  r_makecatkey(&key, file.parid, file.name);
  r_packcatkey(&key, pkey, 0);

  if (bt_delete(&vol->cat, pkey) < 0 ||
      v_adjvalence(vol, file.parid, 0, -1) < 0)
    return -1;

  /* delete file thread, if any */

  found = v_getfthread(vol, file.cat.u.fil.filFlNum, 0, 0);
  if (found < 0)
    return -1;

  if (found)
    {
      r_makecatkey(&key, file.cat.u.fil.filFlNum, "");
      r_packcatkey(&key, pkey, 0);

      if (bt_delete(&vol->cat, pkey) < 0)
	return -1;
    }

  return 0;
}

/*
 * NAME:	hfs->rename()
 * DESCRIPTION:	change the name of and/or move a file or directory
 */
int hfs_rename(hfsvol *vol, char *srcpath, char *dstpath)
{
  hfsvol *srcvol;
  CatDataRec src, dst;
  long srcid, dstid;
  CatKeyRec key;
  char srcname[HFS_MAX_FLEN + 1], dstname[HFS_MAX_FLEN + 1];
  unsigned char record[HFS_CATRECMAXLEN];
  int found, isdir, moving, reclen;
  node n;

  if (v_getvol(&vol) < 0 ||
      v_resolve(&vol, srcpath, &src, &srcid, srcname, 0) <= 0)
    return -1;

  isdir  = (src.cdrType == cdrDirRec);
  srcvol = vol;

  found = v_resolve(&vol, dstpath, &dst, &dstid, dstname, 0);
  if (found < 0)
    return -1;

  if (vol != srcvol)
    {
      ERROR(EINVAL, "can't move across volumes");
      return -1;
    }

  if (dstid == 0)
    {
      ERROR(ENOENT, "bad destination path");
      return -1;
    }

  if (found &&
      dst.cdrType == cdrDirRec &&
      dst.u.dir.dirDirID != src.u.dir.dirDirID)
    {
      dstid = dst.u.dir.dirDirID;
      strcpy(dstname, srcname);

      found = v_catsearch(vol, dstid, dstname, 0, 0, 0);
      if (found < 0)
	return -1;
    }

  moving = (srcid != dstid);

  if (found)
    {
      char *ptr;

      ptr = strrchr(dstpath, ':');
      if (ptr == 0)
	ptr = dstpath;
      else
	++ptr;

      if (*ptr)
	strcpy(dstname, ptr);

      if (! moving && strcmp(srcname, dstname) == 0)
	return 0;  /* source and destination are the same */

      if (moving || d_relstring(srcname, dstname))
	{
	  ERROR(EEXIST, "can't use destination name");
	  return -1;
	}
    }

  /* can't move anything into the root directory's parent */

  if (moving && dstid == HFS_CNID_ROOTPAR)
    {
      ERROR(EINVAL, "can't move above root directory");
      return -1;
    }

  if (moving && isdir)
    {
      long id;

      /* can't move root directory anywhere */

      if (src.u.dir.dirDirID == HFS_CNID_ROOTDIR)
	{
	  ERROR(EINVAL, "can't move root directory");
	  return -1;
	}

      /* make sure we aren't trying to move a directory inside itself */

      for (id = dstid; id != HFS_CNID_ROOTDIR; id = dst.u.dthd.thdParID)
	{
	  if (id == src.u.dir.dirDirID)
	    {
	      ERROR(EINVAL, "can't move directory inside itself");
	      return -1;
	    }

	  if (v_getdthread(vol, id, &dst, 0) <= 0)
	    return -1;
	}
    }

  if (vol->flags & HFS_READONLY)
    {
      ERROR(EROFS, 0);
      return -1;
    }

  /* change volume name */

  if (dstid == HFS_CNID_ROOTPAR)
    {
      if (strlen(dstname) > HFS_MAX_VLEN)
	{
	  ERROR(ENAMETOOLONG, 0);
	  return -1;
	}

      strcpy(vol->mdb.drVN, dstname);
      vol->flags |= HFS_UPDATE_MDB;
    }

  /* remove source record */

  r_makecatkey(&key, srcid, srcname);
  r_packcatkey(&key, record, 0);

  if (bt_delete(&vol->cat, record) < 0)
    return -1;

  /* insert destination record */

  r_makecatkey(&key, dstid, dstname);
  r_packcatkey(&key, record, &reclen);
  r_packcatdata(&src, HFS_RECDATA(record), &reclen);

  if (bt_insert(&vol->cat, record, reclen) < 0)
    return -1;

  /* update thread record */

  if (isdir)
    {
      if (v_getdthread(vol, src.u.dir.dirDirID, &dst, &n) <= 0)
	return -1;

      dst.u.dthd.thdParID = dstid;
      strcpy(dst.u.dthd.thdCName, dstname);

      if (v_putcatrec(&dst, &n) < 0)
	return -1;
    }
  else
    {
      found = v_getfthread(vol, src.u.fil.filFlNum, &dst, &n);
      if (found < 0)
	return -1;

      if (found)
	{
	  dst.u.fthd.fthdParID = dstid;
	  strcpy(dst.u.fthd.fthdCName, dstname);

	  if (v_putcatrec(&dst, &n) < 0)
	    return -1;
	}
    }

  /* update directory valences */

  if (moving)
    {
      if (v_adjvalence(vol, srcid, isdir, -1) < 0 ||
	  v_adjvalence(vol, dstid, isdir,  1) < 0)
	return -1;
    }

  return 0;
}
