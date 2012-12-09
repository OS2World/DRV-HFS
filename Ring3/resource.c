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

/* resource.c
   Resource fork operations
*/

#include <stdio.h>

#include "libhfs/hfs.h"
#include "resource.h"

#define reorderword(byteptr) ((((UInteger)(*(byteptr)))<<8)           \
                             |(((UInteger)(*((byteptr)+1)))))



#define reorderULongInt(byteptr)  (                                       \
                                   (((ULongInt)(*(byteptr)))<<24)         \
                                  |(((ULongInt)(*((byteptr)+1)))<<16)     \
                                  |(((ULongInt)(*((byteptr)+2)))<<8)      \
                                  |((ULongInt)(*((byteptr)+3)))           \
                                  )         

static int read_res(hfsfile *, unsigned, unsigned, char *, unsigned *);

unsigned count_resources(hfsfile *file, ULongInt type)
{
  unsigned long map_ofs, type_list_entry;
  unsigned bytes;
  unsigned n_types, i;
  unsigned char buf[10];

  if(read_res(file, 4, sizeof(ULongInt), buf, &bytes)
     || bytes != sizeof(ULongInt))
    return 0;
  map_ofs = reorderULongInt(buf);

  if(read_res(file, map_ofs+24, 3*sizeof(UInteger), buf, &bytes)
     || bytes != 3*sizeof(UInteger))
    return 0;
  type_list_entry = map_ofs+reorderword(buf)+2;
  n_types = reorderword(buf+4)+1;

  for(i=0; i<n_types; i++, type_list_entry += sizeof(TYPELISTENTRY)) {
    if(read_res(file, type_list_entry, 6, buf, &bytes) 
       || bytes != 6)
      return 0;
    if(*(ULongInt *)buf==type)
      return reorderword(buf+4)+1;
  }
  return 0;
}

/* Return resource ID of the nth resource of a given type */
resid_t get_ind_resource(hfsfile *file, 
			 ULongInt type, unsigned n)
{
  unsigned long map_ofs, type_list_ofs, cur_ofs;
  unsigned bytes;
  unsigned n_types, i, n_res=0;
  unsigned char buf[10];

  if(read_res(file, 4, sizeof(ULongInt), buf, &bytes)
     || bytes != sizeof(ULongInt))
    return 0;
  map_ofs = reorderULongInt(buf);

  if(read_res(file, map_ofs+24, 3*sizeof(UInteger), buf, &bytes)
     || bytes != sizeof(ULongInt))
    return 0;
  type_list_ofs = map_ofs+reorderword(buf);
  n_types = reorderword(buf+4)+1;

  cur_ofs = type_list_ofs+2;
  for(i=0; i<n_types; i++, cur_ofs += sizeof(TYPELISTENTRY)) {
    if(read_res(file, cur_ofs, 8, buf, &bytes)
       || bytes != 8)
      return 0;
    if(*(ULongInt *)buf==type) {
      n_res = reorderword(buf+4)+1;
      cur_ofs = type_list_ofs+reorderword(buf+6);
      break;
    }
  }
  if(i==n_types) return 0;
  if(n>=n_res) return 0;

  cur_ofs += n*sizeof(REFLISTENTRY);
  if(read_res(file, cur_ofs, 2, buf, &bytes)
     || bytes != 2)
    return 0;
  return reorderword(buf);
}

unsigned get_resource_size(hfsfile *file, 
			   ULongInt type, resid_t id)
{
  unsigned long map_ofs, data_ofs, type_list_ofs, cur_ofs;
  unsigned bytes;
  unsigned n_types, i, n_res=0, res_data_ofs=0;
  unsigned char buf[10];

  if(read_res(file, 0, 2*sizeof(ULongInt), buf, &bytes)
     || bytes != 2*sizeof(ULongInt))
    return 0;
  data_ofs = reorderULongInt(buf);
  map_ofs = reorderULongInt(buf+4);

  if(read_res(file, map_ofs+24, 3*sizeof(UInteger), buf, &bytes)
     || bytes != 3*sizeof(UInteger))
    return 0;
  type_list_ofs = map_ofs+reorderword(buf);
  n_types = reorderword(buf+4)+1;

  cur_ofs = type_list_ofs+2;
  for(i=0; i<n_types; i++, cur_ofs += sizeof(TYPELISTENTRY)) {
    if(read_res(file, cur_ofs, 8, buf, &bytes)
       || bytes != 8)
      return 0;
    if(*(ULongInt *)buf==type) {
      n_res = reorderword(buf+4)+1;
      cur_ofs = type_list_ofs+reorderword(buf+6);
      break;
    }
  }
  if(i==n_types) return 0;

  for(i=0; i<n_res; i++, cur_ofs+=sizeof(REFLISTENTRY)) {
    if(read_res(file, cur_ofs, 8, buf, &bytes) || bytes != 8)
      return 0;
    if((short)reorderword(buf)==id) {
      res_data_ofs = data_ofs+reorderword(buf+6)+(buf[5]<<16);
      break;
    }
  }
  if(i==n_res) return 0;

  if(read_res(file, res_data_ofs, 4, buf, &bytes)
     || bytes != 4)
    return 0;
  return reorderULongInt(buf);
}

int get_resource(hfsfile *file, 
		 ULongInt type, resid_t id, char *dest)
{
  unsigned long map_ofs, data_ofs, type_list_ofs, cur_ofs;
  unsigned bytes;
  unsigned n_types, i, n_res=0, res_data_ofs=0, size;
  unsigned char buf[10];

  if(read_res(file, 0, 2*sizeof(ULongInt), buf, &bytes)
     || bytes != 2*sizeof(ULongInt))
    return 0;
  data_ofs = reorderULongInt(buf);
  map_ofs = reorderULongInt(buf+4);

  if(read_res(file, map_ofs+24, 3*sizeof(UInteger), buf, &bytes)
     || bytes != 3*sizeof(UInteger))
    return 0;
  type_list_ofs = map_ofs+reorderword(buf);
  n_types = reorderword(buf+4)+1;

  cur_ofs = type_list_ofs+2;
  for(i=0; i<n_types; i++, cur_ofs += sizeof(TYPELISTENTRY)) {
    if(read_res(file, cur_ofs, 8, buf, &bytes)
       || bytes != 8)
      return 0;
    if(*(ULongInt *)buf==type) {
      n_res = reorderword(buf+4)+1;
      cur_ofs = type_list_ofs+reorderword(buf+6);
      break;
    }
  }
  if(i==n_types) return 0;

  for(i=0; i<n_res; i++, cur_ofs+=sizeof(REFLISTENTRY)) {
    if(read_res(file, cur_ofs, 8, buf, &bytes) || bytes != 8)
      return 0;
    if((short)reorderword(buf)==id) {
      res_data_ofs = data_ofs+reorderword(buf+6)+(buf[5]<<16);
      break;
    }
  }
  if(i==n_res) return 0;

  if(read_res(file, res_data_ofs, 4, buf, &bytes)
     || bytes != 4)
    return 0;
  size = reorderULongInt(buf);
  if(read_res(file, res_data_ofs+4, size, dest, &bytes) || bytes != size)
    return 0;
  return size;
}

/* Read bytes from resource fork. */
static int read_res(hfsfile *file, unsigned start, unsigned count, 
		    char *buf, unsigned *bytes_read)
{
  hfs_setfork(file, 1);
  if(hfs_lseek(file, start, SEEK_SET) < 0 ||
     (*bytes_read=hfs_read(file, buf, count)) == -1) {
    hfs_setfork(file, 0);
    return 1;
  }
  hfs_setfork(file, 0);
  return 0;
}
