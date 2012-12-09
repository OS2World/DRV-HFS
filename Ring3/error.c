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

/* error.c
   Error code translation.
   */

#define INCL_DOSERRORS
#include <os2.h>

#include <errno.h>

#include "error.h"

/* Translate UNIX errno to OS/2 error code. */
APIRET os2_error(int err, APIRET default_error)
{
  if(err==0)
    return NO_ERROR;
  
  switch(err) {
  /*
    case EIO:
    return ERROR_GEN_FAILURE;
  */
  case ENOENT:
    return ERROR_PATH_NOT_FOUND;
  case EEXIST:
    return ERROR_FILE_EXISTS;
  case EROFS:
    return ERROR_WRITE_PROTECT;
  case EISDIR:
    return ERROR_DIRECTORY;
  case ENOSPC:
    return ERROR_DISK_FULL;
  default:
    return default_error;
  }
}



