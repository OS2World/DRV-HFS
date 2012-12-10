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

/* prepare.c
   Prepares a diskette for use with HFS/2.
*/

#define INCL_DOSFILEMGR
#define INCL_DOSERRORS
#define INCL_DOSDEVIOCTL
#define INCL_DOSDEVICES
#define INCL_NOPMAPI
#define INCL_NOCOMMON
#include <os2.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "diskstruc.h"
#include "names.h"
#include "prepare.h"

static int parse_args(short, char **);
static int do_prepare(void);
static int do_restore(void);

char drive_str[] = "A:";
int prepare=0, restore=0;

#define HFS_MAGIC 0x4442

#define swapl(x) ( (x)<<24 | ((x) & 0xff00) << 8 | \
		   ((x) & 0xff0000) >> 8 | (x)>>24 )

int main(int argc, char *argv[])
{
  int rc;

  if(parse_args(argc, argv)) {
    fprintf(stderr, "\nThis program prepares a Macintosh diskette for use with HFS/2.\n"\
	    "Usage: prepare {/P | /R} [d:]\nwhere\n"\
	    "  /P prepares the diskette,\n"\
	    "  /R restores a previously prepared diskette, \n"\
	    "  d: is the drive to use. The default drive is A:.\n");
    return 1;
  }

  if(prepare)
    rc=do_prepare();
  else
    rc=do_restore();

  return rc;
}

static int parse_args(short argc, char *argv[])
{
  int i, got_drive=0;
  for(i=1; i<argc; i++) {
    if(argv[i][0]=='/' || argv[i][0]=='-') {
      /* Switch */
      strupr(argv[i]);
      if(!strcmp(argv[i], "/P")) {
	if(restore) {
	  fprintf(stderr, "Unexpected option %s\n", argv[i]);
	  return 1;
	}
	prepare=1;
      }
      else if(!strcmp(argv[i], "/R")) {
	if(prepare) {
	  fprintf(stderr, "Unexpected option %s\n", argv[i]);
	  return 1;
	}
	restore=1;
      }
      else {
        fprintf(stderr, "Invalid option %s\n", argv[i]);
        return 1;
      }
    }
    else if(strlen(argv[i])==2 && argv[i][1]==':') {
      /* Drive letter */
      if(got_drive) {
        fprintf(stderr, "Unexpected drive letter %s\n", argv[i]);
        return 1;
      }
      if(!isalpha(argv[i][0])) {
        fprintf(stderr, "Invalid drive letter %s\n", argv[i]);
        return 1;
      }
      drive_str[0] = toupper(argv[i][0]);
      got_drive = 1;   
    }
    else {
      /* Unrecognized argument */
      fprintf(stderr, "Invalid argument %s\n", argv[i]);
      return 1;
    }
  }

  /* Check that we got all necessary arguments */
  if(!prepare && !restore) {
    fprintf(stderr, "Run prepare with the /P option "\
	    "to prepare a diskette for use with HFS/2.\n");
    return 1;
  }

  return 0;
}

static int do_prepare(void)
{
  HFILE hfFileHandle;
  ULONG ulAction;

  if(DosOpen(drive_str, &hfFileHandle, &ulAction, 0, FILE_NORMAL, 
	     OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
	     OPEN_FLAGS_DASD | OPEN_FLAGS_NOINHERIT | OPEN_SHARE_DENYNONE |
	     OPEN_ACCESS_READWRITE, 0L) == NO_ERROR) {
    /* Lock the drive */
    UCHAR ch1=0, ch2=0;
    ULONG param_len, data_len;
    if(DosDevIOCtl(hfFileHandle, IOCTL_DISK, DSK_LOCKDRIVE, &ch1, sizeof(UCHAR),
		   &param_len, &ch2, sizeof(UCHAR), &data_len) == NO_ERROR) {
      char name[]="HFS";
      if(DosDevIOCtl(hfFileHandle, IOCTL_DISK, DSK_BEGINFORMAT, name, strlen(name)+1,
		     &param_len, &ch2, sizeof(UCHAR), &data_len) == NO_ERROR) {
	/* Read the Master Directory Block */
	ULONG offset, bytes;
	MDB mdb;

	DosSetFilePtr(hfFileHandle, 512*2, FILE_BEGIN, &offset);
	if(DosRead(hfFileHandle, (PVOID)&mdb, sizeof(MDB), &bytes) 
	   == NO_ERROR) {
	  if(mdb.drSigWord==HFS_MAGIC) {
	    char buf[512], filename[20];
	    HFILE hfDataFile;
	    unsigned long serial = swapl(mdb.drCrDate);
	    
	    /* Open the volume data file */
	    sprintf(filename, "%08lX", serial);
	    if(DosOpen(filename, &hfDataFile, &ulAction, 0, FILE_NORMAL,
		       OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_FAIL_IF_EXISTS,
		       OPEN_FLAGS_NOINHERIT | OPEN_SHARE_DENYNONE | 
		       OPEN_ACCESS_READWRITE, 0L) == NO_ERROR) {
	      /* Read the boot sector */
	      DosSetFilePtr(hfFileHandle, 0, FILE_BEGIN, &offset);
	      if(DosRead(hfFileHandle, (PVOID)buf, sizeof(buf), &bytes)==NO_ERROR) {
		/* Save the boot sector */
		if(DosWrite(hfDataFile, (PVOID)buf, 512, &bytes)==NO_ERROR) {
		  APIRET rc;
		  PEXTENDED_BOOT boot = (PEXTENDED_BOOT)buf;
		  unsigned char maclabel[30], label[CCHMAXPATH];
		  
		  DosClose(hfDataFile);
		  memcpy(buf, FAT_boot_sect, 512);
		  strncpy(boot->Boot_System_ID, "HFS     ", 8);
		  *(unsigned long *)&boot->Boot_Serial = serial;
		  
		  strncpy(maclabel, mdb.drVN, mdb.drVNlen);
		  maclabel[(int)mdb.drVNlen]=0;
		  mac_to_os2_name(maclabel, label);
		  memcpy(boot->Boot_Vol_Label, label, 11);
		  if(strlen(label)<11)
		    memset(boot->Boot_Vol_Label+strlen(label), ' ', 11-mdb.drVNlen);
		  label[11]=0;

		  /* Write the boot sector */
		  DosSetFilePtr(hfFileHandle, 0, FILE_BEGIN, &offset);
		  if((rc=DosWrite(hfFileHandle, (PVOID)buf, 512, &bytes))==NO_ERROR) {
		    fprintf(stderr, "Prepared volume '%s', serial number %08lX.\n",
			    label, serial);
		  }
		  else
		    fprintf(stderr, "Error writing volume, rc = %ld.\n", rc);
		}
		else
		  fprintf(stderr, "Cannot write volume data file.\n");
	      }
	      else
		fprintf(stderr, "Error reading volume.\n");
	      DosClose(hfDataFile);
	    }
	    else
	      fprintf(stderr, "Cannot create volume data file %s.\n", filename);
	  }
	  else
	    fprintf(stderr, "Cannot prepare non-HFS volume.\n");
	}
	else
	  fprintf(stderr, "Error reading volume.\n");
	DosDevIOCtl(hfFileHandle, IOCTL_DISK, DSK_REDETERMINEMEDIA, &ch1, sizeof(UCHAR),
		    &param_len, &ch2, sizeof(UCHAR), &data_len);
      }
      else
	fprintf(stderr, "Cannot initiate format.\n");
      DosDevIOCtl(hfFileHandle, IOCTL_DISK, DSK_UNLOCKDRIVE, &ch1, sizeof(UCHAR),
		  &param_len, &ch2, sizeof(UCHAR), &data_len);
    }
    else
      fprintf(stderr, "Cannot lock drive.\n");
    DosClose(hfFileHandle);
  }
  else
    fprintf(stderr, "Cannot access volume.\n");
  return 0;
}

static int do_restore(void)
{
  HFILE hfFileHandle;
  ULONG ulAction;

  if(DosOpen(drive_str, &hfFileHandle, &ulAction, 0, FILE_NORMAL, 
	     OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
	     OPEN_FLAGS_DASD | OPEN_FLAGS_NOINHERIT | OPEN_SHARE_DENYNONE |
	     OPEN_ACCESS_READWRITE, 0L) == NO_ERROR) {
    /* Lock the drive */
    UCHAR ch1=0, ch2=0;
    ULONG param_len, data_len;
    if(DosDevIOCtl(hfFileHandle, IOCTL_DISK, DSK_LOCKDRIVE, &ch1, sizeof(UCHAR),
		   &param_len, &ch2, sizeof(UCHAR), &data_len) == NO_ERROR) {
//      if(DosDevIOCtl(hfFileHandle, IOCTL_DISK, DSK_BEGINFORMAT, "", 0,
//		     &param_len, &ch2, sizeof(UCHAR), &data_len) == NO_ERROR) {
	/* Read the Master Directory Block */
	ULONG offset, bytes;
	MDB mdb;

	DosSetFilePtr(hfFileHandle, 512*2, FILE_BEGIN, &offset);
	if(DosRead(hfFileHandle, (PVOID)&mdb, sizeof(MDB), &bytes) 
	   == NO_ERROR) {
	  if(mdb.drSigWord==HFS_MAGIC) {
	    char buf[512], filename[20];
	    HFILE hfDataFile;
	    unsigned long serial = swapl(mdb.drCrDate);
	    
	    /* Open the volume data file if it exists*/
	    sprintf(filename, "%08lX", serial);
	    if(DosOpen(filename, &hfDataFile, &ulAction, 0, FILE_NORMAL,
		       OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
		       OPEN_FLAGS_NOINHERIT | OPEN_SHARE_DENYNONE | 
		       OPEN_ACCESS_READWRITE, 0L) == NO_ERROR) {
	      printf("Restoring from saved data file %s\n", filename);
	      /* Read the boot sector from the volume data file */
	      if(DosRead(hfDataFile, (PVOID)buf, 512, &bytes)!=NO_ERROR) {
		fprintf(stderr, "Error reading data file. Boot sector will be cleared instead.\n");
		memset(buf, 0, 512);
	      }
	      DosClose(hfDataFile);
	    }
	    else {
	      fprintf(stderr, "Volume data file not found. "
		      "Boot sector will be cleared.\n");
	      memset(buf, 0, 512);
	    }

	    /* Write the boot sector */
	    DosSetFilePtr(hfFileHandle, 0, FILE_BEGIN, &offset);
	    if(DosWrite(hfFileHandle, (PVOID)buf, 512, &bytes)==NO_ERROR) 
	      fprintf(stderr, "Operation succeeded.\n");
	    else
	      fprintf(stderr, "Error writing volume.\n");
	  }
	  else
	    fprintf(stderr, "Cannot prepare non-HFS volume.\n");
	}
	else
	  fprintf(stderr, "Error reading volume.\n");
	DosDevIOCtl(hfFileHandle, IOCTL_DISK, DSK_REDETERMINEMEDIA, &ch1, sizeof(UCHAR),
		    &param_len, &ch2, sizeof(UCHAR), &data_len);
//      }
//      else
//	fprintf(stderr, "Cannot initiate format.\n");
//      DosDevIOCtl(hfFileHandle, IOCTL_DISK, DSK_UNLOCKDRIVE, &ch1, sizeof(UCHAR),
//		  &param_len, &ch2, sizeof(UCHAR), &data_len);
    }
    else
      fprintf(stderr, "Cannot lock drive.\n");
    DosClose(hfFileHandle);
  }
  else
    fprintf(stderr, "Cannot access volume.\n");
  return 0;
}

