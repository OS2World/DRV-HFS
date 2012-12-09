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

/* resource.h
   Resource fork operations
*/

#ifndef _RESOURCE_H
#define _RESOURCE_H

/* Resource types - first char in LSB */
#define RESTYPE_ICNLIST          0x234e4349    /* ICN# */
#define RESTYPE_ICL4             0x346c6369    /* icl4 */
#define RESTYPE_ICL8             0x386c6369    /* icl8 */

/* Resource ID for custom icon */
#define RESID_CUSTOMICON (-16455)

typedef short resid_t;

typedef unsigned char   UChar;
typedef unsigned short	UInteger;
typedef unsigned long	ULongInt;

typedef struct ResourceHeader {
  ULongInt res_data_ofs;           /* Offset to resource data */
  ULongInt res_map_ofs;            /* Offset to resource map */
  ULongInt res_data_len;           /* Length of resource data */
  ULongInt res_map_len;            /* Length of resource map */
} RESOURCEHEADER, *PRESOURCEHEADER;

typedef struct ResMapHeader {
  char reserved1[16];
  ULongInt reserved2;
  UInteger reserved3;
  UInteger attr;                       /* Resource file attributes */
  UInteger type_list_ofs;              /* Offset from beginning of resource map to type list */
  UInteger name_list_ofs;              /* Offset from beginning of resource map to name list */
} RESMAPHEADER, *PRESMAPHEADER;

/* Resouce type list:
   UInteger n_types;                    /- Number of types in map minus 1 -/
   TYPELISTENTRY typelist[];
*/

typedef struct TypeListEntry {
  ULongInt type;                   /* Resource type */
  UInteger n_res;                  /* Number of resources of this type minus 1 */
  UInteger ref_ofs;                /* Offset from beginning of type list to
			           reference list for resources of this type */
} TYPELISTENTRY, *PTYPELISTENTRY;

typedef struct RefListEntry {
  resid_t res_id;             /* Resource ID */
  UInteger name_ofs;          /* Offset from beginning of resource name list
			         to length UChar of resource name, or -1. */
  UChar attr;                 /* Resource attributes */
  UChar data_ofs_hi;          /* MSB of the following */
  UInteger data_ofs_lo;       /* Lower part of offset from beginning of
			      resource data to length of data for resource. */
  ULongInt reserved;
} REFLISTENTRY, *PREFLISTENTRY;

unsigned count_resources(hfsfile *, ULongInt);
resid_t get_ind_resource(hfsfile *, ULongInt, unsigned);
unsigned get_resource_size(hfsfile *, ULongInt, resid_t);
int get_resource(hfsfile *, ULongInt, resid_t, char *);

#endif /* _RESOURCE_H */
