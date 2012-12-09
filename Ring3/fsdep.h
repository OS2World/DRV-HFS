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

/* fsdep.h
   File system dependent structures.
 */

#define VPDWORKAREASIZE 36
struct vpfsd {
  hfsvol *vol;
  unsigned short hVPB;
  char filler[30];
};

typedef struct vpfsd VPFSD, *PVPFSD;

#define CDDWORKAREASIZE 8
typedef struct cdfsd {
  char filler[8];
} CDFSD, *PCDFSD;

#define SFDWORKAREASIZE 30
typedef struct  sffsd {
  hfsfile *file;
  char filler[26];
} SFFSD, *PSFFSD;

#define FSFSD_WORK_SIZE     24
typedef struct fsfsd {
  hfsdir *dir;
  hfsdirent *this_ent;             /* Current directory entry */
  char *path;                      /* Full macintosh path of directory */
  char *pattern;                   /* Search pattern */
  unsigned short attr;             /* Search attributes */
  short flag;
  char filler[4];
} FSFSD, *PFSFSD;

/* Values for flag - the precise values are important */
#define RETURN_DOT    -2
#define RETURN_DOTDOT -1    
#define RETURN_FILES   0
