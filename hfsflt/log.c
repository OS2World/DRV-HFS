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

/* log.c
   Logging functions
   */

#define INCL_NOPMAPI
#define INCL_NOBASEAPI
#include <os2.h>

#include <ctype.h>

#include <dhcalls.h>
USHORT APIENTRY DevHelp_Yield(void);

#include "log.h"
#include "stdarg.h"
#include "vsprintf.h"

#define LOGBUF_SIZE 16384

static char logbuf[LOGBUF_SIZE+10] = {0};

void init_log(void)
{
  PQUEUEHDR q = (PQUEUEHDR)logbuf;
  q->QSize = LOGBUF_SIZE;
  DevHelp_QueueInit(logbuf);
}

USHORT read_log_data(ULONG phys_addr, USHORT count)
{
  PCHAR p;
  USHORT ModeFlag;

  DevHelp_PhysToVirt(phys_addr, count, &p, &ModeFlag);

  while(count) {
    USHORT rc = DevHelp_QueueRead(logbuf, p);
    if(rc==0) {
      p++;
      --count;
    }
    else
      DevHelp_Yield();
  }
  return 0;
}

USHORT read_nowait(BYTE *pc)
{
  USHORT rc = DevHelp_QueueRead(logbuf, pc);
  if(rc==0) {
    /* Success - put back character */
    DevHelp_QueueWrite(logbuf, *pc);
    return 0;
  }
  return 1;
}

/* put text in log buffer */
static int log_text(const char *text)
{
  USHORT rc, i;

  while(*text) {
    DevHelp_QueueWrite(logbuf, *text);
    text++;
  }
  return 0;
}

/* formatted logging */
int printk(const char *fmt, ...) 
{
  va_list arg;
  static char Buf[256] = {0};

  va_start(arg, fmt);
  vsprintf(Buf, fmt, arg);
  va_end(arg);

  return log_text(Buf);
}

/* debug style hex dump */
#define DUMP_WIDTH 16

void dump(const unsigned char *text, unsigned length)
{
  unsigned i=0;
  while( i<length ) {
    unsigned j;

    printk("%04X ", i);

    for( j=0; j<DUMP_WIDTH; j++ )
      if( i+j < length )
        printk("%02X ", (unsigned)text[i+j]);
      else
        printk("   ");
    printk("   ");
    for( j=0; j<DUMP_WIDTH; j++ )
      if( i+j < length ) {
        if( isprint(text[i+j]) )
          printk("%c", (unsigned)text[i+j]);
        else
          printk(".");
      }
    printk("\n");
    i += DUMP_WIDTH;
  }
}
