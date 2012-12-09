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

/* r0global.h
   Declarations for ring 0 global variables
*/

#ifndef _R0GLOBAL_H
#define _R0GLOBAL_H

/* Pointer to DevHlp entry point */
extern PFN Device_Help;

/* Data block to hold info on control program */
extern CPDATA CPData;

/* Flag set if control program currently attached */
extern int CPAttached;

/* Flag set if control program has been attached at least once */
extern int AttachedEver;

/* C library startup function */
extern void far pascal Startup(void);

/* Locks for the control program's buffers */
extern LIN Lock1, Lock2;

/* Process ID of the control program */
extern PID32 CPPID;

#endif /* _R0GLOBAL_H */
