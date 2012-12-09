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

#ifndef _INITUTIL_H
#define _INITUTIL_H

USHORT GetDeviceTable(ADDEntryPoint ADDEntry, PDEVICETABLE DeviceTable,
		      USHORT DeviceTableLen);
USHORT GetGeometry(ADDEntryPoint ADDEntry, USHORT UnitHandle, 
		   PGEOMETRY pGeometry);
USHORT AllocateUnit(ADDEntryPoint ADDEntry, USHORT UnitHandle);
USHORT DeAllocateUnit(ADDEntryPoint ADDEntry, USHORT UnitHandle);
USHORT ChangeUnitInfo(ADDEntryPoint ADDEntry, USHORT UnitHandle,
		      PUNITINFO pUnitInfo);
void PrintMessage(char *message);

#endif /* _INITUTIL_H */
