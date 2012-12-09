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

/* file.c
   File functions.
*/

#define INCL_DOSFILEMGR
#define INCL_DOSERRORS
#define INCL_NOPMAPI
#include <os2.h>

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#include "libhfs/hfs.h"
#include "libhfs/internal.h"
#include "fsd.h"
#include "fsdep.h"
#include "names.h"
#include "times.h"
#include "dirsearch.h"
#include "ea.h"
#include "attr.h"
#include "error.h"
#include "diskfunc.h"

static unsigned short fileinfo(hfsvol *, hfsfile *, hfsdirent *, char *, 
			       struct sffsi *, 
			       unsigned short, unsigned short,
			       char *, unsigned short,	unsigned long *);
static void transfer_timestamps(struct sffsi *, hfsdirent *);

#define DEFAULT_FILE_TYPE "????"
#define DEFAULT_CREATOR   "????"

#define min(a,b)  ( (a) < (b) ? (a) : (b) )

/******************************************************************************
**
** FS_CHGFILEPTR - Change current location in file
**
** Parameters
** ----------
** struct sffsi *psffsi                 pointer to FSD independant file instance
** struct sffsd *psffsd                 pointer to FSD dependant file instance
** long offset                          signed offset
** unsigned short type                  indicates seek type
**   values:
**     CFP_RELBEGIN     move pointer relative to begining of file
**     CFP_RELCUR       move pointer relative to current position in file
**     CFP_RELEND       move pointer relative to end of file
** unsigned short IOflag                bitfield of I/O suggestions
**   values:
**     IOFL_WRITETHRU   write all updated data before returning
**     IOFL_NOCACHE     don't cache any new data
**
******************************************************************************/

#pragma argsused
short int FS_CHGFILEPTR(struct vpfsd *pvpfsd, 
			struct sffsi *psffsi, struct sffsd *psffsd,
                        long offset, unsigned short type, 
                        unsigned short IOflag)
{
  switch(type) {
  case CFP_RELBEGIN:
    psffsi->sfi_position = offset;
    return NO_ERROR;
  case CFP_RELCUR:
    psffsi->sfi_position += offset;
    return NO_ERROR;
  case CFP_RELEND:
    psffsi->sfi_position = psffsi->sfi_size+offset;
    return NO_ERROR;
  default:
    return ERROR_NOT_SUPPORTED;
  }
}


/******************************************************************************
**
** FS_CLOSE - Close an open file
**
** Parameters
** ----------
** unsigned short type                  indicates close type
**   values:
**     FS_CL_ORDINARY   this is not the final close of the file
**     FS_CL_FORPROC    this is the final close of the file for the process
**     FS_CL_FORSYS     this is the final close of the file for the whole system
** unsigned short IOflag                bitfield of I/O suggestions
**   values:
**     IOFL_WRITETHRU   write all updated data before returning
**     IOFL_NOCACHE     don't cache any new data
** struct sffsi *psffsi                 pointer to FSD independant file instance
** struct sffsd *psffsd                 pointer to FSD dependant file instance
**
******************************************************************************/

#pragma argsused
short int FS_CLOSE(struct vpfsd *pvpfsd, 
		   unsigned short type, unsigned short IOflag,
                   struct sffsi *psffsi, struct sffsd *psffsd)
{
  if(psffsi->sfi_mode & OPEN_FLAGS_DASD)
    return NO_ERROR;
  
  if(hfs_close(psffsd->file) < 0)
    return os2_error(errno, ERROR_GEN_FAILURE);
  return NO_ERROR;
}

/******************************************************************************
**
** FS_COMMIT - Commit a file to disk
**
** Parameters
** ----------
** unsigned short type                  indicates commit type
**   values:
**     FS_COMMIT_ONE    commit this one file
**     FS_COMMIT_ALL    commit all files
** unsigned short IOflag                bitfield of I/O suggestions
**   values:
**     IOFL_WRITETHRU   write all updated data before returning
**     IOFL_NOCACHE     don't cache any new data
** struct sffsi *psffsi                 pointer to FSD independant file instance
** struct sffsd *psffsd                 pointer to FSD dependant file instance
**
******************************************************************************/

#pragma argsused
short int FS_COMMIT(struct vpfsd *pvpfsd,
		    unsigned short type, unsigned short IOflag,
                    struct sffsi *psffsi, struct sffsd *psffsd)
{
  if(psffsi->sfi_mode & OPEN_FLAGS_DASD)
    return NO_ERROR;

  if(hfs_flush(pvpfsd->vol) < 0)
    return os2_error(errno, ERROR_GEN_FAILURE);
  return NO_ERROR;
}

/******************************************************************************
**
** FS_COPY - Copy a file
**
** Parameters
** ----------
** unsigned short flag                  indicates flavor of call
**   values:
**     DCPY_EXISTING    if destination file exists, replace it
**     DCPY_APPEND      source file should be appended to destination file
** struct cdfsi *pcdfsi                 pointer to FSD independant current directory
** struct cdfsd *pcdfsd                 pointer to FSD dependant current directory
** char *pSrc                           pointer to source filename
** unsigned short iSrcCurDirEnd         offset to the end of the current directory in pSrc
** char *pDst                           pointer to destination filename
** unsigned short iDstCurDirEnd         offset to the end of the current directory in pDst
** unsigned short nameType              0x40 = destination is non 8.3 filename
**
******************************************************************************/

#pragma argsused
short int FS_COPY(struct vpfsd *pvpfsd,
		  unsigned short flag, struct cdfsi *pcdfsi,
                  struct cdfsd *pcdfsd, char *pSrc, 
                  unsigned short iSrcCurDirEnd, char *pDst,
                  unsigned short iDstCurDirEnd, unsigned short nameType)
{
  return ERROR_CANNOT_COPY;
}


/******************************************************************************
**
** FS_DELETE - Delete a file
**
** Parameters
** ----------
** struct cdfsi *pcdfsi                 pointer to FSD independant current directory
** struct cdfsd *pcdfsd                 pointer to FSD dependant current directory
** char *pFile                          pointer to filename to delete
** unsigned short iCurDirEnd            offset to the end of the current directory in pFile
**
******************************************************************************/

#pragma argsused
short int FS_DELETE(struct vpfsd *pvpfsd,
		    struct cdfsi *pcdfsi, struct cdfsd *pcdfsd, char *pFile, 
                    unsigned short iCurDirEnd)
{
  char macname[CCHMAXPATH];

  if(os2_to_mac_name(pFile+2, macname))
    return ERROR_ACCESS_DENIED;

  if(hfs_delete(pvpfsd->vol, macname) < 0)
    return os2_error(errno, ERROR_WRITE_FAULT);

  return NO_ERROR;
}

/******************************************************************************
**
** FS_FILEATTRIBUTE - Get/Set DOS file attributes
**
** Parameters
** ----------
** unsigned short flag                  indicates flavor of call
**   values:
**     FA_RETRIEVE      retrieve attribute
**     FA_SET           set attribute
** struct cdfsi *pcdfsi                 pointer to FSD independant current directory
** struct cdfsd *pcdfsd                 pointer to FSD dependant current directory
** char *pName                          pointer to filename
** unsigned short iCurDirEnd            offset to the end of the current directory in pName
** unsigned short *pAttr                pointer to the attribute
**
******************************************************************************/

#pragma argsused
short int FS_FILEATTRIBUTE(struct vpfsd *pvpfsd, unsigned short flag, 
			   struct cdfsi *pcdfsi, struct cdfsd *pcdfsd,
			   char *pName, unsigned short iCurDirEnd, 
			   unsigned short *pAttr)
{
  char path[CCHMAXPATH];
  hfsdirent ent;
  APIRET rc;

  if(os2_to_mac_name(pName+2, path))
    return ERROR_ACCESS_DENIED;

  if(hfs_stat(pvpfsd->vol, path, &ent) < 0) {
    switch(errno) {
    case ENOENT:
      return ERROR_FILE_NOT_FOUND;
    default:
      return os2_error(errno, ERROR_ACCESS_DENIED);
    }
  }

  switch(flag) {
  case FA_RETRIEVE:
    *pAttr = get_file_attributes(&ent);
    break;
  case FA_SET:
    if((rc=set_file_attributes(&ent, *pAttr)) != NO_ERROR)
      return rc;
    if(hfs_setattr(pvpfsd->vol, path, &ent) < 0)
      return os2_error(errno, ERROR_ACCESS_DENIED);
    break;
  default:
    return ERROR_NOT_SUPPORTED;
  }
  
  return NO_ERROR;
}


/******************************************************************************
**
** FS_FILEINFO - Get/Set file information
**
** Parameters
** ----------
** unsigned short flag                  indicates flavor of call
**   values:
**     FI_RETRIEVE      retrieve information
**     FI_SET           set information
** struct sffsi *psffsi                 pointer to FSD independant file instance
** struct sffsd *psffsd                 pointer to FSD dependant file instance
** unsigned short level                 level of information to get/set
** char *pData                          pointer to information area
** unsigned short cbData                size of area pointed to by pData
** unsigned short IOflag                bitfield of I/O suggestions
**   values:
**     IOFL_WRITETHRU   write all updated data before returning
**     IOFL_NOCACHE     don't cache any new data
** unsigned long *oError                offset where error occurred in GEA/FEA
**                                      list (EA calls only)
**
******************************************************************************/

#pragma argsused
short int FS_FILEINFO(struct vpfsd *pvpfsd,
		      unsigned short flag, struct sffsi *psffsi,
                      struct sffsd *psffsd, unsigned short level, char *pData, 
                      unsigned short cbData, unsigned short IOflag, 
                      unsigned long *oError)
{
  hfsdirent ent;
  if(hfs_fstat(psffsd->file, &ent) < 0)
    return os2_error(errno, ERROR_ACCESS_DENIED);

  return fileinfo(NULL, psffsd->file, &ent, NULL, psffsi, level, flag, 
		  pData, cbData, oError);
}

/******************************************************************************
**
** FS_MOVE - Move/rename a file
**
** Parameters
** ----------
** struct cdfsi *pcdfsi                 pointer to FSD independant current directory
** struct cdfsd *pcdfsd                 pointer to FSD dependant current directory
** char *pSrc                           pointer to source filename
** unsigned short iSrcCurDirEnd         offset to the end of the current directory in pSrc
** char *pDst                           pointer to destination filename
** unsigned short iDstCurDirEnd         offset to the end of the current directory in pDst
** unsigned short flags                 0x40 = destination is non 8.3 filename
**
******************************************************************************/

#pragma argsused
short int FS_MOVE(struct vpfsd *pvpfsd,
		  struct cdfsi *pcdfsi, struct cdfsd *pcdfsd,
                  char *pSrc, unsigned short iSrcCurDirEnd,
                  char *pDst, unsigned short iDstCurDirEnd,
                  unsigned short flags)
{
  char srcpath[CCHMAXPATH], destpath[CCHMAXPATH];

  if(os2_to_mac_name(pSrc+2, srcpath))
    return ERROR_ACCESS_DENIED;
  if(os2_to_mac_name(pDst+2, destpath))
    return ERROR_ACCESS_DENIED;

  if(hfs_rename(pvpfsd->vol, srcpath, destpath) < 0)
    return os2_error(errno, ERROR_ACCESS_DENIED);

  return NO_ERROR;
}


/******************************************************************************
**
** FS_NEWSIZE - Change size of file
**
** Parameters
** ----------
** struct sffsi *psffsi                 pointer to FSD independant file instance
** struct sffsd *psffsd                 pointer to FSD dependant file instance
** unsigned long len                    new length of file
** unsigned short IOflag                bitfield of I/O suggestions
**   values:
**     IOFL_WRITETHRU   write all updated data before returning
**     IOFL_NOCACHE     don't cache any new data
**
******************************************************************************/

#pragma argsused
short int FS_NEWSIZE(struct vpfsd *pvpfsd,
		     struct sffsi *psffsi, struct sffsd *psffsd,
                     unsigned long len, unsigned short IOflag)
{
  hfsdirent ent;

  if(psffsi->sfi_mode & OPEN_FLAGS_DASD)
    return ERROR_CANNOT_MAKE;

  if(hfs_fstat(psffsd->file, &ent) < 0)
    return os2_error(errno, ERROR_ACCESS_DENIED);

  if(len > ent.dsize) {
    char buf[HFS_BLOCKSZ];
    int bytes = len - ent.dsize;
    long lrc;

    if(hfs_lseek(psffsd->file, 0, SEEK_END) < 0)
      return os2_error(errno, ERROR_ACCESS_DENIED);

    /* This is ugly */
    memset(buf, 0, sizeof(buf));
    while(bytes) {
      if(hfs_write(psffsd->file, buf, min(bytes, sizeof(buf))) == -1)
	return os2_error(errno, ERROR_WRITE_FAULT);
      bytes -= lrc;
    }
  }
  else if(len < ent.dsize)
    if(hfs_truncate(psffsd->file, len) < 0)
      return os2_error(errno, ERROR_WRITE_FAULT);

  psffsi->sfi_size = len;
  return NO_ERROR;
}

/******************************************************************************
**
** FS_OPENCREATE - Open or create a new file
**
** Parameters
** ----------
** struct cdfsi *pcdfsi                 pointer to FSD independant current directory
** struct cdfsd *pcdfsd                 pointer to FSD dependant current directory
** char *pName                          pointer to filename
** unsigned short iCurDirEnd            offset to the end of the current directory in pName
** struct sffsi *psffsi                 pointer to FSD independant file instance
** struct sffsd *psffsd                 pointer to FSD dependant file instance
** unsigned long openmode               sharing and access mode
** unsigned short openflag              action to take when file exists/doesn't exist
** unsigned short *pAction              returns the action that the IFS took
** unsigned short attr                  OS/2 file attributes
** char *pEABuf                         pointer to EAs to attach to new file
** unsigned short *pfgenFlag            flags returned by the IFS
**   values:
**     FOC_NEEDEAS      indicates there are critical EAs associated with the file
** unsigned long *oError                offset where error occurred in FEA list
**
******************************************************************************/

#pragma argsused
short int FS_OPENCREATE(struct vpfsd *pvpfsd,
			struct cdfsi *pcdfsi, struct cdfsd *pcdfsd,
                        char *pName, unsigned short iCurDirEnd,
                        struct sffsi *psffsi, struct sffsd *psffsd,
                        unsigned long openmode, unsigned short openflag,
                        unsigned short *pAction, unsigned short attr,
                        char *pEABuf, unsigned short *pfgenFlag,
                        unsigned long *oError)
{
  char path[CCHMAXPATH];
  hfsdirent ent;
  int rc;

#ifdef DEBUG
  printf("FS_OPENCREATE: file = '%s'\n", pName);
#endif

  if(openmode & OPEN_FLAGS_DASD) {
    psffsi->sfi_size = pvpfsd->vol->mdb.drNmAlBlks*pvpfsd->vol->mdb.drAlBlkSiz
      + pvpfsd->vol->mdb.drAlBlSt*pvpfsd->vol->sector_size;
    psffsi->sfi_position=0;
    return NO_ERROR;
  }

  if(os2_to_mac_name(pName+2, path))
    return ERROR_ACCESS_DENIED;

  if((pvpfsd->vol->flags & HFS_READONLY) &&
     ((openmode & 7) == OPEN_ACCESS_READWRITE ||
      (openmode & 7) == OPEN_ACCESS_WRITEONLY))
    return ERROR_WRITE_PROTECT;

  rc = hfs_stat(pvpfsd->vol, path, &ent);
  if(rc < 0) {
    if(errno != ENOENT)
      return os2_error(errno, ERROR_ACCESS_DENIED);

    /* File does not exist */
    switch(openflag & 0xf0) {
    case OPEN_ACTION_CREATE_IF_NEW:
      if(hfs_create(pvpfsd->vol, path, 
		    DEFAULT_FILE_TYPE, DEFAULT_CREATOR) < 0)
	return os2_error(errno, ERROR_WRITE_FAULT);

      psffsd->file = hfs_open(pvpfsd->vol, path);
      if(psffsd->file == NULL)
	return os2_error(errno, ERROR_ACCESS_DENIED);

      if(hfs_fstat(psffsd->file, &ent) < 0) {
	rc = errno;
	hfs_close(psffsd->file);
	return os2_error(rc, ERROR_ACCESS_DENIED);
      }
      set_file_attributes(&ent, attr);
      
      /* Grow file if necessary */
      if(psffsi->sfi_size) {
	/* This is ugly */
	char buf[HFS_BLOCKSZ];
	int bytes = psffsi->sfi_size;
	long lrc;

	memset(buf, 0, sizeof(buf));
	while(bytes) {
	  lrc = hfs_write(psffsd->file, buf, min(bytes, sizeof(buf)));
	  if(lrc == -1) {
	    rc = errno;
	    hfs_close(psffsd->file);
	    return os2_error(rc, ERROR_WRITE_FAULT);
	  }
	  bytes -= lrc;
	}
      }

      transfer_timestamps(psffsi, &ent);
      psffsi->sfi_tstamp |= ST_PCREAT | ST_PWRITE;
      if(hfs_fsetattr(psffsd->file, &ent) < 0) {
	rc = errno;
	hfs_close(psffsd->file);
	return os2_error(errno, ERROR_WRITE_FAULT);
      }
      *pAction=FILE_CREATED;

      return NO_ERROR;
    case OPEN_ACTION_FAIL_IF_NEW:
      return ERROR_FILE_NOT_FOUND;
    default:
      return ERROR_ACCESS_DENIED;
    }
  }
  /* File exists */
  switch(openflag & 0x0f) {
  case OPEN_ACTION_OPEN_IF_EXISTS:
    psffsd->file = hfs_open(pvpfsd->vol, path);
    if(psffsd->file == NULL)
      return os2_error(errno, ERROR_ACCESS_DENIED);
    
    *pAction=FILE_EXISTED;
    psffsi->sfi_size = ent.dsize;
    transfer_timestamps(psffsi, &ent);
    return NO_ERROR;
  case OPEN_ACTION_REPLACE_IF_EXISTS:
    if(hfs_delete(pvpfsd->vol, path) < 0)
      return os2_error(errno, ERROR_WRITE_FAULT);
    if(hfs_create(pvpfsd->vol, path, 
		  DEFAULT_FILE_TYPE, DEFAULT_CREATOR) < 0)
      return os2_error(errno, ERROR_WRITE_FAULT);
    
    psffsd->file = hfs_open(pvpfsd->vol, path);
    if(psffsd->file == NULL)
      return os2_error(errno, ERROR_ACCESS_DENIED);

    if(hfs_fstat(psffsd->file, &ent) < 0) {
      rc = errno;
      hfs_close(psffsd->file);
      return os2_error(rc, ERROR_ACCESS_DENIED);
    }
    set_file_attributes(&ent, attr);
    
    /* Grow file if necessary */
    if(psffsi->sfi_size) {
      /* This is ugly */
      char buf[HFS_BLOCKSZ];
      int bytes = psffsi->sfi_size;
      long lrc;
      
      memset(buf, 0, sizeof(buf));
      while(bytes) {
	lrc = hfs_write(psffsd->file, buf, min(bytes, sizeof(buf)));
	if(lrc == -1) {
	  rc = errno;
	  hfs_close(psffsd->file);
	  return os2_error(rc, ERROR_WRITE_FAULT);
	}
	bytes -= lrc;
      }
    }
    
    transfer_timestamps(psffsi, &ent);
    psffsi->sfi_tstamp |= ST_PCREAT | ST_PWRITE;
    *pAction=FILE_CREATED;
    
    if(hfs_fsetattr(psffsd->file, &ent) < 0) {
      rc = errno;
      hfs_close(psffsd->file);
      return os2_error(errno, ERROR_WRITE_FAULT);
    }
    
    return NO_ERROR;
  case OPEN_ACTION_FAIL_IF_EXISTS:
    return ERROR_OPEN_FAILED;
  default:
    return ERROR_ACCESS_DENIED;
  }
}

/******************************************************************************
**
** FS_PATHINFO - get/set file information by filename
**
** Parameters
** ----------
** unsigned short flag                  indicates flavor of call
**   values:
**     PI_RETRIEVE      retrieve information
**     PI_SET           set information
** struct cdfsi *pcdfsi                 pointer to FSD independant current directory
** struct cdfsd *pcdfsd                 pointer to FSD dependant current directory
** char *pName                          pointer to filename
** unsigned short iCurDirEnd            offset to the end of the current directory in pName
** unsigned short level                 level of information to get/set
** char *pData                          pointer to information area
** unsigned short cbData                size of area pointed to by pData
** unsigned long *oError                offset where error occurred in GEA/FEA
**                                      list (EA calls only)
**
******************************************************************************/

#pragma argsused
short int FS_PATHINFO(struct vpfsd *pvpfsd,
		      unsigned short flag, struct cdfsi *pcdfsi,
                      struct cdfsd *pcdfsd, char *pName, 
                      unsigned short iCurDirEnd, unsigned short level, 
                      char *pData, unsigned short cbData, 
                      unsigned long *oError)
{
  char macname[CCHMAXPATH];
  hfsdirent ent;

  if(os2_to_mac_name(pName+2, macname))
    return ERROR_ACCESS_DENIED;
  if(hfs_stat(pvpfsd->vol, macname, &ent) < 0)
    return os2_error(errno, ERROR_ACCESS_DENIED);

  return fileinfo(pvpfsd->vol, NULL, &ent, macname, NULL, level, flag, 
		  pData, cbData, oError);
}

/******************************************************************************
**
** FS_READ - read data from a file
**
** Parameters
** ----------
** struct sffsi *psffsi                 pointer to FSD independant file instance
** struct sffsd *psffsd                 pointer to FSD dependant file instance
** char *pData                          pointer to buffer
** unsigned short *pLen                 length of buffer
** unsigned short IOflag                bitfield of I/O suggestions
**   values:
**     IOFL_WRITETHRU   write all updated data before returning
**     IOFL_NOCACHE     don't cache any new data
**
******************************************************************************/

#pragma argsused
short int FS_READ(struct vpfsd *pvpfsd,
		  struct sffsi *psffsi, struct sffsd *psffsd,
                  char *pData, unsigned short *pLen, unsigned short IOflag)
{
  if(psffsi->sfi_position >= psffsi->sfi_size) {
    *pLen=0;
    return NO_ERROR;
  }

  if(psffsi->sfi_mode & OPEN_FLAGS_DASD) {
    hfsvolent ent;
    unsigned block_size, num_blocks;
    char *buf;

    hfs_vstat(pvpfsd->vol, &ent);
    block_size = HFS_BLOCKSZ;
    num_blocks = *pLen/block_size+3;
    buf=(char *)malloc(num_blocks*block_size);

    if(read_blocks(psffsi->sfi_position/block_size, num_blocks,
		   block_size, ent.sector_size, ent.hVPB, buf) < 0) {
      free(buf);
      return ERROR_READ_FAULT;
    }
    memcpy(pData, buf+psffsi->sfi_position%ent.sector_size, *pLen);
    free(buf);
    psffsi->sfi_position += *pLen;
  }
  else {
    /* We need to use another buffer because the shared memory is now 
       inaccessible to the ring 0 piece */
    char *buf = (char *)malloc(*pLen);
    long bytes_read;

    if(hfs_lseek(psffsd->file, psffsi->sfi_position, SEEK_SET)<0 ||
       (bytes_read=hfs_read(psffsd->file, buf, *pLen))==-1) {
      *pLen=0;
      free(buf);
      return ERROR_READ_FAULT;
    }
    memcpy(pData, buf, bytes_read);
    free(buf);
    psffsi->sfi_position += bytes_read;
    *pLen = bytes_read;
  }
  return NO_ERROR;
}

/******************************************************************************
**
** FS_WRITE - write data to a file
**
** Parameters
** ----------
** struct sffsi *psffsi                 pointer to FSD independant file instance
** struct sffsd *psffsd                 pointer to FSD dependant file instance
** char *pData                          pointer to buffer
** unsigned short *pLen                 length of buffer
** unsigned short IOflag                bitfield of I/O suggestions
**   values:
**     IOFL_WRITETHRU   write all updated data before returning
**     IOFL_NOCACHE     don't cache any new data
**
******************************************************************************/

#pragma argsused
short int FS_WRITE(struct vpfsd *pvpfsd,
		   struct sffsi *psffsi, struct sffsd *psffsd,
                   char *pData, unsigned short *pLen, unsigned short IOflag)
{
  /* If positioned beyond end of file, disallow write */
  if(psffsi->sfi_position > psffsi->sfi_size) {
    *pLen=0;
    return NO_ERROR;
  }

  if(psffsi->sfi_mode & OPEN_FLAGS_DASD) {
    hfsvolent ent;
    unsigned block_size;
    int start_offset, end_offset;
    char *buf;

    hfs_vstat(pvpfsd->vol, &ent);
    block_size = HFS_BLOCKSZ;

    start_offset = psffsi->sfi_position%block_size;
    end_offset = (start_offset+*pLen)%block_size;
    buf=(char *)malloc(*pLen+block_size*3);

    /* Read first partial sector */
    if(start_offset) {
      if(read_blocks(psffsi->sfi_position/block_size, 1,
		     block_size, ent.sector_size, ent.hVPB, buf) < 0) {
	free(buf);
	return ERROR_READ_FAULT;
      }
    }
    /* Read last partial sector */
    if(end_offset) {
      if(read_blocks((psffsi->sfi_position+*pLen)/block_size, 1,
		     block_size, ent.sector_size, ent.hVPB, 
		     buf+block_size*((start_offset+*pLen)/block_size)) < 0) {
	free(buf);
	return ERROR_READ_FAULT;
      }
    }
    
    memcpy(buf+start_offset, pData, *pLen);
    if(write_blocks(psffsi->sfi_position/block_size,
		    (start_offset+*pLen)/block_size, block_size,
		    ent.sector_size, ent.hVPB, buf) < 0) {
      free(buf);
      return ERROR_WRITE_FAULT;
    }
    free(buf);
    psffsi->sfi_position += *pLen;
  }
  else {
    /* We allocate a temporary buffer because pData is inaccessible 
       to the ring 0 piece */
    char *buf = (char *)malloc(*pLen);
    memcpy(buf, pData, *pLen);

    if(hfs_lseek(psffsd->file, psffsi->sfi_position, SEEK_SET)<0 || 
       hfs_write(psffsd->file, buf, *pLen)==-1) {
      *pLen=0;
      free(buf);
      return ERROR_READ_FAULT;
    }
    psffsi->sfi_position += *pLen;
    if(psffsi->sfi_position > psffsi->sfi_size)
      psffsi->sfi_size=psffsi->sfi_position;
    psffsi->sfi_tstamp |= ST_SWRITE | ST_PWRITE;
  }
  return NO_ERROR;
}

/* Common code for FS_FILEINFO and FS_PATHINFO */
/* file is NULL and path and vol are valid for non-open files 
   and directories. */
/* file is valid and for open files. */
static unsigned short fileinfo(hfsvol *vol, hfsfile *file, hfsdirent *ent,
			       char *path, struct sffsi *psffsi,
			       unsigned short level, unsigned short flag,
			       char *pData, unsigned short cbData,
			       unsigned long *oError)
{
  int rc, file_opened=0;
  PFILESTATUS p;

#ifdef DEBUG
  printf("fileinfo, level=%hu, flag=%hu, datalen=%hu\n", level, flag, cbData);
#endif

  switch(level) {
  case FIL_STANDARD:
    switch(flag) {
    case FI_RETRIEVE:
      if(cbData<sizeof(FILESTATUS))
	return ERROR_BUFFER_OVERFLOW;
      catrec_to_filebuf(pData, ent, level);
      return NO_ERROR;
    case FI_SET:
      p = (PFILESTATUS)pData;
      if(cbData<sizeof(FILESTATUS))
	return ERROR_INSUFFICIENT_BUFFER;
      if(*(unsigned short *)(&p->fdateCreation) 
	 || *(unsigned short *)(&p->ftimeCreation)) {
	if(psffsi) {
	  psffsi->sfi_cdate=*(unsigned short *)(&p->fdateCreation);
	  psffsi->sfi_ctime=*(unsigned short *)(&p->ftimeCreation);
	  psffsi->sfi_tstamp |= ST_PCREAT;
	  psffsi->sfi_tstamp &= ~ST_SCREAT;
	}
	ent->crdate = os2_to_unix_time(p->fdateCreation, p->ftimeCreation);
      }
      if(*(unsigned short *)(&p->fdateLastWrite) 
	 || *(unsigned short *)(&p->ftimeLastWrite)) {
	if(psffsi) {
	  psffsi->sfi_mdate=*(unsigned short *)(&p->fdateLastWrite);
	  psffsi->sfi_mtime=*(unsigned short *)(&p->ftimeLastWrite);
	  psffsi->sfi_tstamp |= ST_PWRITE;
	  psffsi->sfi_tstamp &= ~ST_SWRITE;
	}
	ent->mddate = os2_to_unix_time(p->fdateLastWrite, p->ftimeLastWrite);
      }
      rc = set_file_attributes(ent, p->attrFile);
      if(rc!=NO_ERROR)
	return rc;
      if(file == NULL) {
	if(hfs_setattr(vol, path, ent) < 0)
	  return os2_error(errno, ERROR_ACCESS_DENIED);
      }
      else {
	if(hfs_fsetattr(file, ent) < 0)
	  return os2_error(errno, ERROR_ACCESS_DENIED);
      }
      return NO_ERROR;
    }
    return NO_ERROR;

  case FIL_QUERYEASIZE:
    switch(flag) {
    case FI_RETRIEVE:
      if(cbData<sizeof(FILESTATUS2))
	return ERROR_BUFFER_OVERFLOW;
      catrec_to_filebuf(pData, ent, level);
      if(ent->flags & HFS_ISDIR)
	((PFILESTATUS2)pData)->cbList = sizeof(ULONG);
      else {
	if(file==NULL) {
	  file = hfs_open(vol, path);
	  if(file == NULL)
	    return os2_error(errno, ERROR_ACCESS_DENIED);
	  file_opened = 1;
	}
	((PFILESTATUS2)pData)->cbList = ea_size(file);
	if(file_opened)
	  hfs_close(file);
      }
      return NO_ERROR;
    case FI_SET:
      return NO_ERROR; /* Pretend we succeeded in setting EAs */
    }
    return NO_ERROR;

  case FIL_QUERYEASFROMLIST:
  case FIL_QUERYALLEAS:
    switch(flag) {
    case FI_RETRIEVE:
      /* We need a temporary buffer because pData buffer cannot be accessed
	 from ring 0 now */
      if(ent->flags & HFS_ISDIR) {
	PFEALIST pfealist = (PFEALIST)pData;
	pfealist->cbList = sizeof(ULONG);
      }
      else {
	char *tmpbuf = malloc(cbData);

	if(file==NULL) {
	  file = hfs_open(vol, path);
	  if(file == NULL)
	    return os2_error(errno, ERROR_ACCESS_DENIED);
	  file_opened = 1;
	}
	if(level==FIL_QUERYEASFROMLIST)
	  rc = build_fealist_from_gealist(file, pData, tmpbuf, cbData);
	else
	  rc = build_fealist(file, tmpbuf, cbData);
	if(file_opened)
	  hfs_close(file);

	if(rc==NO_ERROR)
	  memcpy(pData, tmpbuf, *(PULONG)tmpbuf);
	else
	  *(PULONG)pData = *(PULONG)tmpbuf;
	free(tmpbuf);
      }
      return rc;
    }
    return NO_ERROR;
    
  default:
    return ERROR_INVALID_LEVEL;
  }
}

void transfer_timestamps(struct sffsi *psffsi, hfsdirent *ent)
{
  union {
    FTIME t;
    unsigned short i;
  } ut;
  union {
    FDATE d;
    unsigned short i;
  } ud;

  ut.t = unix_to_os2_time(ent->crdate);
  ud.d = unix_to_os2_date(ent->crdate);
  psffsi->sfi_ctime = ut.i;
  psffsi->sfi_cdate = ud.i;

  ut.t = unix_to_os2_time(ent->mddate);
  ud.d = unix_to_os2_date(ent->mddate);
  psffsi->sfi_mtime = ut.i;
  psffsi->sfi_mdate = ud.i;

  psffsi->sfi_atime = psffsi->sfi_adate = 0;
}
