/*
 * HFS/2 - A Hierarchical File System Driver for OS/2
 * Copyright (C) 1996, 1997 Marcus Better
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

/* vol.c
   Volume management.
*/

#define INCL_DOSERRORS
#define INCL_DOSFILEMGR
#define INCL_DOSDEVICES
#define INCL_DOSDEVIOCTL
#define INCL_NOPMAPI
#include <os2.h>

#include <stdlib.h>
#include <string.h>
#ifdef DEBUG
#include <stdio.h>
#endif

#include "libhfs/hfs.h"
#include "libhfs/internal.h"
#include "fsd.h"
#include "fsdep.h"
#include "../r0r3shar.h"
#include "diskfunc.h"
#include "vol.h"
#include "cache.h"
#include "fsctl.h"
#include "mountstatus.h"
#include "userfunc.h"

static int mac_to_os2_label(const unsigned char *src, unsigned char *dest);
static int is_readonly(unsigned short, hfsvol *);
/*static int setvolume(unsigned short, unsigned short);*/

/******************************************************************************
** Name of IFS
******************************************************************************/

const char FS_NAME[] = "HFS";

const unsigned long FSID = 0x32534648;  /* 'HFS2', used in FS_FSINFO */


/*-----------------------------------------------------------------------------
--
-- Volume management
--
-----------------------------------------------------------------------------*/


/******************************************************************************
**
** FS_MOUNT - Examine a volume to determine if the IFS knows its format
**
** Parameters
** ----------
** unsigned short flag                  indicates function to perform
**   values:
**     MOUNT_MOUNT              mount or accept the volume
**     MOUNT_VOL_REMOVED        volume has been removed
**     MOUNT_RELEASE            release all resources associated with the volume
**     MOUNT_ACCEPT             accept the volume in preparation for formatting
** struct vpfsi far *pvpfsi             pointer to FSD independant volume parameters
** struct vpfsd far *pvpfsd             pointer to FSD dependant volume parameters
** unsigned short hVPB                  volume handle
** char far *pBoot                      pointer to sector 0 data
**
******************************************************************************/

short int FS_MOUNT(unsigned short flag, struct vpfsi *pvpfsi,
                   struct vpfsd *pvpfsd, unsigned short hVPB,
		   unsigned short vol_descr, unsigned char *pBoot)
{
  unsigned short oldVPB;
  unsigned long len_parm;
  unsigned char label[CCHMAXPATH], maclabel[HFS_MAX_VLEN+1];
  hfsvolent volent;

#ifdef DEBUG
  printf("Mount, flag = %hu, hVPB = %hu\n", flag, hVPB);
#endif
  switch(flag) {
    case MOUNT_MOUNT:
      /* The cache uses hVPB as part of the key, so we flush it here. */
      flush_cache();

      /* Check for duplicate VPB */
      oldVPB = hVPB;
      if( do_FSCtl(NULL, 0, NULL,
                   &oldVPB, sizeof(unsigned short), &len_parm,
                   FSCTL_FUNC_FINDDUPHVPB)
	 == NO_ERROR ) {
	/* We should read the vol info and bitmap again here for the old VPB,
	   as the volume may have been written while it was removed. */

	/* Mark the VPB as duplicate */
	set_vol_status(hVPB, pvpfsi->vpi_drive, MTSTAT_DUPLICATE);
	/* If necessary, mark the old VPB as mounted */
	if(get_mount_status(oldVPB)==MTSTAT_REMOVED)
	  set_vol_status(oldVPB, pvpfsi->vpi_drive, MTSTAT_MOUNTED);
        return NO_ERROR;
      }
 
      /* Mount the volume */
      pvpfsd->vol = hfs_mount(hVPB, 1, pvpfsi->vpi_bsize, 0);
      if(pvpfsd->vol == NULL) {
#ifdef DEBUG
        printf("hfs_mount failed!\n");
#endif
	return ERROR_VOLUME_NOT_MOUNTED;
      }

      if(hfs_vstat(pvpfsd->vol, &volent) < 0) {
#ifdef DEBUG
        printf("hfs_vstat failed!\n");
#endif
	return ERROR_VOLUME_NOT_MOUNTED;
      }

      /* Fill vpfsi fields */
      /* Use creation timestamp as ID */
      pvpfsi->vpi_vid = volent.crdate;
      strcpy(maclabel, pvpfsd->vol->mdb.drVN);
      mac_to_os2_label(maclabel, label);
      strncpy(pvpfsi->vpi_text, label, 12);
      pvpfsi->vpi_text[11]=0;

      set_vol_status(hVPB, pvpfsi->vpi_drive, MTSTAT_MOUNTED);
      if(is_readonly(hVPB, pvpfsd->vol))
	pvpfsd->vol->flags |= HFS_READONLY;
#ifdef DEBUG
      printf("Got HFS volume\n");
#endif
      return NO_ERROR;

    case MOUNT_VOL_REMOVED:
      flush_cache();
      /* If the disk was removed, forget about flushing. We should do a
         setvolume() here if volume is dirty, but it does not work. */
      pvpfsd->vol->flags |= HFS_READONLY;
      return NO_ERROR;

    case MOUNT_RELEASE:
      switch(get_mount_status(hVPB)) {
      case MTSTAT_REMOVED:
      case MTSTAT_MOUNTED:
	hfs_umount(pvpfsd->vol);
	break;
      case MTSTAT_DUPLICATE:
#ifdef DEBUG
	printf("Trying to unmount non-mounted volume!\n");
#endif
	break;
      }
      flush_cache();
      set_vol_status(hVPB, 0, MTSTAT_FREE);
      return NO_ERROR;

    case MOUNT_ACCEPT:
      return ERROR_NOT_SUPPORTED;

    default:
      return ERROR_NOT_SUPPORTED;
  }
}

/******************************************************************************
**
** FS_FSINFO - Get/Set file system information
**
** Parameters
** ----------
** unsigned short flag                  indicates function to perform
**   values:
**     INFO_RETREIVE                    retrieve information
**     INFO_SET                         set information
** unsigned short hVPB                  VPB handle
** struct vpfsi *pvpfsi                 pointer to FSD independent volume parameters
** struct vpfsd *pvpfsd                 pointer to FSD dependant volume parameters
** char far *pData                      pointer to data buffer
** unsigned short cbData                length of data buffer
** unsigned short level                 type of information to return
**
******************************************************************************/

#pragma argsused
short int FS_FSINFO(unsigned short flag, unsigned short hVPB,
                    struct vpfsi *vpfsi, struct vpfsd *vpfsd, 
                    char *pData, unsigned short cbData, unsigned short level)
{
  if( flag==INFO_RETRIEVE ) {
    switch(level) {
      case FSIL_ALLOC:
        if( cbData < sizeof(FSALLOCATE) )
          return ERROR_BUFFER_OVERFLOW;
        ((PFSALLOCATE)pData)->idFileSystem = FSID;
        ((PFSALLOCATE)pData)->cSectorUnit = 
	  vpfsd->vol->mdb.drAlBlkSiz / vpfsd->vol->sector_size;
        ((PFSALLOCATE)pData)->cUnit = vpfsd->vol->mdb.drNmAlBlks;
        ((PFSALLOCATE)pData)->cUnitAvail = vpfsd->vol->mdb.drFreeBks;
        ((PFSALLOCATE)pData)->cbSector = vpfsd->vol->sector_size;
        break;
      case FSIL_VOLSER:
        if( cbData < sizeof(FSINFO) )
          return ERROR_BUFFER_OVERFLOW;
        ((PFSINFO2)pData)->ulVSN = vpfsi->vpi_vid;
	strcpy(((PFSINFO2)pData)->vol.szVolLabel, vpfsi->vpi_text);
	((PFSINFO2)pData)->vol.cch = strlen(vpfsi->vpi_text);
        break;
      default:
        return ERROR_INVALID_LEVEL;
    }
  }
  else if( flag==INFO_SET ) {
    if( level != FSIL_VOLSER )
      return ERROR_INVALID_LEVEL;
    if( vpfsi->vpi_vid != ((PFSINFO2)pData)->ulVSN )
      return ERROR_CANNOT_MAKE;  /* Cannot change serial number */
    strcpy(vpfsi->vpi_text, ((PFSINFO2)pData)->vol.szVolLabel);
    /* Update VIB */
    strcpy(vpfsd->vol->mdb.drVN, vpfsi->vpi_text);
  }
  else 
    return ERROR_NOT_SUPPORTED;

  return NO_ERROR;
}


/******************************************************************************
**
** FS_FLUSHBUF - Flush buffers for a specified volume
**
** Parameters
** ----------
** struct vpfsd *pvpfsd                 pointer to FSD dependant volume parameters
** unsigned short flag                  indicates whether to discard or retain cache
**   values:
**     FLUSH_RETAIN     retain cached information
**     FLUSH_DISCARD    discard cached information
**
******************************************************************************/

#pragma argsused
short int FS_FLUSHBUF(struct vpfsd *vpfsd, unsigned short flag)
{
  if(vpfsd->vol) {
    if(hfs_flush(vpfsd->vol) < 0) {
#ifdef DEBUG
      printf("FS_FLUSHBUF: hfs_flush() failed!\n");
#endif
      return ERROR_WRITE_FAULT;
    }
  }
  if(flag==FLUSH_DISCARD)
    flush_cache();
  return NO_ERROR;
}

/******************************************************************************
**
** FS_ATTACH - Attach or detach a drive or device
**
** Parameters
** ----------
** unsigned short flag                  indicates attaching/detaching
**   values:
**     FSA_ATTACH               attach drive/device
**     FSA_DETACH               detach drive/device
**     FSA_ATTACH_INFO          return info on attached drive/device
** char *pDev                           drive or device that is being attached/detached
** struct vpfsd *pvpfsd                 pointer to FSD dependant volume parameters
** struct cdfsd *pcdfsd                 pointer to FSD dependant current directory
** void *pParm                          pointer to FSD dependant attachment info
** unsigned short far *pLen             length of area pointed to by pParm
**
******************************************************************************/

#pragma argsused
short int FSD_ATTACH(unsigned short flag, char *pDev, struct vpfsd *pvpfsd,
                    struct cdfsd *pcdfsd, void *pParm,
                    unsigned short *pLen)
{
  return ERROR_NOT_SUPPORTED;
}

/******************************************************************************
** Mount status functions
******************************************************************************/

unsigned short get_mount_status(unsigned short hVPB)
{
  unsigned long lenparam=sizeof(unsigned short);
  MTSTATSTRUC mts;
  unsigned short *param = (unsigned short *)&mts;

  *param = hVPB;
  if( do_FSCtl(NULL, 0, NULL, param, sizeof(MTSTATSTRUC), &lenparam,
	       FSCTL_FUNC_GET_MTSTAT) != NO_ERROR ) {
#ifdef DEBUG
    printf("FSCTL_FUNC_GET_MTSTAT failed!\n");
#endif
    return (unsigned short)MTSTAT_ERROR;
  }
  return mts.status;
}

void set_vol_status(unsigned short hVPB, unsigned short drive, 
		    unsigned short status)
{
  unsigned long lenparam=sizeof(MTSTATSTRUC);
  MTSTATSTRUC mts;

  mts.drive = drive;
  mts.status = status;
  mts.hVPB = hVPB;
  if( do_FSCtl(NULL, 0, NULL, &mts, sizeof(MTSTATSTRUC), &lenparam,
	       FSCTL_FUNC_SET_MTSTAT) != NO_ERROR ) {
#ifdef DEBUG
    printf("FSCTL_FUNC_SET_MTSTAT failed!\n");
#endif
  }
}

/******************************************************************************
** Volume label conversion
******************************************************************************/

#define os2_invalid_char(c) strchr("<>:\"/\\|?*", (c))

/* Convert a volume label. No characters quoted. */
static int mac_to_os2_label(const unsigned char *src, unsigned char *dest)
{
  int i;
  char *p=dest;

  for(i=0; i<strlen(src); i++) {
    unsigned char c=src[i];
    if(c>=32 && c<=127 && !os2_invalid_char(c))
      *p++ = c;
  }
  *p=0;
  return 0;
}

/* Check if the volume is read-only by reading and trying to
   write back a sector. */
static int is_readonly(unsigned short hVPB, hfsvol *vol)
{
  RWSTRUC rw;
  char *buf;
  APIRET rc;
  unsigned long len_parm, len_data;
  
  buf = malloc(vol->sector_size);
  len_parm = sizeof(RWSTRUC);
  len_data = vol->sector_size;
  rw.starting_sector = 2;
  rw.n_sectors = 1;
  rw.hVPB = hVPB;
  rc = do_FSCtl(buf, vol->sector_size, &len_data,
		&rw, sizeof(rw), &len_parm,
		FSCTL_FUNC_READSECTORS);
  if(rc != NO_ERROR) {
    free(buf);
    return 0;
  }
  rc = do_FSCtl(buf, vol->sector_size, &len_data,
		&rw, sizeof(rw), &len_parm,
		FSCTL_FUNC_WRITESECTORS_NOHARDERR);
  free(buf);
#ifdef DEBUG
  printf("Write check rc = %lu\n", rc);
#endif
  if(rc != NO_ERROR)
    return 1;
  return 0;
}    

/* Force a specified volume to be inserted in the drive. */
/* flag = 0 for popup dialog, 1 for no popup dialog. */
/*
static int setvolume(unsigned short hVPB, unsigned short flag)
{
  unsigned short param[2];
  unsigned long lenparam;

  param[0] = hVPB;
  param[1] = flag;
  lenparam = sizeof(param);
  return do_FSCtl(NULL, 0, NULL, param, sizeof(param), &lenparam,
                  FSCTL_FUNC_SETVOLUME);
}
*/

