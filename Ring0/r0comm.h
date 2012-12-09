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

/* r0comm.h 
   Ring 0 side of communications
*/

#ifndef _R0COMM_H
#define _R0COMM_H

/* Lock handle */
typedef char HLOCK[12];

/* The mounted drives structures */
#define MAX_DRIVE            25
#define MAX_MOUNTSTRUCS      50

typedef struct MountStruc {
  unsigned short drive;
  unsigned short status;
  unsigned short hVPB;
} MOUNTSTRUC;

int PreSetup(void);
int PostSetup(void);

#endif /* _R0COMM_H */
