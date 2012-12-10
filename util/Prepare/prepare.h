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

/* prepare.h
*/

/* from boot.c */
extern char FAT_boot_sect[512];

typedef signed char	Char;
typedef unsigned char	UChar;
typedef signed char	SignedByte;
typedef signed short	Integer;
typedef unsigned short	UInteger;
typedef signed long	LongInt;
typedef unsigned long	ULongInt;
typedef char		Str15[16];
typedef char		Str31[32];
typedef long		OSType;

typedef struct {
  UInteger	xdrStABN;	/* first allocation block */
  UInteger	xdrNumABlks;	/* number of allocation blocks */
} ExtDescriptor;

typedef ExtDescriptor ExtDataRec[3];

typedef struct {
  Integer	drSigWord;	/* volume signature (0x4244 for HFS) */
  LongInt	drCrDate;	/* date and time of volume creation */
  LongInt	drLsMod;	/* date and time of last modification */
  Integer	drAtrb;		/* volume attributes */
  UInteger	drNmFls;	/* number of files in root directory */
  UInteger	drVBMSt;	/* first block of volume bit map (always 3) */
  UInteger	drAllocPtr;	/* start of next allocation search */
  UInteger	drNmAlBlks;	/* number of allocation blocks in volume */
  ULongInt	drAlBlkSiz;	/* size (in bytes) of allocation blocks */
  ULongInt	drClpSiz;	/* default clump size */
  UInteger	drAlBlSt;	/* first allocation block in volume */
  LongInt	drNxtCNID;	/* next unused catalog node ID (dir/file ID) */
  UInteger	drFreeBks;	/* number of unused allocation blocks */
  char          drVNlen;        /* length of volume name */
  char		drVN[27];	/* volume name (1-27 chars) */
  LongInt	drVolBkUp;	/* date and time of last backup */
  Integer	drVSeqNum;	/* volume backup sequence number */
  ULongInt	drWrCnt;	/* volume write count */
  ULongInt	drXTClpSiz;	/* clump size for extents overflow file */
  ULongInt	drCTClpSiz;	/* clump size for catalog file */
  UInteger	drNmRtDirs;	/* number of directories in root directory */
  ULongInt	drFilCnt;	/* number of files in volume */
  ULongInt	drDirCnt;	/* number of directories in volume */
  LongInt	drFndrInfo[8];	/* information used by the Finder */
  UInteger	drVCSize;	/* size (in blocks) of volume cache */
  UInteger	drVBMCSize;	/* size (in blocks) of volume bitmap cache */
  UInteger	drCtlCSize;	/* size (in blocks) of common volume cache */
  ULongInt	drXTFlSize;	/* size (in bytes) of extents overflow file */
  ExtDataRec	drXTExtRec;	/* first extent record for extents file */
  ULongInt	drCTFlSize;	/* size (in bytes) of catalog file */
  ExtDataRec	drCTExtRec;	/* first extent record for catalog file */
} MDB;

