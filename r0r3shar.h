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

/* r0r3shar.h
   Shared structures between the ring 0/ring 3 pieces
*/

#ifndef _R0R3SHAR_H
#define _R0R3SHAR_H

#if defined (__OS2__)
typedef void *LIN;
#pragma pack(1)
#endif

/* Mount status codes for the mount list */
#define MTSTAT_FREE        0
#define MTSTAT_MOUNTED     1
#define MTSTAT_DUPLICATE   2
#define MTSTAT_REMOVED     3
#define MTSTAT_ERROR      -1

/* FSCtl function codes */

typedef unsigned short FSCTLFUNCCODE;

#define FSCTL_FUNC_INIT                            0x9000
#define FSCTL_FUNC_NEXT                            0x9001
#define FSCTL_FUNC_KILL                            0x9002
#define FSCTL_FUNC_GETLOGDATA                      0x9003
#define FSCTL_FUNC_READSECTORS                     0x9004
#define FSCTL_FUNC_WRITESECTORS                    0x9005
#define FSCTL_FUNC_FINDDUPHVPB                     0x9006
#define FSCTL_FUNC_GET_MTSTAT                      0x9007
#define FSCTL_FUNC_SET_MTSTAT                      0x9008
#define FSCTL_FUNC_WILDMATCH                       0x9009
#define FSCTL_FUNC_UPPERCASE                       0x900a
#define FSCTL_FUNC_SETVOLUME                       0x900b
#define FSCTL_FUNC_WRITESECTORS_NOHARDERR          0x900c

/* User FSCtl functions start at 0x9100 and are defined in hfsctl.h */

typedef ULONG PID32;

typedef struct InitStruc {
  LIN OpData;
  LIN Buf;
  PID32 CPPID;
} INITSTRUC, *PINITSTRUC;

typedef struct RWStruc {
  unsigned short n_sectors;
  unsigned long starting_sector;
  unsigned short hVPB;
} RWSTRUC, *PRWSTRUC;

typedef struct WildStruc {
  char pattern[256], text[256];
} WILDSTRUC, *PWILDSTRUC;

typedef struct VolParmStruc {
  struct vpfsi vpfsi;
  struct vpfsd vpfsd;
} VOLPARMSTRUC, *PVOLPARMSTRUC;

typedef struct MtStatStruc {
  unsigned short drive;
  unsigned short status;
  unsigned short hVPB;
} MTSTATSTRUC, *PMTSTATSTRUC;

/* Control program function codes */

typedef unsigned short CPFUNCCODE;

#define CPFC_ATTACH                 0x8000
#define CPFC_CANCELLOCKREQUEST      0x8001
#define CPFC_CHDIR                  0x8002
#define CPFC_CHGFILEPTR             0x8003
#define CPFC_CLOSE                  0x8004
#define CPFC_COMMIT                 0x8005
#define CPFC_COPY                   0x8006
#define CPFC_DELETE                 0x8007
#define CPFC_FILEATTRIBUTE          0x8008
#define CPFC_FILEINFO               0x8009
#define CPFC_FILELOCKS              0x800a
#define CPFC_FINDCLOSE              0x800b
#define CPFC_FINDFIRST              0x800c
#define CPFC_FINDFROMNAME           0x800d
#define CPFC_FINDNEXT               0x800e
#define CPFC_FLUSHBUF               0x800f
#define CPFC_FSINFO                 0x8010
#define CPFC_MKDIR                  0x8011
#define CPFC_MOVE                   0x8012
#define CPFC_MOUNT                  0x8013
#define CPFC_NEWSIZE                0x8014
#define CPFC_OPENCREATE             0x8015
#define CPFC_PATHINFO               0x8016
#define CPFC_PROCESSNAME            0x8017
#define CPFC_READ                   0x8018
#define CPFC_RMDIR                  0x8019
#define CPFC_VERIFYUNCNAME          0x801a
#define CPFC_WRITE                  0x801b

/* User functions */
#define CPFC_GET_RESFORK_SIZE       0x8100
#define CPFC_READ_RESFORK           0x8101
#define CPFC_SYNC                   0x8102
#define CPFC_SYNC_ALL               0x8103

typedef struct OpData {
  union {
    CPFUNCCODE funccode;
    unsigned short rc;
  } i;
  union {
    struct {
      unsigned short flag;
      struct vpfsd vpfsd;
      struct cdfsd cdfsd;
      unsigned short Len;
      char Dev[CCHMAXPATH];
    } attach;
    struct {
      struct sffsi sffsi;
      struct sffsd sffsd;
      struct filelock LockRange;
    } cancellockrequest;
    struct {
      struct vpfsd vpfsd;
      unsigned short flag;
      struct cdfsi cdfsi;
      struct cdfsd cdfsd;
      unsigned short iCurDirEnd;
      char Dir[CCHMAXPATH];
    } chdir;
    struct {
      struct vpfsd vpfsd;
      struct sffsi sffsi;
      struct sffsd sffsd;
      long offset;
      unsigned short type;
      unsigned short IOflag;
    } chgfileptr;
    struct {
      struct vpfsd vpfsd;
      unsigned short type;
      unsigned short IOflag;
      struct sffsi sffsi;
      struct sffsd sffsd;
    } close;
    struct {
      struct vpfsd vpfsd;
      unsigned short type;
      unsigned short IOflag;
      struct sffsi sffsi;
      struct sffsd sffsd;
    } commit;
    struct {
      struct vpfsd vpfsd;
      unsigned short flag;
      struct cdfsi cdfsi;
      struct cdfsd cdfsd;
      unsigned short iSrcCurDirEnd;
      unsigned short iDstCurDirEnd;
      unsigned short nameType;
      char Src[CCHMAXPATH];
      char Dst[CCHMAXPATH];
    } copy;
    struct {
      struct vpfsd vpfsd;
      struct cdfsi cdfsi;
      struct cdfsd cdfsd;
      unsigned short iCurDirEnd;
      char File[CCHMAXPATH];
    } fsdelete;
    struct {
      struct vpfsd vpfsd;
      unsigned short flag;
      struct cdfsi cdfsi;
      struct cdfsd cdfsd;
      unsigned short iCurDirEnd;
      unsigned short Attr;
      char Name[CCHMAXPATH];
    } fileattribute;
    struct {
      struct vpfsd vpfsd;
      unsigned short flag;
      struct sffsi sffsi;
      struct sffsd sffsd;
      unsigned short level;
      unsigned short cbData;
      unsigned short IOflag;
      unsigned long oError;
    } fileinfo;
    struct {
      struct sffsi sffsi;
      struct sffsd sffsd;
      struct filelock UnLockRange;
      struct filelock LockRange;
      unsigned long timeout;
      unsigned long flags;
    } filelocks;
    struct {
      struct vpfsd vpfsd;
      struct fsfsi fsfsi;
      struct fsfsd fsfsd;
    } findclose;
    struct {
      struct vpfsd vpfsd;
      struct cdfsi cdfsi;
      struct cdfsd cdfsd;
      unsigned short iCurDirEnd;
      unsigned short attr;
      struct fsfsi fsfsi;
      struct fsfsd fsfsd;
      unsigned short cbData;
      unsigned short cMatch;
      unsigned short level;
      unsigned short flags;
      unsigned long oError;
      char Name[CCHMAXPATH];
    } findfirst;
    struct {
      struct vpfsd vpfsd;
      struct fsfsi fsfsi;
      struct fsfsd fsfsd;
      unsigned short cbData;
      unsigned short cMatch;
      unsigned short level;
      unsigned short position;
      unsigned short flags;
      unsigned long oError;
      char Name[CCHMAXPATH];
    } findfromname;
    struct {
      struct vpfsd vpfsd;
      struct fsfsi fsfsi;
      struct fsfsd fsfsd;
      unsigned short cbData;
      unsigned short cMatch;
      unsigned short level;
      unsigned short flags;
      unsigned long oError;
    } findnext;
    struct {
      struct vpfsd vpfsd;
      unsigned short flag;
    } flushbuf;
    struct {
      unsigned short flag;
      unsigned short hVPB;
      struct vpfsi vpfsi;
      struct vpfsd vpfsd;
      unsigned short cbData;
      unsigned short level;
    } fsinfo;
    struct {
      struct vpfsd vpfsd;
      struct cdfsi cdfsi;
      struct cdfsd cdfsd;
      unsigned short iCurDirEnd;
      unsigned short flags;
      unsigned long oError;
      char Name[CCHMAXPATH];
    } mkdir;
    struct {
      unsigned short flag;
      struct vpfsi vpfsi;
      struct vpfsd vpfsd;
      unsigned short hVPB;
      unsigned short vol_descr;
    } mount;
    struct {
      struct vpfsd vpfsd;
      struct cdfsi cdfsi;
      struct cdfsd cdfsd;
      unsigned short iSrcCurDirEnd;
      unsigned short iDstCurDirEnd;
      unsigned short flags;
      char Src[CCHMAXPATH];
      char Dst[CCHMAXPATH];
    } move;
    struct {
      struct vpfsd vpfsd;
      struct sffsi sffsi;
      struct sffsd sffsd;
      unsigned long len;
      unsigned short IOflag;
    } newsize;
    struct {
      struct vpfsd vpfsd;
      struct cdfsi cdfsi;
      struct cdfsd cdfsd;
      unsigned short iCurDirEnd;
      struct sffsi sffsi;
      struct sffsd sffsd;
      unsigned long ulOpenMode;
      unsigned short usOpenFlag;
      unsigned short usAction;
      unsigned short usAttr;
      unsigned short fgenflag;
      unsigned long oError;
      char Name[CCHMAXPATH];
    } opencreate;
    struct {
      struct vpfsd vpfsd;
      unsigned short flag;
      struct cdfsi cdfsi;
      struct cdfsd cdfsd;
      unsigned short iCurDirEnd;
      unsigned short level;
      unsigned short cbData;
      unsigned long oError;
      char Name[CCHMAXPATH];
    } pathinfo;
    struct {
      char NameBuf[CCHMAXPATH];
    } processname;
    struct {
      struct vpfsd vpfsd;
      struct sffsi sffsi;
      struct sffsd sffsd;
      unsigned short Len;
      unsigned short IOflag;
    } read;
    struct {
      struct vpfsd vpfsd;
      struct cdfsi cdfsi;
      struct cdfsd cdfsd;
      unsigned short iCurDirEnd;
      char Name[CCHMAXPATH];
    } rmdir;
    struct {
      unsigned short flag;
      char Name[CCHMAXPATH];
    } verifyuncname;
    struct {
      struct vpfsd vpfsd;
      struct sffsi sffsi;
      struct sffsd sffsd;
      unsigned short Len;
      unsigned short IOflag;
    } write;
    
    /* User functions */
    struct {
      struct vpfsd vpfsd;
      struct sffsi sffsi;
      struct sffsd sffsd;
      unsigned long size;
    } get_resfork_size;
    struct {
      struct vpfsd vpfsd;
      struct sffsi sffsi;
      struct sffsd sffsd;
      unsigned long start;
      unsigned long bytes;
    } read_resfork;
    struct {
      struct vpfsi vpfsi;
      struct vpfsd vpfsd;
    } sync;
  } p;
} OPDATA;

#endif /* _R0R3SHAR_H */
