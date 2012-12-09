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

/* r0struct.h
   Ring 0 structures
*/

#ifndef _R0STRUCT_H
#define _R0STRUCT_H

typedef unsigned long SEM;

typedef struct cpdata {
  /* Ring 0 is messing with the buffers */
  SEM BufLock;

  /* A command is in the buffers */
  SEM CmdReady;
  
  /* The results of a command are in the buffers */
  SEM CmdComplete;
  
  /* Pointer to the shared operation buffer */
  OPDATA *OpData;
  
  /* Pointer to the shared data buffer */
  void *Buf;
} CPDATA;

/*
#define MAXCPRDYWAIT (10*1000)
#define MAXCPRESWAIT (10*1000)
*/

#define MAXCPRDYWAIT -1
#define MAXCPRESWAIT -1

#endif /* _R0STRUCT_H */
