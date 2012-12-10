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

/* trans.h
   Character set translation.
*/

#ifndef _TRANS_H
#define _TRANS_H

/* Tables that maps Macintosh characters in the range 128-255 to the
   corresponding character in codepage 850, if any. */
const unsigned char mac_to_cp850_map[] = {
  142, 143, 128, 144, 165, 153, 154, 160, 
  133, 131, 132, 198, 134, 135, 130, 138,
  136, 137, 161, 141, 140, 139, 164, 162,
  149, 147, 148, 228, 163, 151, 150, 129,
    0, 248, 189, 156, 245, 250, 244, 225, 
  169, 184,   0, 239, 249,   0, 146, 157,
    0, 241,   0,   0, 190, 230, 208,   0,
    0,   0,   0, 166, 167,   0, 145, 155,
  168, 173, 170,   0,   0,   0,   0, 174,
  175,   0, 255, 183, 199, 229,   0,   0,
  240,   0,   0,   0,   0,   0, 246,   0,
  152,   0,   0, 207,   0,   0,   0,   0,
    0,   0, 247,   0,   0, 182, 210, 181,
  211, 212, 214, 215, 216, 222, 224, 226,
    0, 227, 233, 234, 235,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0
};

/* The inverse is built when it is needed */
static unsigned char cp850_to_mac_map[128];

/* Macros for the maps */
#define mac_to_cp850(c) (mac_to_cp850_map[(c)-128])
#define cp850_to_mac(c) (cp850_to_mac_map[(c)-128])

/* Characters used in the filename conventions */
#define MAC_PATH_SEPARATOR      ':'
#define OS2_PATH_SEPARATOR      '\\'

#define MAX_MAC_FN_LEN          31

const unsigned char os2_invalid_chars[] = 
  {'<', '>', ':', '"', '/', '\\', '|', '?', '*', 0};

#endif _TRANS_H
