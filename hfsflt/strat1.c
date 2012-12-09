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

/* strat1.c
   Strategy 1 entry point.
*/

#define INCL_NOBASEAPI
#define INCL_NOPMAPI
#include <os2.h>

#include <devcmd.h>
#include <strat2.h>
#include <reqpkt.h>
#include <iorb.h>
#include <dhcalls.h>

#include "globals.h"
#include "hfsflt.h"
#ifdef DEBUG
#include "log.h"
#endif

/* Semaphore that controls open/close */
static ULONG open_sem = 0;

VOID FAR _loadds Strat1()
{
  PRPH           pRPH;
  USHORT         Cmd;

  _asm {
    mov word ptr pRPH[0], bx
    mov word ptr pRPH[2], es
  }

  Cmd = pRPH->Cmd;
  pRPH->Status = STATUS_DONE;
 
  if ((Cmd == CMDInitBase) && !InitComplete ) {
    Init((PRPINITIN) pRPH);
  }
#ifdef DEBUG
  else if(Cmd == CMDOpen) {
    /* Set the semaphore to prevent multiple opens */
/*     if(DevHelp_SemRequest((ULONG)&open_sem, 0)) { */
/*       pRPH->Status |= STERR; */
/*       pRPH->Status |= 0x14;  /- device already in use -/ */
/*     } */
  }
  else if(Cmd == CMDClose) {
/*     DevHelp_SemClear((ULONG)&open_sem); */
  }
  else if(Cmd == CMDINPUT) {
    PRP_RWV pRP = (PRP_RWV) pRPH;
    USHORT rc;
    rc = read_log_data(pRP->XferAddr, pRP->NumSectors);
    if(rc)
      pRPH->Status |= STERR | rc;
  }
  else if(Cmd == CMDNDR) {
    RP_NONDESTRUCREAD *pRP = (RP_NONDESTRUCREAD *) pRPH;
    if(read_nowait(&pRP->character));
  }      
#endif
  else
    pRPH->Status |= STATUS_ERR_UNKCMD;
}
