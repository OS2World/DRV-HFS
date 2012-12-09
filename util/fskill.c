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

/* fskill.c
   Send a command to FSD to disconnect from control program.
*/

#define INCL_DOSFILEMGR
#define INCL_NOPMAPI
#define INCL_NOCOMMON
#include <os2.h>

#include <stdio.h>

#define FSCTL_FUNC_KILL               0x9002

int main()
{
   int rc;

   rc = DosFSCtl(NULL, 0, NULL, NULL, 0, NULL, 
                 FSCTL_FUNC_KILL, "HFS", -1, FSCTL_FSDNAME);
   printf("ret = 0x%x %d\n", rc, rc);

   return 0;
}
