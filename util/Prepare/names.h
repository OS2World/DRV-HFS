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

/* names.h
   Filename translation functions.
*/

#ifndef _NAMES_H
#define _NAMES_H

/* Filename translation methods */
#define FN_TR_CP850        1
#define FN_TR_7BIT         2
#define FN_TR_8BIT         3

int os2_to_mac_name(const unsigned char *, unsigned char *);
int mac_to_os2_name(const unsigned char *, unsigned char *);
int mac_to_os2_name_wild(const unsigned char *, unsigned char *);
unsigned is_short_filename(char *);

#endif /* _NAMES_H */
