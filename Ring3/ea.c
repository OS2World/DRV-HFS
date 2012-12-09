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

/* ea.c
   Extended attributes.
*/

#define INCL_DOSFILEMGR
#define INCL_DOSERRORS
#define INCL_NOPMAPI
#include <os2.h>

#include <string.h>

#include "libhfs/hfs.h"
#include "resource.h"
#include "ea.h"
#include "icon.h"

static unsigned short build_icon_ea(hfsfile *, char *, unsigned short);
static unsigned short icon_ea_len(hfsfile *);

static char icon_ea_name[] = ".ICON";

/* Builds list of all EAs for a file. */
unsigned short build_fealist(hfsfile *file, char *buf, unsigned short bufsize)
{
  char *p=buf+sizeof(ULONG);
  PFEALIST pfealist=(PFEALIST)buf;
  PFEA pfea;
  unsigned short rc;
  
  if(bufsize < sizeof(ULONG))
    return ERROR_BUFFER_OVERFLOW;

  /* We set the cbList field in case we get an overflow later */
  pfealist->cbList = ea_size(file);

  /* .ICON */
  if(icon_ea_len(file)) {
    pfea=(PFEA)p;
    rc=build_icon_ea(file, p, buf+bufsize-p);
    if(rc!=NO_ERROR)
      return rc;
    p+=sizeof(FEA)+pfea->cbName+pfea->cbValue+1;
  }

  pfealist->cbList = p-buf;
  return NO_ERROR;
}

/* Builds list of EAs specified in GEA list */
unsigned short build_fealist_from_gealist(hfsfile *file,
					  char *geabuf, char *feabuf,
					  unsigned short bufsize)
{
  PFEALIST pfealist=(PFEALIST)feabuf;
  PGEALIST pgealist=(PGEALIST)geabuf;
  char *p=feabuf+sizeof(ULONG), *q=geabuf+sizeof(ULONG);

  if(bufsize < sizeof(ULONG))
    return ERROR_BUFFER_OVERFLOW;

  pfealist->cbList = ea_size(file);

  while(q-geabuf < pgealist->cbList) {
    if(!strcmp(q+1, icon_ea_name)) {
      PFEA pfea=(PFEA)p;
      unsigned short rc = build_icon_ea(file, p, feabuf+bufsize-p);
      if(rc!=NO_ERROR)
	return rc;
      p+=sizeof(FEA)+pfea->cbName+pfea->cbValue+1;
    }
    q += *q+2;
  }
  pfealist->cbList = p-feabuf;
  return NO_ERROR;
}

/* Returns size of EAs for a file */
unsigned ea_size(hfsfile *file)
{
  unsigned short size=0;

  if(icon_ea_len(file))
    size+=sizeof(FEA)+strlen(icon_ea_name)+1+icon_ea_len(file);

  size+=sizeof(ULONG); /* cbList */
  return size;
}

static unsigned short icon_ea_len(hfsfile *file)
{
  int s;
  
  s=icon_file_size(file, RESID_CUSTOMICON);
  if(s)
    return 2*sizeof(USHORT)+s;

  return 0;
}

/* Creates the .ICON EA. */
static unsigned short build_icon_ea(hfsfile *file, char *buf, 
				    unsigned short bufsize)
{
  char *p = buf+sizeof(FEA);
  PFEA pfea = (PFEA)buf;
  int icon_len;

  pfea->fEA = 0;
  pfea->cbName = strlen(icon_ea_name);
  pfea->cbValue = 0;

  if(bufsize < sizeof(FEA)+strlen(icon_ea_name)+1)
    return ERROR_BUFFER_OVERFLOW;

  strcpy(p, icon_ea_name);
  p+=strlen(icon_ea_name)+1;
  
  if(!(icon_len=icon_ea_len(file)))
    return NO_ERROR;

  if(buf+bufsize-p < icon_len)
    return ERROR_BUFFER_OVERFLOW;

  *(PUSHORT)p = EAT_ICON;
  p+=sizeof(USHORT);
  *(PUSHORT)p = icon_len-4;
  p+=sizeof(USHORT);

  build_icon_file(file, RESID_CUSTOMICON, p);
  pfea->cbValue = icon_len;

  return NO_ERROR;
}
