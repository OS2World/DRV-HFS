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

/* fltlog.c
   Prints messages from HFSFLT.FLT to standard output.
   */

#define INCL_DOSFILEMGR    /* File System values */
#define INCL_DOSDEVIOCTL   /* DosDevIOCtl values */
#define INCL_DOSERRORS
#include <os2.h>

#include <stdio.h>

int main()
{
  BYTE abBuf[1];
  HFILE hfFilter, hfOutput;
  ULONG ulAction, cbRead, cbWritten;
  APIRET rc;
  
  if(rc=DosOpen("HFSFLT$", &hfFilter, &ulAction, 0,
		FILE_NORMAL, FILE_OPEN,
		OPEN_ACCESS_READONLY | OPEN_SHARE_DENYNONE,
		(PEAOP2) NULL) != NO_ERROR) {
    fprintf(stderr, "Cannot open HFSFLT$, rc = %hu\n");
    return 1;
  }
  
  if(rc=DosOpen("CON", &hfOutput, &ulAction, 0,
		FILE_NORMAL, FILE_OPEN,
		OPEN_ACCESS_WRITEONLY | OPEN_SHARE_DENYWRITE,
		(PEAOP2) NULL) != NO_ERROR) {
    fprintf(stderr, "Cannot open output, rc = %hu\n");
    return 1;
  }
  
  do {
    DosRead(hfFilter, abBuf, sizeof(abBuf), &cbRead);
    DosWrite(hfOutput, abBuf, cbRead, &cbWritten);
  } while(cbRead);
  
  DosClose(hfFilter);
  DosClose(hfOutput);

  return 0;
}
