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

/* diskfunc.c
   Functions for reading and writing sectors.
*/

#define INCL_DOSFILEMGR
#define INCL_DOSERRORS
#define INCL_NOPMAPI
#include <os2.h>

#include <string.h>
#include <errno.h>

#include "libhfs/hfs.h"
#include "fsd.h"
#include "fsdep.h"
#include "../r0r3shar.h"
#include "diskfunc.h"
#include "cache.h"
#include "fsctl.h"

/* Maximum device sector size possible */
#define MAX_SECTOR_SIZE 4096

static char tmpbuf[MAX_SECTOR_SIZE];

static unsigned read_sector(unsigned long, unsigned short, unsigned short,
			    unsigned short, void *);
static unsigned write_sector(unsigned long, unsigned short, unsigned short,
			     unsigned short, void *);

/* read_blocks and write_blocks translate between HFS blocks and actual
   device sectors. */
int read_blocks(unsigned long start, unsigned short num_blocks,
		unsigned short block_size, unsigned short sector_size,
		unsigned short hVPB, void *buf)
{
  unsigned rc;

  unsigned long first_sector = start*block_size/sector_size;
  unsigned long first_block_offset = (start*block_size)%sector_size;
  unsigned long middle_sectors;

  /* First partial sector */
  if(first_block_offset) {
    unsigned first_bytes = sector_size-first_block_offset;
    if(first_bytes > num_blocks*block_size)
      first_bytes = num_blocks*block_size;
    if((rc=read_sector(first_sector, 1, sector_size, hVPB, tmpbuf))!=NO_ERROR)
      goto error;
    memcpy(buf, tmpbuf+first_block_offset, first_bytes);
    buf += first_bytes;
    num_blocks -= first_bytes/block_size;
    first_sector++;
  }

  /* Middle sectors */
  middle_sectors = num_blocks*block_size/sector_size;
  if(middle_sectors) {
    if((rc=read_sector(first_sector, middle_sectors, sector_size, hVPB, buf))
       !=NO_ERROR)
      goto error;
    buf += middle_sectors*sector_size;
    first_sector += middle_sectors;
    num_blocks -= middle_sectors*sector_size/block_size;
  }

  /* Last partial sector */
  if(num_blocks) {
    if((rc=read_sector(first_sector, 1, sector_size, hVPB, tmpbuf))
       != NO_ERROR)
      goto error;
    memcpy(buf, tmpbuf, num_blocks*block_size);
  }
  return NO_ERROR;

error:
  return -EIO;
}

int write_blocks(unsigned long start, unsigned short num_blocks,
		 unsigned short block_size, unsigned short sector_size,
		 unsigned short hVPB, void *buf)
{
  unsigned rc;
  
  unsigned long first_sector = start*block_size/sector_size;
  unsigned long first_block_offset = (start*block_size)%sector_size;
  unsigned long middle_sectors;
  
  /* First partial sector */
  if(first_block_offset) {
    unsigned first_bytes = sector_size-first_block_offset;
    if(first_bytes > num_blocks*block_size)
      first_bytes = num_blocks*block_size;
    if((rc=read_sector(first_sector, 1, sector_size, hVPB, tmpbuf))!=NO_ERROR)
      goto error;
    memcpy(tmpbuf+first_block_offset, buf, first_bytes);
    if((rc=write_sector(first_sector, 1, sector_size, hVPB, tmpbuf))!=NO_ERROR)
      goto error;
    buf += first_bytes;
    num_blocks -= first_bytes/block_size;
    first_sector++;
  }

  /* Middle sectors */
  middle_sectors = num_blocks*block_size/sector_size;
  if(middle_sectors) {
    if((rc=write_sector(first_sector, middle_sectors, sector_size, hVPB, buf))
       != NO_ERROR)
      goto error;
    buf += middle_sectors*sector_size;
    first_sector += middle_sectors;
    num_blocks -= middle_sectors*sector_size/block_size;
  }

  /* Last partial sector */
  if(num_blocks) {
    if((rc=read_sector(first_sector, 1, sector_size, hVPB, tmpbuf))
       != NO_ERROR)
      goto error;
    memcpy(tmpbuf, buf, num_blocks*block_size);
    if((rc=write_sector(first_sector, 1, sector_size, hVPB, tmpbuf))
       != NO_ERROR)
      goto error;
  }
  return NO_ERROR;

error:
  return -EIO;
}

/* read_sector and write_sector deal with actual (device) sectors. */
static unsigned read_sector(unsigned long start, unsigned short num_sectors,
			    unsigned short sector_size,
			    unsigned short hVPB, void *buf)
{
  unsigned long i, first_miss=0, n_sec=0;

  /* Collect consecutive cache misses into one read request */
  for(i=start; i<start+num_sectors; i++) {
    char *p = search_cache(hVPB, i);
    if(p) {
      /* Get the found sector first, so that it isn't subsequently 
	 invalidated */
      memcpy(buf+n_sec*sector_size, p, sector_size);

      /* Read consecutive sectors not in cache */
      if(n_sec) {
	RWSTRUC rw;
	APIRET rc;
	unsigned long len_data, len_rw;
	unsigned j;
	
	rw.starting_sector = first_miss;
	rw.n_sectors = n_sec;
	rw.hVPB = hVPB;

	if((rc=do_FSCtl(buf, n_sec*sector_size, &len_data, 
		    &rw, sizeof(rw), &len_rw, FSCTL_FUNC_READSECTORS))
	   != NO_ERROR)
	  return rc;
	/* Insert sectors into cache */
	for(j=first_miss; j<first_miss+n_sec; j++) {
	  cache_insert(hVPB, j, sector_size, buf);
	  buf += sector_size;
	}

	n_sec=first_miss=0;
      }
      buf+=sector_size; /* The sector already copied from the cache */
    }
    else {
      /* Cache miss */
      if(!first_miss)
	first_miss=i;
      n_sec++;
    }
  }
  /* Read last bunch of sectors */
  if(n_sec) {
    RWSTRUC rw;
    APIRET rc;
    unsigned long len_data, len_rw;
    unsigned j;
    
    rw.starting_sector = first_miss;
    rw.n_sectors = n_sec;
    rw.hVPB = hVPB;
    
    if((rc=do_FSCtl(buf, n_sec*sector_size, &len_data, 
		&rw, sizeof(rw), &len_rw, FSCTL_FUNC_READSECTORS))
       != NO_ERROR)
      return rc;

    /* Insert sectors into cache */
    for(j=first_miss; j<first_miss+n_sec; j++) {
      cache_insert(hVPB, j, sector_size, buf);
      buf += sector_size;
    }
  }

  return NO_ERROR;
}

static unsigned write_sector(unsigned long start, unsigned short num_sectors,
			     unsigned short sector_size, 
			     unsigned short hVPB, void *buf)
{
  RWSTRUC rw;
  APIRET rc;
  unsigned long len_data, len_rw;
  unsigned i;

  rw.starting_sector = start;
  rw.n_sectors = num_sectors;
  rw.hVPB = hVPB;

  rc = do_FSCtl(buf, num_sectors*sector_size, &len_data,
                &rw, sizeof(rw), &len_rw, FSCTL_FUNC_WRITESECTORS);

  /* Update the cache */
  for(i=start; i<start+num_sectors; i++) {
    if(rc==NO_ERROR)
      cache_insert(hVPB, i, sector_size, buf);
    else
      cache_delete(hVPB, i);
    buf+=sector_size;
  }
  return rc;
}
