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

/*  stdarg.h

    These stdarg macros were derived from those in Borland C++ 3.1,
    with modifications for far argument list pointers.
*/

#ifndef _STDARG_H
#define _STDARG_H

typedef void far *va_list;

#define __size(x) ((sizeof(x)+sizeof(int)-1) & ~(sizeof(int)-1))

#define va_start(ap, parmN) ((void)((ap) = (va_list)((char far *)(&parmN)+__size(parmN))))

#define va_arg(ap, type) (*(type far *)(((*(char far * far *)&(ap))+=__size(type))-(__size(type))))
#define va_end(ap)          ((void)0)

#endif /* _STDARG_H */
