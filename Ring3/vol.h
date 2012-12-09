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

/* vol.h
*/

#ifndef _VOL_H
#define _VOL_H

/*
   Declaration for alternate FSINFO struct, which is incorrect in
   IBM VisualAge C++ BSEDOS.H
*/

typedef struct _FSINFO2
{
  ULONG ulVSN;
  VOLUMELABEL vol;
} FSINFO2;
typedef FSINFO2 *PFSINFO2;

#endif /* _VOL_H */
