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

/* diskstrc.h
   Disk structures
*/

#ifndef _DISKSTRC_H
#define _DISKSTRC_H

#define HFS_SIGNATURE 0x4442

/* Extended BPB */
typedef struct Extended_BPB {
  unsigned short BytePerSector;
  unsigned char SectorPerCluster;
  unsigned short ReservedSectors;
  unsigned char NumberOfFats;
  unsigned short RootEntries;
  unsigned short TotalSectors;
  unsigned char MediaDescriptor;
  unsigned short SectorsPerFat;
  unsigned short SectorsPerTrack;
  unsigned short Heads;
  unsigned long HiddenSectors;
  unsigned long Ext_TotalSectors;
} EXTENDED_BPB;

/* Extended DOS boot block. */
typedef struct Extended_Boot {
  unsigned char Boot_jmp[3];
  unsigned char Boot_OEM[8];
  struct Extended_BPB Boot_BPB;
  unsigned char Boot_DriveNumber;
  unsigned char Boot_CurrentHead;
  unsigned char Boot_Sig;              /* = 0x29. Indicate Extended Boot */
  unsigned char Boot_Serial[4];
  unsigned char Boot_Vol_Label[11];
  unsigned char Boot_System_ID[8];
} EXTENDED_BOOT, *PEXTENDED_BOOT;

typedef unsigned short WORD;
typedef unsigned long LONGWORD;
typedef unsigned char BYTE;

#define reorderlongword(byteptr)  (                                         \
                                   (((LONGWORD)(*(byteptr)))<<24)           \
                                  |(((LONGWORD)(*((byteptr)+1)))<<16)       \
                                  |(((LONGWORD)(*((byteptr)+2)))<<8)        \
                                  |((LONGWORD)(*((byteptr)+3)))             \
                                  )         

struct volinfo {
  WORD     SigWord;      /* 0x4244 */
  LONGWORD CrDate;       /* initialization date/time */
  LONGWORD LsMod;        /* last modification date */
  WORD     Atrb;         /* vol. attributes */
  WORD     NmFls;        /* Number of files in dir */
  WORD     VBMSt;        /* 1st block of vol bitmap */
  WORD     AllocPtr;     /* Internal Use Only */
  WORD     NmAlBlks;     /* number of allocation blocks */
  LONGWORD AlBlkSiz;     /* Size of an allocation block */
  LONGWORD ClpSiz;       /* clump size */
  WORD     AlBlSt;       /* first block in Bit Map */
  LONGWORD NxtCNID;      /* next unused dir ID or fn */
  WORD     FreeBks;      /* number of free allocation block */
  BYTE     VNlen;        /* Length of Volume Name */
  BYTE     VN[26];       /* Volume Name Chars */
  LONGWORD VolBkUp;      /* Date of Last Backup */
  WORD     VSeqNum;      /* Internal Use */
  LONGWORD WrCnt;        /* Volume Write Count */
  LONGWORD XTClpSiz;     /* Clump size of extents tree file */
  LONGWORD CTClpSiz;     /* Clump size of catalog tree file */
  WORD     NmRtDirs;     /* Number of directories in Root */
  LONGWORD FilCnt;       /* Number of files on volume */
  LONGWORD DirCnt;       /* Number of dirs on volume */
  BYTE     FndrInfo[32]; /* Finder Information */
  WORD     VCSize;       /* Internal Use */
  WORD     VCBMSize;     /* Internal Use */
  WORD     CtlCSize;     /* Internal Use */
  /*
  LONGWORD XTFlSize;     /- Length of extents tree -/
  EXTENT_RECORD   XTExtRec; /- Extents Record for Extents Tree -/
  LONGWORD CTFlSize;     /- Length of catalog tree -/
  EXTENT_RECORD   CTExtRec; /- First extent record for catalog tree -/
  */
};

#endif /* _DISKSTRC_H */
