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

/* globals.h
   External declarations for the internal data.
   */

#ifndef _GLOBALS_H
#define _GLOBALS_H

#include "hfsflt.h"

/* Static data */

extern PFN       Device_Help;
extern ULONG     ppDataSeg;
extern USHORT    InitComplete;
extern USHORT    ADDHandle;
extern USHORT    UnitEntryCount;
extern UNITENTRY UnitEntryTable[MAX_UNITS];
extern PIORB     IORBQueueHead, IORBQueueTail;
extern USHORT    QueueLocked, QueueBusy;
extern UCHAR     SectorBuffer[512];
extern char      FAT_boot_sect[512];
extern UCHAR     IORBBuf[MAX_IORB_SIZE];

extern USHORT    EndData;

/* Initialization data */

extern BYTE      DeviceTable[MAX_DEVICETABLE_SIZE];
extern MSGTABLE  MsgTbl;

#endif
