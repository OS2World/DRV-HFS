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

/* initutil.c
   Helper functions for the driver initialization.
   */

#define INCL_NOBASEAPI
#define INCL_NOPMAPI
#include <os2.h>

#include <devcmd.h>
#include <devclass.h>
#include <dskinit.h>

#define INCL_INITRP_ONLY
#include <reqpkt.h>

#include <iorb.h>
#include <addcalls.h>

#include <dhcalls.h>

#include "globals.h"
#include "hfsflt.h"
#include "initutil.h"

static UCHAR InitIORB[MAX_IORB_SIZE] = {0};

USHORT GetDeviceTable(ADDEntryPoint ADDEntry, PDEVICETABLE DeviceTable,
		      USHORT DeviceTableLen)
{
  PIORB_CONFIGURATION pIOCF = (PIORB_CONFIGURATION) InitIORB;

  pIOCF->iorbh.Length          = sizeof(IORB_CONFIGURATION);
  pIOCF->iorbh.UnitHandle      = 0;
  pIOCF->iorbh.CommandCode     = IOCC_CONFIGURATION;
  pIOCF->iorbh.CommandModifier = IOCM_GET_DEVICE_TABLE;
  pIOCF->iorbh.RequestControl  = IORB_ASYNC_POST;

  pIOCF->pDeviceTable          = DeviceTable;
  pIOCF->DeviceTableLen        = DeviceTableLen;

  return SendIORB((PIORB) pIOCF, ADDEntry);
}

/* Get device geometry */
USHORT GetGeometry(ADDEntryPoint ADDEntry, USHORT UnitHandle, 
		   PGEOMETRY pGeometry)
{
  PIORB_GEOMETRY pIOGEO = (PIORB_GEOMETRY) InitIORB;

  pIOGEO->iorbh.Length          = sizeof(IORB_GEOMETRY);
  pIOGEO->iorbh.UnitHandle      = UnitHandle;
  pIOGEO->iorbh.CommandCode     = IOCC_GEOMETRY;
  pIOGEO->iorbh.CommandModifier = IOCM_GET_DEVICE_GEOMETRY;
  pIOGEO->pGeometry             = pGeometry;
  pIOGEO->GeometryLen           = sizeof(GEOMETRY);

  return SendIORB((PIORB) pIOGEO, ADDEntry);
}

/* Allocate a unit */
USHORT AllocateUnit(ADDEntryPoint ADDEntry, USHORT UnitHandle)
{
  PIORB_UNIT_CONTROL pIOUC = (PIORB_UNIT_CONTROL) InitIORB;

  pIOUC->iorbh.Length          = sizeof(IORB_UNIT_CONTROL);
  pIOUC->iorbh.UnitHandle      = UnitHandle;
  pIOUC->iorbh.CommandCode     = IOCC_UNIT_CONTROL;
  pIOUC->iorbh.CommandModifier = IOCM_ALLOCATE_UNIT;

  return SendIORB((PIORB) pIOUC, ADDEntry);
}

/* Deallocate a unit */
USHORT DeAllocateUnit(ADDEntryPoint ADDEntry, USHORT UnitHandle)
{
  PIORB_UNIT_CONTROL pIOUC = (PIORB_UNIT_CONTROL) InitIORB;

  pIOUC->iorbh.Length          = sizeof(IORB_UNIT_CONTROL);
  pIOUC->iorbh.UnitHandle      = UnitHandle;
  pIOUC->iorbh.CommandCode     = IOCC_UNIT_CONTROL;
  pIOUC->iorbh.CommandModifier = IOCM_DEALLOCATE_UNIT;

  return SendIORB((PIORB) pIOUC, ADDEntry);
}

/* Change unit info for a unit */
USHORT ChangeUnitInfo(ADDEntryPoint ADDEntry, USHORT UnitHandle,
		      PUNITINFO pUnitInfo)
{
  PIORB_UNIT_CONTROL pIOUC = (PIORB_UNIT_CONTROL) InitIORB;

  pIOUC->iorbh.Length          = sizeof(IORB_UNIT_CONTROL);
  pIOUC->iorbh.UnitHandle      = UnitHandle;
  pIOUC->iorbh.CommandCode     = IOCC_UNIT_CONTROL;
  pIOUC->iorbh.CommandModifier = IOCM_CHANGE_UNITINFO;
  pIOUC->Flags                 = 0;
  pIOUC->pUnitInfo             = pUnitInfo;
  pIOUC->UnitInfoLen           = sizeof(UNITINFO);

  return SendIORB((PIORB) pIOUC, ADDEntry);
}

/* Print a message */
void PrintMessage(char *message)
{
  MsgTbl.MsgStrings[0] = message;
  DevHelp_Save_Message((NPBYTE)&MsgTbl);
}
