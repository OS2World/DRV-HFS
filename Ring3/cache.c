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

/* cache.c
   The sector cache
*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "cache.h"

static void cleanup(void);
static unsigned hash(unsigned short, unsigned long, unsigned);
static unsigned find_cache_entry(unsigned short, unsigned long);
static void remove_cache_entry(unsigned);

static unsigned hash_table_size = HASH_TABLE_SIZE;

static unsigned *hash_table;
static PCACHEENTRY cache_buffers;

static unsigned long cache_hit, cache_miss;

/* Place a new element in the cache. */
unsigned cache_insert(unsigned short hVPB, unsigned long secno, 
		      unsigned short num_bytes, char *buf)
{
  unsigned h=hash(hVPB, secno, hash_table_size);
  unsigned i=find_cache_entry(hVPB, secno);

  if(i) {
    /* Entry already exists - update data */
    memcpy(cache_buffers[i].data, buf, num_bytes);
    return 0;
  }

  i=cache_buffers[0].prev_prio;

  /* Remove oldest element */
  remove_cache_entry(i);

  cache_buffers[i].hVPB = hVPB;
  cache_buffers[i].secno = secno;
  cache_buffers[i].data = malloc(num_bytes);
  memcpy(cache_buffers[i].data, buf, num_bytes);

  /* Insert the new element at the head of the priority list */
  cache_buffers[i].prev_prio = 0;
  cache_buffers[i].next_prio = cache_buffers[0].next_prio;
  cache_buffers[cache_buffers[0].next_prio].prev_prio=i;
  cache_buffers[0].next_prio=i;
  /* Insert the new element into its hash chain */
  cache_buffers[i].prev = 0;
  cache_buffers[i].next = hash_table[h];
  if(hash_table[h])
    cache_buffers[hash_table[h]].prev = i;
  hash_table[h] = i;

  return 0;
}

char *search_cache(unsigned short hVPB, unsigned long secno)
{
  unsigned i=find_cache_entry(hVPB, secno);
  if(i) {
    cache_hit++;
    /* Move entry to head of priority list */
    cache_buffers[cache_buffers[i].prev_prio].next_prio =
      cache_buffers[i].next_prio;
    cache_buffers[cache_buffers[i].next_prio].prev_prio =
      cache_buffers[i].prev_prio;

    cache_buffers[i].prev_prio=0;
    cache_buffers[i].next_prio=cache_buffers[0].next_prio;
    cache_buffers[cache_buffers[0].next_prio].prev_prio=i;
    cache_buffers[0].next_prio=i;
    
    return cache_buffers[i].data;
  }
  else {
    cache_miss++;
    return NULL;
  }
}

void cache_delete(unsigned short hVPB, unsigned long secno)
{
  int i = find_cache_entry(hVPB, secno);

  remove_cache_entry(i);
  /* Re-insert entry at end of priority list */
  cache_buffers[i].next_prio=0;
  cache_buffers[i].prev_prio=cache_buffers[0].prev_prio;
  cache_buffers[cache_buffers[0].prev_prio].next_prio=i;
  cache_buffers[0].prev_prio=i;
}

unsigned init_cache(void)
{
  cache_buffers=(PCACHEENTRY)calloc(MAX_ENTRIES, sizeof(CACHEENTRY));
  hash_table=(unsigned *)calloc(HASH_TABLE_SIZE, sizeof(unsigned));
  if( !(cache_buffers && hash_table) ) {
    printf("Fatal: Cannot allocate memory for cache!\n");
    exit(-1);
  }
  flush_cache();
  atexit(cleanup);
  return 0;
}

void flush_cache(void)
{
  int i;
  /* Initialize hash table */
  for(i=0; i<HASH_TABLE_SIZE; i++)
    hash_table[i]=0;
  /* Initialize cache entries */
  for(i=1; i<MAX_ENTRIES; i++) {
    cache_buffers[i].prev = cache_buffers[i].next = 0;
    cache_buffers[i].next_prio = i+1;
    cache_buffers[i].prev_prio = i-1;
    cache_buffers[i].secno = cache_buffers[i].hVPB = 0;
    if(cache_buffers[i].data != NULL)
      free(cache_buffers[i].data);
    cache_buffers[i].data = NULL;
  }
  /* Buffer 0 is a sentinel for the priority list */
  cache_buffers[MAX_ENTRIES-1].next_prio=0;
  cache_buffers[0].next_prio=1;
  cache_buffers[0].prev_prio=MAX_ENTRIES-1;
}

static void remove_cache_entry(unsigned i)
{
  /* Remove entry from priority list */
  cache_buffers[cache_buffers[i].prev_prio].next_prio =
    cache_buffers[i].next_prio;
  cache_buffers[cache_buffers[i].next_prio].prev_prio =
    cache_buffers[i].prev_prio;

  /* Remove entry from hash chain */
  if(cache_buffers[i].prev)
    cache_buffers[cache_buffers[i].prev].next = cache_buffers[i].next;
  else
    hash_table[hash(cache_buffers[i].hVPB, cache_buffers[i].secno,
		    hash_table_size)] = cache_buffers[i].next;
  if(cache_buffers[i].next)
    cache_buffers[cache_buffers[i].next].prev = cache_buffers[i].prev;

  /* Release data buffer */
  if(cache_buffers[i].data != NULL) {
    free(cache_buffers[i].data);
    cache_buffers[i].data = NULL;
  }
}

static unsigned find_cache_entry(unsigned short hVPB, unsigned long secno)
{
  unsigned i=hash_table[hash(hVPB, secno, hash_table_size)];

  while(i && (cache_buffers[i].hVPB != hVPB || cache_buffers[i].secno != secno))
    i=cache_buffers[i].next;
  return i;
}

static unsigned hash(unsigned short hVPB, unsigned long secno, unsigned n)
{
  /* This is ridiculous and will have to be replaced... */
  return (hVPB^secno) % n;
}

static void cleanup(void)
{
  if(hash_table)
    free(hash_table);
  if(cache_buffers)
    free(cache_buffers);
}
