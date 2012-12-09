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

/* filter.c
   IORB processing.
*/

#define INCL_NOBASEAPI
#define INCL_NOPMAPI
#include <os2.h>

#include <devcmd.h>

#define INCL_INITRP_ONLY
#include <reqpkt.h>

#include <iorb.h>
#include <dhcalls.h>

#include "hfsflt.h"
#include "globals.h"
#ifdef DEBUG
#include "log.h"
#endif

static void NotifyDone(PIORB pIORB);
static void IORBError(PIORB pIORB, USHORT ErrorCode);
static void EnQueue(PIORB pIORB);
static PIORB DeQueue(void);
static void ServiceQueue(void);
static void FilterHandler(PIORB pIORB);
static void UnsupportedCmd(PIORB pIORB);
static void FilterDeviceTable(PIORB pIORB);

VOID FAR _loadds FilterIORBEntry(PIORB pIORB)
{
#ifdef DEBUG
  printk("Command: %hu, modifier: %hu, unit: %hX\r\n",
	 pIORB->CommandCode, pIORB->CommandModifier, pIORB->UnitHandle);
#endif
  if(pIORB->CommandCode == IOCC_CONFIGURATION)
    pIORB->UnitHandle = 0;   /* Map configuration requests to first unit */
  
  if(pIORB->UnitHandle >= UnitEntryCount) {
   /* No such unit number */
    IORBError(pIORB, IOERR_UNIT_NOT_ALLOCATED);
    NotifyDone(pIORB);
    return;
  }

  DISABLE;
  EnQueue(pIORB);
  ENABLE;
  ServiceQueue();
}

/* Process an IORB */
static void FilterHandler(PIORB pIORB)
{
  switch(pIORB->CommandCode) {
  case IOCC_CONFIGURATION:
    switch(pIORB->CommandModifier) {
    case IOCM_COMPLETE_INIT:
      CommandDone(pIORB);
      break;
    case IOCM_GET_DEVICE_TABLE:
      FilterDeviceTable(pIORB);
      break;
    default:
      UnsupportedCmd(pIORB);
      break;
    }
    break;
  case IOCC_EXECUTE_IO:
    switch(pIORB->CommandModifier) {
    case IOCM_READ:
      FilterRead(pIORB);
      break;
    default:
      CallOriginalADD(pIORB);
      CommandDone(pIORB);
    }
    break;
  case IOCC_UNIT_CONTROL:
    switch(pIORB->CommandModifier) {
    case IOCM_ALLOCATE_UNIT:
      if(UnitEntryTable[pIORB->UnitHandle].Allocated)
	IORBError(pIORB, IOERR_UNIT_ALLOCATED);
      else
	UnitEntryTable[pIORB->UnitHandle].Allocated = 1;
      CommandDone(pIORB);
      break;
    case IOCM_DEALLOCATE_UNIT:
      if(!UnitEntryTable[pIORB->UnitHandle].Allocated)
	IORBError(pIORB, IOERR_UNIT_NOT_ALLOCATED);
      else
	UnitEntryTable[pIORB->UnitHandle].Allocated = 0;
      CommandDone(pIORB);
      break;
    default:
      CallOriginalADD(pIORB);
      CommandDone(pIORB);
    }
    break;
  case IOCC_FORMAT:
  case IOCC_UNIT_STATUS:
  case IOCC_DEVICE_CONTROL:
  case IOCC_ADAPTER_PASSTHRU:
  case IOCC_GEOMETRY:
  case IOCC_RESOURCE:
  default:
    CallOriginalADD(pIORB);
    CommandDone(pIORB);
    break;
  }
}

/* Pass a request downstream */
void CallOriginalADD(PIORB pIORB)
{
  USHORT SaveUnitHandle, SaveReqCntrl;
  NotifyAddr SaveNotify;

  SaveUnitHandle = pIORB->UnitHandle;
  SaveReqCntrl   = pIORB->RequestControl;
  SaveNotify     = pIORB->NotifyAddress;

  pIORB->UnitHandle = UnitEntryTable[SaveUnitHandle].UnitHandle;
  pIORB->RequestControl &= ~IORB_CHAIN;
  SendIORB(pIORB, UnitEntryTable[SaveUnitHandle].pADDEntry);

  pIORB->UnitHandle     = SaveUnitHandle;
  pIORB->RequestControl = SaveReqCntrl;
  pIORB->NotifyAddress  = SaveNotify;
}

/* Called when an IORB has been processed. Notifies the caller
   and restarts the queue. */
void CommandDone(PIORB pIORB)
{
  QueueBusy = 0;        /* Inidicate the IORB has been processed */
  NotifyDone(pIORB);    /* Notify the caller */
  ServiceQueue();       /* Attempt to restart the queue */
}  

/* Unsupported IO request */
static void UnsupportedCmd(PIORB pIORB)
{
  IORBError(pIORB, IOERR_CMD_NOT_SUPPORTED);
  CommandDone(pIORB);
}

/* Call the notification functions (if any) for an IORB. */
static void NotifyDone(PIORB pIORB)
{
  pIORB->Status |= IORB_DONE;
  if( pIORB->RequestControl & IORB_ASYNC_POST )
    pIORB->NotifyAddress(pIORB);
}

/* Set error code in IORB. */
static void IORBError(PIORB pIORB, USHORT ErrorCode)
{
  if(ErrorCode) {
    pIORB->Status |= IORB_ERROR;
    pIORB->ErrorCode = ErrorCode;
  }
}

/* Add an IORB (and any chained IORBs) to the queue. 
   Must be called with interrupts disabled. */
void EnQueue(PIORB pIORB)
{
  if(IORBQueueTail)
    IORBQueueTail->pNxtIORB = pIORB;
  else
    IORBQueueHead = pIORB;
  while(pIORB->RequestControl & IORB_CHAIN)
    pIORB = pIORB->pNxtIORB;
  IORBQueueTail = pIORB;
  pIORB->pNxtIORB = NULL;
}

/* Remove the first element from the queue. Returns NULL if queue is empty. 
   Must be called with interrupts disabled. */
PIORB DeQueue(void)
{
  PIORB ReturnIORB = IORBQueueHead;

  if(IORBQueueHead)
    IORBQueueHead = IORBQueueHead->pNxtIORB;

  if(IORBQueueHead==NULL)
    IORBQueueTail = NULL;

  return ReturnIORB;
}

/* Service the IORB queue. */
static void ServiceQueue(void)
{
  PIORB pIORB;

  DISABLE;
  if(!QueueLocked) {
    QueueLocked = 1;
    /* Service the queue while there is still an IORB to process
       and no previous IORB is busy. */
    while((pIORB=DeQueue()) != NULL && !QueueBusy) {
      QueueBusy = 1;
      ENABLE;

      FilterHandler(pIORB);
      DISABLE;
    }
    QueueLocked = 0;
  }
  ENABLE;
}

/* Create a device table. */
static void FilterDeviceTable(PIORB pIORB)
{
  PIORB_CONFIGURATION pIO = (PIORB_CONFIGURATION)pIORB;

  if(pIO->DeviceTableLen < 6)
    IORBError(pIORB, IOERR_CMD_SYNTAX);
  else {
    pIO->pDeviceTable->ADDLevelMajor = ADD_LEVEL_MAJOR;
    pIO->pDeviceTable->ADDLevelMinor = ADD_LEVEL_MINOR;
    pIO->pDeviceTable->ADDHandle = ADDHandle;
    pIO->pDeviceTable->TotalAdapters = 0;
  }
  CommandDone(pIORB);
}
