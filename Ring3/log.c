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

/* log.c
   Logging output
*/

#include <stdio.h>
#include <ctype.h>

#include "log.h"

#define DUMP_WIDTH 16

void dump(const unsigned char *text, unsigned length)
{
  unsigned i=0;
  while( i<length ) {
    unsigned j;

    printf("%04X  ", i);

    for( j=0; j<DUMP_WIDTH; j++ )
      if( i+j < length )
        printf("%02X ", (unsigned)text[i+j]);
      else
        printf("   ");
    printf("   ");
    for( j=0; j<DUMP_WIDTH; j++ )
      if( i+j < length ) {
        if( isprint(text[i+j]) )
          printf("%c", (unsigned)text[i+j]);
        else
          printf(".");
      }
    printf("\n");
    i += DUMP_WIDTH;
  }
}