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

/* readres.c
   Utility to read the resource fork of a file.

   Usage: readres macfile resfile
*/

#define INCL_NOPMAPI
#define INCL_NOCOMMON
#define INCL_DOSERRORS
#define INCL_DOSFILEMGR
#include <os2.h>

#include <stdio.h>
#include <stdlib.h>

#include "../hfsctl.h"

int main(int argc, char *argv[])
{
  HFILE macfile;
  FILE *resfile;
  APIRET rc;
  ULONG action, resfork_size, lenparm, lendata;
  char *buf;
  HFS_READ_RESFORK_PARAMS read_params;

  if(argc != 3) {
    fprintf(stderr, "Usage: readres macfile resfile\n");
    return 1;
  }

  rc = DosOpen(argv[1], &macfile, &action, 0, 0,
	       OPEN_ACTION_OPEN_IF_EXISTS,
	       OPEN_SHARE_DENYWRITE | OPEN_ACCESS_READONLY, NULL);
  if(rc!=NO_ERROR) {
    fprintf(stderr, "Cannot open %s\n", argv[1]);
    return 1;
  }

  resfile=fopen(argv[2], "wb");
  if(!resfile) {
    fprintf(stderr, "Cannot open %s\n", argv[2]);
    return 1;
  }

  lenparm=0;
  rc=DosFSCtl(NULL, 0, NULL, &resfork_size, sizeof(ULONG), &lenparm,
	      HFS_FUNC_GET_RESFORK_SIZE, NULL, macfile, FSCTL_HANDLE);
  if(rc!=NO_ERROR) {
    fprintf(stderr, "Error reading resource fork.\n");
    return 1;
  }

  if((buf=(char *)malloc(resfork_size))==NULL) {
    fprintf(stderr, "Cannot allocate memory.\n");
    return 1;
  }

  read_params.start = 0;
  read_params.bytes = resfork_size;
  lendata = 0;
  rc=DosFSCtl(buf, resfork_size, &lendata, 
	      &read_params, sizeof(read_params), &lenparm,
	      HFS_FUNC_READ_RESFORK, NULL, macfile, FSCTL_HANDLE);
  if(rc!=NO_ERROR || lendata != resfork_size) {
    fprintf(stderr, "Error reading resource fork.\n");
    return 1;
  }

  fwrite(buf, 1, resfork_size, resfile);
  free(buf);
  DosClose(macfile);
  fclose(resfile);
  return 0;
}
