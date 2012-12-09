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

/* format.c
   The FORMAT command.
*/

#define INCL_NOPMAPI
#define INCL_DOSERRORS
#define INCL_DOSDEVICES
#define INCL_DOSDEVIOCTL
#include <os2.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "uhfs.h"
#include "misc.h"
#include "libhfs/hfs.h"

int parse_args(short argc, char *argv[]);
void wait_enter(void);
void read_stdin(char *, int);
int do_format(void);

char drive_str[] = "A:";
char volume_label[HFS_MAX_VLEN+1] = "Untitled";

#pragma argsused
int far pascal _loadds FORMAT(short argc, char **argv, char **envp)
{
  char answer;

  if(parse_args(argc, argv))
    return 1;

  do {
    print("Insert a new diskette in drive %s and press Enter. ", drive_str);
    wait_enter();
    
    print("Formatting drive %s\r\n", drive_str);
    do_format();

    print("\r\nFormat another? (Y/N) ");
    read_stdin(&answer, 1);
  } while( answer == 'Y' || answer == 'y' );
  
  return 0;
}

/* Format the drive */
int do_format(void)
{
  HFILE hf;
  USHORT action;
  APIRET rc;

  /* Open the drive */
  if((rc=DosOpen(drive_str, &hf, &action, 0, 0, 
		 OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
		 OPEN_FLAGS_DASD | OPEN_SHARE_DENYREADWRITE | 
		 OPEN_ACCESS_READWRITE, 0)) == NO_ERROR) {
    /* Get drive parameters */
    char param[2] = {GET_BPB_FROM_DEVICE, 0};
    BIOSPARAMETERBLOCK bpb;
    
    if((rc=DosDevIOCtl(&bpb, param, DSK_GETDEVICEPARAMS, IOCTL_DISK, hf))
       == NO_ERROR) {
      char param[1] = {0}, data[1] = {0};
      unsigned long total_sectors;
      
      /*
      print("Sectors: %hu\r\n", bpb.cSectors);
      print("Large sectors: %lu\r\n", bpb.cLargeSectors);
      print("Cylinders: %hu\r\n", bpb.cCylinders);
      print("Media type: %d\r\n", bpb.bDeviceType);
      print("Removable: %s\r\n", bpb.fsDeviceAttr & 1 ? "no" : "yes");
      */

      total_sectors = bpb.cSectors;
      if(total_sectors == 0)
	total_sectors = bpb.cLargeSectors;

      if(total_sectors >= 800*1024L/HFS_BLOCKSZ) {
	/* Lock drive */
	if((rc=DosDevIOCtl(data, param, DSK_LOCKDRIVE, IOCTL_DISK, hf))
	   == NO_ERROR) {
	  char param[]="HFS", data[1]={0};
	  
	  /* Begin format */
	  if((rc=DosDevIOCtl(data, param, DSK_BEGINFORMAT, IOCTL_DISK, hf))
	     == NO_ERROR) {
	    hfs_format(hf, total_sectors, 0, volume_label);
	    
	    param[0]=0;
	    DosDevIOCtl(data, param, DSK_REDETERMINEMEDIA, IOCTL_DISK, hf);
	  }
	  else
	    print("Begin format failed, rc=%hu.\r\n", rc);
	  
	  /* Unlock drive */
	  DosDevIOCtl(data, param, DSK_UNLOCKDRIVE, IOCTL_DISK, hf);
	}
	else
	  print("Failed to lock drive, rc=%hu\r\n", rc);
      }
      else
	print("Volume must be at least 800K.\r\n");
    }
    else
      print("Failed to get device params, rc=%hu\r\n", rc);
    
    /* Close drive */
    DosClose(hf);
  }
  else
    print("Cannot open drive, rc=%hu.\r\n", rc);
  
  return rc;
}

/* Wait for the user to press Enter */
void wait_enter(void)
{
  USHORT bytes;
  static char buf[20];

  do
    DosRead(0, buf, sizeof(buf), &bytes);
  while(bytes == sizeof(buf));
}

/* Read input from keyboard */
void read_stdin(char *buf, int maxlen)
{
  USHORT bytes;

  DosRead(0, buf, maxlen, &bytes);
  if(bytes==maxlen)
    wait_enter();
}

/* Parse command line arguments */
int parse_args(short argc, char *argv[])
{
  int i, got_drive=0;
  for(i=1; i<argc; i++) {
    if(argv[i][0]=='/' || argv[i][0]=='-') {
      /* Switch */
      strupr(argv[i]);
      if(strstr(argv[i], "/FS:")==argv[i]) {
	/* ignored */
      }
      else if(strstr(argv[i], "/V:")==argv[i]) {
	if(strlen(argv[i]) == 3) {
	  if(i+1<argc && argv[i+1][0]!='/' && argv[i+1][0]!='-')
	    strncpy(volume_label, argv[i+1], HFS_MAX_VLEN);
	}
	else
	  strncpy(volume_label, argv[i]+3, HFS_MAX_VLEN);
      }
      else if(!strcmp(argv[i], "/V")) {
	/* ignored */
      }
      else {
        print("Invalid option %s\r\n", argv[i]);
        return 1;
      }
    }
    else if(strlen(argv[i])==2 && argv[i][1]==':') {
      /* Drive letter */
      if(got_drive) {
        print("Unexpected drive letter %s\r\n", argv[i]);
        return 1;
      }
      if(!isalpha(argv[i][0])) {
        print("Invalid drive letter %s\r\n", argv[i]);
        return 1;
      }
      drive_str[0] = toupper(argv[i][0]);
      got_drive = 1;   
    }
    else {
      /* Unrecognized argument */
      print("Invalid argument %s\r\n", argv[i]);
      return 1;
    }
  }

  /* Check that we got all necessary arguments */
  if(!got_drive) {
    print("No drive specified.\r\n");
    return 1;
  }

  return 0;
}
