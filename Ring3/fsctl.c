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

/* fsctl.c
   Perform DosFSCtl calls
*/

#define INCL_DOSERRORS
#define INCL_DOSFILEMGR
#define INCL_NOPMAPI
#include <os2.h>

#include <string.h>

#include "r3global.h"
#include "fsctl.h"

/* Call the HFS driver with DosFSCtl */
APIRET do_FSCtl(PVOID pData, ULONG cbData, PULONG pcbData,
		PVOID pParms, ULONG cbParms, PULONG pcbParms,
		ULONG function)
{
  char name[10];

  strncpy(name, FS_NAME, sizeof(name));
  return DosFSCtl(pData, cbData, pcbData, pParms, cbParms, pcbParms,
		  function, name, -1, FSCTL_FSDNAME);
}
