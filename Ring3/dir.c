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

/* dir.c
   Directory management.
*/

#define INCL_DOSERRORS
#define INCL_NOCOMMON
#define INCL_NOPMAPI
#define INCL_DOSFILEMGR
#include <os2.h>

#include <stdlib.h>
#include <errno.h>

#include "libhfs/hfs.h"
#include "fsd.h"
#include "fsdep.h"
#include "names.h"
#include "error.h"

/******************************************************************************
**
** FS_CHDIR - Change current directory
**
** Parameters
** ----------
** unsigned short flag                  indicates flavor of call
**   values:
**     CD_EXPLICIT      creating a new current directory
**     CD_VERIFY        verifying a current directory
**     CD_FREE          freeing an instance of a current directory
** struct cdfsi *pcdfsi                 pointer to FSD independant current directory
** struct cdfsd *pcdfsd                 pointer to FSD dependant current directory
** char *pDir                           pointer to directory to change to
** unsigned short iCurDirEnd            offset to the end of the current directory in pDir
**
******************************************************************************/

#pragma argsused
short int FS_CHDIR(struct vpfsd *pvpfsd, unsigned short flag, struct cdfsi *pcdfsi, 
                   struct cdfsd *pcdfsd, char *pDir, 
                   unsigned short iCurDirEnd)
{
  char macname[CCHMAXPATH];
  hfsdirent ent;

  switch(flag) {
  case CD_EXPLICIT:
    if(os2_to_mac_name(pDir+2, macname))
      return ERROR_ACCESS_DENIED;
    if(hfs_stat(pvpfsd->vol, macname, &ent) < 0)
      return os2_error(errno, ERROR_PATH_NOT_FOUND);
    if(!(ent.flags & HFS_ISDIR))
      return ERROR_PATH_NOT_FOUND;
      
    /*
    if(hfs_chdir(pvpfsd->vol, macname) < 0) {
      switch(errno) {
      case ENOTDIR:
	return ERROR_ACCESS_DENIED;
      default:
	return ERROR_PATH_NOT_FOUND;
      }
    }
    */
    return NO_ERROR;

  case CD_VERIFY:
    if(os2_to_mac_name(pcdfsi->cdi_curdir+2, macname))
      return ERROR_PATH_NOT_FOUND;

    /* Check that filename is a directory */
    if(hfs_chdir(pvpfsd->vol, macname) < 0) {
      switch(errno) {
      case ENOTDIR:
      default:
	return ERROR_PATH_NOT_FOUND;
      }
    }
    return NO_ERROR;

  case CD_FREE:
    return NO_ERROR;

  default:
    return ERROR_NOT_SUPPORTED;
  }
}


/******************************************************************************
**
** FS_MKDIR - Make a new directory
**
** Parameters
** ----------
** struct cdfsi *pcdfsi                 pointer to FSD independant current directory
** struct cdfsd *pcdfsd                 pointer to FSD dependant current directory
** char *pName                          pointer to directory name to create
** unsigned short iCurDirEnd            offset to the end of the current directory 
**                                      in pName
** char *pEABuf                         pointer to EAs to attach to new directory
** unsigned short flags                 0x40 = directory is non 8.3 filename
** unsigned long *oError                offset where error occurred in FEA list
**
******************************************************************************/

#pragma argsused
short int FS_MKDIR(struct vpfsd *pvpfsd, struct cdfsi *pcdfsi, struct cdfsd *pcdfsd,
                   char *pName, unsigned short iCurDirEnd, char *pEABuf, 
                   unsigned short flags, unsigned long *oError)
{
  char path[CCHMAXPATH];

  if(os2_to_mac_name(pName+2, path))
    return ERROR_ACCESS_DENIED;

  if(hfs_mkdir(pvpfsd->vol, path) < 0) {
    switch(errno) {
    case EEXIST:
      return ERROR_ACCESS_DENIED;
    case ENOENT:
      return ERROR_PATH_NOT_FOUND;
    default:
      return ERROR_ACCESS_DENIED;
    }
  }
  return NO_ERROR;
}


/******************************************************************************
**
** FS_RMDIR - Delete directory
**
** Parameters
** ----------
** struct cdfsi *pcdfsi                 pointer to FSD independant current directory
** struct cdfsd *pcdfsd                 pointer to FSD dependant current directory
** char *pName                          pointer to directory name to delete
** unsigned short iCurDirEnd            offset to the end of the current directory in pName
**
******************************************************************************/

#pragma argsused
short int FS_RMDIR(struct vpfsd *pvpfsd, struct cdfsi *pcdfsi, struct cdfsd  *pcdfsd,
                   char *pName, unsigned short iCurDirEnd)
{
  char macname[CCHMAXPATH];

  if(os2_to_mac_name(pName+2, macname))
    return ERROR_ACCESS_DENIED;
  if(hfs_rmdir(pvpfsd->vol, macname) < 0) {
    switch(errno) {
    case ENOTDIR:
      return ERROR_ACCESS_DENIED;
    case ENOTEMPTY:
      return ERROR_DIR_NOT_EMPTY;
    default:
      return ERROR_WRITE_FAULT;
    }
  }
  return NO_ERROR;
}
