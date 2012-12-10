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

/* diskstruc.h
   Disk structures
*/

#ifndef _DISKSTRUC_H
#define _DISKSTRUC_H

#pragma pack(1)

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

#endif /* _DISKSTRUC_H */
