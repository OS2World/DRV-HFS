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

/* r0comm.c
   Ring 0 side of communications
*/

#define INCL_DOSERRORS
#define INCL_DOSMISC
#define INCL_NOPMAPI
#include <os2.h>

#include <dhcalls.h>

#include <string.h>
#include <ctype.h>
#include <dos.h>

#include "fsd.h"
#include "..\r0r3shar.h"
#include "r0struct.h"
#include "r0comm.h"
#include "fsh.h"
#include "r0global.h"
#include "..\banner.h"
#include "..\hfsctl.h"

#ifdef DEBUG
#include "log.h"
#endif

/* Local prototypes */
static void DetachCP(void);
static int parse_args(char *);

/* Options */
int silent;

/* The list of mounted volumes */
static MOUNTSTRUC MountList[MAX_MOUNTSTRUCS];

/******************************************************************************
**
** FS_INIT - Initialize the IFS
**
** Parameters
** ---------
** char far *szParm                    pointer to command line parameters
** unsigned long pDevHlp               pointer to DevHlp entry point
** unsigned long far *pMiniFSD         pointer to data passed between the
**                                     mini-FSD and the IFS
**
******************************************************************************/

#pragma argsused
short int far pascal _export _loadds FS_INIT(char far *szParm,
                                             unsigned long pDevHlp,
                                             unsigned long far *pMiniFSD)
{
  SEL GDTSels[2];
  PVOID dummy;
  int rc;
  static char alloc1err[]="Error allocating lock handles\r\n";
  static char alloc2err[]="Error allocating GDT selectors\r\n";

  Startup();
  Device_Help = (PFN)pDevHlp;

  if(szParm != NULL && parse_args(szParm))
    return ERROR_BAD_ARGUMENTS;

  if(!silent)
    DosPutMessage(1, sizeof(banner)-1, banner);

  /* Allocate virtual memory for the two lock handles */
  rc = DevHelp_VMAlloc(VMDHA_SWAP, 2*sizeof(HLOCK), -1L, &Lock1, &dummy);
  if (rc != NO_ERROR) {
    DosPutMessage(1, sizeof(alloc1err)-1, alloc1err);
    return rc;
  }
  Lock2 = Lock1 + sizeof(HLOCK);

  /* Allocate 2 GDT selectors to communicate with the control program */
  rc = DevHelp_AllocGDTSelector(GDTSels, 2);
  if (rc != NO_ERROR) {
    DosPutMessage(1, sizeof(alloc2err)-1, alloc2err);
    return rc;
  }

  /* Set the pointers to the control program communication buffer */
  CPData.OpData = (OPDATA *)MK_FP(GDTSels[0], 0);
  CPData.Buf = (void *)MK_FP(GDTSels[1], 0);

  /* Say the control program has never been attached */
  /* Probably unnecessary */
  CPAttached = 0;
  AttachedEver = 0;

  return NO_ERROR;
}

/******************************************************************************
**
** FS_SHUTDOWN - Prepare for a system shutdown
**
** Parameters
** ---------
** unsigned short usType               flavor of call
**   values:
**     SD_BEGIN                  beginning the shutdown process
**     SD_COMPLETE               finishing the shutdown process
** unsigned long ulReserved            reserved
**
******************************************************************************/

#pragma argsused
short int far pascal _export _loadds FS_SHUTDOWN(unsigned short usType,
                                                 unsigned long ulReserved)
{
  return NO_ERROR;
}

/******************************************************************************
**
** FS_FSCTL - Extended IFS control
**
** Parameters
** ---------
** union argdat far *pArgdat
** unsigned short iArgType             argument type
**   values:
**     FSCTL_ARG_FILEINSTANCE    filehandle directed
**     FSCTL_ARG_CURDIR          pathname directed
**     FSCTL_ARG_NULL            FSD directed
** unsigned short func                 function code
**   values:
**     FSCTL_FUNC_NONE           NULL function
**     FSCTL_FUNC_NEW_INFO       return error code information
**     FSCTL_FUNC_EASIZE         return max EA size and max EA list size
** char far *pParm                     UNVERIFIED pointer to parameter area
** unsigned short lenParm              size of area pointed to by pParm
** unsigned short far *plenParmOut     length of parameters passed in pParm
** char far *pData                     UNVERIFIED pointer to information area
** unsigned short lenData              size of area pointed to by pData
** unsigned short far *plenDataOut     length of parameters passed in pData
**
******************************************************************************/

#pragma argsused
short int far pascal _export _loadds FS_FSCTL(union argdat far *pArgdat,
                                              unsigned short iArgType,
                                              unsigned short func,
                                              char far *pParm,
                                              unsigned short lenParm,
                                              unsigned short far *plenParmOut,
                                              char far *pData,
                                              unsigned short lenData,
                                              unsigned short far *plenDataOut)
{
  int rc;
  char far *p;
  unsigned short oldVPB;
  ULONG phys_list_count;

  switch (func) {
  case FSCTL_FUNC_NONE:
    return NO_ERROR;

  case FSCTL_FUNC_NEW_INFO:
  case FSCTL_FUNC_EASIZE:
    return ERROR_NOT_SUPPORTED;

#ifdef DEBUG
  case FSCTL_FUNC_GETLOGDATA:
    /* Wait on the log semaphore */
    if( (rc=FSH_SEMWAIT(&logbuf_sem, TO_INFINITE)) == ERROR_INTERRUPT )
      return rc;

    /* Check the user buffer and length field */
    if( (rc=FSH_PROBEBUF(PB_OPWRITE, pData, lenData)) != NO_ERROR )
      return rc;
    if( (rc=FSH_PROBEBUF(PB_OPWRITE, plenDataOut, sizeof(unsigned short))) != NO_ERROR )
      return rc;

    /* If no data present, set the semaphore */
    if( logbuf_head == logbuf_tail ) {
      *plenDataOut = 0;
      if((rc=FSH_SEMSET(&logbuf_sem)) == ERROR_INTERRUPT)
	return rc;
      return NO_ERROR;
    }

    /* Copy data */
    p = pData;
    for( ; logbuf_head!=logbuf_tail && p<pData+lenData; logbuf_head=(logbuf_head+1)%LOGBUF_SIZE )
      *p++ = logbuf[logbuf_head];
    *plenDataOut = p-pData;
    if ((rc = FSH_SEMSET(&logbuf_sem)) == ERROR_INTERRUPT)
      return rc;

    return NO_ERROR;
#endif

  case FSCTL_FUNC_READSECTORS: {
    struct vpfsi *pvpfsi;
    struct vpfsd *pvpfsd;
    unsigned short sector_size;

    /* Verify parameter and data areas */
    rc = FSH_PROBEBUF(PB_OPREAD, pParm, sizeof(RWSTRUC));
    if( rc != NO_ERROR )
      return rc;
    rc = FSH_PROBEBUF(PB_OPWRITE, &((RWSTRUC *)pParm)->n_sectors,
		      sizeof(unsigned short));
    if( rc != NO_ERROR )
      return rc;

    rc=FSH_GETVOLPARM(((RWSTRUC *)pParm)->hVPB, &pvpfsi, &pvpfsd);
    if(rc!=NO_ERROR)
      return rc;
    sector_size = pvpfsi->vpi_bsize;

    rc = FSH_PROBEBUF(PB_OPWRITE, pData,
		      ((RWSTRUC *)pParm)->n_sectors * sector_size);
    if( rc != NO_ERROR )
      return rc;
    rc = FSH_PROBEBUF(PB_OPWRITE, plenParmOut, sizeof(unsigned short));
    if( rc != NO_ERROR )
      return rc;
    rc = FSH_PROBEBUF(PB_OPWRITE, plenDataOut, sizeof(unsigned short));
    if( rc != NO_ERROR )
      return rc;
    /* Verify buffer size */
    if( lenData < ((RWSTRUC *)pParm)->n_sectors * sector_size ) {
      *plenDataOut = ((RWSTRUC *)pParm)->n_sectors * sector_size;
      return ERROR_BUFFER_OVERFLOW;
    }

    /* Read sectors */
    rc = FSH_DOVOLIO(DVIO_OPREAD,
		     DVIO_ALLFAIL|DVIO_ALLRETRY|DVIO_ALLABORT,
		     ((RWSTRUC *)pParm)->hVPB,
		     pData,
		     &((RWSTRUC *)pParm)->n_sectors,
		     ((RWSTRUC *)pParm)->starting_sector);
    *plenParmOut = sizeof(unsigned short);
    *plenDataOut = ((RWSTRUC *)pParm)->n_sectors * sector_size;
    return rc;
  }

  case FSCTL_FUNC_WRITESECTORS: {
    struct vpfsi *pvpfsi;
    struct vpfsd *pvpfsd;
    unsigned short sector_size;

    /* Verify parameter and data areas */
    rc = FSH_PROBEBUF(PB_OPREAD, pParm, sizeof(RWSTRUC));
    if( rc != NO_ERROR )
      return rc;
    rc = FSH_PROBEBUF(PB_OPWRITE, &((RWSTRUC *)pParm)->n_sectors,
		      sizeof(unsigned short));
    if( rc != NO_ERROR )
      return rc;

    rc=FSH_GETVOLPARM(((RWSTRUC *)pParm)->hVPB, &pvpfsi, &pvpfsd);
    if(rc!=NO_ERROR)
      return rc;
    sector_size = pvpfsi->vpi_bsize;

    rc = FSH_PROBEBUF(PB_OPREAD, pData,
		      ((RWSTRUC *)pParm)->n_sectors * sector_size);
    if( rc != NO_ERROR )
      return rc;
    rc = FSH_PROBEBUF(PB_OPWRITE, plenParmOut, sizeof(unsigned short));
    if( rc != NO_ERROR )
      return rc;
    rc = FSH_PROBEBUF(PB_OPWRITE, plenDataOut, sizeof(unsigned short));
    if( rc != NO_ERROR )
      return rc;
    /* Verify buffer size */
    if( lenData < ((RWSTRUC *)pParm)->n_sectors * sector_size ) {
      *plenDataOut = ((RWSTRUC *)pParm)->n_sectors * sector_size;
      return ERROR_BUFFER_OVERFLOW;
    }

    /* Write sectors */
    rc = FSH_DOVOLIO(DVIO_OPWRITE,
		     DVIO_ALLFAIL|DVIO_ALLRETRY|DVIO_ALLABORT,
		     ((RWSTRUC *)pParm)->hVPB,
		     pData,
		     &((RWSTRUC *)pParm)->n_sectors,
		     ((RWSTRUC *)pParm)->starting_sector);
    *plenParmOut = sizeof(unsigned short);
    *plenDataOut = ((RWSTRUC *)pParm)->n_sectors * sector_size;
    return rc;
  }

  case FSCTL_FUNC_WRITESECTORS_NOHARDERR: {
    /* Write without hard error signaling */
    struct vpfsi *pvpfsi;
    struct vpfsd *pvpfsd;
    unsigned short sector_size;

    /* Verify parameter and data areas */
    rc = FSH_PROBEBUF(PB_OPREAD, pParm, sizeof(RWSTRUC));
    if( rc != NO_ERROR )
      return rc;
    rc = FSH_PROBEBUF(PB_OPWRITE, &((RWSTRUC *)pParm)->n_sectors,
		      sizeof(unsigned short));
    if( rc != NO_ERROR )
      return rc;

    rc=FSH_GETVOLPARM(((RWSTRUC *)pParm)->hVPB, &pvpfsi, &pvpfsd);
    if(rc!=NO_ERROR)
      return rc;
    sector_size = pvpfsi->vpi_bsize;

    rc = FSH_PROBEBUF(PB_OPREAD, pData,
		      ((RWSTRUC *)pParm)->n_sectors * sector_size);
    if( rc != NO_ERROR )
      return rc;
    rc = FSH_PROBEBUF(PB_OPWRITE, plenParmOut, sizeof(unsigned short));
    if( rc != NO_ERROR )
      return rc;
    rc = FSH_PROBEBUF(PB_OPWRITE, plenDataOut, sizeof(unsigned short));
    if( rc != NO_ERROR )
      return rc;
    /* Verify buffer size */
    if( lenData < ((RWSTRUC *)pParm)->n_sectors * sector_size ) {
      *plenDataOut = ((RWSTRUC *)pParm)->n_sectors * sector_size;
      return ERROR_BUFFER_OVERFLOW;
    }

    /* Write sectors */
    rc = FSH_DOVOLIO(DVIO_OPWRITE | DVIO_OPHARDERR,
		     DVIO_ALLFAIL|DVIO_ALLRETRY|DVIO_ALLABORT,
		     ((RWSTRUC *)pParm)->hVPB,
		     pData,
		     &((RWSTRUC *)pParm)->n_sectors,
		     ((RWSTRUC *)pParm)->starting_sector);
    *plenParmOut = sizeof(unsigned short);
    *plenDataOut = ((RWSTRUC *)pParm)->n_sectors * sector_size;
    return rc;
  }

  case FSCTL_FUNC_FINDDUPHVPB:
    /* Verify parameter area */
    rc = FSH_PROBEBUF(PB_OPREAD, pParm, sizeof(unsigned short));
    if( rc != NO_ERROR )
      return rc;
    rc = FSH_PROBEBUF(PB_OPWRITE, pParm, sizeof(unsigned short));
    if( rc != NO_ERROR )
      return rc;
    rc = FSH_PROBEBUF(PB_OPWRITE, plenParmOut, sizeof(unsigned short));
    if( rc != NO_ERROR )
      return rc;

    /* Call FS helper */
    rc = FSH_FINDDUPHVPB(*(unsigned short *)pParm, &oldVPB);
    *(unsigned short *)pParm = oldVPB;
    *plenParmOut = sizeof(unsigned short);
    return rc;

  case FSCTL_FUNC_GET_MTSTAT: {
    PMTSTATSTRUC mts=(PMTSTATSTRUC)pParm;
    unsigned short i;

    rc = FSH_PROBEBUF(PB_OPREAD, pParm, sizeof(unsigned short));
    if( rc != NO_ERROR )
      return rc;
    rc = FSH_PROBEBUF(PB_OPWRITE, pParm, sizeof(MTSTATSTRUC));
    if( rc != NO_ERROR )
      return rc;
    rc = FSH_PROBEBUF(PB_OPWRITE, plenParmOut, sizeof(unsigned short));
    if( rc != NO_ERROR )
      return rc;

    /* Search the mount list for the given hVPB */
    i=0;
    while(i<MAX_MOUNTSTRUCS && *(unsigned short *)pParm != MountList[i].hVPB)
      i++;
    if(i==MAX_MOUNTSTRUCS)
      return ERROR_INVALID_PARAMETER;

    mts->drive = MountList[i].drive;
    mts->status = MountList[i].status;
    mts->hVPB = MountList[i].hVPB;
    *plenParmOut = sizeof(MTSTATSTRUC);
    return NO_ERROR;
  }

  case FSCTL_FUNC_SET_MTSTAT: {
    PMTSTATSTRUC mts=(PMTSTATSTRUC)pParm;
    unsigned short i;

    rc = FSH_PROBEBUF(PB_OPREAD, pParm, sizeof(MTSTATSTRUC));
    if( rc != NO_ERROR )
      return rc;

    i=0;
    while(i<MAX_MOUNTSTRUCS && MountList[i].hVPB!=mts->hVPB)
      i++;
    if(i==MAX_MOUNTSTRUCS) {
      /* Find a free mount struc */
      i=0;
      while(i<MAX_MOUNTSTRUCS && MountList[i].status!=MTSTAT_FREE)
	i++;
      if(i==MAX_MOUNTSTRUCS)
	return ERROR_BUFFER_OVERFLOW;
    }
    MountList[i].hVPB = mts->hVPB;
    MountList[i].drive = mts->drive;
    MountList[i].status = mts->status;
    return NO_ERROR;
  }

  case FSCTL_FUNC_WILDMATCH:
    rc = FSH_PROBEBUF(PB_OPREAD, pParm, sizeof(WILDSTRUC));
    if( rc != NO_ERROR )
      return rc;

    return FSH_WILDMATCH(((PWILDSTRUC)pParm)->pattern,
			 ((PWILDSTRUC)pParm)->text);

  case FSCTL_FUNC_UPPERCASE:
    rc = FSH_PROBEBUF(PB_OPREAD, pData, lenData);
    if( rc != NO_ERROR )
      return rc;
    rc = FSH_PROBEBUF(PB_OPWRITE, pData, lenData);
    if( rc != NO_ERROR )
      return rc;

    return FSH_UPPERCASE(pData, lenData, pData);

  case FSCTL_FUNC_SETVOLUME:
    /* Verify parameter area */
    rc = FSH_PROBEBUF(PB_OPREAD, pParm, 2*sizeof(unsigned short));
    if( rc != NO_ERROR )
      return rc;
#ifdef DEBUG
    printk("setvolume hVPB=%hu, flag=%hu\n", *(unsigned short *)pParm,
	   *(unsigned short *)(pParm+2));
#endif
    rc = FSH_SETVOLUME(*(unsigned short *)pParm, *(unsigned short *)(pParm+2));
    return rc;

  case FSCTL_FUNC_INIT:
    /* If control program has never been attached, ignore these checks */
    if (AttachedEver) {
      /* Make sure the control program isn't already attached */
      if (CPAttached)
	return 0xef00;

      /* Make sure that it is OK to attach at this time */
      rc = FSH_SEMWAIT(&CPData.BufLock, 0);
      if (rc != NO_ERROR)
	return 0xef01;
    }

    /* Verify size of and addressability of the pParm area */
    if (lenParm != sizeof(INITSTRUC))
      return 0xef02;
    rc = FSH_PROBEBUF(PB_OPREAD, pParm, lenParm);
    if (rc != NO_ERROR)
      return rc;

    /* Lock down the operation buffer */
    rc = DevHelp_VMLock(VMDHL_WRITE | VMDHL_LONG,
			((INITSTRUC *)pParm)->OpData, sizeof(OPDATA), 
			-1L, Lock1, &phys_list_count);
    if (rc != NO_ERROR)
      return rc;

    /* Map the GDT selector we got at init time to the memory we just locked */
    rc = DevHelp_LinToGDTSelector(FP_SEG(CPData.OpData),
				  ((INITSTRUC *)pParm)->OpData, 
				  sizeof(OPDATA));
    if (rc != NO_ERROR)
      return rc;

    /* Lock down the data buffer */
    rc = DevHelp_VMLock(VMDHL_WRITE | VMDHL_LONG,
			((INITSTRUC *)pParm)->Buf, 65536, 
			-1L, Lock2, &phys_list_count);
    if (rc != NO_ERROR)
      return rc;

    /* Map the GDT selector we got at init time to the memory we just locked */
    rc = DevHelp_LinToGDTSelector(FP_SEG(CPData.Buf), 
				  ((INITSTRUC *)pParm)->Buf,
				  65536);
    if (rc != NO_ERROR)
      return rc;
    
    /* The operation data area is now OK to use */
    rc = FSH_SEMCLEAR(&CPData.BufLock);
    if (rc != NO_ERROR)
      return rc;

    /* Save control program's process ID */
    CPPID = ((INITSTRUC *)pParm)->CPPID;

    /* The control program is now attached */
    CPAttached = AttachedEver = 1;

#ifdef DEBUG
    printk("Attached to control program, PID = %lu\n", (unsigned long)CPPID);
#endif

    /* FALL THROUGH */

  case FSCTL_FUNC_NEXT:
    if (!CPAttached)
      return 0xef03;

    /* Set the semaphore that indicates the control program is ready for
       another operation */
    rc = FSH_SEMSET(&CPData.CmdReady);
    if (rc == ERROR_INTERRUPT) {
      DetachCP();
      return rc;
    }

    /* Clear the semaphore that indicates the control program just returned */
    rc = FSH_SEMCLEAR(&CPData.CmdComplete);
    if (rc == ERROR_INTERRUPT) {
      DetachCP();
      return rc;
    }

    /* Wait for the semaphore that indicates another operation is ready */
    rc = FSH_SEMWAIT(&CPData.CmdReady, -1);
    if (rc == ERROR_INTERRUPT)
      DetachCP();

    return rc;

  case FSCTL_FUNC_KILL:
    DetachCP();
    return NO_ERROR;

    /* User functions */
  case HFS_FUNC_GET_RESFORK_SIZE: {
    struct vpfsi *pvpfsi;
    struct vpfsd *pvpfsd;

    rc = PreSetup();
    if (rc != NO_ERROR)
      return rc;

    if(iArgType != FSCTL_ARG_FILEINSTANCE) {
      FSH_SEMCLEAR(&CPData.BufLock);
      return ERROR_INVALID_CATEGORY;
    }

    if(FSH_PROBEBUF(PB_OPWRITE, pParm, sizeof(unsigned long)) != NO_ERROR ||
       FSH_PROBEBUF(PB_OPWRITE, plenParmOut, sizeof(unsigned short)) != NO_ERROR) {
      FSH_SEMCLEAR(&CPData.BufLock);
      return ERROR_INVALID_PARAMETER;
    }

    if(lenParm<sizeof(unsigned long)) {
      FSH_SEMCLEAR(&CPData.BufLock);
      *plenParmOut = sizeof(unsigned long);
      return ERROR_BUFFER_OVERFLOW;
    }

    if (!CPAttached) {
      FSH_SEMCLEAR(&CPData.BufLock);
      return ERROR_NOT_READY;
    }

    CPData.OpData->i.funccode = CPFC_GET_RESFORK_SIZE;
    FSH_GETVOLPARM(pArgdat->sf.psffsi->sfi_hVPB, &pvpfsi, &pvpfsd);
    CPData.OpData->p.get_resfork_size.vpfsd = *pvpfsd;
    CPData.OpData->p.get_resfork_size.sffsi = *pArgdat->sf.psffsi;
    CPData.OpData->p.get_resfork_size.sffsd = *pArgdat->sf.psffsd;

    rc = PostSetup();
    if (rc != NO_ERROR)
      return rc;

    *pvpfsd = CPData.OpData->p.get_resfork_size.vpfsd;
    *pArgdat->sf.psffsi = CPData.OpData->p.get_resfork_size.sffsi;
    *pArgdat->sf.psffsd = CPData.OpData->p.get_resfork_size.sffsd;
    rc = CPData.OpData->i.rc;

    *(unsigned long *)pParm = CPData.OpData->p.get_resfork_size.size;
    *plenParmOut = sizeof(unsigned long);
    FSH_SEMCLEAR(&CPData.BufLock);
    return rc;
  }

  case HFS_FUNC_READ_RESFORK: {
    struct vpfsi *pvpfsi;
    struct vpfsd *pvpfsd;
    PHFS_READ_RESFORK_PARAMS param = (PHFS_READ_RESFORK_PARAMS)pParm;

    rc = PreSetup();
    if (rc != NO_ERROR)
      return rc;

    if(iArgType != FSCTL_ARG_FILEINSTANCE) {
      FSH_SEMCLEAR(&CPData.BufLock);
      return ERROR_INVALID_CATEGORY;
    }

    if(FSH_PROBEBUF(PB_OPREAD, pParm, sizeof(HFS_READ_RESFORK_PARAMS)) != NO_ERROR ||
       FSH_PROBEBUF(PB_OPWRITE, plenDataOut, sizeof(unsigned short)) != NO_ERROR ||
       FSH_PROBEBUF(PB_OPWRITE, pData, param->bytes) != NO_ERROR) {
      FSH_SEMCLEAR(&CPData.BufLock);
      return ERROR_INVALID_PARAMETER;
    }

    if(lenParm<sizeof(unsigned long)) {
      FSH_SEMCLEAR(&CPData.BufLock);
      *plenParmOut = sizeof(unsigned long);
      return ERROR_BUFFER_OVERFLOW;
    }

    if (!CPAttached) {
      FSH_SEMCLEAR(&CPData.BufLock);
      return ERROR_NOT_READY;
    }

    CPData.OpData->i.funccode = CPFC_READ_RESFORK;
    FSH_GETVOLPARM(pArgdat->sf.psffsi->sfi_hVPB, &pvpfsi, &pvpfsd);
    CPData.OpData->p.read_resfork.vpfsd = *pvpfsd;
    CPData.OpData->p.read_resfork.sffsi = *pArgdat->sf.psffsi;
    CPData.OpData->p.read_resfork.sffsd = *pArgdat->sf.psffsd;
    CPData.OpData->p.read_resfork.start = param->start;
    CPData.OpData->p.read_resfork.bytes = param->bytes;

    rc = PostSetup();
    if (rc != NO_ERROR)
      return rc;

    *pvpfsd = CPData.OpData->p.read_resfork.vpfsd;
    *pArgdat->sf.psffsi = CPData.OpData->p.read_resfork.sffsi;
    *pArgdat->sf.psffsd = CPData.OpData->p.read_resfork.sffsd;
    rc = CPData.OpData->i.rc;

    if(CPData.OpData->p.read_resfork.bytes > param->bytes)
      CPData.OpData->p.read_resfork.bytes = param->bytes;

    if(rc==NO_ERROR)
      memcpy(pData, CPData.Buf, CPData.OpData->p.read_resfork.bytes);
    *plenDataOut = CPData.OpData->p.read_resfork.bytes;
    FSH_SEMCLEAR(&CPData.BufLock);
    return rc;
  }

  case HFS_FUNC_SYNC: {
    struct vpfsi *pvpfsi;
    struct vpfsd *pvpfsd;
    int i, drive;

    rc = PreSetup();
    if (rc != NO_ERROR)
      return rc;

    if(iArgType != FSCTL_ARG_CURDIR) {
      FSH_SEMCLEAR(&CPData.BufLock);
      return ERROR_INVALID_CATEGORY;
    }

    if (!CPAttached) {
      FSH_SEMCLEAR(&CPData.BufLock);
      return ERROR_NOT_READY;
    }

    if(pArgdat->cd.pPath[0] < 'A' || pArgdat->cd.pPath[0] > 'Z')
      return ERROR_INVALID_PARAMETER;
    drive = pArgdat->cd.pPath[0]-'A';
    i=0;
    while(i<MAX_MOUNTSTRUCS && (MountList[i].drive!=drive
				|| MountList[i].status!=MTSTAT_MOUNTED))
      i++;
    if(i==MAX_MOUNTSTRUCS)
      return ERROR_NOT_READY;

    CPData.OpData->i.funccode = CPFC_SYNC;
    FSH_GETVOLPARM(MountList[i].hVPB, &pvpfsi, &pvpfsd);
    CPData.OpData->p.sync.vpfsi = *pvpfsi;
    CPData.OpData->p.sync.vpfsd = *pvpfsd;

    rc = PostSetup();
    if (rc != NO_ERROR)
      return rc;

    *pvpfsi = CPData.OpData->p.sync.vpfsi;
    *pvpfsd = CPData.OpData->p.sync.vpfsd;
    rc = CPData.OpData->i.rc;

    FSH_SEMCLEAR(&CPData.BufLock);
    return rc;
  }

  case HFS_FUNC_SYNC_ALL: {
    rc = PreSetup();
    if (rc != NO_ERROR)
      return rc;

    if(iArgType != FSCTL_ARG_NULL) {
      FSH_SEMCLEAR(&CPData.BufLock);
      return ERROR_INVALID_CATEGORY;
    }

    if (!CPAttached) {
      FSH_SEMCLEAR(&CPData.BufLock);
      return ERROR_NOT_READY;
    }

    CPData.OpData->i.funccode = CPFC_SYNC_ALL;

    rc = PostSetup();
    if (rc != NO_ERROR)
      return rc;

    rc = CPData.OpData->i.rc;

    FSH_SEMCLEAR(&CPData.BufLock);
    return rc;
  }
  
  default:
    return ERROR_NOT_SUPPORTED;
  }
}

/******************************************************************************
**
** FS_EXIT - Notify FSD that a process is ending
**
** Parameters
** ---------
** unsigned short uid                   user ID of process
** unsigned short pid                   process ID of process
** unsigned short pdb                   DOS mode process ID of process
**
******************************************************************************/

#pragma argsused
void far pascal _export _loadds FS_EXIT(unsigned short uid, unsigned short pid,
                                        unsigned short pdb)
{
  /* If the control program is exiting, detach ourselves from it */
  if (CPAttached && pid == CPPID)
    DetachCP();
}

/******************************************************************************
**
** DetachCP - Detach the control program from the IFS
**
******************************************************************************/

static void DetachCP(void) {
#ifdef DEBUG
  printk("Detaching from CP\n");
#endif
  CPAttached = 0; /* We leave AttachedEver set */
  DevHelp_VMUnLock(Lock1);
  DevHelp_VMUnLock(Lock2);
  FSH_SEMCLEAR(&CPData.CmdComplete);
  FSH_SEMCLEAR(&CPData.BufLock);
}

/******************************************************************************
**
** PreSetup - Stuff that needs to be done before we setup the buffers for a
**            call
**
******************************************************************************/

int PreSetup(void) {
  int rc;

  /* If control program isn't attached, immediately return */
  if (!CPAttached)
    return ERROR_NOT_READY;

  /* Make sure this thread can change the buffers */
  rc = FSH_SEMREQUEST(&CPData.BufLock, MAXCPRDYWAIT);
  if (rc == ERROR_SEM_TIMEOUT)
    return ERROR_NOT_READY;
  else if (rc != NO_ERROR)
    return rc;

  /* And make sure the control program isn't changing them */
  rc = FSH_SEMWAIT(&CPData.CmdComplete, MAXCPRDYWAIT);
  if (rc == ERROR_SEM_TIMEOUT) {
    FSH_SEMCLEAR(&CPData.BufLock);
    return ERROR_NOT_READY;
  }
  else if (rc != NO_ERROR) {
    FSH_SEMCLEAR(&CPData.BufLock);
    return rc;
  }

  return NO_ERROR;
}


/******************************************************************************
**
** PostSetup - Stuff that needs to be done after we setup the buffers for a
**             call
**
******************************************************************************/

int PostSetup(void) {
  int rc;

  /* Set sem that we'll wait on */
  rc = FSH_SEMSET(&CPData.CmdComplete);
  if (rc != NO_ERROR) {
    FSH_SEMCLEAR(&CPData.CmdComplete);
    FSH_SEMCLEAR(&CPData.BufLock);
    return rc;
  }

  /* Tell the control program to do the operation */
  rc = FSH_SEMCLEAR(&CPData.CmdReady);

  /* Wait until the CP is done */
  rc = FSH_SEMWAIT(&CPData.CmdComplete, MAXCPRESWAIT);
  if (!CPAttached || rc == ERROR_SEM_TIMEOUT) {
    /* If we got a timeout, abort */
    FSH_SEMCLEAR(&CPData.BufLock);
    return ERROR_NOT_READY;
  }
  else if (rc != NO_ERROR) {
    /* Some other error occurred, abort */
    FSH_SEMCLEAR(&CPData.BufLock);
    return rc;
  }

  return NO_ERROR;
}

/* Parse command line */
static int parse_args(char *args)
{
  char *p;
  int rc=0;

  p = args;
  while(*p) {
    while(*p && isspace(*p))
      p++;
    if(!*p)
      break;

    switch(*p) {
    case '/':
      p++;
      if(!*p) {
        rc=1;
	break;
      }
      switch(toupper(*p)) {
      case 'Q':
	p++;
	silent=1;
	break;
      default:
        rc=1;
      }
      break;
    default:
      rc=1;
    }

    if(rc) break;

    if(*p && !isspace(*p))
      return 1;
  }
  return rc;
}
