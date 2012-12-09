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

/* attr.c
   File attributes
*/

#define INCL_DOSFILEMGR
#define INCL_DOSERRORS
#define INCL_NOPMAPI
#include <os2.h>

#include "libhfs/hfs.h"
#include "attr.h"

APIRET set_file_attributes(hfsdirent *ent, unsigned short attr)
{
  if(!(ent->flags & HFS_ISDIR) && (attr & FILE_DIRECTORY))
    return ERROR_ACCESS_DENIED;
  if(attr & FILE_HIDDEN)
    ent->fdflags |= HFS_FNDR_ISINVISIBLE;

  return NO_ERROR;
}

unsigned short get_file_attributes(hfsdirent *ent)
{
  unsigned short attr = FILE_NORMAL;

  if(ent->flags & HFS_ISDIR)
    attr |= FILE_DIRECTORY;
  if(ent->fdflags & HFS_FNDR_ISINVISIBLE)
    attr |= FILE_HIDDEN;

  return attr;
}
