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

/* userfunc.c
   User FSCtl functions.
*/

#define INCL_DOSERRORS
#define INCL_NOPMAPI
#include <os2.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "libhfs/hfs.h"
#include "fsd.h"
#include "fsdep.h"
#include "userfunc.h"
#include "error.h"

/* Get the length of the resource fork of a file */
unsigned short hfs_resfork_size(struct sffsi *psffsi,
				struct sffsd *psffsd,
				unsigned long *size)
{
  hfsdirent ent;

  if(hfs_fstat(psffsd->file, &ent) < 0)
    return os2_error(errno, ERROR_ACCESS_DENIED);
  *size = ent.rsize;
  return NO_ERROR;
}

/* Read bytes from the resource fork. Note that bytes must be at most 64K. */
unsigned short hfs_read_resfork(struct vpfsd *pvpfsd,
				struct sffsi *psffsi,
				struct sffsd *psffsd,
				unsigned long start,
				unsigned long *bytes,
				char *buf)
{
  long bytes_read;
  char *tmp;

  /* Buffer is inaccessible to Ring 0, so we must use a temporary buffer */
  tmp=(char *)malloc(*bytes);

  hfs_setfork(psffsd->file, 1);
  if(hfs_lseek(psffsd->file, start, SEEK_SET) < 0 ||
     (bytes_read=hfs_read(psffsd->file, tmp, *bytes)) == -1) {
    hfs_setfork(psffsd->file, 0);
    free(tmp);
    return os2_error(errno, ERROR_READ_FAULT);
  }
  memcpy(buf, tmp, bytes_read);
  free(tmp);
  *bytes = bytes_read;

  return NO_ERROR;
}

/* Sync a volume */
unsigned short hfs_sync(struct vpfsd *pvpfsd)
{
  if(hfs_flush(pvpfsd->vol) < 0)
    return os2_error(errno, ERROR_GEN_FAILURE);
  return NO_ERROR;
}

/* Sync all volumes */
unsigned short hfs_sync_all(void)
{
  hfs_flushall();
  return NO_ERROR;
}
