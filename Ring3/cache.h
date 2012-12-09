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

/* cache.h
   The sector cache
*/

#ifndef _CACHE_H
#define _CACHE_H

#define MAX_ENTRIES         128
#define HASH_TABLE_SIZE     64

typedef struct _cacheentry {
  unsigned long secno;
  unsigned short hVPB;
  char *data;
  unsigned next, prev, next_prio, prev_prio;
} CACHEENTRY, *PCACHEENTRY;

unsigned cache_insert(unsigned short hVPB, unsigned long secno, 
		      unsigned short num_bytes, char *);
char *search_cache(unsigned short hVPB, unsigned long secno);
void cache_delete(unsigned short hVPB, unsigned long secno);
unsigned init_cache(void);
void flush_cache(void);

#endif /* _CACHE_H */
