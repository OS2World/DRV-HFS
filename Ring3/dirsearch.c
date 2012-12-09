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

/* dirsearch.c
   Directory search functions.
*/

#define INCL_DOSERRORS
#define INCL_DOSFILEMGR
#define INCL_NOCOMMON
#define INCL_NOPMAPI
#include <os2.h>

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#ifdef DEBUG
#include <stdio.h>
#include "log.h"
#endif

#include "libhfs/hfs.h"
#include "libhfs/internal.h"
#include "fsd.h"
#include "fsdep.h"
#include "../r0r3shar.h"
#include "dirsearch.h"
#include "names.h"
#include "times.h"
#include "r3global.h"
#include "fsctl.h"
#include "ea.h"
#include "attr.h"
#include "error.h"

static unsigned short do_findnext(struct vpfsd *, struct fsfsd *, char *,
				  unsigned short, unsigned short *,
				  unsigned short, unsigned short);

/******************************************************************************
**
** FS_FINDFIRST - Begin a new directory search
**
** Parameters
** ----------
** struct cdfsi *pcdfsi                 pointer to FSD independant current directory
** struct cdfsd *pcdfsd                 pointer to FSD dependant current directory
** char *pName                          pointer to filename mask
** unsigned short iCurDirEnd            offset to the end of the current directory in pName
** unsigned short attr                  attribute mask
** struct fsfsi *pfsfsi                 pointer to FSD independant search record
** struct fsfsd *pfsfsd                 pointer to FSD dependant search record
** char *pData                          pointer to information area
** unsigned short cbData                size of area pointed to by pData
** unsigned short *pcMatch              maximum number of entries to return
**                                      number of entries actually returned
** unsigned short level                 level of information to return
** unsigned short flags                 indicates whether to return position information
**   values:
**     FF_NOPOS         don't return any position information
**     FF_GETPOS        return position information in buffer
** unsigned long *oError                offset where error occurred in GEA
**                                      list (EA levels only)
**
******************************************************************************/

#pragma argsused
short int FS_FINDFIRST(struct vpfsd *pvpfsd,
		       struct cdfsi *pcdfsi, struct cdfsd *pcdfsd,
                       char *pName, unsigned short iCurDirEnd,
                       unsigned short attr, struct fsfsi *pfsfsi,
                       struct fsfsd *pfsfsd, char *pData,
                       unsigned short cbData, unsigned short *pcMatch,
                       unsigned short level, unsigned short flags,
                       unsigned long *oError)
{
  char name[CCHMAXPATH], mac_pattern[CCHMAXPATH], *pattern_ofs;
  hfsdirent ent;
  unsigned short rc;
  
#ifdef DEBUG
  printf("FS_FINDFIRST, attr = %hu\n", attr);
#endif

  if(os2_to_mac_name(pName+2, name))
    return ERROR_PATH_NOT_FOUND;

  pattern_ofs = strrchr(name, ':');
  if(pattern_ofs==NULL)
    strcpy(mac_pattern,name);
  else {
    pattern_ofs++;
    strcpy(mac_pattern, pattern_ofs);
    *pattern_ofs = 0; /* zero-terminate path */
  }

  if(hfs_stat(pvpfsd->vol, name, &ent) < 0)
    return os2_error(errno, ERROR_ACCESS_DENIED);

  pfsfsd->dir = hfs_opendir(pvpfsd->vol, name);
  if(pfsfsd->dir == NULL)
    return os2_error(errno, ERROR_ACCESS_DENIED);

  pfsfsd->this_ent = malloc(sizeof(hfsdirent));
  *(pfsfsd->this_ent) = ent;

  if(!strcmp(mac_pattern, "*"))
    pfsfsd->flag = RETURN_DOT;    /* Start by returning . directory */
  else
    pfsfsd->flag = RETURN_FILES;
  
  pfsfsd->attr = attr;
  pfsfsd->path = (char *)malloc(CCHMAXPATH);
  strcpy(pfsfsd->path, name);
  pfsfsd->pattern = (char *)malloc(CCHMAXPATH);
  mac_to_os2_name_wild(mac_pattern, pfsfsd->pattern);

  rc = do_findnext(pvpfsd, pfsfsd, pData, cbData, 
		   pcMatch, level, flags);
  if( rc==ERROR_NO_MORE_FILES )
    return ERROR_PATH_NOT_FOUND;
  else
    return rc;

  /* Note that pData doesn't contain an EAOP for level==FIL_QUERYEASIZE,
     because the IFS takes care of that */
}

/******************************************************************************
**
** FS_FINDFROMNAME - Restart directory search
**
** Parameters
** ----------
** struct fsfsi *pfsfsi                 pointer to FSD independant search record
** struct fsfsd *pfsfsd                 pointer to FSD dependant search record
** char *pData                          pointer to information area
** unsigned short cbData                size of area pointed to by pData
** unsigned short *pcMatch              maximum number of entries to return*
**                                      number of entries actually returned
** unsigned short level                 level of information to return
** unsigned long position               position in directory to restart search from
** char *pName                          pointer to filename to restart search from
** unsigned short flags                 indicates whether to return position information
**   values:
**     FF_NOPOS         don't return any position information
**     FF_GETPOS        return position information in buffer
** unsigned long *oError                offset where error occurred in GEA
**                                      list (EA levels only)
**
******************************************************************************/

#pragma argsused
short int FS_FINDFROMNAME(struct vpfsd *pvpfsd,
			  struct fsfsi *pfsfsi, struct fsfsd *pfsfsd,
                          char *pData, unsigned short cbData,
                          unsigned short *pcMatch, unsigned short level,
                          unsigned long position, char *pName,
                          unsigned short flags, unsigned long *oError)
{
  /* We ignore the position. This is not nice. */
  return do_findnext(pvpfsd, pfsfsd, pData, cbData, 
		     pcMatch, level, flags);
}

/******************************************************************************
**
** FS_FINDNEXT - Continue directory search
**
** Parameters
** ----------
** struct fsfsi *pfsfsi                 pointer to FSD independant search record
** struct fsfsd *pfsfsd                 pointer to FSD dependant search record
** char *pData                          pointer to information area
** unsigned short cbData                size of area pointed to by pData
** unsigned short *pcMatch              maximum number of entries to return*
**                                      number of entries actually returned
** unsigned short level                 level of information to return
** unsigned short flag                  indicates whether to return position information
**   values:
**     FF_NOPOS         don't return any position information
**     FF_GETPOS        return position information in buffer
** unsigned long *oError                offset where error occurred in GEA
**                                      list (EA levels only)
**
******************************************************************************/

#pragma argsused
short int FS_FINDNEXT(struct vpfsd *pvpfsd,
		      struct fsfsi *pfsfsi, struct fsfsd *pfsfsd,
                      char *pData, unsigned short cbData, 
                      unsigned short *pcMatch, unsigned short level,
                      unsigned short flag, unsigned long *oError)
{
  return do_findnext(pvpfsd, pfsfsd, pData, cbData, 
		     pcMatch, level, flag);
}

/******************************************************************************
**
** FS_FINDCLOSE - End a directory search
**
** Parameters
** ----------
** struct fsfsi *pfsfsi                 pointer to FSD independant search record
** struct fsfsd *pfsfsd                 pointer to FSD dependant search record
**
******************************************************************************/

#pragma argsused
short int FS_FINDCLOSE(struct vpfsd *pvpfsd,
		       struct fsfsi *pfsfsi, struct fsfsd *pfsfsd)
{
  hfs_closedir(pfsfsd->dir);
  free(pfsfsd->path);
  free(pfsfsd->pattern);
  free(pfsfsd->this_ent);
  return NO_ERROR;
}

/*****************************************************************************
** do_findnext
******************************************************************************/

static unsigned short do_findnext(struct vpfsd *pvpfsd, struct fsfsd *pfsfsd,
				  char *buf, unsigned short buflen,
				  unsigned short *entries, 
				  unsigned short level, unsigned short flag)
{
  unsigned short max_entries=*entries;
  char *p=buf, *buf_end = buf+buflen, *gealist=NULL;
  char tempbuf[sizeof(FOUNDFILECOMMON)+5];
  unsigned short rc=NO_ERROR;

  if(level==FIL_QUERYEASFROMLIST) {
    /* Copy GEA list so it doesn't get overwritten */
    gealist = (char *)malloc(((PGEALIST)buf)->cbList);
    memcpy(gealist, buf, ((PGEALIST)buf)->cbList);
  }

  *entries=0;
  while( p<buf_end && *entries<max_entries ) {
    if( pfsfsd->flag==RETURN_DOT || pfsfsd->flag==RETURN_DOTDOT ) {
      int len = catrec_to_filebuf(tempbuf, pfsfsd->this_ent, level);
      int extra = flag==FF_GETPOS ? sizeof(long) : 0;
      int hidden = ((PFOUNDFILECOMMON)tempbuf)->attrFile & FILE_HIDDEN ? 1 : 0;

      if(!len)
	continue;
      if( !(pfsfsd->attr&FILE_DIRECTORY) && pfsfsd->attr )
	continue; /* if directory attr is not "may have" */
      if( pfsfsd->attr & 0xff00 & ~(MUST_HAVE_DIRECTORY | MUST_HAVE_HIDDEN))
	continue; /* if "must have" attr other than dir or hidden are set */
      if( pfsfsd->attr&MUST_HAVE_HIDDEN&0xff00 && !hidden)
	continue;
      if( !(pfsfsd->attr & FILE_HIDDEN) && hidden )
	continue;

      if( p+len+extra > buf_end ) {
	rc = ERROR_BUFFER_OVERFLOW;
	break;
      }
      if( flag==FF_GETPOS ) {
	*(long *)p = 0; /* not used */
	p += sizeof(long);
      }
      memcpy(p, tempbuf, len);
      p += len;

      if(level==FIL_QUERYEASIZE || level==FIL_QUERYEASFROMLIST || 
	 level==FIL_QUERYALLEAS) {
	*(PULONG)p = sizeof(ULONG);
	p+=sizeof(ULONG);
      }

      if(p+4 > buf_end) {
	rc = ERROR_BUFFER_OVERFLOW;
	break;
      }
      if( pfsfsd->flag==RETURN_DOT ) {
	*p++ = 1;
	strcpy(p, ".");
	p += 2;
      }
      else {
      	*p++ = 2;
	strcpy(p, "..");
	p += 3;
      }
      pfsfsd->flag++;
      (*entries)++;
    }
    else {
      unsigned char os2_name[CCHMAXPATH];
      hfsdirent ent;

      if(hfs_readdir(pfsfsd->dir, &ent) < 0)
	break;
      
      if(mac_to_os2_name(ent.name, os2_name))
	continue;

      if(wildcard_match(pfsfsd->pattern, os2_name)==NO_ERROR
	 && attrib_match(pfsfsd->attr, &ent)) {
	int len = catrec_to_filebuf(tempbuf, &ent, level);
	int extra = flag==FF_GETPOS ? sizeof(long) : 0;

#ifdef DEBUG
	printf("do_findnext, name = %s\n", os2_name);
#endif

	if(!len)
	  continue;

	if( p+len+extra > buf_end ) {
	  rc = ERROR_BUFFER_OVERFLOW;
	  break;
	}
	if( flag==FF_GETPOS ) {
	  *(long *)p = 0; /* not used */
	  p += sizeof(long);
	}
	memcpy(p, tempbuf, len);
	p += len;

	if(ent.flags & HFS_ISDIR) {
	  if(level==FIL_QUERYEASIZE || level==FIL_QUERYEASFROMLIST ||
	     level==FIL_QUERYALLEAS)
	    *(PULONG)p = sizeof(ULONG);
	}
	else {
	  char path[CCHMAXPATH];
	  hfsfile *file;

	  strcpy(path, pfsfsd->path);
	  strcat(path, ent.name);
	  file = hfs_open(pvpfsd->vol, path);
	  if(file==NULL)
	    continue;
	  rc = put_eainfo(file, p, buf_end-p, gealist, level);
	  hfs_close(file);
	}
	
	if(level==FIL_QUERYEASIZE || 
	   ((level==FIL_QUERYEASFROMLIST || level==FIL_QUERYALLEAS) && 
	    rc!=NO_ERROR))
	  p+=sizeof(ULONG);
	else if(level==FIL_QUERYEASFROMLIST || level==FIL_QUERYALLEAS)
	  p+=*(PULONG)p;
	  
	/* Transfer filename including terminating zero */
	if(p+strlen(os2_name)+2 > buf_end) {
	  rc = ERROR_BUFFER_OVERFLOW;
	  break;
	}
	*p++ = strlen(os2_name);
	strcpy(p, os2_name);
	p+=strlen(os2_name)+1;
	(*entries)++;
	rc = NO_ERROR;
      }
    }
  }
  if(gealist)
    free(gealist);
  if(rc!=NO_ERROR)
    return rc;
  if( *entries )
    return NO_ERROR;
  else
    return ERROR_NO_MORE_FILES;
}

unsigned short catrec_to_filebuf(char *buf, hfsdirent *ent, 
				 unsigned short level)
{
  PFOUNDFILECOMMON p = (PFOUNDFILECOMMON)buf;
  char *pc = buf+sizeof(FOUNDFILECOMMON);

  p->fdateCreation = unix_to_os2_date(ent->crdate);
  p->ftimeCreation = unix_to_os2_time(ent->crdate);
  p->fdateLastWrite = unix_to_os2_date(ent->mddate);
  p->ftimeLastWrite = unix_to_os2_time(ent->mddate);
  p->fdateLastAccess = unix_to_os2_date(0);
  p->ftimeLastAccess = unix_to_os2_time(0);
  p->attrFile = get_file_attributes(ent);
  if(ent->flags & HFS_ISDIR) {
    p->cbFile = 0;
    p->cbFileAlloc = 0;
  }
  else {
    p->cbFile = ent->dsize;
    p->cbFileAlloc = ent->dsize;
  }

  return pc-buf;
}

/* Place EA information for specified level in buffer */
unsigned short put_eainfo(hfsfile *file, char *buf, unsigned bufsize,
			  char *gealist, unsigned short level)
{
  unsigned short rc;
  char *tmpbuf;

  if(bufsize<sizeof(ULONG))
    return ERROR_BUFFER_OVERFLOW;

  switch(level) {
  case FIL_STANDARD:
    return NO_ERROR;
  case FIL_QUERYEASIZE:
    *(PULONG)buf = ea_size(file);
    return NO_ERROR;
  case FIL_QUERYEASFROMLIST:
  case FIL_QUERYALLEAS:
    /* Use temp buffer for EAs as shared memory is inaccessible from ring 0 */
    tmpbuf = malloc(bufsize);
    if(level==FIL_QUERYEASFROMLIST)
      rc = build_fealist_from_gealist(file, gealist, tmpbuf, bufsize);
    else
      rc = build_fealist(file, tmpbuf, bufsize);
    if(rc==NO_ERROR)
      memcpy(buf, tmpbuf, *(PULONG)tmpbuf);
    else
      *(PULONG)buf = *(PULONG)tmpbuf;
    free(tmpbuf);
    return rc;
  default:
    return ERROR_INVALID_LEVEL;
  }
}

unsigned short wildcard_match(char *pattern, char *text)
{
  WILDSTRUC w;
  unsigned long len_parm, len_data;

  strcpy(w.pattern, pattern);
  strcpy(w.text, text);

  /* Uppercase pattern and text */
  if(do_FSCtl(w.pattern, sizeof(w.pattern), &len_data,
	      NULL, 0, NULL, FSCTL_FUNC_UPPERCASE) != NO_ERROR) {
#ifdef DEBUG
    printf("FSCTL_FUNC_UPPERCASE failed!\n");
#endif
    return 0;
  }
  if(do_FSCtl(w.text, sizeof(w.text), &len_data,
	      NULL, 0, NULL, FSCTL_FUNC_UPPERCASE) != NO_ERROR) {
#ifdef DEBUG
    printf("FSCTL_FUNC_UPPERCASE failed!\n");
#endif
    return 0;
  }

  return do_FSCtl(NULL, 0, NULL, &w, sizeof(WILDSTRUC), &len_parm,
		  FSCTL_FUNC_WILDMATCH);
}

unsigned attrib_match(unsigned short attr, hfsdirent *ent)
{
  int hidden = get_file_attributes(ent) & FILE_HIDDEN ? 1 : 0;

  if(attr&MUST_HAVE_HIDDEN&0xff00 && !hidden)
    return 0;
  if(!(attr&FILE_HIDDEN) && hidden)
    return 0;

  if(attr&MUST_HAVE_SYSTEM&0xff00 || attr&MUST_HAVE_READONLY&0xff00)
    return 0;

  if(attr&MUST_HAVE_DIRECTORY&0xff00 && !(ent->flags & HFS_ISDIR))
    return 0;

  if(!(attr&FILE_DIRECTORY) && (ent->flags & HFS_ISDIR))
    return 0;

  if(!(attr&0x40) && !is_short_filename(ent->name))
    return 0;

  return 1;
}
