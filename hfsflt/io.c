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

/* io.c
   The input/output requests.
*/

#define INCL_NOBASEAPI
#define INCL_NOPMAPI
#include <os2.h>

#include <devcmd.h>

#define INCL_INITRP_ONLY
#include <reqpkt.h>

#include <iorb.h>
#include <dhcalls.h>
#include <addcalls.h>

#include "hfsflt.h"
#include "globals.h"
#include "diskstrc.h"

static USHORT copy_to_scatter_gather(PBYTE buf, ULONG count,
				     PSCATGATENTRY pSGList, USHORT cSGList,
				     ULONG offset);
static PIORB FAR SendIORBNotify(PIORB pIORB);
static int mac_to_os2_label(const unsigned char *src, unsigned char *dest);
static int os2_invalid_char(char c);

/* Read request. If the boot sector is requested, we read the third
   sector and check if it's a Hierarchical File System, and if this is
   the case, we fake the boot sector.
   */
void FilterRead(PIORB pIORB)
{
  PIORB_EXECUTEIO pIO = (PIORB_EXECUTEIO)pIORB;
  int fake=0;
  USHORT UnitHandle = UnitEntryTable[pIORB->UnitHandle].UnitHandle;
  ADDEntryPoint pADDEntry = UnitEntryTable[pIORB->UnitHandle].pADDEntry;

  if(pIO->RBA == 0) {
    ULONG phys_addr = ppDataSeg+(USHORT)SectorBuffer;
    
    if(read_sector(2, 1, phys_addr, pADDEntry, UnitHandle)==0)
      if(*(PUSHORT)SectorBuffer == HFS_SIGNATURE) {
	struct volinfo *vi = (struct volinfo *)SectorBuffer;
	PEXTENDED_BOOT boot = (PEXTENDED_BOOT)FAT_boot_sect;
	int i, j;
        unsigned char c;

	/* Transfer serial number and volume label */
	*(unsigned long *)boot->Boot_Serial = 
	  reorderlongword((unsigned char *)&vi->CrDate);
	
	for(i=0, j=0; i<vi->VNlen && i<sizeof(vi->VN) && j<11; i++) {
          c = vi->VN[i];
          if(c>=32 && c<=127 && !os2_invalid_char(c))
            boot->Boot_Vol_Label[j++] = c;
        }
	for( ; j<11; j++)
	  boot->Boot_Vol_Label[j] = ' ';

	fake = 1;
      }
  }
  /* Perform the read */
  CallOriginalADD(pIORB);

  if(fake)
    copy_to_scatter_gather(FAT_boot_sect, SECTOR_SIZE, 
			   pIO->pSGList, pIO->cSGList, 0);
  
  CommandDone(pIORB);
}

/* Read sectors into memory. */
USHORT read_sector(ULONG RBA, USHORT count, ULONG ppBuf,
		   ADDEntryPoint pADDEntry, USHORT UnitHandle)
{
  PIORB_EXECUTEIO pIORB = (PIORB_EXECUTEIO)IORBBuf;
  PSCATGATENTRY pScatGat = (PSCATGATENTRY) pIORB->iorbh.DMWorkSpace;

  pScatGat->ppXferBuf = ppBuf;
  pScatGat->XferBufLen = count*SECTOR_SIZE;

  pIORB->iorbh.Length = sizeof(IORB_EXECUTEIO);
  pIORB->iorbh.UnitHandle = UnitHandle;
  pIORB->iorbh.CommandCode = IOCC_EXECUTE_IO;
  pIORB->iorbh.CommandModifier = IOCM_READ;
  pIORB->cSGList = 1;
  pIORB->pSGList = pScatGat;
  pIORB->ppSGList = ppDataSeg + (USHORT)pScatGat;
  pIORB->RBA = RBA;
  pIORB->BlockCount = count;
  pIORB->BlocksXferred = 0;
  pIORB->BlockSize = SECTOR_SIZE;
  pIORB->Flags = 0;
  return SendIORB((PIORB)pIORB, pADDEntry);
}  

/* Copy bytes into a scatter/gather list at a specified offset. */
static USHORT copy_to_scatter_gather(PBYTE buf, ULONG count,
				     PSCATGATENTRY pSGList, USHORT cSGList,
				     ULONG offset)
{
  ADD_XFER_DATA XferData;

  XferData.Mode = BUFFER_TO_SGLIST;
  XferData.cSGList = cSGList;
  XferData.pSGList = pSGList;
  XferData.pBuffer = buf;
  XferData.numTotalBytes = count;
  XferData.iSGList = 0;
  XferData.SGOffset = offset;
  return f_ADD_XferBuffData(&XferData);
}

/* Send an IORB and wait for it to finish.
   Returns nonzero on error. */
USHORT SendIORB(PIORB pIORB, ADDEntryPoint ADDEntry)
{
  pIORB->NotifyAddress  = SendIORBNotify;
  pIORB->RequestControl = IORB_ASYNC_POST;
  pIORB->ErrorCode      = 0;
  pIORB->Status         = 0;

  ADDEntry(pIORB);

  DISABLE;

  while ( !(pIORB->Status & IORB_DONE) )
  {
    DevHelp_ProcBlock( (ULONG) pIORB, -1L, WAIT_IS_NOT_INTERRUPTABLE );
    DISABLE;
  }
  ENABLE;

  return pIORB->ErrorCode;
}

static PIORB FAR _loadds SendIORBNotify(PIORB pIORB)
{
  USHORT AwakeCount;

  DevHelp_ProcRun((ULONG) pIORB, &AwakeCount);

  return pIORB; /* Return value is unimportant */
}

static int os2_invalid_char(char c)
{
  return 
    c=='<' ||
    c=='>' ||
    c==':' ||
    c=='"' ||
    c=='/' ||
    c=='\\' ||
    c=='|' ||
    c=='?' ||
    c=='*';
}
