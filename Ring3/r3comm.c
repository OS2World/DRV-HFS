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

/* r3comm.c
   Ring 3 side of communications
*/

#define INCL_DOSERRORS
#define INCL_DOSMEMMGR
#define INCL_DOSFILEMGR
#define INCL_DOSPROCESS
#define INCL_NOCOMMON
#define INCL_NOPMAPI
#include <os2.h>

#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "libhfs/hfs.h"
#include "fsd.h"
#include "fsdep.h"
#include "../r0r3shar.h"
#include "r3global.h"
#include "logger.h"
#include "log.h"
#include "cache.h"
#include "../banner.h"
#include "names.h"
#include "userfunc.h"

static int parse_args(int, char **);

/* Options */
int sync_always=1;

int main(int argc, char **argv) {
  INITSTRUC initdata;
  unsigned long leninitdata;
  OPDATA *OpData;
  void *Buf;
  unsigned int rc;
  char name[10];
  PTIB ptib;
  PPIB ppib;
#ifdef DEBUG
  TID log_id;
#endif

  printf(banner);

#ifdef DEBUG
  printf("Debug version\n");
#endif

  /* Parse command line */
  if(parse_args(argc, argv)) {
    fprintf(stderr, 
	    "\nUsage: HFS [options]\n"
	    "Options:\n"
	    "  /FN:mode    Select filename translation mode.\n"
	    "              mode must be CP850, 7BIT or 8BIT.\n"
            "  /NOSYNC     Do not sync volumes automatically.\n"
	    );
    return 1;
  }

  /* Initialize the cache */
  init_cache();

  /* Start the logger thread */
#ifdef DEBUG
  log_id = _beginthread(log_thread, NULL, 32768, NULL);
  if( log_id == -1 ) {
    printf("Failed to start logging thread!\n");
    exit(1);
  }
#endif

  /* Allocate the two buffers */
  rc = DosAllocMem((void * *)&OpData, sizeof(initdata.OpData),
                   PAG_READ | PAG_WRITE | PAG_COMMIT);
  if (rc != NO_ERROR) {
    printf("alloc failed - OpData: %d\n", rc);
    exit(1);
  }
  rc = DosAllocMem(&Buf, 65536, PAG_READ | PAG_WRITE | PAG_COMMIT);
  if (rc != NO_ERROR) {
    printf("alloc failed - Buf: %d\n", rc);
    exit(1);
  }

  /* Get info blocks */
  if( (rc=DosGetInfoBlocks(&ptib, &ppib)) != NO_ERROR ) {
    printf("DosGetInfoBlocks failed - rc = %hd\n", rc);
    exit(1);
  }

  /* Set up the initialization data to be passed to the IFS */
  initdata.OpData = OpData;
  initdata.Buf = Buf;
  initdata.CPPID = ppib->pib_ulpid;

  /* We want the FS router to route by IFS name, so copy the name into a
     temporary buffer because the router could destroy the name. */
  strncpy(name, FS_NAME, sizeof(name));

  /* Call the IFS to initialize */
  leninitdata = sizeof(initdata);
  rc = DosFSCtl(NULL, 0, NULL, &initdata, sizeof(initdata), &leninitdata,
                FSCTL_FUNC_INIT, name, -1, FSCTL_FSDNAME);
  if (rc != NO_ERROR) {
    printf("Failed to connect with IFS. Please make sure that\n"
           "the control program is not already running, and that\n"
           "you have the line\n"
           "        DEVICE=d:\\path\\HFS.IFS\n"
           "in your CONFIG.SYS file.\n");
    exit(1);
  }

  /* We returned from the IFS-- that means we have an operation waiting.
     Figure out what it is and route it appropriately. */
  /*  while (_read_kbd(0,0,0) == -1) {*/
  while(1) {
#ifdef DEBUG
    printf("req=%x: ", OpData->i.funccode);
#endif

    switch (OpData->i.funccode) {
    case CPFC_ATTACH:
      OpData->i.rc = FSD_ATTACH(OpData->p.attach.flag, OpData->p.attach.Dev,
				&OpData->p.attach.vpfsd, 
				&OpData->p.attach.cdfsd,
				Buf, &OpData->p.attach.Len);
      break;

    case CPFC_MOUNT:
      OpData->i.rc = FS_MOUNT(OpData->p.mount.flag, &OpData->p.mount.vpfsi,
			      &OpData->p.mount.vpfsd, OpData->p.mount.hVPB,
			      OpData->p.mount.vol_descr, Buf);
      break;

    case CPFC_CANCELLOCKREQUEST:
      OpData->i.rc = FS_CANCELLOCKREQUEST(&OpData->p.cancellockrequest.sffsi,
					  &OpData->p.cancellockrequest.sffsd,
					  &OpData->p.cancellockrequest.LockRange);
      break;
      
    case CPFC_CHDIR:
      OpData->i.rc = FS_CHDIR(&OpData->p.chdir.vpfsd, 
			      OpData->p.chdir.flag,
			      &OpData->p.chdir.cdfsi,
			      &OpData->p.chdir.cdfsd,
			      OpData->p.chdir.Dir,
			      OpData->p.chdir.iCurDirEnd);
      break;

    case CPFC_CHGFILEPTR:
      OpData->i.rc = FS_CHGFILEPTR(&OpData->p.chgfileptr.vpfsd,
				   &OpData->p.chgfileptr.sffsi,
				   &OpData->p.chgfileptr.sffsd,
				   OpData->p.chgfileptr.offset,
				   OpData->p.chgfileptr.type,
				   OpData->p.chgfileptr.IOflag);
      break;

    case CPFC_CLOSE:
      OpData->i.rc = FS_CLOSE(&OpData->p.close.vpfsd,
			      OpData->p.close.type,
			      OpData->p.close.IOflag,
			      &OpData->p.close.sffsi,
			      &OpData->p.close.sffsd);
      break;

    case CPFC_COMMIT:
      OpData->i.rc = FS_COMMIT(&OpData->p.commit.vpfsd,
			       OpData->p.commit.type,
			       OpData->p.commit.IOflag,
			       &OpData->p.commit.sffsi,
			       &OpData->p.commit.sffsd);
      break;
      
    case CPFC_COPY:
      OpData->i.rc = FS_COPY(&OpData->p.copy.vpfsd,
			     OpData->p.copy.flag,
			     &OpData->p.copy.cdfsi,
			     &OpData->p.copy.cdfsd,
			     OpData->p.copy.Src,
			     OpData->p.copy.iSrcCurDirEnd,
			     OpData->p.copy.Dst,
			     OpData->p.copy.iDstCurDirEnd,
			     OpData->p.copy.nameType);
      break;

    case CPFC_DELETE:
      OpData->i.rc = FS_DELETE(&OpData->p.fsdelete.vpfsd,
			       &OpData->p.fsdelete.cdfsi,
			       &OpData->p.fsdelete.cdfsd,
			       OpData->p.fsdelete.File,
			       OpData->p.fsdelete.iCurDirEnd);
      break;

    case CPFC_FILEATTRIBUTE:
      OpData->i.rc = FS_FILEATTRIBUTE(&OpData->p.fileattribute.vpfsd,
				      OpData->p.fileattribute.flag,
				      &OpData->p.fileattribute.cdfsi,
				      &OpData->p.fileattribute.cdfsd,
				      OpData->p.fileattribute.Name,
				      OpData->p.fileattribute.iCurDirEnd,
				      &OpData->p.fileattribute.Attr);
      break;

    case CPFC_FILEINFO:
      OpData->i.rc = FS_FILEINFO(&OpData->p.fileinfo.vpfsd,
				 OpData->p.fileinfo.flag,
				 &OpData->p.fileinfo.sffsi,
				 &OpData->p.fileinfo.sffsd,
				 OpData->p.fileinfo.level,
				 (char *)Buf,
				 OpData->p.fileinfo.cbData,
				 OpData->p.fileinfo.IOflag,
				 &OpData->p.fileinfo.oError);
      break;

    case CPFC_FILELOCKS:
      OpData->i.rc = FS_FILELOCKS(&OpData->p.filelocks.sffsi,
                                  &OpData->p.filelocks.sffsd,
                                  &OpData->p.filelocks.UnLockRange,
                                  &OpData->p.filelocks.LockRange,
                                  OpData->p.filelocks.timeout,
                                  OpData->p.filelocks.flags);
      break;

    case CPFC_FINDCLOSE:
      OpData->i.rc = FS_FINDCLOSE(&OpData->p.findclose.vpfsd,
				  &OpData->p.findclose.fsfsi,
				  &OpData->p.findclose.fsfsd);
      break;

    case CPFC_FINDFIRST:
      OpData->i.rc = FS_FINDFIRST(&OpData->p.findfirst.vpfsd,
                                  &OpData->p.findfirst.cdfsi,
                                  &OpData->p.findfirst.cdfsd,
                                  OpData->p.findfirst.Name,
                                  OpData->p.findfirst.iCurDirEnd,
                                  OpData->p.findfirst.attr,
                                  &OpData->p.findfirst.fsfsi,
                                  &OpData->p.findfirst.fsfsd,
                                  (char *)Buf,
                                  OpData->p.findfirst.cbData,
                                  &OpData->p.findfirst.cMatch,
                                  OpData->p.findfirst.level,
                                  OpData->p.findfirst.flags,
                                  &OpData->p.findfirst.oError);
      break;

    case CPFC_FINDFROMNAME:
      OpData->i.rc = FS_FINDFROMNAME(&OpData->p.findfromname.vpfsd,
                                     &OpData->p.findfromname.fsfsi,
                                     &OpData->p.findfromname.fsfsd,
                                     (char *)Buf,
                                     OpData->p.findfromname.cbData,
                                     &OpData->p.findfromname.cMatch,
                                     OpData->p.findfromname.level,
                                     OpData->p.findfromname.position,
                                     OpData->p.findfromname.Name,
                                     OpData->p.findfromname.flags,
                                     &OpData->p.findfirst.oError);
      break;

    case CPFC_FINDNEXT:
      OpData->i.rc = FS_FINDNEXT(&OpData->p.findnext.vpfsd,
                                 &OpData->p.findnext.fsfsi,
                                 &OpData->p.findnext.fsfsd,
                                 (char *)Buf,
                                 OpData->p.findnext.cbData,
                                 &OpData->p.findnext.cMatch,
                                 OpData->p.findnext.level,
                                 OpData->p.findnext.flags,
                                 &OpData->p.findfirst.oError);
      break;

    case CPFC_FLUSHBUF:
      OpData->i.rc = FS_FLUSHBUF(&OpData->p.flushbuf.vpfsd,
                                 OpData->p.flushbuf.flag);
      break;

    case CPFC_FSINFO:
      OpData->i.rc = FS_FSINFO(OpData->p.fsinfo.flag,
                               OpData->p.fsinfo.hVPB,
                               &OpData->p.fsinfo.vpfsi,
                               &OpData->p.fsinfo.vpfsd,
                               (char *)Buf,
                               OpData->p.fsinfo.cbData,
                               OpData->p.fsinfo.level);
      break;

    case CPFC_MKDIR:
      OpData->i.rc = FS_MKDIR(&OpData->p.mkdir.vpfsd,
			      &OpData->p.mkdir.cdfsi,
			      &OpData->p.mkdir.cdfsd,
			      OpData->p.mkdir.Name,
			      OpData->p.mkdir.iCurDirEnd,
			      (char *)Buf,
			      OpData->p.mkdir.flags,
			      &OpData->p.mkdir.oError);
      break;

    case CPFC_MOVE:
      OpData->i.rc = FS_MOVE(&OpData->p.move.vpfsd,
			     &OpData->p.move.cdfsi,
			     &OpData->p.move.cdfsd,
			     OpData->p.move.Src,
			     OpData->p.move.iSrcCurDirEnd,
			     OpData->p.move.Dst,
			     OpData->p.move.iDstCurDirEnd,
			     OpData->p.move.flags);
      break;

    case CPFC_NEWSIZE:
      OpData->i.rc = FS_NEWSIZE(&OpData->p.newsize.vpfsd,
				&OpData->p.newsize.sffsi,
				&OpData->p.newsize.sffsd,
				OpData->p.newsize.len,
				OpData->p.newsize.IOflag);
      break;

    case CPFC_OPENCREATE:
      OpData->i.rc = FS_OPENCREATE(&OpData->p.opencreate.vpfsd,
				   &OpData->p.opencreate.cdfsi,
				   &OpData->p.opencreate.cdfsd,
				   OpData->p.opencreate.Name,
				   OpData->p.opencreate.iCurDirEnd,
				   &OpData->p.opencreate.sffsi,
				   &OpData->p.opencreate.sffsd,
				   OpData->p.opencreate.ulOpenMode,
				   OpData->p.opencreate.usOpenFlag,
				   &OpData->p.opencreate.usAction,
				   OpData->p.opencreate.usAttr,
				   (char *)Buf,
				   &OpData->p.opencreate.fgenflag,
				   &OpData->p.opencreate.oError);
      break;

    case CPFC_PATHINFO:
      OpData->i.rc = FS_PATHINFO(&OpData->p.pathinfo.vpfsd,
				 OpData->p.pathinfo.flag,
				 &OpData->p.pathinfo.cdfsi,
				 &OpData->p.pathinfo.cdfsd,
				 OpData->p.pathinfo.Name,
				 OpData->p.pathinfo.iCurDirEnd,
				 OpData->p.pathinfo.level,
				 (char *)Buf,
				 OpData->p.pathinfo.cbData,
				 &OpData->p.pathinfo.oError);
      break;

    case CPFC_PROCESSNAME:
      OpData->i.rc = FS_PROCESSNAME(OpData->p.processname.NameBuf);
      break;

    case CPFC_READ:
      OpData->i.rc = FS_READ(&OpData->p.read.vpfsd,
			     &OpData->p.read.sffsi,
			     &OpData->p.read.sffsd,
			     (char *)Buf,
			     &OpData->p.read.Len,
			     OpData->p.read.IOflag);
      break;

    case CPFC_RMDIR:
      OpData->i.rc = FS_RMDIR(&OpData->p.rmdir.vpfsd,
			      &OpData->p.rmdir.cdfsi,
			      &OpData->p.rmdir.cdfsd,
			      OpData->p.rmdir.Name,
			      OpData->p.rmdir.iCurDirEnd);
      break;

    case CPFC_VERIFYUNCNAME:
      OpData->i.rc = FS_VERIFYUNCNAME(OpData->p.verifyuncname.flag,
                                      OpData->p.verifyuncname.Name);
      break;

    case CPFC_WRITE:
      OpData->i.rc = FS_WRITE(&OpData->p.write.vpfsd,
			      &OpData->p.write.sffsi,
			      &OpData->p.write.sffsd,
			      (char *)Buf,
			      &OpData->p.write.Len,
			      OpData->p.write.IOflag);
      break;

      /* User functions */
    case CPFC_GET_RESFORK_SIZE:
      OpData->i.rc = hfs_resfork_size(&OpData->p.get_resfork_size.sffsi,
				      &OpData->p.get_resfork_size.sffsd,
				      &OpData->p.get_resfork_size.size);
      break;
	
    case CPFC_READ_RESFORK:
      OpData->i.rc = hfs_read_resfork(&OpData->p.read_resfork.vpfsd,
				      &OpData->p.read_resfork.sffsi,
				      &OpData->p.read_resfork.sffsd,
				      OpData->p.read_resfork.start,
				      &OpData->p.read_resfork.bytes,
				      Buf);
      break;

    case CPFC_SYNC:
      OpData->i.rc = hfs_sync(&OpData->p.sync.vpfsd);
      break;

    case CPFC_SYNC_ALL:
      OpData->i.rc = hfs_sync_all();
      break;

    default:
#ifdef DEBUG
      printf("Unknown IFS function code: %d\n", OpData->i.funccode);
#endif
      exit(-1);
      /*        OpData->i.rc = ERROR_NOT_SUPPORTED;*/
      break;
    }
    
    /* Flush all volumes if /SYNC option was given */
    if(sync_always)
      hfs_flushall();

    /* We want the FS router to route by IFS name, so copy the name into a
       temporary buffer because the router could destroy the name. */
    strncpy(name, FS_NAME, sizeof(name));

    /* Say we're done with this one, and wait for next one. */
    rc = DosFSCtl(NULL, 0, NULL, NULL, 0, NULL, FSCTL_FUNC_NEXT, name,
                  -1, FSCTL_FSDNAME);
    if (rc != NO_ERROR) {
      printf("Fatal error: FSCtl failed - next: %d\n", rc);
      exit(1);
    }
  }
  return 0;
}

static int parse_args(int argc, char *argv[])
{
  int i;
  for(i=1; i<argc; i++) {
    if(argv[i][0]=='/' || argv[i][0]=='-') {
      /* Switch */
      strupr(argv[i]);
      if(strstr(argv[i], "/FN:")==argv[i]) {
	char *p=argv[i]+4;
	if(!strcmp(p, "CP850"))
	  filename_translation=FN_TR_CP850;
	else if(!strcmp(p, "7BIT"))
	  filename_translation=FN_TR_7BIT;
	else if(!strcmp(p, "8BIT"))
	  filename_translation=FN_TR_8BIT;
	else {
	  fprintf(stderr, "Invalid filename translation mode %s\n", p);
	  return 1;
	}
      }
      else if(!strcmp(argv[i], "/NOSYNC")) {
	sync_always = 0;
      }
      else {
        fprintf(stderr, "Invalid option %s\n", argv[i]);
        return 1;
      }
    }
    else {
      /* Unrecognized argument */
      fprintf(stderr, "Invalid argument %s\n", argv[i]);
      return 1;
    }
  }

  return 0;
}
