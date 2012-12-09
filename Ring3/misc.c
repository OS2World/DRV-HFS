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

/* misc.c
   Miscellaneous FSD functions
*/

#define INCL_NOPMAPI
#define INCL_DOSERRORS
#include <os2.h>

#include "fsd.h"

/******************************************************************************
**
** FS_PROCESSNAME - Canonicalize a filename
**
** Parameters
** ----------
** char *pNameBuf                       filename to canonicalize
**
******************************************************************************/

#pragma argsused
short int FS_PROCESSNAME(char *pNameBuf)
{
  return NO_ERROR;
}


/******************************************************************************
*******************************************************************************
**
** Locking support entry points
**
*******************************************************************************
******************************************************************************/



/******************************************************************************
**
** FS_CANCELLOCKREQUEST - unlock a range
**
** Parameters
** ----------
** struct sffsi *psffsi                 pointer to FSD independant file instance
** struct sffsd *psffsd                 pointer to FSD dependant file instance
** void *pLockRange                     range to unlock
**
******************************************************************************/

#pragma argsused
short int FS_CANCELLOCKREQUEST(struct sffsi *psffsi, struct sffsd *psffsd,
                               struct filelock *pLockRange)
{
  return ERROR_NOT_SUPPORTED;
}


/******************************************************************************
**
** FS_FILELOCKS - lock a range
**
** Parameters
** ----------
** struct sffsi *psffsi                 pointer to FSD independant file instance
** struct sffsd *psffsd                 pointer to FSD dependant file instance
** void *pUnLockRange                   range to unlock
** void *pLockRange                     range to lock
** unsigned long timeout                time in milliseconds to wait
** unsigned long flags                  flags
**   values:
**     0x01                     sharing of this file region is allowed
**     0x02                     atomic lock request
**
******************************************************************************/

#pragma argsused
short int FS_FILELOCKS(struct sffsi *psffsi, struct sffsd *psffsd,
                       struct filelock *pUnLockRange, struct filelock *pLockRange,
                       unsigned long timeout, unsigned long flags)
{
  return ERROR_NOT_SUPPORTED;
}



/******************************************************************************
*******************************************************************************
**
** UNC entry point
**
*******************************************************************************
******************************************************************************/



/******************************************************************************
**
** FS_VERIFYUNCNAME - Check if the IFS controls the server in question
**
** Parameters
** ----------
** unsigned short flag                  flags
**   values:
**     VUN_PASS1                pass 1 poll
**     VUN_PASS2                pass 2 poll
** char *pName                          pointer to server in UNC format
**
******************************************************************************/

#pragma argsused
short int FS_VERIFYUNCNAME(unsigned short flag, char *pName)
{
  return ERROR_NOT_SUPPORTED;
}
