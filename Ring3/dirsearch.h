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

/* dirsearch.h
*/

#ifndef _DIRSEARCH_H
#define _DIRSEARCH_H

/* Undocumented info level - query all EAs */
#define FIL_QUERYALLEAS 4

/* Found file buffer, common part */
typedef struct FoundFileCommon {
  FDATE   fdateCreation;            /*  Date of file creation. */
  FTIME   ftimeCreation;            /*  Time of file creation. */
  FDATE   fdateLastAccess;          /*  Date of last access. */
  FTIME   ftimeLastAccess;          /*  Time of last access. */
  FDATE   fdateLastWrite;           /*  Date of last write. */
  FTIME   ftimeLastWrite;           /*  Time of last write. */
  ULONG   cbFile;                   /*  Size of file. */
  ULONG   cbFileAlloc;              /*  Allocation size. */
  USHORT  attrFile;                 /*  File attributes. */
  /* Note: attrFile is USHORT. It is the same as in FILEFINDBUF, the 16-bit
           found file struct. In FILEFINDBUF3 and 4 the attrFile
           field is an ULONG.
  */
} FOUNDFILECOMMON, *PFOUNDFILECOMMON;

unsigned short catrec_to_filebuf(char *, hfsdirent *, unsigned short);
unsigned short wildcard_match(char *, char *);
unsigned attrib_match(unsigned short, hfsdirent *);
unsigned short put_eainfo(hfsfile *, char *, unsigned, char *, unsigned short);

#endif /* _DIRSEARCH_H */
