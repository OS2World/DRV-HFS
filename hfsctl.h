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

/* hfsctl.h
   Defines the functions and data types used in the FSCtl user functions
   that can be used by other programs.
*/

#ifndef _HFSCTL_H
#define _HFSCTL_H

/* User FSCtl functions */
#define HFS_FUNC_GET_RESFORK_SIZE     0x9100
#define HFS_FUNC_READ_RESFORK         0x9101
#define HFS_FUNC_SYNC                 0x9102
#define HFS_FUNC_SYNC_ALL             0x9103

typedef struct hfs_read_resfork_params {
  unsigned long start;
  unsigned short bytes;
} HFS_READ_RESFORK_PARAMS, *PHFS_READ_RESFORK_PARAMS;

/*
   This is the specification of the FSCtl functions.

   ===
   HFS_FUNC_GET_RESFORK_SIZE
   Routing method: FSCTL_HANDLE

   Input:
     none

   Output:
     pParms        ULONG    Size of resource fork.

   ===
   HFS_FUNC_READ_RESFORK
   Routing method: FSCTL_HANDLE

   Input:
     pParms        HFS_READ_RESFORK_PARAMS

   Output:
     plenDataOut   Number of bytes read.
     pData         Resource fork data.

   ===
   HFS_FUNC_SYNC
   Routing method: FSCTL_PATHNAME

   Input:
     none

   Output:
     none

   Returns: 
     NO_ERROR on success, error code on failure

   ===
   HFS_FUNC_SYNC_ALL
   Routing method: FSCTL_FSDNAME

   Input:
     none 

   Output:
     none

   Returns:
     NO_ERROR on success, error code on failure

*/

#endif /* _HFSCTL_H */

