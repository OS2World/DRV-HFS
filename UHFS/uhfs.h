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

/* uhfs.h
   Prototypes for the IFS utilities
*/

#ifndef _UHFS_H
#define _UHFS_H

int far pascal _loadds CHKDSK(short argc, char **argv, char **envp);
int far pascal _loadds FORMAT(short argc, char **argv, char **envp);
int far pascal _loadds RECOVER(short argc, char **argv, char **envp);
int far pascal _loadds SYS(short argc, char **argv, char **envp);
void far pascal Startup(void);

#endif /* _UHFS_H */
