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
   Logging facility
*/

#define INCL_DOSERRORS
#define INCL_NOPMAPI
#include <os2.h>

#include <dhcalls.h>

#include <string.h>
#include <stdarg.h>
#include <dos.h>
#include <ctype.h>

#include "fsd.h"
#include "..\r0r3shar.h"
#include "r0struct.h"
#include "fsh.h"
#include "r0global.h"
#include "r0comm.h"
#include "log.h"
#include "vsprintf.h"

static int log_text(const unsigned char *text);

/* The log buffer is a circular queue */
char logbuf[LOGBUF_SIZE];
unsigned short logbuf_head, logbuf_tail;

/* A semaphore which is cleared when data is present. FS_FSCTL waits on this
   semaphore when returning data. */
unsigned long logbuf_sem;

/******************************************************************************
**
** log_text - put text in the log buffer
**
******************************************************************************/

static int log_text(const unsigned char *text)
{
  int rc, i;

  for( i=0; i<strlen(text); i++ ) {
    logbuf[logbuf_tail++] = text[i];
    if( logbuf_tail==LOGBUF_SIZE )
      logbuf_tail=0;
  }

  /* Clear the log semaphore */
  if ((rc = FSH_SEMCLEAR(&logbuf_sem)) != NO_ERROR)
    return rc;

  return NO_ERROR;
}

/******************************************************************************
**
** printk - formatted logging
**
******************************************************************************/

int printk(const unsigned char *fmt, ...) 
{
  va_list arg;
  char Buf[256];

  va_start(arg, fmt);
  vsprintf(Buf, fmt, arg);
  va_end(arg);

  return log_text(Buf);
}

/******************************************************************************
**
** dump - debug style hex dump
**
******************************************************************************/

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
