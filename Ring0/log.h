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

/* log.h
   Logging facility
*/

#ifndef _LOG_H
#define _LOG_H

#include <stdarg.h>

/* Size of log buffer */
#define LOGBUF_SIZE 16384

extern char logbuf[LOGBUF_SIZE];
extern unsigned short logbuf_head, logbuf_tail;
extern unsigned long logbuf_sem;

int printk(const unsigned char *fmt, ...);
void dump(const unsigned char *, unsigned);

#endif /* _LOG_H */