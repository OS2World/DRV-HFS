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

/* stubs.c
   Stubs for the FS_xxx calls
*/

#define INCL_DOSERRORS
#define INCL_NOPMAPI
#include <os2.h>

#include <dhcalls.h>

#include <string.h>
#include <dos.h>

#include "fsd.h"
#include "..\r0r3shar.h"
#include "r0struct.h"
#include "fsh.h"
#include "r0global.h"
#include "r0comm.h"
#ifdef DEBUG
#include "log.h"
#endif

/******************************************************************************
** Name of IFS
******************************************************************************/

char pascal FS_NAME[] = "HFS";

/******************************************************************************
** Attributes of IFS
******************************************************************************/

/*
#define SUP_REMOTE
#define SUP_UNC
#define SUP_LOCK
#define SUP_LVL7
#define SUP_FILEIO
#define SUP_PAGEIO
*/

unsigned long int pascal FS_ATTRIBUTE = 0
#if defined(SUP_REMOTE)
                        | FSA_REMOTE   /* If file system is remote          */
#endif
#if defined(SUP_UNC)
                        | FSA_UNC      /* If IFS supports UNC               */
#endif
#if defined(SUP_LOCK)
                        | FSA_LOCK     /* If IFS supports file locking      */
#endif
#if defined(SUP_LVL7)
                        | FSA_LVL7     /* If IFS supports QPathInfo level 7 */
#endif
                        ;

/******************************************************************************
** Primary entry points
******************************************************************************/

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

short int far pascal _export _loadds FS_MOUNT(unsigned short flag, 
					      struct vpfsi far *pvpfsi,
					      struct vpfsd far *pvpfsd, 
					      unsigned short hVPB, 
					      char far *pBoot)
{
  unsigned short rc;

  if(!CPAttached) {
#ifdef DEBUG
    printk("FS_MOUNT: CP not attached\n");
#endif
    return ERROR_VOLUME_NOT_MOUNTED;
  }

  rc = PreSetup();
  if (rc != NO_ERROR)
    return rc;

  /* Make sure the control program is still attached (this is how we
     know our buffers are still valid) */
  if (!CPAttached) {
    FSH_SEMCLEAR(&CPData.BufLock);
    return (ERROR_NOT_READY);
  }

  /* Setup parameters -- from here to the PostSetup(), we MUST NOT block */
  CPData.OpData->i.funccode = CPFC_MOUNT;
  CPData.OpData->p.mount.flag = flag;
  CPData.OpData->p.mount.vpfsi = *pvpfsi;
  CPData.OpData->p.mount.vpfsd = *pvpfsd;
  CPData.OpData->p.mount.vol_descr = 0;
  CPData.OpData->p.mount.hVPB = hVPB;
  if( flag==MOUNT_MOUNT )
    memcpy(CPData.Buf, pBoot, 512);
  rc = PostSetup();
  if (rc != NO_ERROR)
    return rc;

  /* Copy results */
  *pvpfsi = CPData.OpData->p.mount.vpfsi;
  *pvpfsd = CPData.OpData->p.mount.vpfsd;

  rc = CPData.OpData->i.rc;

  /* We CANNOT access the buffers after this line */
  FSH_SEMCLEAR(&CPData.BufLock);

  return rc;
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
** char far *pDev                       drive or device that is being attached/detached
** struct vpfsd far *pvpfsd             pointer to FSD dependant volume parameters
** struct cdfsd far *pcdfsd             pointer to FSD dependant current directory
** void far *pParm                      UNVERIFIED pointer to FSD dependant attachment info
** unsigned short far *pLen             length of area pointed to by pParm
**
******************************************************************************/

#pragma argsused
short int far pascal _export _loadds FS_ATTACH(unsigned short flag,
                                               char far *pDev,
                                               struct vpfsd far *pvpfsd,
                                               struct cdfsd far *pcdfsd,
                                               void far *pParm,
                                               unsigned short far *pLen) 
{
  return ERROR_NOT_SUPPORTED;
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
** unsigned short hVPB                  volume handle
** char far *pData                      UNVERIFIED pointer to data buffer
** unsigned short cbData                length of data buffer
** unsigned short level                 type of information to return
**
******************************************************************************/

#pragma argsused
short int far pascal _export _loadds FS_FSINFO(unsigned short flag, unsigned short hVPB,
                               char far *pData, unsigned short cbData,
                               unsigned short level)
{
  unsigned short rc;
  struct vpfsi *pvpfsi;
  struct vpfsd *pvpfsd;

  rc = PreSetup();
  if (rc != NO_ERROR)
    return rc;

  /* Verify parameters */
  if (flag == INFO_SET && cbData != 0)
    rc = FSH_PROBEBUF(PB_OPREAD, pData, cbData);
  else if (cbData != 0)
    rc = FSH_PROBEBUF(PB_OPWRITE, pData, cbData);
  if (rc != NO_ERROR) {
     FSH_SEMCLEAR(&CPData.BufLock);
     return rc;
  }

  /* Make sure the control program is still attached (this is how we
     know our buffers are still valid) */
  if (!CPAttached) {
    FSH_SEMCLEAR(&CPData.BufLock);
    return (ERROR_NOT_READY);
  }

  /* Setup parameters -- from here to the PostSetup(), we MUST NOT block */
  CPData.OpData->i.funccode = CPFC_FSINFO;
  CPData.OpData->p.fsinfo.flag = flag;
  CPData.OpData->p.fsinfo.hVPB = hVPB;
  FSH_GETVOLPARM(hVPB, &pvpfsi, &pvpfsd);
  CPData.OpData->p.fsinfo.vpfsi = *pvpfsi;
  CPData.OpData->p.fsinfo.vpfsd = *pvpfsd;
  memcpy(CPData.Buf, pData, cbData);
  CPData.OpData->p.fsinfo.cbData = cbData;
  CPData.OpData->p.fsinfo.level = level;

  rc = PostSetup();
  if (rc != NO_ERROR)
    return rc;

  /* Copy results */
  if (flag == INFO_RETRIEVE)
    memcpy(pData, CPData.Buf, cbData);
  else if(flag == INFO_SET)
    *pvpfsi = CPData.OpData->p.fsinfo.vpfsi;

  rc = CPData.OpData->i.rc;

  /* We CANNOT access the buffers after this line */
  FSH_SEMCLEAR(&CPData.BufLock);

  return rc;
}


/******************************************************************************
**
** FS_FLUSHBUF - Flush buffers for a specified volume
**
** Parameters
** ----------
** unsigned short hVPB                  handle to volume to flush
** unsigned short flag                  indicates whether to discard or retain cache
**   values:
**     FLUSH_RETAIN     retain cached information
**     FLUSH_DISCARD    discard cached information
**
******************************************************************************/

#pragma argsused
short int far pascal _export _loadds FS_FLUSHBUF(unsigned short hVPB, unsigned short flag)
{
  unsigned short rc;
  struct vpfsi *pvpfsi;
  struct vpfsd *pvpfsd;

  rc = PreSetup();
  if (rc != NO_ERROR)
    return rc;

  // Make sure the control program is still attached (this is how we
  // know our buffers are still valid)
  if (!CPAttached) {
    FSH_SEMCLEAR(&CPData.BufLock);
    return (ERROR_NOT_READY);
  }

  // Setup parameters -- from here to the PostSetup(), we MUST NOT block
  CPData.OpData->i.funccode = CPFC_FLUSHBUF;
  FSH_GETVOLPARM(hVPB, &pvpfsi, &pvpfsd);
  CPData.OpData->p.flushbuf.vpfsd = *pvpfsd;
  CPData.OpData->p.flushbuf.flag = flag;

  rc = PostSetup();
  if (rc != NO_ERROR)
    return rc;

  *pvpfsd = CPData.OpData->p.flushbuf.vpfsd;
  rc = CPData.OpData->i.rc;

  // We CANNOT access the buffers after this line
  FSH_SEMCLEAR(&CPData.BufLock);

  return rc;
}



/*-----------------------------------------------------------------------------
--
-- Directory management
--
-----------------------------------------------------------------------------*/



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
** struct cdfsi far *pcdfsi             pointer to FSD independant current directory
** struct cdfsd far *pcdfsd             pointer to FSD dependant current directory
** char far *pDir                       pointer to directory to change to
** unsigned short iCurDirEnd            offset to the end of the current directory in pDir
**
******************************************************************************/

#pragma argsused
short int far pascal _export _loadds FS_CHDIR(unsigned short flag, struct cdfsi far *pcdfsi,
                              struct cdfsd far *pcdfsd, char far *pDir,
                              unsigned short iCurDirEnd)
{
  unsigned short rc;
  struct vpfsi *pvpfsi;
  struct vpfsd *pvpfsd;

  rc = PreSetup();
  if (rc != NO_ERROR)
    return rc;


  // Verify parameters

  // Make sure the control program is still attached (this is how we
  // know our buffers are still valid)
  if (!CPAttached) {
    FSH_SEMCLEAR(&CPData.BufLock);
    return (ERROR_NOT_READY);
  }


  // Setup parameters -- from here to the PostSetup(), we MUST NOT block
  CPData.OpData->i.funccode = CPFC_CHDIR;
  CPData.OpData->p.chdir.flag = flag;
  if (flag != CD_FREE) {
    CPData.OpData->p.chdir.cdfsi = *pcdfsi;
    FSH_GETVOLPARM(pcdfsi->cdi_hVPB, &pvpfsi, &pvpfsd);
    CPData.OpData->p.chdir.vpfsd = *pvpfsd;
  }
  CPData.OpData->p.chdir.cdfsd = *pcdfsd;
  if (flag == CD_EXPLICIT)
    strncpy(CPData.OpData->p.chdir.Dir, pDir, sizeof(CPData.OpData->p.chdir.Dir));
  CPData.OpData->p.chdir.iCurDirEnd = iCurDirEnd;

  rc = PostSetup();
  if (rc != NO_ERROR)
    return rc;

  // Copy results
  *pcdfsd = CPData.OpData->p.chdir.cdfsd;
  if(flag!=CD_FREE)
    *pvpfsd = CPData.OpData->p.chdir.vpfsd;

  rc = CPData.OpData->i.rc;

  // We CANNOT access the buffers after this line
  FSH_SEMCLEAR(&CPData.BufLock);

  return rc;
}


/******************************************************************************
**
** FS_MKDIR - Make a new directory
**
** Parameters
** ----------
** struct cdfsi far *pcdfsi             pointer to FSD independant current directory
** struct cdfsd far *pcdfsd             pointer to FSD dependant current directory
** char far *pName                      pointer to directory name to create
** unsigned short iCurDirEnd            offset to the end of the current directory
**                                      in pName
** char *pEABuf                         UNVERIFIED pointer to EAs to attach
**                                      to new directory
** unsigned short flags                 0x40 = directory is non 8.3 filename
**
******************************************************************************/

#pragma argsused
short int far pascal _export _loadds FS_MKDIR(struct cdfsi far *pcdfsi, struct cdfsd far *pcdfsd,
                              char far *pName, unsigned short iCurDirEnd,
                              char far *pEABuf, unsigned short flags)
{
  unsigned short rc;
  struct vpfsi *pvpfsi;
  struct vpfsd *pvpfsd;

  rc = PreSetup();
  if (rc != NO_ERROR)
    return rc;


  // Verify parameters
  if (FP_OFF(pEABuf)) {
    rc = FSH_PROBEBUF(PB_OPREAD, pEABuf, sizeof(EAOP));
    if (rc != NO_ERROR) {
      FSH_SEMCLEAR(&CPData.BufLock);
      return rc;
    }

    // Check that at least the first ULONG is accessible
    rc = FSH_PROBEBUF(PB_OPREAD, ((EAOP *)pEABuf)->fpFEAList, sizeof(ULONG));
    if (rc != NO_ERROR) {
      FSH_SEMCLEAR(&CPData.BufLock);
      return rc;
    }

    // Check that it is <=64K
    if (((EAOP *)pEABuf)->fpFEAList->cbList > 0xffff) {
      FSH_SEMCLEAR(&CPData.BufLock);
      return ERROR_INVALID_PARAMETER;
    }

    // Check that we can access the whole list
    rc = FSH_PROBEBUF(PB_OPREAD, ((EAOP *)pEABuf)->fpFEAList,
                      ((EAOP *)pEABuf)->fpFEAList->cbList);
    if (rc != NO_ERROR) {
      FSH_SEMCLEAR(&CPData.BufLock);
      return rc;
    }
  }


  // Make sure the control program is still attached (this is how we
  // know our buffers are still valid)
  if (!CPAttached) {
     FSH_SEMCLEAR(&CPData.BufLock);
     return (ERROR_NOT_READY);
  }


  // Setup parameters -- from here to the PostSetup(), we MUST NOT block
  CPData.OpData->i.funccode = CPFC_MKDIR;
  FSH_GETVOLPARM(pcdfsi->cdi_hVPB, &pvpfsi, &pvpfsd);
  CPData.OpData->p.mkdir.vpfsd = *pvpfsd;
  CPData.OpData->p.mkdir.cdfsi = *pcdfsi;
  CPData.OpData->p.mkdir.cdfsd = *pcdfsd;
  strncpy(CPData.OpData->p.mkdir.Name, pName, sizeof(CPData.OpData->p.mkdir.Name));
  CPData.OpData->p.mkdir.iCurDirEnd = iCurDirEnd;
  if (FP_OFF(pEABuf))
    memcpy(CPData.Buf, ((EAOP *)pEABuf)->fpFEAList,
           ((EAOP *)pEABuf)->fpFEAList->cbList);
  CPData.OpData->p.mkdir.flags = flags;

  rc = PostSetup();
  if (rc != NO_ERROR)
    return rc;


  // Copy results
  *pcdfsd = CPData.OpData->p.mkdir.cdfsd;
  *pvpfsd = CPData.OpData->p.chdir.vpfsd;
  if (FP_OFF(pEABuf))
    ((EAOP *)pEABuf)->oError = CPData.OpData->p.mkdir.oError;

  rc = CPData.OpData->i.rc;

  // We CANNOT access the buffers after this line
  FSH_SEMCLEAR(&CPData.BufLock);

  return rc;
}


/******************************************************************************
**
** FS_RMDIR - Delete directory
**
** Parameters
** ----------
** struct cdfsi far *pcdfsi             pointer to FSD independant current directory
** struct cdfsd far *pcdfsd             pointer to FSD dependant current directory
** char far *pName                      pointer to directory name to delete
** unsigned short iCurDirEnd            offset to the end of the current directory in pName
**
******************************************************************************/

#pragma argsused
short int far pascal _export _loadds FS_RMDIR(struct cdfsi far *pcdfsi, struct cdfsd far *pcdfsd,
                              char far *pName, unsigned short iCurDirEnd)
{
  unsigned short rc;
  struct vpfsi *pvpfsi;
  struct vpfsd *pvpfsd;

  rc = PreSetup();
  if (rc != NO_ERROR)
    return rc;

  // Verify parameters

  // Make sure the control program is still attached (this is how we
  // know our buffers are still valid)
  if (!CPAttached) {
    FSH_SEMCLEAR(&CPData.BufLock);
    return (ERROR_NOT_READY);
  }


  // Setup parameters -- from here to the PostSetup(), we MUST NOT block
  CPData.OpData->i.funccode = CPFC_RMDIR;
  FSH_GETVOLPARM(pcdfsi->cdi_hVPB, &pvpfsi, &pvpfsd);
  CPData.OpData->p.rmdir.vpfsd = *pvpfsd;
  CPData.OpData->p.rmdir.cdfsi = *pcdfsi;
  CPData.OpData->p.rmdir.cdfsd = *pcdfsd;
  strncpy(CPData.OpData->p.rmdir.Name, pName, sizeof(CPData.OpData->p.rmdir.Name));
  CPData.OpData->p.rmdir.iCurDirEnd = iCurDirEnd;

  rc = PostSetup();
  if (rc != NO_ERROR)
    return rc;


  // Copy results
  *pcdfsd = CPData.OpData->p.rmdir.cdfsd;
  *pvpfsd = CPData.OpData->p.chdir.vpfsd;

  rc = CPData.OpData->i.rc;

  // We CANNOT access the buffers after this line
  FSH_SEMCLEAR(&CPData.BufLock);

  return rc;
}



/*-----------------------------------------------------------------------------
--
-- File management
--
-----------------------------------------------------------------------------*/



/******************************************************************************
**
** FS_CHGFILEPTR - Change current location in file
**
** Parameters
** ----------
** struct sffsi far *psffsi             pointer to FSD independant file instance
** struct sffsd far *psffsd             pointer to FSD dependant file instance
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
short int far pascal _export _loadds FS_CHGFILEPTR(struct sffsi far *psffsi, struct sffsd far *psffsd,
                                   long offset, unsigned short type, unsigned short IOflag)
{
  unsigned short rc;
  struct vpfsi *pvpfsi;
  struct vpfsd *pvpfsd;

  rc = PreSetup();
  if (rc != NO_ERROR)
    return rc;


  // Verify parameters

  // Make sure the control program is still attached (this is how we
  // know our buffers are still valid)
  if (!CPAttached) {
    FSH_SEMCLEAR(&CPData.BufLock);
    return (ERROR_NOT_READY);
  }


  // Setup parameters -- from here to the PostSetup(), we MUST NOT block
  CPData.OpData->i.funccode = CPFC_CHGFILEPTR;
  FSH_GETVOLPARM(psffsi->sfi_hVPB, &pvpfsi, &pvpfsd);
  CPData.OpData->p.chgfileptr.vpfsd = *pvpfsd;
  CPData.OpData->p.chgfileptr.sffsi = *psffsi;
  CPData.OpData->p.chgfileptr.sffsd = *psffsd;
  CPData.OpData->p.chgfileptr.offset = offset;
  CPData.OpData->p.chgfileptr.type = type;
  CPData.OpData->p.chgfileptr.IOflag = IOflag;

  rc = PostSetup();
  if (rc != NO_ERROR)
    return rc;


  /* Copy results */
  *pvpfsd = CPData.OpData->p.chgfileptr.vpfsd;    
  *psffsi = CPData.OpData->p.chgfileptr.sffsi;
  *psffsd = CPData.OpData->p.chgfileptr.sffsd;

  rc = CPData.OpData->i.rc;

  // We CANNOT access the buffers after this line
  FSH_SEMCLEAR(&CPData.BufLock);

  return rc;
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
** struct sffsi far *psffsi             pointer to FSD independant file instance
** struct sffsd far *psffsd             pointer to FSD dependant file instance
**
******************************************************************************/

#pragma argsused
short int far pascal _export _loadds FS_CLOSE(unsigned short type, unsigned short IOflag,
                              struct sffsi far *psffsi, struct sffsd far *psffsd)
{
  unsigned short rc;
  struct vpfsi *pvpfsi;
  struct vpfsd *pvpfsd;

  rc = PreSetup();
  if (rc != NO_ERROR)
    return rc;


  // Verify parameters

  // Make sure the control program is still attached (this is how we
  // know our buffers are still valid)
  if (!CPAttached) {
    FSH_SEMCLEAR(&CPData.BufLock);
    return (ERROR_NOT_READY);
  }


  // Setup parameters -- from here to the PostSetup(), we MUST NOT block
  CPData.OpData->i.funccode = CPFC_CLOSE;
  FSH_GETVOLPARM(psffsi->sfi_hVPB, &pvpfsi, &pvpfsd);
  CPData.OpData->p.close.vpfsd = *pvpfsd;
  CPData.OpData->p.close.type = type;
  CPData.OpData->p.close.IOflag = IOflag;
  CPData.OpData->p.close.sffsi = *psffsi;
  CPData.OpData->p.close.sffsd = *psffsd;

  rc = PostSetup();
  if (rc != NO_ERROR)
    return rc;


  /* Copy results */
  *pvpfsd = CPData.OpData->p.close.vpfsd;
  *psffsi = CPData.OpData->p.close.sffsi;
  *psffsd = CPData.OpData->p.close.sffsd;

  rc = CPData.OpData->i.rc;

  /* We CANNOT access the buffers after this line */
  FSH_SEMCLEAR(&CPData.BufLock);

  return rc;
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
** struct sffsi far *psffsi             pointer to FSD independant file instance
** struct sffsd far *psffsd             pointer to FSD dependant file instance
**
******************************************************************************/

#pragma argsused
short int far pascal _export _loadds FS_COMMIT(unsigned short type, unsigned short IOflag,
                               struct sffsi far *psffsi, struct sffsd far *psffsd)
{
  unsigned short rc;
  struct vpfsi *pvpfsi;
  struct vpfsd *pvpfsd;

  rc = PreSetup();
  if (rc != NO_ERROR)
    return rc;


  // Verify parameters

  // Make sure the control program is still attached (this is how we
  // know our buffers are still valid)
  if (!CPAttached) {
    FSH_SEMCLEAR(&CPData.BufLock);
    return (ERROR_NOT_READY);
  }


  // Setup parameters -- from here to the PostSetup(), we MUST NOT block
  CPData.OpData->i.funccode = CPFC_COMMIT;
  FSH_GETVOLPARM(psffsi->sfi_hVPB, &pvpfsi, &pvpfsd);
  CPData.OpData->p.commit.vpfsd = *pvpfsd;
  CPData.OpData->p.commit.type = type;
  CPData.OpData->p.commit.IOflag = IOflag;
  CPData.OpData->p.commit.sffsi = *psffsi;
  CPData.OpData->p.commit.sffsd = *psffsd;

  rc = PostSetup();
  if (rc != NO_ERROR)
    return rc;

  /* Copy results */
  *pvpfsd = CPData.OpData->p.commit.vpfsd;
  *psffsi = CPData.OpData->p.commit.sffsi;
  *psffsd = CPData.OpData->p.commit.sffsd;

  rc = CPData.OpData->i.rc;

  // We CANNOT access the buffers after this line
  FSH_SEMCLEAR(&CPData.BufLock);

  return rc;
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
** struct cdfsi far *pcdfsi             pointer to FSD independant current directory
** struct cdfsd far *pcdfsd             pointer to FSD dependant current directory
** char far *pSrc                       pointer to source filename
** unsigned short iSrcCurDirEnd         offset to the end of the current directory in pSrc
** char far *pDst                       pointer to destination filename
** unsigned short iDstCurDirEnd         offset to the end of the current directory in pDst
** unsigned short nameType              0x40 = destination is non 8.3 filename
**
******************************************************************************/

#pragma argsused
short int far pascal _export _loadds FS_COPY(unsigned short flag, struct cdfsi far *pcdfsi,
                             struct cdfsd far *pcdfsd, char far *pSrc,
                             unsigned short iSrcCurDirEnd, char far *pDst,
                             unsigned short iDstCurDirEnd, unsigned short nameType)
{
  unsigned short rc;
  struct vpfsi *pvpfsi;
  struct vpfsd *pvpfsd;

  rc = PreSetup();
  if (rc != NO_ERROR)
    return rc;


  // Verify parameters

  // Make sure the control program is still attached (this is how we
  // know our buffers are still valid)
  if (!CPAttached) {
    FSH_SEMCLEAR(&CPData.BufLock);
    return (ERROR_NOT_READY);
  }


  // Setup parameters -- from here to the PostSetup(), we MUST NOT block
  CPData.OpData->i.funccode = CPFC_COPY;
  FSH_GETVOLPARM(pcdfsi->cdi_hVPB, &pvpfsi, &pvpfsd);
  CPData.OpData->p.copy.vpfsd = *pvpfsd;
  CPData.OpData->p.copy.flag = flag;
  CPData.OpData->p.copy.cdfsi = *pcdfsi;
  CPData.OpData->p.copy.cdfsd = *pcdfsd;
  strncpy(CPData.OpData->p.copy.Src, pSrc, sizeof(CPData.OpData->p.copy.Src));
  CPData.OpData->p.copy.iSrcCurDirEnd = iSrcCurDirEnd;
  strncpy(CPData.OpData->p.copy.Dst, pDst, sizeof(CPData.OpData->p.copy.Dst));
  CPData.OpData->p.copy.iDstCurDirEnd = iDstCurDirEnd;
  CPData.OpData->p.copy.nameType = nameType;

  rc = PostSetup();
  if (rc != NO_ERROR)
    return rc;


  // Copy results
  *pvpfsd = CPData.OpData->p.copy.vpfsd;
  *pcdfsd = CPData.OpData->p.copy.cdfsd;

  rc = CPData.OpData->i.rc;

  // We CANNOT access the buffers after this line
  FSH_SEMCLEAR(&CPData.BufLock);

  return rc;
}


/******************************************************************************
**
** FS_DELETE - Delete a file
**
** Parameters
** ----------
** struct cdfsi far *pcdfsi             pointer to FSD independant current directory
** struct cdfsd far *pcdfsd             pointer to FSD dependant current directory
** char far *pFile                      pointer to filename to delete
** unsigned short iCurDirEnd            offset to the end of the current directory in pFile
**
******************************************************************************/

#pragma argsused
short int far pascal _export _loadds FS_DELETE(struct cdfsi far *pcdfsi, struct cdfsd far *pcdfsd,
                               char far *pFile, unsigned short iCurDirEnd)
{
  unsigned short rc;
  struct vpfsi *pvpfsi;
  struct vpfsd *pvpfsd;

  rc = PreSetup();
  if (rc != NO_ERROR)
    return rc;


  // Verify parameters

  // Make sure the control program is still attached (this is how we
  // know our buffers are still valid)
  if (!CPAttached) {
    FSH_SEMCLEAR(&CPData.BufLock);
    return (ERROR_NOT_READY);
  }


  // Setup parameters -- from here to the PostSetup(), we MUST NOT block
  CPData.OpData->i.funccode = CPFC_DELETE;
  FSH_GETVOLPARM(pcdfsi->cdi_hVPB, &pvpfsi, &pvpfsd);
  CPData.OpData->p.fsdelete.vpfsd = *pvpfsd;
  CPData.OpData->p.fsdelete.cdfsi = *pcdfsi;
  CPData.OpData->p.fsdelete.cdfsd = *pcdfsd;
  strncpy(CPData.OpData->p.fsdelete.File, pFile, sizeof(CPData.OpData->p.fsdelete.File));
  CPData.OpData->p.fsdelete.iCurDirEnd = iCurDirEnd;

  rc = PostSetup();
  if (rc != NO_ERROR)
    return rc;


  /* Copy results */
  *pvpfsd = CPData.OpData->p.fsdelete.vpfsd;
  *pcdfsd = CPData.OpData->p.fsdelete.cdfsd;

  rc = CPData.OpData->i.rc;

  // We CANNOT access the buffers after this line
  FSH_SEMCLEAR(&CPData.BufLock);

  return rc;
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
** struct cdfsi far *pcdfsi             pointer to FSD independant current directory
** struct cdfsd far *pcdfsd             pointer to FSD dependant current directory
** char far *pName                      pointer to filename
** unsigned short iCurDirEnd            offset to the end of the current directory in pName
** unsigned short far *pAttr            pointer to the attribute
**
******************************************************************************/

#pragma argsused
short int far pascal _export _loadds FS_FILEATTRIBUTE(unsigned short flag, struct cdfsi far *pcdfsi,
                                      struct cdfsd far *pcdfsd, char far *pName,
                                      unsigned short iCurDirEnd, unsigned short far *pAttr)
{
  unsigned short rc;
  struct vpfsi *pvpfsi;
  struct vpfsd *pvpfsd;

  rc = PreSetup();
  if (rc != NO_ERROR)
     return rc;


  // Verify parameters

  // Make sure the control program is still attached (this is how we
  // know our buffers are still valid)
  if (!CPAttached) {
    FSH_SEMCLEAR(&CPData.BufLock);
    return (ERROR_NOT_READY);
  }


  // Setup parameters -- from here to the PostSetup(), we MUST NOT block
  CPData.OpData->i.funccode = CPFC_FILEATTRIBUTE;
  FSH_GETVOLPARM(pcdfsi->cdi_hVPB, &pvpfsi, &pvpfsd);
  CPData.OpData->p.fileattribute.vpfsd = *pvpfsd;
  CPData.OpData->p.fileattribute.flag = flag;
  CPData.OpData->p.fileattribute.cdfsi = *pcdfsi;
  CPData.OpData->p.fileattribute.cdfsd = *pcdfsd;
  strncpy(CPData.OpData->p.fileattribute.Name, pName, sizeof(CPData.OpData->p.fileattribute.Name));
  CPData.OpData->p.fileattribute.iCurDirEnd = iCurDirEnd;
  CPData.OpData->p.fileattribute.Attr = *pAttr;

  rc = PostSetup();
  if (rc != NO_ERROR)
    return rc;


  // Copy results
  if (flag == FA_RETRIEVE)
    *pAttr = CPData.OpData->p.fileattribute.Attr;
  *pcdfsd = CPData.OpData->p.fileattribute.cdfsd;
  *pvpfsd = CPData.OpData->p.fileattribute.vpfsd;

  rc = CPData.OpData->i.rc;

  // We CANNOT access the buffers after this line
  FSH_SEMCLEAR(&CPData.BufLock);

  return rc;
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
** struct sffsi far *psffsi             pointer to FSD independant file instance
** struct sffsd far *psffsd             pointer to FSD dependant file instance
** unsigned short level                 level of information to get/set
** char far *pData                      UNVERIFIED? pointer to information area
** unsigned short cbData                size of area pointed to by pData
** unsigned short IOflag                bitfield of I/O suggestions
**   values:
**     IOFL_WRITETHRU   write all updated data before returning
**     IOFL_NOCACHE     don't cache any new data
**
******************************************************************************/

#pragma argsused
short int far pascal _export _loadds FS_FILEINFO(unsigned short flag, struct sffsi far *psffsi,
                                 struct sffsd far *psffsd, unsigned short level,
                                 char far *pData, unsigned short cbData,
                                 unsigned short IOflag)
{
  unsigned short rc;
  struct vpfsi *pvpfsi;
  struct vpfsd *pvpfsd;

  rc = PreSetup();
  if (rc != NO_ERROR)
    return rc;

  /* Verify parameters */
  if (flag == FI_SET && cbData != 0)
    rc = FSH_PROBEBUF(PB_OPREAD, pData, cbData);
  else if (cbData != 0)
    rc = FSH_PROBEBUF(PB_OPWRITE, pData, cbData);
  if (rc != NO_ERROR) {
     FSH_SEMCLEAR(&CPData.BufLock);
     return rc;
  }

  /* Special EA check */
  if (flag == FI_RETRIEVE && (level == 3 || level == 4)) {
    /* Check to make sure the buffer's at least big enough for an EAOP */
    if (cbData < sizeof(EAOP)) {
      FSH_SEMCLEAR(&CPData.BufLock);
      return ERROR_INVALID_PARAMETER;
    }

    if(level==3) {
      /* Check accessability of GEA */

      /* Check that at least the first ULONG is accessible */
      rc = FSH_PROBEBUF(PB_OPREAD, ((EAOP *)pData)->fpGEAList, sizeof(ULONG));
      if (rc != NO_ERROR) {
        FSH_SEMCLEAR(&CPData.BufLock);
        return rc;
      }

      /* Check that it is <=64K */
      if (((EAOP *)pData)->fpGEAList->cbList > 0xffff) {
        FSH_SEMCLEAR(&CPData.BufLock);
        return ERROR_INVALID_PARAMETER;
      }

      if (((EAOP *)pData)->fpGEAList->cbList == 0) {
	FSH_SEMCLEAR(&CPData.BufLock);
	return ERROR_BAD_FORMAT;
      }

      /* Check that we can access the whole list */
      rc = FSH_PROBEBUF(PB_OPREAD, ((EAOP *)pData)->fpGEAList,
                        ((EAOP *)pData)->fpGEAList->cbList);
      if (rc != NO_ERROR) {
        FSH_SEMCLEAR(&CPData.BufLock);
        return rc;
      }
    }
  
    /* Same thing for FEA */

    /* Check that at least the first ULONG is accessible */
    rc = FSH_PROBEBUF(PB_OPWRITE, ((EAOP *)pData)->fpFEAList, sizeof(ULONG));
    if (rc != NO_ERROR) {
      FSH_SEMCLEAR(&CPData.BufLock);
      return rc;
    }

    /* Check that it is <=64K */
    if (((EAOP *)pData)->fpFEAList->cbList > 0xffff) {
      FSH_SEMCLEAR(&CPData.BufLock);
      return ERROR_INVALID_PARAMETER;
    }

    /* Check that we can access the whole list */
    rc = FSH_PROBEBUF(PB_OPWRITE, ((EAOP *)pData)->fpFEAList,
                      ((EAOP *)pData)->fpFEAList->cbList);
    if (rc != NO_ERROR) {
      FSH_SEMCLEAR(&CPData.BufLock);
      return rc;
    }
  }

  if (flag == FI_SET && level == 2) {
    /* Check that at least the first ULONG is accessible */
    rc = FSH_PROBEBUF(PB_OPREAD, ((EAOP *)pData)->fpFEAList, sizeof(ULONG));
    if (rc != NO_ERROR)
      return rc;

    /* Check that it is <=64K */
    if (((EAOP *)pData)->fpFEAList->cbList > 0xffff)
      return ERROR_INVALID_PARAMETER;

    /* Check that we can access the whole list */
    rc = FSH_PROBEBUF(PB_OPREAD, ((EAOP *)pData)->fpFEAList,
                      ((EAOP *)pData)->fpFEAList->cbList);
    if (rc != NO_ERROR)
      return rc;
  }

  /* Make sure the control program is still attached (this is how we
     know our buffers are still valid) */
  if (!CPAttached) {
    FSH_SEMCLEAR(&CPData.BufLock);
    return (ERROR_NOT_READY);
  }

  /* Setup parameters -- from here to the PostSetup(), we MUST NOT block */
  CPData.OpData->i.funccode = CPFC_FILEINFO;
  FSH_GETVOLPARM(psffsi->sfi_hVPB, &pvpfsi, &pvpfsd);
  CPData.OpData->p.fileinfo.vpfsd = *pvpfsd;
  CPData.OpData->p.fileinfo.flag = flag;
  CPData.OpData->p.fileinfo.sffsi = *psffsi;
  CPData.OpData->p.fileinfo.sffsd = *psffsd;
  CPData.OpData->p.fileinfo.level = level;
  /* Copy either the buffer, FEA or GEA list as appropriate */
  if (flag == FI_RETRIEVE && level == 3) {
    memcpy(CPData.Buf, ((EAOP *)pData)->fpGEAList,
                       ((EAOP *)pData)->fpGEAList->cbList);
    /* cbData is FEA list size, as FEA list will be returned in buffer */
    CPData.OpData->p.fileinfo.cbData = ((EAOP *)pData)->fpFEAList->cbList;
  } 
  else if(level==4)
    CPData.OpData->p.fileinfo.cbData = ((EAOP *)pData)->fpFEAList->cbList;
  else if (flag == FI_SET && level == 2) {
    memcpy(CPData.Buf, ((EAOP *)pData)->fpFEAList,
	   ((EAOP *)pData)->fpFEAList->cbList);
    CPData.OpData->p.fileinfo.cbData = ((EAOP *)pData)->fpFEAList->cbList;
  } 
  else {
    memcpy(CPData.Buf, pData, cbData);
    CPData.OpData->p.fileinfo.cbData = cbData;
  }

  CPData.OpData->p.fileinfo.IOflag = IOflag;
  
  rc = PostSetup();
  if (rc != NO_ERROR)
    return rc;

  /* Copy results */
  if (flag == FI_RETRIEVE)
    if (level==3 || level==4)
      memcpy(((EAOP *)pData)->fpFEAList, CPData.Buf,
	     ((FEALIST *)CPData.Buf)->cbList);
    else
      memcpy(pData, CPData.Buf, cbData);

  if (flag == FI_SET && level == 2)
    ((EAOP *)pData)->oError = CPData.OpData->p.fileinfo.oError;

  *pvpfsd = CPData.OpData->p.fileinfo.vpfsd;
  *psffsi = CPData.OpData->p.fileinfo.sffsi;
  *psffsd = CPData.OpData->p.fileinfo.sffsd;

  rc = CPData.OpData->i.rc;

  /* We CANNOT access the buffers after this line */
  FSH_SEMCLEAR(&CPData.BufLock);

  return rc;
}


/******************************************************************************
**
** FS_FILEIO - Atomic I/O operations
**
** Parameters
** ----------
** struct sffsi far *psffsi             pointer to FSD independant file instance
** struct sffsd far *psffsd             pointer to FSD dependant file instance
** char far *pCmdList                   UNVERIFIED pointer to information area
** unsigned short cbCmdList             size of area pointed to by pCmdList
** unsigned short far *poError          UNVERIFIED pointer to offset within
**                                      pCmdList of command that caused an error
** unsigned short IOflag                bitfield of I/O suggestions
**   values:
**     IOFL_WRITETHRU   write all updated data before returning
**     IOFL_NOCACHE     don't cache any new data
**
******************************************************************************/

#pragma argsused
short int far pascal _export _loadds FS_FILEIO(struct sffsi far *psffsi, struct sffsd far *psffsd,
                               char far *pCmdList, unsigned short cbCmdList,
                               unsigned short far *poError, unsigned short IOflag)
{
  return ERROR_NOT_SUPPORTED;
}

/******************************************************************************
**
** FS_MOVE - Move/rename a file
**
** Parameters
** ----------
** struct cdfsi far *pcdfsi             pointer to FSD independant current directory
** struct cdfsd far *pcdfsd             pointer to FSD dependant current directory
** char far *pSrc                       pointer to source filename
** unsigned short iSrcCurDirEnd         offset to the end of the current directory in pSrc
** char far *pDst                       pointer to destination filename
** unsigned short iDstCurDirEnd         offset to the end of the current directory in pDst
** unsigned short flags                 0x40 = destination is non 8.3 filename
**
******************************************************************************/

#pragma argsused
short int far pascal _export _loadds FS_MOVE(struct cdfsi far *pcdfsi, struct cdfsd far *pcdfsd,
                             char far *pSrc, unsigned short iSrcCurDirEnd,
                             char far *pDst, unsigned short iDstCurDirEnd,
                             unsigned short flags)
{
  unsigned short rc;
  struct vpfsi *pvpfsi;
  struct vpfsd *pvpfsd;

  rc = PreSetup();
  if (rc != NO_ERROR)
    return rc;


  // Verify parameters

  // Make sure the control program is still attached (this is how we
  // know our buffers are still valid)
  if (!CPAttached) {
    FSH_SEMCLEAR(&CPData.BufLock);
    return (ERROR_NOT_READY);
  }


  // Setup parameters -- from here to the PostSetup(), we MUST NOT block
  CPData.OpData->i.funccode = CPFC_MOVE;
  FSH_GETVOLPARM(pcdfsi->cdi_hVPB, &pvpfsi, &pvpfsd);
  CPData.OpData->p.move.vpfsd = *pvpfsd;
  CPData.OpData->p.move.cdfsi = *pcdfsi;
  CPData.OpData->p.move.cdfsd = *pcdfsd;
  strncpy(CPData.OpData->p.move.Src, pSrc, sizeof(CPData.OpData->p.move.Src));
  CPData.OpData->p.move.iSrcCurDirEnd = iSrcCurDirEnd;
  strncpy(CPData.OpData->p.move.Dst, pDst, sizeof(CPData.OpData->p.move.Dst));
  CPData.OpData->p.move.iDstCurDirEnd = iDstCurDirEnd;
  CPData.OpData->p.move.flags = flags;

  rc = PostSetup();
  if (rc != NO_ERROR)
    return rc;


  // Copy results
  *pvpfsd = CPData.OpData->p.move.vpfsd;
  *pcdfsd = CPData.OpData->p.move.cdfsd;

  rc = CPData.OpData->i.rc;

  // We CANNOT access the buffers after this line
  FSH_SEMCLEAR(&CPData.BufLock);

  return rc;
}


/******************************************************************************
**
** FS_NEWSIZE - Change size of file
**
** Parameters
** ----------
** struct sffsi far *psffsi             pointer to FSD independant file instance
** struct sffsd far *psffsd             pointer to FSD dependant file instance
** unsigned long len                    new length of file
** unsigned short IOflag                bitfield of I/O suggestions
**   values:
**     IOFL_WRITETHRU   write all updated data before returning
**     IOFL_NOCACHE     don't cache any new data
**
******************************************************************************/

#pragma argsused
short int far pascal _export _loadds FS_NEWSIZE(struct sffsi far *psffsi, struct sffsd far *psffsd,
                                unsigned long len, unsigned short IOflag)
{
  unsigned short rc;
  struct vpfsi *pvpfsi;
  struct vpfsd *pvpfsd;

  rc = PreSetup();
  if (rc != NO_ERROR)
    return rc;


  // Verify parameters

  // Make sure the control program is still attached (this is how we
  // know our buffers are still valid)
  if (!CPAttached) {
    FSH_SEMCLEAR(&CPData.BufLock);
    return (ERROR_NOT_READY);
  }


  // Setup parameters -- from here to the PostSetup(), we MUST NOT block
  CPData.OpData->i.funccode = CPFC_NEWSIZE;
  FSH_GETVOLPARM(psffsi->sfi_hVPB, &pvpfsi, &pvpfsd);
  CPData.OpData->p.newsize.vpfsd = *pvpfsd;
  CPData.OpData->p.newsize.sffsi = *psffsi;
  CPData.OpData->p.newsize.sffsd = *psffsd;
  CPData.OpData->p.newsize.len = len;
  CPData.OpData->p.newsize.IOflag = IOflag;

  rc = PostSetup();
  if (rc != NO_ERROR)
    return rc;

  // Copy results
  *pvpfsd = CPData.OpData->p.newsize.vpfsd;
  *psffsi = CPData.OpData->p.newsize.sffsi;
  *psffsd = CPData.OpData->p.newsize.sffsd;

  rc = CPData.OpData->i.rc;

  // We CANNOT access the buffers after this line
  FSH_SEMCLEAR(&CPData.BufLock);

  return rc;
}


/******************************************************************************
**
** FS_OPENCREATE - Open or create a new file
**
** Parameters
** ----------
** struct cdfsi far *pcdfsi             pointer to FSD independant current directory
** struct cdfsd far *pcdfsd             pointer to FSD dependant current directory
** char far *pName                      pointer to filename
** unsigned short iCurDirEnd            offset to the end of the current directory in pName
** struct sffsi far *psffsi             pointer to FSD independant file instance
** struct sffsd far *psffsd             pointer to FSD dependant file instance
** unsigned long ulOpenMode             sharing and access mode
** unsigned short usOpenFlag            action to take when file exists/doesn't exist
** unsigned short far *pusAction        returns the action that the IFS took
** unsigned short usAttr                OS/2 file attributes
** char far *pEABuf                     UNVERIFIED pointer to EAs to attach to new file
** unsigned short far *pfgenflag        flags returned by the IFS
**   values:
**     FOC_NEEDEAS      indicates there are critical EAs associated with the file
**
******************************************************************************/

#pragma argsused
short int far pascal _export _loadds 
FS_OPENCREATE(struct cdfsi far *pcdfsi, 
	      struct cdfsd far *pcdfsd,
	      char far *pName, unsigned short iCurDirEnd,
	      struct sffsi far *psffsi, struct sffsd far *psffsd,
	      unsigned long ulOpenMode, unsigned short usOpenFlag,
	      unsigned short far *pusAction, unsigned short usAttr,
	      char far *pEABuf, unsigned short far *pfgenflag)
{
  unsigned short rc;
  struct vpfsi *pvpfsi;
  struct vpfsd *pvpfsd;

  rc = PreSetup();
  if (rc != NO_ERROR)
    return rc;

  // Verify parameters
  /* Seems that pEABuf can be something like 0003:0000 although NULL pointer
     was passed to DosOpen. If offset is zero, we treat it as NULL. */
  if (FP_OFF(pEABuf)) {
    rc = FSH_PROBEBUF(PB_OPREAD, pEABuf, sizeof(EAOP));
    if (rc != NO_ERROR) {
      FSH_SEMCLEAR(&CPData.BufLock);
      return rc;
    }
    else {
      // Check that at least the first ULONG is accessible
      rc = FSH_PROBEBUF(PB_OPREAD, ((EAOP *)pEABuf)->fpFEAList, sizeof(ULONG));
      if (rc != NO_ERROR) {
        FSH_SEMCLEAR(&CPData.BufLock);
        return rc;
      }

      // Check that it is <=64K
      if (((EAOP *)pEABuf)->fpFEAList->cbList > 0xffff) {
        FSH_SEMCLEAR(&CPData.BufLock);
        return ERROR_INVALID_PARAMETER;
      }

      // Check that we can access the whole list
      rc = FSH_PROBEBUF(PB_OPREAD, ((EAOP *)pEABuf)->fpFEAList,
                        ((EAOP *)pEABuf)->fpFEAList->cbList);
      if (rc != NO_ERROR) {
        FSH_SEMCLEAR(&CPData.BufLock);
        return rc;
      }
    }
  }

  // Make sure the control program is still attached (this is how we
  // know our buffers are still valid)
  if (!CPAttached) {
    FSH_SEMCLEAR(&CPData.BufLock);
    return (ERROR_NOT_READY);
  }

  // Setup parameters -- from here to the PostSetup(), we MUST NOT block
  CPData.OpData->i.funccode = CPFC_OPENCREATE;
  FSH_GETVOLPARM(psffsi->sfi_hVPB, &pvpfsi, &pvpfsd);
  CPData.OpData->p.opencreate.vpfsd = *pvpfsd;
  CPData.OpData->p.opencreate.cdfsi = *pcdfsi;
  CPData.OpData->p.opencreate.cdfsd = *pcdfsd;
  strncpy(CPData.OpData->p.opencreate.Name, pName, sizeof(CPData.OpData->p.opencreate.Name));
  CPData.OpData->p.opencreate.iCurDirEnd = iCurDirEnd;
  CPData.OpData->p.opencreate.sffsi = *psffsi;
  CPData.OpData->p.opencreate.sffsd = *psffsd;
  CPData.OpData->p.opencreate.ulOpenMode = ulOpenMode;
  CPData.OpData->p.opencreate.usOpenFlag = usOpenFlag;
  CPData.OpData->p.opencreate.usAttr = usAttr;
  if (FP_OFF(pEABuf))
    memcpy(CPData.Buf, ((EAOP *)pEABuf)->fpFEAList,
           ((EAOP *)pEABuf)->fpFEAList->cbList);

  rc = PostSetup();
  if (rc != NO_ERROR)
    return rc;

  // Copy results
  *pvpfsd = CPData.OpData->p.opencreate.vpfsd;
  *pcdfsd = CPData.OpData->p.opencreate.cdfsd;
  *psffsi = CPData.OpData->p.opencreate.sffsi;
  *psffsd = CPData.OpData->p.opencreate.sffsd;
  *pusAction = CPData.OpData->p.opencreate.usAction;
  *pfgenflag = CPData.OpData->p.opencreate.fgenflag;
  if (FP_OFF(pEABuf))
    ((EAOP *)pEABuf)->oError = CPData.OpData->p.opencreate.oError;
  rc = CPData.OpData->i.rc;

  // We CANNOT access the buffers after this line
  FSH_SEMCLEAR(&CPData.BufLock);

  return rc;
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
** struct cdfsi far *pcdfsi             pointer to FSD independant current directory
** struct cdfsd far *pcdfsd             pointer to FSD dependant current directory
** char far *pName                      pointer to filename
** unsigned short iCurDirEnd            offset to the end of the current directory in pName
** unsigned short level                 level of information to get/set
** char far *pData                      UNVERIFIED pointer to information area
** unsigned short cbData                size of area pointed to by pData
**
******************************************************************************/

#pragma argsused
short int far pascal _export _loadds FS_PATHINFO(unsigned short flag, struct cdfsi far *pcdfsi,
                                 struct cdfsd far *pcdfsd, char far *pName,
                                 unsigned short iCurDirEnd, unsigned short level,
                                 char far *pData, unsigned short cbData)
{
  unsigned short rc;
  struct vpfsi *pvpfsi;
  struct vpfsd *pvpfsd;

  rc = PreSetup();
  if (rc != NO_ERROR)
    return rc;

  flag &= 0x01;    /* Ignore write-through flag */

  // Verify parameters
  if (flag == PI_RETRIEVE && cbData != 0)
    rc = FSH_PROBEBUF(PB_OPWRITE, pData, cbData);
  else if (cbData != 0)
    rc = FSH_PROBEBUF(PB_OPREAD, pData, cbData);
  if (rc != NO_ERROR) {
    FSH_SEMCLEAR(&CPData.BufLock);
    return rc;
  }

  // Special EA check
  if (flag == PI_RETRIEVE && (level == 3 || level == 4)) {
    // Check to make sure the buffer's at least big enough for an EAOP
    if (cbData < sizeof(EAOP)) {
      FSH_SEMCLEAR(&CPData.BufLock);
      return ERROR_INVALID_PARAMETER;
    }

    if(level==3) {
      // Check accessability of GEA
      
      // Check that at least the first ULONG is accessible
      rc = FSH_PROBEBUF(PB_OPREAD, ((EAOP *)pData)->fpGEAList, sizeof(ULONG));
      if (rc != NO_ERROR) {
	FSH_SEMCLEAR(&CPData.BufLock);
	return rc;
      }
      
      // Check that it is <=64K
      if (((EAOP *)pData)->fpGEAList->cbList > 0xffff) {
	FSH_SEMCLEAR(&CPData.BufLock);
	return ERROR_INVALID_PARAMETER;
      }

      if (((EAOP *)pData)->fpGEAList->cbList == 0) {
	FSH_SEMCLEAR(&CPData.BufLock);
	return ERROR_BAD_FORMAT;
      }

      // Check that we can access the whole list
      rc = FSH_PROBEBUF(PB_OPREAD, ((EAOP *)pData)->fpGEAList,
			((EAOP *)pData)->fpGEAList->cbList);
      if (rc != NO_ERROR) {
	FSH_SEMCLEAR(&CPData.BufLock);
	return rc;
      }
    }

    // Same thing for FEA

    // Check that at least the first ULONG is accessible
    rc = FSH_PROBEBUF(PB_OPWRITE, ((EAOP *)pData)->fpFEAList, sizeof(ULONG));
    if (rc != NO_ERROR) {
      FSH_SEMCLEAR(&CPData.BufLock);
      return rc;
    }

    // Check that it is <=64K
    if (((EAOP *)pData)->fpFEAList->cbList > 0xffff) {
      FSH_SEMCLEAR(&CPData.BufLock);
      return ERROR_INVALID_PARAMETER;
    }

    // Check that we can access the whole list
    rc = FSH_PROBEBUF(PB_OPWRITE, ((EAOP *)pData)->fpFEAList,
                      ((EAOP *)pData)->fpFEAList->cbList);
    if (rc != NO_ERROR) {
      FSH_SEMCLEAR(&CPData.BufLock);
      return rc;
    }
  }

  if (flag == PI_SET && level == 2) {
    // Check that at least the first ULONG is accessible
    rc = FSH_PROBEBUF(PB_OPREAD, ((EAOP *)pData)->fpFEAList, sizeof(ULONG));
    if (rc != NO_ERROR) {
      FSH_SEMCLEAR(&CPData.BufLock);
      return rc;
    }

    // Check that it is <=64K
    if (((EAOP *)pData)->fpFEAList->cbList > 0xffff) {
      FSH_SEMCLEAR(&CPData.BufLock);
      return ERROR_INVALID_PARAMETER;
    }

    // Check that we can access the whole list
    rc = FSH_PROBEBUF(PB_OPREAD, ((EAOP *)pData)->fpFEAList,
                      ((EAOP *)pData)->fpFEAList->cbList);
    if (rc != NO_ERROR) {
      FSH_SEMCLEAR(&CPData.BufLock);
      return rc;
    }
  }


  // Make sure the control program is still attached (this is how we
  // know our buffers are still valid)
  if (!CPAttached) {
    FSH_SEMCLEAR(&CPData.BufLock);
    return (ERROR_NOT_READY);
  }


  // Setup parameters -- from here to the PostSetup(), we MUST NOT block
  CPData.OpData->i.funccode = CPFC_PATHINFO;
  FSH_GETVOLPARM(pcdfsi->cdi_hVPB, &pvpfsi, &pvpfsd);
  CPData.OpData->p.pathinfo.vpfsd = *pvpfsd;
  CPData.OpData->p.pathinfo.flag = flag;
  CPData.OpData->p.pathinfo.cdfsi = *pcdfsi;
  CPData.OpData->p.pathinfo.cdfsd = *pcdfsd;
  strncpy(CPData.OpData->p.pathinfo.Name, pName, sizeof(CPData.OpData->p.pathinfo.Name));
  CPData.OpData->p.pathinfo.iCurDirEnd = iCurDirEnd;
  CPData.OpData->p.pathinfo.level = level;


  /* Copy either the buffer, FEA or GEA list as appropriate */
  if (flag == PI_RETRIEVE && level == 3) {
    memcpy(CPData.Buf, ((EAOP *)pData)->fpGEAList,
                       ((EAOP *)pData)->fpGEAList->cbList);
    /* cbData is FEA list size, as FEA list will be returned */
    CPData.OpData->p.pathinfo.cbData = ((EAOP *)pData)->fpFEAList->cbList;
  }
  else if(level==4)
    CPData.OpData->p.pathinfo.cbData = ((EAOP *)pData)->fpFEAList->cbList;
  else if (flag == FI_SET && level == 2) {
    memcpy(CPData.Buf, ((EAOP *)pData)->fpFEAList,
	   ((EAOP *)pData)->fpFEAList->cbList);
    CPData.OpData->p.pathinfo.cbData = ((EAOP *)pData)->fpFEAList->cbList;
  }
  else {
    memcpy(CPData.Buf, pData, cbData);
    CPData.OpData->p.pathinfo.cbData = cbData;
  }

  rc = PostSetup();
  if (rc != NO_ERROR)
    return rc;


  // Copy results
  if (flag == PI_RETRIEVE) {
    // If level 3 or 4, copy results starting at the oError entry in the EAOP
    if (level != 3 && level != 4)
      memcpy(pData, CPData.Buf, cbData);
    else
      memcpy(((EAOP *)pData)->fpFEAList, CPData.Buf,
             ((FEALIST *)CPData.Buf)->cbList);
  }
  if (flag == PI_RETRIEVE && level == 3 ||
      flag == PI_RETRIEVE && level == 4 ||
      flag == PI_SET && level == 2)
    ((EAOP *)pData)->oError = CPData.OpData->p.pathinfo.oError;
  *pvpfsd = CPData.OpData->p.pathinfo.vpfsd;
  *pcdfsd = CPData.OpData->p.pathinfo.cdfsd;

  rc = CPData.OpData->i.rc;

  // We CANNOT access the buffers after this line
  FSH_SEMCLEAR(&CPData.BufLock);

  return rc;
}


/******************************************************************************
**
** FS_READ - read data from a file
**
** Parameters
** ----------
** struct sffsi far *psffsi             pointer to FSD independant file instance
** struct sffsd far *psffsd             pointer to FSD dependant file instance
** char far *pData                      UNVERIFIED pointer to buffer
** unsigned short far *pLen             length of buffer
** unsigned short IOflag                bitfield of I/O suggestions
**   values:
**     IOFL_WRITETHRU   write all updated data before returning
**     IOFL_NOCACHE     don't cache any new data
**
******************************************************************************/

#pragma argsused
short int far pascal _export _loadds FS_READ(struct sffsi far *psffsi, struct sffsd far *psffsd,
                             char far *pData, unsigned short far *pLen,
                             unsigned short IOflag)
{
  unsigned short rc;
  struct vpfsi *pvpfsi;
  struct vpfsd *pvpfsd;

  rc = PreSetup();
  if (rc != NO_ERROR)
    return rc;


  // Verify parameters
  if (*pLen != 0)
    rc = FSH_PROBEBUF(PB_OPWRITE, pData, *pLen);
  if (rc != NO_ERROR) {
    FSH_SEMCLEAR(&CPData.BufLock);
    return rc;
  }


  // Make sure the control program is still attached (this is how we
  // know our buffers are still valid)
  if (!CPAttached) {
    FSH_SEMCLEAR(&CPData.BufLock);
    return (ERROR_NOT_READY);
  }


  // Setup parameters -- from here to the PostSetup(), we MUST NOT block
  CPData.OpData->i.funccode = CPFC_READ;
  FSH_GETVOLPARM(psffsi->sfi_hVPB, &pvpfsi, &pvpfsd);
  CPData.OpData->p.read.vpfsd = *pvpfsd;
  CPData.OpData->p.read.sffsi = *psffsi;
  CPData.OpData->p.read.sffsd = *psffsd;
  CPData.OpData->p.read.Len = *pLen;
  CPData.OpData->p.read.IOflag = IOflag;

  rc = PostSetup();
  if (rc != NO_ERROR)
    return rc;


  /* Copy results */
  *pvpfsd = CPData.OpData->p.read.vpfsd;
  *psffsi = CPData.OpData->p.read.sffsi;
  *psffsd = CPData.OpData->p.read.sffsd;
  *pLen = CPData.OpData->p.read.Len;
  memcpy(pData, CPData.Buf, *pLen);

  rc = CPData.OpData->i.rc;

  // We CANNOT access the buffers after this line
  FSH_SEMCLEAR(&CPData.BufLock);

  return rc;
}


/******************************************************************************
**
** FS_WRITE - write data to a file
**
** Parameters
** ----------
** struct sffsi far *psffsi             pointer to FSD independant file instance
** struct sffsd far *psffsd             pointer to FSD dependant file instance
** char far *pData                      UNVERIFIED pointer to buffer
** unsigned short far *pLen             length of buffer
** unsigned short IOflag                bitfield of I/O suggestions
**   values:
**     IOFL_WRITETHRU   write all updated data before returning
**     IOFL_NOCACHE     don't cache any new data
**
******************************************************************************/

#pragma argsused
short int far pascal _export _loadds FS_WRITE(struct sffsi far *psffsi, struct sffsd far *psffsd,
                              char far *pData, unsigned short far *pLen,
                              unsigned short IOflag)
{
  unsigned short rc;
  struct vpfsi *pvpfsi;
  struct vpfsd *pvpfsd;

  rc = PreSetup();
  if (rc != NO_ERROR)
    return rc;


  // Verify parameters
  if (*pLen != 0)
    rc = FSH_PROBEBUF(PB_OPREAD, pData, *pLen);
  if (rc != NO_ERROR) {
    FSH_SEMCLEAR(&CPData.BufLock);
    return rc;
  }


  // Make sure the control program is still attached (this is how we
  // know our buffers are still valid)
  if (!CPAttached) {
    FSH_SEMCLEAR(&CPData.BufLock);
    return (ERROR_NOT_READY);
  }


  // Setup parameters -- from here to the PostSetup(), we MUST NOT block
  CPData.OpData->i.funccode = CPFC_WRITE;
  FSH_GETVOLPARM(psffsi->sfi_hVPB, &pvpfsi, &pvpfsd);
  CPData.OpData->p.write.vpfsd = *pvpfsd;
  CPData.OpData->p.write.sffsi = *psffsi;
  CPData.OpData->p.write.sffsd = *psffsd;
  memcpy(CPData.Buf, pData, *pLen);
  CPData.OpData->p.write.Len = *pLen;
  CPData.OpData->p.write.IOflag = IOflag;

  rc = PostSetup();
  if (rc != NO_ERROR)
    return rc;


  // Copy results
  *pvpfsd = CPData.OpData->p.write.vpfsd;
  *psffsi = CPData.OpData->p.write.sffsi;
  *psffsd = CPData.OpData->p.write.sffsd;
  *pLen = CPData.OpData->p.write.Len;

  rc = CPData.OpData->i.rc;

  // We CANNOT access the buffers after this line
  FSH_SEMCLEAR(&CPData.BufLock);

  return rc;
}



/*-----------------------------------------------------------------------------
--
-- Directory management
--
-----------------------------------------------------------------------------*/



/******************************************************************************
**
** FS_FINDCLOSE - End a directory search
**
** Parameters
** ----------
** struct fsfsi far *pfsfsi             pointer to FSD independant search record
** struct fsfsd far *pfsfsd             pointer to FSD dependant search record
**
******************************************************************************/

#pragma argsused
short int far pascal _export _loadds FS_FINDCLOSE(struct fsfsi far *pfsfsi, struct fsfsd far *pfsfsd)
{
  unsigned short rc;
  struct vpfsi *pvpfsi;
  struct vpfsd *pvpfsd;

  rc = PreSetup();
  if (rc != NO_ERROR)
    return rc;

  // Verify parameters

  // Make sure the control program is still attached (this is how we
  // know our buffers are still valid)
  if (!CPAttached) {
    FSH_SEMCLEAR(&CPData.BufLock);
    return (ERROR_NOT_READY);
  }

  // Setup parameters -- from here to the PostSetup(), we MUST NOT block
  CPData.OpData->i.funccode = CPFC_FINDCLOSE;
  FSH_GETVOLPARM(pfsfsi->fsi_hVPB, &pvpfsi, &pvpfsd);
  CPData.OpData->p.findclose.vpfsd = *pvpfsd;
  CPData.OpData->p.findclose.fsfsi = *pfsfsi;
  CPData.OpData->p.findclose.fsfsd = *pfsfsd;

  rc = PostSetup();
  if (rc != NO_ERROR)
    return rc;


  // Copy results
  *pfsfsd = CPData.OpData->p.findclose.fsfsd;

  rc = CPData.OpData->i.rc;

  // We CANNOT access the buffers after this line
  FSH_SEMCLEAR(&CPData.BufLock);

  return rc;
}


/******************************************************************************
**
** FS_FINDFIRST - Begin a new directory search
**
** Parameters
** ----------
** struct cdfsi far *pcdfsi             pointer to FSD independant current directory
** struct cdfsd far *pcdfsd             pointer to FSD dependant current directory
** char far *pName                      pointer to filename mask
** unsigned short iCurDirEnd            offset to the end of the current directory in pName
** unsigned short attr                  attribute mask
** struct fsfsi far *pfsfsi             pointer to FSD independant search record
** struct fsfsd far *pfsfsd             pointer to FSD dependant search record
** char far *pData                      UNVERIFIED pointer to information area
** unsigned short cbData                size of area pointed to by pData
** unsigned short far *pcMatch          maximum number of entries to return*
**                                      number of entries actually returned
** unsigned short level                 level of information to return
** unsigned short flags                 indicates whether to return position information
**   values:
**     FF_NOPOS         don't return any position information
**     FF_GETPOS        return position information in buffer
**
******************************************************************************/

#pragma argsused
short int far pascal _export _loadds FS_FINDFIRST(struct cdfsi far *pcdfsi, struct cdfsd far *pcdfsd,
                                  char far *pName, unsigned short iCurDirEnd,
                                  unsigned short attr, struct fsfsi far *pfsfsi,
                                  struct fsfsd far *pfsfsd, char far *pData,
                                  unsigned short cbData, unsigned short far *pcMatch,
                                  unsigned short level, unsigned short flags)
{
  unsigned short rc;
  struct vpfsi *pvpfsi;
  struct vpfsd *pvpfsd;

  rc = PreSetup();
  if (rc != NO_ERROR)
    return rc;

  /* Verify parameters */
  if (cbData != 0)
    rc = FSH_PROBEBUF(PB_OPWRITE, pData, cbData);
  if (rc != NO_ERROR) {
    FSH_SEMCLEAR(&CPData.BufLock);
    return rc;
  }

  /* Special EA check */
  if (level == 3 || level == 4) {
    /* Check to make sure the buffer's at least big enough for an EAOP */
    if (cbData < sizeof(EAOP)) {
      FSH_SEMCLEAR(&CPData.BufLock);
      return ERROR_INVALID_PARAMETER;
    }

    if(level==3) {
      /* Check that at least the first ULONG is accessible */
      rc = FSH_PROBEBUF(PB_OPREAD, ((EAOP *)pData)->fpGEAList, sizeof(ULONG));
      if (rc != NO_ERROR) {
	FSH_SEMCLEAR(&CPData.BufLock);
	return rc;
      }
      
      /* Check that it is <=64K */
      if (((EAOP *)pData)->fpGEAList->cbList > 0xffff) {
	FSH_SEMCLEAR(&CPData.BufLock);
	return ERROR_INVALID_PARAMETER;
      }

      if (((EAOP *)pData)->fpGEAList->cbList == 0) {
	FSH_SEMCLEAR(&CPData.BufLock);
	return ERROR_BAD_FORMAT;
      }

      /* Check that we can access the whole list */
      rc = FSH_PROBEBUF(PB_OPREAD, ((EAOP *)pData)->fpGEAList,
			((EAOP *)pData)->fpGEAList->cbList);
      if (rc != NO_ERROR) {
	FSH_SEMCLEAR(&CPData.BufLock);
	return rc;
      }
    }
  }

  /* Make sure the control program is still attached (this is how we
     know our buffers are still valid) */
  if (!CPAttached) {
    FSH_SEMCLEAR(&CPData.BufLock);
    return (ERROR_NOT_READY);
  }

  /* Setup parameters -- from here to the PostSetup(), we MUST NOT block */
  CPData.OpData->i.funccode = CPFC_FINDFIRST;
  FSH_GETVOLPARM(pfsfsi->fsi_hVPB, &pvpfsi, &pvpfsd);
  CPData.OpData->p.findfirst.vpfsd = *pvpfsd;
  CPData.OpData->p.findfirst.cdfsi = *pcdfsi;
  CPData.OpData->p.findfirst.cdfsd = *pcdfsd;
  strncpy(CPData.OpData->p.findfirst.Name, pName, sizeof(CPData.OpData->p.findfirst.Name));
  CPData.OpData->p.findfirst.iCurDirEnd = iCurDirEnd;
  CPData.OpData->p.findfirst.attr = attr;
  CPData.OpData->p.findfirst.fsfsi = *pfsfsi;
  CPData.OpData->p.findfirst.fsfsd = *pfsfsd;
  /* Copy either the buffer or the GEA list as appropriate */
  if (level==3) {
    memcpy(CPData.Buf, ((EAOP *)pData)->fpGEAList,
                       ((EAOP *)pData)->fpGEAList->cbList);
    CPData.OpData->p.findfirst.cbData = cbData-sizeof(EAOP);
  }
  else if(level==4)
    CPData.OpData->p.findfirst.cbData = cbData-sizeof(EAOP);
  else {
    memcpy(CPData.Buf, pData, cbData);
    CPData.OpData->p.findfirst.cbData = cbData;
  }
  CPData.OpData->p.findfirst.cMatch = *pcMatch;
  CPData.OpData->p.findfirst.level = level;
  CPData.OpData->p.findfirst.flags = flags;

  rc = PostSetup();
  if (rc != NO_ERROR)
    return rc;

  /* Copy results */
  *pcdfsd = CPData.OpData->p.findfirst.cdfsd;
  *pfsfsd = CPData.OpData->p.findfirst.fsfsd;
  /* If level 3, copy results starting at the oError entry in the EAOP */
  if(level==3 || level==4) {
    memcpy(pData + sizeof(EAOP), CPData.Buf, cbData - sizeof(EAOP));
    ((EAOP *)pData)->oError = CPData.OpData->p.findfirst.oError;
  }
  else
    memcpy(pData, CPData.Buf, cbData);
  *pcMatch = CPData.OpData->p.findfirst.cMatch;

  rc = CPData.OpData->i.rc;

  /* We CANNOT access the buffers after this line */
  FSH_SEMCLEAR(&CPData.BufLock);

  return rc;
}


/******************************************************************************
**
** FS_FINDFROMNAME - Restart directory search
**
** Parameters
** ----------
** struct fsfsi far *pfsfsi             pointer to FSD independant search record
** struct fsfsd far *pfsfsd             pointer to FSD dependant search record
** char far *pData                      UNVERIFIED pointer to information area
** unsigned short cbData                size of area pointed to by pData
** unsigned short far *pcMatch          maximum number of entries to return*
**                                      number of entries actually returned
** unsigned short level                 level of information to return
** unsigned long position               position in directory to restart search from
** char far *pName                      pointer to filename to restart search from
** unsigned short flags                 indicates whether to return position information
**   values:
**     FF_NOPOS         don't return any position information
**     FF_GETPOS        return position information in buffer
**
******************************************************************************/

#pragma argsused
short int far pascal _export _loadds FS_FINDFROMNAME(struct fsfsi far *pfsfsi, struct fsfsd far *pfsfsd,
                                     char far *pData, unsigned short cbData,
                                     unsigned short far *pcMatch, unsigned short level,
                                     unsigned long position, char far *pName,
                                     unsigned short flags)
{
  unsigned short rc;
  struct vpfsi *pvpfsi;
  struct vpfsd *pvpfsd;
  
  rc = PreSetup();
  if (rc != NO_ERROR)
    return rc;
  
  
  // Verify parameters
  if (cbData != 0)
    rc = FSH_PROBEBUF(PB_OPWRITE, pData, cbData);
  if (rc != NO_ERROR) {
    FSH_SEMCLEAR(&CPData.BufLock);
    return rc;
  }
  
  // Special EA check
  if (level == 3 || level == 4) {
    // Check to make sure the buffer's at least big enough for an EAOP
    if (cbData < sizeof(EAOP)) {
      FSH_SEMCLEAR(&CPData.BufLock);
      return ERROR_INVALID_PARAMETER;
    }
    
    if(level==3) {
      // Check that at least the first ULONG is accessible
      rc = FSH_PROBEBUF(PB_OPREAD, ((EAOP *)pData)->fpGEAList, sizeof(ULONG));
      if (rc != NO_ERROR) {
	FSH_SEMCLEAR(&CPData.BufLock);
	return rc;
      }
      
      // Check that it is <=64K
      if (((EAOP *)pData)->fpGEAList->cbList > 0xffff) {
	FSH_SEMCLEAR(&CPData.BufLock);
	return ERROR_INVALID_PARAMETER;
      }
      
      if (((EAOP *)pData)->fpGEAList->cbList == 0) {
	FSH_SEMCLEAR(&CPData.BufLock);
	return ERROR_BAD_FORMAT;
      }

      // Check that we can access the whole list
      rc = FSH_PROBEBUF(PB_OPREAD, ((EAOP *)pData)->fpGEAList, 
			((EAOP *)pData)->fpGEAList->cbList);
      if (rc != NO_ERROR) {
	FSH_SEMCLEAR(&CPData.BufLock);
	return rc;
      }
    }
  }
  
  
  // Make sure the control program is still attached (this is how we
  // know our buffers are still valid)
  if (!CPAttached) {
    FSH_SEMCLEAR(&CPData.BufLock);
    return (ERROR_NOT_READY);
  }

  
  // Setup parameters -- from here to the PostSetup(), we MUST NOT block
  CPData.OpData->i.funccode = CPFC_FINDFROMNAME;
  FSH_GETVOLPARM(pfsfsi->fsi_hVPB, &pvpfsi, &pvpfsd);
  CPData.OpData->p.findfromname.vpfsd = *pvpfsd;
  CPData.OpData->p.findfromname.fsfsi = *pfsfsi;
  CPData.OpData->p.findfromname.fsfsd = *pfsfsd;
  if (level==3) {
    memcpy(CPData.Buf, ((EAOP *)pData)->fpGEAList,
                       ((EAOP *)pData)->fpGEAList->cbList);
    CPData.OpData->p.findfromname.cbData = cbData-sizeof(EAOP);
  }
  else if(level==4)
    CPData.OpData->p.findfromname.cbData = cbData-sizeof(EAOP);
  else {
    memcpy(CPData.Buf, pData, cbData);
    CPData.OpData->p.findfromname.cbData = cbData;
  }
  CPData.OpData->p.findfromname.cMatch = *pcMatch;
  CPData.OpData->p.findfromname.level = level;
  CPData.OpData->p.findfromname.position = position;
  strncpy(CPData.OpData->p.findfromname.Name, pName, sizeof(CPData.OpData->p.findfromname.Name));
  CPData.OpData->p.findfromname.flags = flags;
  
  rc = PostSetup();
  if (rc != NO_ERROR)
    return rc;
  
  
  // Copy results
  *pfsfsd = CPData.OpData->p.findfromname.fsfsd;
  // If level 3, copy results starting at the oError entry in the EAOP
  if(level==3 || level==4) {
    memcpy(pData + sizeof(EAOP), CPData.Buf, cbData - sizeof(EAOP));
    ((EAOP *)pData)->oError = CPData.OpData->p.findfromname.oError;
  }
  else
    memcpy(pData, CPData.Buf, cbData);
  *pcMatch = CPData.OpData->p.findfromname.cMatch;
  
  rc = CPData.OpData->i.rc;
  
  // We CANNOT access the buffers after this line
  FSH_SEMCLEAR(&CPData.BufLock);
  
  return rc;
}


/******************************************************************************
**
** FS_FINDNEXT - Continue directory search
**
** Parameters
** ----------
** struct fsfsi far *pfsfsi             pointer to FSD independant search record
** struct fsfsd far *pfsfsd             pointer to FSD dependant search record
** char far *pData                      UNVERIFIED pointer to information area
** unsigned short cbData                size of area pointed to by pData
** unsigned short far *pcMatch          maximum number of entries to return*
**                                      number of entries actually returned
** unsigned short level                 level of information to return
** unsigned short flags                 indicates whether to return position information
**   values:
**     FF_NOPOS         don't return any position information
**     FF_GETPOS        return position information in buffer
**
******************************************************************************/

#pragma argsused
short int far pascal _export _loadds FS_FINDNEXT(struct fsfsi far *pfsfsi, 
						 struct fsfsd far *pfsfsd,
						 char far *pData, 
						 unsigned short cbData,
						 unsigned short far *pcMatch, 
						 unsigned short level,
						 unsigned short flags)
{
  unsigned short rc;
  struct vpfsi *pvpfsi;
  struct vpfsd *pvpfsd;
   
  rc = PreSetup();
  if (rc != NO_ERROR)
    return rc;


  // Verify parameters
  if (cbData != 0)
    rc = FSH_PROBEBUF(PB_OPWRITE, pData, cbData);
  if (rc != NO_ERROR) {
    FSH_SEMCLEAR(&CPData.BufLock);
    return rc;
  }

  // Special EA check
  if (level == 3 || level == 4) {
    // Check to make sure the buffer's at least big enough for an EAOP
    if (cbData < sizeof(EAOP)) {
      FSH_SEMCLEAR(&CPData.BufLock);
      return ERROR_INVALID_PARAMETER;
    }

    if(level==3) {
      // Check that at least the first ULONG is accessible
      rc = FSH_PROBEBUF(PB_OPREAD, ((EAOP *)pData)->fpGEAList, sizeof(ULONG));
      if (rc != NO_ERROR) {
	FSH_SEMCLEAR(&CPData.BufLock);
	return rc;
      }
      
      // Check that it is <=64K
      if (((EAOP *)pData)->fpGEAList->cbList > 0xffff) {
	FSH_SEMCLEAR(&CPData.BufLock);
	return ERROR_INVALID_PARAMETER;
      }
      
      if (((EAOP *)pData)->fpGEAList->cbList == 0) {
	FSH_SEMCLEAR(&CPData.BufLock);
	return ERROR_BAD_FORMAT;
      }

      // Check that we can access the whole list
      rc = FSH_PROBEBUF(PB_OPREAD, ((EAOP *)pData)->fpGEAList, 
			((EAOP *)pData)->fpGEAList->cbList);
      if (rc != NO_ERROR) {
	FSH_SEMCLEAR(&CPData.BufLock);
	return rc;
      }
    }
  }


  // Make sure the control program is still attached (this is how we
  // know our buffers are still valid)
  if (!CPAttached) {
    FSH_SEMCLEAR(&CPData.BufLock);
    return (ERROR_NOT_READY);
  }

  
  // Setup parameters -- from here to the PostSetup(), we MUST NOT block
  CPData.OpData->i.funccode = CPFC_FINDNEXT;
  FSH_GETVOLPARM(pfsfsi->fsi_hVPB, &pvpfsi, &pvpfsd);
  CPData.OpData->p.findnext.vpfsd = *pvpfsd;
  CPData.OpData->p.findnext.fsfsi = *pfsfsi;
  CPData.OpData->p.findnext.fsfsd = *pfsfsd;
  if (level==3) {
    memcpy(CPData.Buf, ((EAOP *)pData)->fpGEAList,
                       ((EAOP *)pData)->fpGEAList->cbList);
    CPData.OpData->p.findnext.cbData = cbData-sizeof(EAOP);
  }
  else if(level==4)
    CPData.OpData->p.findnext.cbData = cbData-sizeof(EAOP);
  else {
    memcpy(CPData.Buf, pData, cbData);
    CPData.OpData->p.findnext.cbData = cbData;
  }
  CPData.OpData->p.findnext.cMatch = *pcMatch;
  CPData.OpData->p.findnext.level = level;
  CPData.OpData->p.findnext.flags = flags;

  rc = PostSetup();
  if (rc != NO_ERROR)
    return rc;

  // Copy results
  *pfsfsd = CPData.OpData->p.findnext.fsfsd;
  // If level 3, copy results starting at the oError entry in the EAOP
  if(level==3 || level==4) {
    memcpy(pData + sizeof(EAOP), CPData.Buf, cbData - sizeof(EAOP));
    ((EAOP *)pData)->oError = CPData.OpData->p.findnext.oError;
  }
  else
    memcpy(pData, CPData.Buf, cbData);
  *pcMatch = CPData.OpData->p.findnext.cMatch;

  rc = CPData.OpData->i.rc;

  // We CANNOT access the buffers after this line
  FSH_SEMCLEAR(&CPData.BufLock);

  return rc;
}


/******************************************************************************
**
** FS_FINDNOTIFYCLOSE - End a directory update request
**
** Parameters
** ----------
** unsigned short handle                handle of update request to close
**
******************************************************************************/

#pragma argsused
short int far pascal _export _loadds FS_FINDNOTIFYCLOSE(unsigned short handle)
{
  return ERROR_NOT_SUPPORTED;
}


/******************************************************************************
**
** FS_FINDNOTIFYFIRST - Begin a new directory update request
**
** Parameters
** ----------
** struct cdfsi far *pcdfsi             pointer to FSD independant current directory
** struct cdfsd far *pcdfsd             pointer to FSD dependant current directory
** char far *pName                      pointer to filename mask
** unsigned short iCurDirEnd            offset to the end of the current directory in pName
** unsigned short attr                  attribute mask
** unsigned short far *pHandle          pointer to place where FSD stores its handle
** char far *pData                      UNVERIFIED pointer to information area
** unsigned short cbData                size of area pointed to by pData
** unsigned short far *pcMatch          maximum number of entries to return*
**                                      number of entries actually returned
** unsigned short level                 level of information to return
** unsigned long timeout                timeout in milliseconds
**
******************************************************************************/

#pragma argsused
short int far pascal _export _loadds FS_FINDNOTIFYFIRST(struct cdfsi far *pcdfsi, struct cdfsd far *pcdfsd,
                                        char far *pName, unsigned short iCurDirEnd,
                                        unsigned short attr, unsigned short far *pHandle,
                                        char far *pData, unsigned short cbData,
                                        unsigned short far *pcMatch, unsigned short level,
                                        unsigned long timeout)
{
  return ERROR_NOT_SUPPORTED;
}


/******************************************************************************
**
** FS_FINDNOTIFYNEXT - Continue directory update request
**
** Parameters
** ----------
** unsigned short handle                directory update handle
** char far *pData                      UNVERIFIED pointer to information area
** unsigned short cbData                size of area pointed to by pData
** unsigned short far *pcMatch          maximum number of entries to return*
**                                      number of entries actually returned
** unsigned short level                 level of information to return
** unsigned long timeout                timeout in milliseconds
**
******************************************************************************/

#pragma argsused
short int far pascal _export _loadds FS_FINDNOTIFYNEXT(unsigned short handle, char far *pData,
                                       unsigned short cbData, unsigned short far *pcMatch,
                                       unsigned short level, unsigned long timeout)
{
  return ERROR_NOT_SUPPORTED;
}



/*-----------------------------------------------------------------------------
--
-- FSD Extended Interface
--
-----------------------------------------------------------------------------*/



/******************************************************************************
**
** FS_IOCTL - Perform an IOCTL on the IFS
**
** Parameters
** ----------
** struct sffsi far *psffsi             pointer to FSD independant file instance
** struct sffsd far *psffsd             pointer to FSD dependant file instance
** unsigned short cat                   function category
** unsigned short func                  function to perform
** char far *pParm                      UNVERIFIED pointer to parameter area
** unsigned short lenParm               size of area pointed to by pParm
** unsigned short far *pParmLenInOut    length of parameters passed in pParm
** char far *pData                      UNVERIFIED pointer to information area
** unsigned short lenData               size of area pointed to by pData
** unsigned short far *pDataLenInOut    length of parameters passed in pData
**
******************************************************************************/

#pragma argsused
short int far pascal _export _loadds FS_IOCTL(struct sffsi far *psffsi, struct sffsd far *psffsd,
                              unsigned short cat, unsigned short func,
                              char far *pParm, unsigned short lenParm,
                              unsigned far *pParmLenInOut, char far *pData,
                              unsigned short lenData, unsigned far *pDataLenInOut)
{
  struct vpfsi *pvpfsi;
  struct vpfsd *pvpfsd;
  int rc;

  FSH_GETVOLPARM(psffsi->sfi_hVPB, &pvpfsi, &pvpfsd);
  if(pParmLenInOut==NULL && pDataLenInOut==NULL)
    rc = FSH_DEVIOCTL(0, pvpfsi->vpi_hDEV, psffsi->sfi_selfsfn, cat, func, 
		      pParm, lenParm, pData, lenData);
  else {
    rc = FSH_DEVIOCTL(0, pvpfsi->vpi_hDEV, psffsi->sfi_selfsfn, cat, func, 
		      pParm, *pParmLenInOut, pData, *pDataLenInOut);
    if(pParmLenInOut)
      *pParmLenInOut = lenParm;
    if(pDataLenInOut)
      *pDataLenInOut = lenData;
  }
  return rc;
}

/*-----------------------------------------------------------------------------
--
-- Miscellaneous Functions
--
-----------------------------------------------------------------------------*/



/******************************************************************************
**
** FS_NMPIPE - Perform a named pipe operation
**
** Parameters
** ----------
** struct sffsi far *psffsi             pointer to FSD independant file instance
** struct sffsd far *psffsd             pointer to FSD dependant file instance
** unsigned short OpType                operation to perform
**   values:
**     NMP_GetPHandState
**     NMP_SetPHandState
**     NMP_PipeQInfo
**     NMP_PeekPipe
**     NMP_ConnectPipe
**     NMP_DisconnectPipe
**     NMP_TransactPipe
**     NMP_READRAW
**     NMP_WRITERAW
**     NMP_WAITPIPE
**     NMP_CALLPIPE
**     NMP_QNmPipeSemState
** union npoper far *pOpRec             data for operation
** char far *pData                      pointer to user data
** char far *pName                      pointer to remote named pipe
**
******************************************************************************/

#pragma argsused
short int far pascal _export _loadds FS_NMPIPE(struct sffsi far *psffsi, struct sffsd far *psffsd,
                               unsigned short OpType, union npoper far *pOpRec,
                               char far *pData, char far *pName)
{
  return ERROR_NOT_SUPPORTED;
}


/******************************************************************************
**
** FS_PROCESSNAME - Canonicalize a filename
**
** Parameters
** ----------
** char far *pNameBuf                   filename to canonicalize
**
******************************************************************************/

#pragma argsused
short int far pascal _export _loadds FS_PROCESSNAME(char far *pNameBuf)
{
  unsigned short rc;

  rc = PreSetup();
  if (rc != NO_ERROR)
    return rc;


  // Verify parameters

  // Make sure the control program is still attached (this is how we
  // know our buffers are still valid)
  if (!CPAttached) {
    FSH_SEMCLEAR(&CPData.BufLock);
    return (ERROR_NOT_READY);
  }


  // Setup parameters -- from here to the PostSetup(), we MUST NOT block
  CPData.OpData->i.funccode = CPFC_PROCESSNAME;
  strncpy(CPData.OpData->p.processname.NameBuf, pNameBuf, 
             sizeof(CPData.OpData->p.processname.NameBuf));

  rc = PostSetup();
  if (rc != NO_ERROR)
    return rc;


  // Copy results
  strcpy(pNameBuf, CPData.OpData->p.processname.NameBuf);

  rc = CPData.OpData->i.rc;

  // We CANNOT access the buffers after this line
  FSH_SEMCLEAR(&CPData.BufLock);

  return rc;
}


/******************************************************************************
**
** FS_SETSWAP - Inform IFS that it owns the swap file
**
** Parameters
** ----------
** struct sffsi far *psffsi             pointer to FSD independant file instance
** struct sffsd far *psffsd             pointer to FSD dependant file instance
**
******************************************************************************/

#pragma argsused
short int far pascal _export _loadds FS_SETSWAP(struct sffsi far *psffsi,
                                        struct sffsd far *psffsd)
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



#if defined(SUP_FILEIO)
/******************************************************************************
**
** FS_CANCELLOCKREQUEST - Unlock a range in a file
**
** Parameters
** ----------
** struct sffsi far *psffsi             pointer to FSD independant file instance
** struct sffsd far *psffsd             pointer to FSD dependant file instance
** void far *pLockRange                 range to unlock
**
******************************************************************************/

#pragma argsused
short int far pascal _export _loadds FS_CANCELLOCKREQUEST(struct sffsi far *psffsi, struct sffsd far *psffsd,
                                          void far *pLockRange)
{
  unsigned short rc;

  rc = PreSetup();
  if (rc != NO_ERROR)
    return rc;


  // Verify parameters

  // Make sure the control program is still attached (this is how we
  // know our buffers are still valid)
  if (!CPAttached) {
    FSH_SEMCLEAR(&CPData.BufLock);
    return (ERROR_NOT_READY);
  }


  // Setup parameters -- from here to the PostSetup(), we MUST NOT block
  CPData.OpData->i.funccode = CPFC_CANCELLOCKREQUEST;
  CPData.OpData->p.cancellockrequest.sffsi = *psffsi;
  CPData.OpData->p.cancellockrequest.sffsd = *psffsd;
  CPData.OpData->p.cancellockrequest.LockRange = *pLockRange;

  rc = PostSetup();
  if (rc != NO_ERROR)
    return rc;


  // Copy results
  *psffsi = CPData.OpData->p.cancellockrequest.sffsi;
  *psffsd = CPData.OpData->p.cancellockrequest.sffsd;

  rc = CPData.OpData->i.rc;

  // We CANNOT access the buffers after this line
  FSH_SEMCLEAR(&CPData.BufLock);

  return rc;
}


/******************************************************************************
**
** FS_FILELOCKS - Lock a range in a file
**
** Parameters
** ----------
** struct sffsi far *psffsi             pointer to FSD independant file instance
** struct sffsd far *psffsd             pointer to FSD dependant file instance
** void far *pUnLockRange               range to unlock
** void far *pLockRange                 range to lock
** unsigned long timeout                time in milliseconds to wait
** unsigned long flags                  flags
**   values:
**     0x01                     sharing of this file region is allowed
**     0x02                     atomic lock request
**
******************************************************************************/

#pragma argsused
short int far pascal _export _loadds FS_FILELOCKS(struct sffsi far *psffsi, struct sffsd far *psffsd,
                                  void far *pUnLockRange, void far *pLockRange,
                                  unsigned long timeout, unsigned long flags)
{
  unsigned short rc;

  rc = PreSetup();
  if (rc != NO_ERROR)
    return rc;


  // Verify parameters

  // Make sure the control program is still attached (this is how we
  // know our buffers are still valid)
  if (!CPAttached) {
    FSH_SEMCLEAR(&CPData.BufLock);
    return (ERROR_NOT_READY);
  }


  // Setup parameters -- from here to the PostSetup(), we MUST NOT block
  CPData.OpData->i.funccode = CPFC_FILELOCKS;
  CPData.OpData->p.filelocks.sffsi = *psffsi;
  CPData.OpData->p.filelocks.sffsd = *psffsd;
  CPData.OpData->p.filelocks.UnLockRange = *pUnLockRange;
  CPData.OpData->p.filelocks.LockRange = *pLockRange;
  CPData.OpData->p.filelocks.timeout = timeout;
  CPData.OpData->p.filelocks.flags = flags;

  rc = PostSetup();
  if (rc != NO_ERROR)
    return rc;


  // Copy results
  *psffsi = CPData.OpData->p.filelocks.sffsi;
  *psffsd = CPData.OpData->p.filelocks.sffsd;

  rc = CPData.OpData->i.rc;

  // We CANNOT access the buffers after this line
  FSH_SEMCLEAR(&CPData.BufLock);

  return rc;
}

#endif

/******************************************************************************
*******************************************************************************
**
** Swapper entry points
**
*******************************************************************************
******************************************************************************/

#if defined(SUP_PAGEIO)

/******************************************************************************
**
** FS_ALLOCATEPAGESPACE - Adjust size of the page file
**
** Parameters
** ----------
** struct sffsi far *psffsi             pointer to FSD independant file instance
** struct sffsd far *psffsd             pointer to FSD dependant file instance
** unsigned long ISize                  new size of page file
** unsigned long IWantContig            minimum continuity requirement
**
******************************************************************************/

#pragma argsused
short int far pascal _export _loadds FS_ALLOCATEPAGESPACE(struct sffsi far *psffsi,
                                          struct sffsd far *psffsd, 
                                          unsigned long lSize, 
                                          unsigned long lWantContig)
{
  return ERROR_NOT_SUPPORTED;
}


/******************************************************************************
**
** FS_DOPAGEIO - Perform I/O to the page file
**
** Parameters
** ----------
** struct sffsi far *psffsi             pointer to FSD independant file instance
** struct sffsd far *psffsd             pointer to FSD dependant file instance
** struct PageCmdHeader far *pPageCmdList operations to perform
**
******************************************************************************/

#pragma argsused
short int far pascal _export _loadds FS_DOPAGEIO(struct sffsi far *psffsi, struct sffsd far *psffsd,
                                 struct PageCmdHeader far *pPageCmdList)
{
  return ERROR_NOT_SUPPORTED;
}


/******************************************************************************
**
** FS_OPENPAGEFILE - Open the paging file
**
** Parameters
** ----------
** unsigned long far *pFlag             flags
**     PGIO_FIRSTOPEN           first open of page file
**     PGIO_PADDR               physical addresses required
**     PGIO_VADDR               16:16 virtual address required
** unsigned long far *pcMaxReq          pointer to maximum request list size
** char far *pName                      pointer to filename
** struct sffsi far *psffsi             pointer to FSD independant file instance
** struct sffsd far *psffsd             pointer to FSD dependant file instance
** unsigned long OpenMode               sharing and access mode
** unsigned short OpenFlag              action to take when file exists/doesn't exist
** unsigned short Attr                  OS/2 file attributes
** unsigned long Reserved               reserved
**
******************************************************************************/

#pragma argsused
short int far pascal _export _loadds FS_OPENPAGEFILE(unsigned long far *pFlags,
						     unsigned long far *pcMaxReq,
						     char far *pName, struct sffsi far *psffsi,
						     struct sffsd far *psffsd, 
						     unsigned short OpenMode,
						     unsigned short OpenFlag, unsigned short Attr,
						     unsigned long Reserved)
{
  return ERROR_NOT_SUPPORTED;
}

#endif
