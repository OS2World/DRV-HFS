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

/* sync.c
   Send a sync command to the control program.
*/

#define INCL_NOPMAPI
#define INCL_DOSFILEMGR
#define INCL_DOSERRORS
#include <os2.h>

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "../hfsctl.h"

#define FSID  0x32534648        /* File system ID, 'HFS2' */

static int parse_args(short argc, char *argv[]);

/* The drive given on the command line */
char drive_str[] = "A:\\";
int drive;
int got_drive;

int main(int argc, char *argv[])
{
  APIRET rc;

  if(parse_args(argc, argv)) {
    printf("\nsync writes any buffered data to the specified volume.\n");
    printf("Usage: sync [d:]\n");
    printf("where d: is the drive to sync. The default is A:\n");
    return 1;
  }

  if(got_drive) {
    /* Sync a specified drive */
    FSALLOCATE fsalloc;

    /* Check that it is HFS */
    if((rc=DosQueryFSInfo(drive+1, FSIL_ALLOC, &fsalloc, sizeof(fsalloc)))
       != NO_ERROR) {
      printf("Cannot read file system information.\n");
      return 1;
    }
    if(fsalloc.idFileSystem != FSID) {
      printf("Not an HFS volume!\n");
      return 1;
    }

    /* Sync the drive */
    if((rc=DosFSCtl(NULL, 0, NULL,
		    /*		    &drive, sizeof(char), &lenparm,*/
		    NULL, 0, NULL,
		    HFS_FUNC_SYNC, drive_str, -1, FSCTL_PATHNAME)) 
       != NO_ERROR) {
      printf("Operation failed, rc = %ld.\n", rc);
      return 1;
    }
  }
  else {
    /* Sync all drives */
    if((rc=DosFSCtl(NULL, 0, NULL,
		    NULL, 0, NULL,
		    HFS_FUNC_SYNC_ALL, "HFS", -1, FSCTL_FSDNAME)) 
       != NO_ERROR) {
      printf("Operation failed, rc = %ld.\n", rc);
      return 1;
    }
  }

  printf("The media can now safely be removed.\n");
  return 0;
}

static int parse_args(short argc, char *argv[])
{
  int i;
  for(i=1; i<argc; i++) {
    if(argv[i][0]=='/' || argv[i][0]=='-') {
      /* Switch */
      strupr(argv[i]);
      /* Check for switches here - none supported */
      fprintf(stderr, "Invalid option %s\n", argv[i]);
      return 1;
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
      drive = drive_str[0]-'A';
      got_drive = 1;   
    }
    else {
      /* Unrecognized argument */
      fprintf(stderr, "Invalid argument %s\n", argv[i]);
      return 1;
    }
  }

  /* Check that we got all necessary arguments */

  return 0;
}
