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

/* logger.c
   Thread that takes care of log info
*/

#define INCL_DOSFILEMGR
#define INCL_DOSERRORS
#define INCL_DOSPROCESS
#define INCL_NOPMAPI
#include <os2.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "libhfs/hfs.h"
#include "fsd.h"
#include "fsdep.h"
#include "../r0r3shar.h"
#include "r3global.h"

#define BUFSIZE 4096
static char Buf[BUFSIZE];
char name[30];

void log_thread(void *arg)
{
  FILE *fp = (FILE *)arg;
  ULONG dataio;
  APIRET rc;

#ifdef DEBUG
  printf("Logging thread running\n");
#endif
  strcpy(name, FS_NAME);

  for (;;) {
    dataio = 0;
    memset(Buf, 0, BUFSIZE);
    if ((rc = DosFSCtl(Buf, BUFSIZE, &dataio,
                       NULL, 0, NULL,
                       FSCTL_FUNC_GETLOGDATA, name,
                       -1,  FSCTL_FSDNAME)) != NO_ERROR) {
      printf("logger: DosFSCtl failed!\n");
      _endthread();
    }
    printf(Buf);
    if(fp) {
      fprintf(fp, Buf);
      fflush(fp);
    }
  }
}
