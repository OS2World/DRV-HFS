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

/* r0global.c
   Ring 0 global variables
*/

#define INCL_DOSERRORS
#define INCL_NOPMAPI
#include <os2.h>

#include <dhcalls.h>

#include "fsd.h"
#include "..\r0r3shar.h"
#include "r0struct.h"
#include "r0global.h"

/* Pointer to device helper entry point */
PFN Device_Help;

/* Data block to hold info on control program */
CPDATA CPData;

/* Flag set if control program currently attached */
int CPAttached;

/* Flag set if control program has been attached at least once */
int AttachedEver;

/* Locks for the control program's buffers */
LIN Lock1, Lock2;

/* Process ID of the control program */
PID32 CPPID;
