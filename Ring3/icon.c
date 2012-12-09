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

/* icon.c
   Icon conversions.
*/

#define INCL_GPIBITMAPS
#include <os2.h>

#include <stdlib.h>
#include <memory.h>

#include "libhfs/hfs.h"
#include "resource.h"
#include "icon.h"
#include "colour.h"

#define RESBUF_SIZE    4096

static int convert_bitmap(char *, char *, int, int, int, int);
static void set_bfh_offset(char *p, int n, USHORT offset);
static int build_colour_header(char *, int, int, int);
static int build_bitmaparrayfileheader(char *, int, int);
static int build_bitmapfileheader(char *, int, int);
static int build_bitmapinfoheader(char *, int, int, int);

/* Build an icon file from a specified icon family */
int build_icon_file(hfsfile *file, resid_t id, char *buf)
{
  char *p=buf, *q=buf, resbuf[RESBUF_SIZE];

  /* Since an icon family might have only some of the icon sizes,
     we must determine which are present.
     We currently only convert one bitmap. */

  if(get_resource_size(file, RESTYPE_ICL8, id)) {
    /* 32x32, 256 colours */
    q+=build_colour_header(p, 32, 32, 8);
    set_bfh_offset(p, 0, q-buf);
    if(!get_resource(file, RESTYPE_ICNLIST, id, resbuf))
      return 0;

    /* Zero out the XOR mask */
    memset(q, 0, 128);
    q+=128;
    /* Convert the AND mask */
    q += convert_bitmap(resbuf+128, q, 32, 32, 1, 1);
    
    set_bfh_offset(p, 1, q-buf);
    if(!get_resource(file, RESTYPE_ICL8, id, resbuf))
      return 0;
    q += convert_bitmap(resbuf, q, 32, 32, 8, 0);
  }
  else if(get_resource_size(file, RESTYPE_ICL4, id)) {
    /* 32x32, 16 colours */
    q+=build_colour_header(p, 32, 32, 4);
    set_bfh_offset(p, 0, q-buf);
    if(!get_resource(file, RESTYPE_ICNLIST, id, resbuf))
      return 0;
    
    /* Zero out the XOR mask */
    memset(q, 0, 128);
    q+=128;
    /* Convert the AND mask */
    q += convert_bitmap(resbuf+128, q, 32, 32, 1, 1);
    
    set_bfh_offset(p, 1, q-buf);
    if(!get_resource(file, RESTYPE_ICL4, id, resbuf))
      return 0;
    q += convert_bitmap(resbuf, q, 32, 32, 4, 0);
  }

  return q-buf;
}

int icon_file_size(hfsfile *file, resid_t id)
{
  if(!get_resource_size(file, RESTYPE_ICNLIST, id))
    return 0;

  if(get_resource_size(file, RESTYPE_ICL8, id))
    return sizeof(BITMAPARRAYFILEHEADER)+
      sizeof(BITMAPFILEHEADER)+(3+256)*sizeof(RGB)+
      32*64/8+32*32;
  else if(get_resource_size(file, RESTYPE_ICL4, id))
    return sizeof(BITMAPARRAYFILEHEADER)+
      sizeof(BITMAPFILEHEADER)+(3+16)*sizeof(RGB)+
      32*64/8+32*32/2;
  else
    return 0;
}

/* Flip a bitmap. OS/2 bitmaps start at lower left corner,
   while Mac bitmaps start at upper left corner. */
static int convert_bitmap(char *src, char *dest, 
			  int xsize, int ysize, int bpp, int invert)
{
  char *p=dest;
  int i, rowlen = xsize*bpp/8;
  char *q=src+rowlen*(ysize-1);

  for(i=0; i<ysize; i++) {
    int j;
    for(j=0; j<rowlen; j++)
      *p++ = invert ? ~q[j] : q[j];
    q-=rowlen;
  }
  return p-dest;
}

/* Set the bitmap offset field in the nth BITMAPFILEHEADER in a given
   BITMAPARRAYFILEHEADER. */
static void set_bfh_offset(char *p, int n, USHORT offset)
{
  PBITMAPARRAYFILEHEADER ba = (PBITMAPARRAYFILEHEADER)p;
  PBITMAPFILEHEADER bfh = (PBITMAPFILEHEADER)(&ba->bfh);
  int i;

  for(i=0; i<n; i++) {
    bfh = (PBITMAPFILEHEADER)((char *)bfh + sizeof(BITMAPFILEHEADER) +
      (1 << bfh->bmp.cBitCount) * sizeof(RGB));
  }
  bfh->offBits = offset;
}

/* Build the bitmap header structures for colour bitmaps. */
static int build_colour_header(char *buf, int xsize, int ysize, int bpp)
{
  int i;
  char *p=buf;

  p+=build_bitmaparrayfileheader(p, xsize, ysize);
  p+=build_bitmapfileheader(p, xsize, ysize);
  p+=build_bitmapinfoheader(p, xsize, ysize*2, 1);
  *p++=0; *p++=0; *p++=0;             /* black */
  *p++=0xff; *p++=0xff; *p++=0xff;    /* white */

  p+=build_bitmapfileheader(p, xsize, ysize);
  p+=build_bitmapinfoheader(p, xsize, ysize, bpp);

  /* The colour table */
  if(bpp==4) {
    memcpy(p, colour_table_4, sizeof(colour_table_4));
    p+=sizeof(colour_table_4);
  }    
  else if(bpp==8) {
    memcpy(p, colour_table_8, sizeof(colour_table_8));
    p+=sizeof(colour_table_8);
  }
  else {
    for(i=0; i < 1<<bpp; i++) {
      char c = i*256/(1<<bpp);
      *p++=c; *p++=c; *p++=c;
    }
  }
  return p-buf;
}

static int build_bitmaparrayfileheader(char *p,
				       int xsize,
				       int ysize)
{
  PBITMAPARRAYFILEHEADER ba = (PBITMAPARRAYFILEHEADER)p;

  ba->usType = BFT_BITMAPARRAY;
  ba->cbSize = 0x28;
  ba->offNext = 0;
  ba->cxDisplay = ba->cyDisplay = 0;
  return (char *)&ba->bfh - p;
}

int build_bitmapfileheader(char *p, int xsize, int ysize)
{
  PBITMAPFILEHEADER bfh = (PBITMAPFILEHEADER)p;

  bfh->usType = BFT_COLORICON;
  bfh->cbSize = 0x1a;
  bfh->xHotspot = xsize/2;
  bfh->yHotspot = ysize/2;
  bfh->offBits = 0;
  return (char *)&bfh->bmp - p;
}

static int build_bitmapinfoheader(char *p, int xsize, int ysize, int bpp)
{
  PBITMAPINFOHEADER bi = (PBITMAPINFOHEADER)p;

  bi->cbFix = 0xc;
  bi->cx = xsize;
  bi->cy = ysize;
  bi->cPlanes = 1;
  bi->cBitCount = bpp;
  return sizeof(BITMAPINFOHEADER);
}
