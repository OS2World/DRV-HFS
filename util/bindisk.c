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

/* bindisk.c
   Utility to read first few sectors of a disk.
 */

#define INCL_DOSFILEMGR
#define INCL_DOSERRORS
#include <os2.h>

#include <stdio.h>

#define BUFSIZE (1024*32)

int main(int argc, char *argv[])
{
  APIRET rc;
  HFILE hf;
  ULONG action, bytes_read;
  FILE *outfile;
  static char buf[BUFSIZE];

  if(argc!=3) {
    fprintf(stderr, "Usage: bindisk d: imgfile\n");
    return 1;
  }
  
  if((rc=DosOpen(argv[1], &hf, &action, 0, FILE_NORMAL,
	     OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
	     OPEN_FLAGS_DASD | OPEN_SHARE_DENYREADWRITE, NULL))!=NO_ERROR) {
    fprintf(stderr, "Cannot open drive %s, error %ld\n", argv[1], rc);
    return 1;
  }

  if((outfile=fopen(argv[2], "wb"))==NULL) {
    fprintf(stderr, "Cannot create output file %s\n", argv[2]);
    return 1;
  }

  if(DosRead(hf, buf, BUFSIZE, &bytes_read)!=NO_ERROR) {
    fprintf(stderr, "Error reading volume.\n");
    return 1;
  }

  if(fwrite(buf, sizeof(char), bytes_read, outfile)<bytes_read) {
    fprintf(stderr, "Error writing output file.\n");
    return 1;
  }

  DosClose(hf);
  fclose(outfile);
  return 0;
}
