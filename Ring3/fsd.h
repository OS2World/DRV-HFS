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

/* fsd.h
   Ring 3 file system driver entry points and data structures.
*/

#ifndef _FSD_H
#define _FSD_H

#pragma pack(1)

/*  FS_ATTRIBUTE bit field values */

#define FSA_REMOTE  0x00000001  /* Set if REMOTE FSD                  */
#define FSA_UNC     0x00000002  /* Set if FSD implements UNC support  */
#define FSA_LOCK    0x00000004  /* Set if FSD needs lock notification */
#define FSA_LVL7    0x00000008  /* Set if FSD will accept Level 7     */
     /* DosQueryPathInfo requests - These are requests for the case-  */
     /* perserved "path" of the input file/path name.                 */
#define FSA_PSVR    0x00000010  /* Set if FSD manages Remote NmPipes  */

/* Volume Parameter structures */

#define VPBTEXTLEN 12

struct  vpfsi {
    unsigned long   vpi_vid;                /* 32 bit volume ID */
    unsigned long   vpi_hDEV;               /* handle to device driver */
    unsigned short  vpi_bsize;              /* sector size in bytes */
    unsigned long   vpi_totsec;             /* total number of sectors */
    unsigned short  vpi_trksec;             /* sectors / track */
    unsigned short  vpi_nhead;              /* number of heads */
    char            vpi_text[VPBTEXTLEN];   /* volume name */
    void *          vpi_pDCS;               /* device capability struc */
    void *          vpi_pVCS;               /* volume characteristics struc */
    unsigned char   vpi_drive;              /* drive (0=A) */
    unsigned char   vpi_unit;               /* unit code */
};  /* vpfsi */

/*
 * Predefined volume IDs: [note - keep this list in sync with list in
 * dos/dosinc/vpb.inc!]
 */
/* Unique ID for vpb_ID field for unreadable media. */
#define UNREAD_ID  0x534E4A52L          /* Stored as (bytes) 0x52,4A,4E,53. */

/*
 * Unique ID for vpb_ID field for damaged volume (recognized by IFS but cannot
 * be normally mounted).
 */
#define DAMAGED_ID 0x0L                 /* Stored as (bytes) 0,0,0,0. */

/* file system dependent - volume params */
struct vpfsd;

/* Current Directory structures */

struct cdfsi {
    unsigned short  cdi_hVPB;           /* VPB handle for associated device */
    unsigned short  cdi_end;            /* end of root portion of curdir */
    char            cdi_flags;          /* flags indicating state of cdfsd */
    char            cdi_curdir[CCHMAXPATH]; /* text of current directory */
};

/* bit values for cdi_flags (state of cdfsd structure */

#define CDI_ISVALID 0x80                /* format is known */
#define CDI_ISROOT  0x40                /* cur dir == root */
#define CDI_ISCURRENT   0x20

struct cdfsd;

/* Per open-instance (System File) structures */

struct sffsi {
    unsigned long   sfi_mode;       /* access/sharing mode */
    unsigned short  sfi_hVPB;       /* volume info. */
    unsigned short  sfi_ctime;      /* file creation time */
    unsigned short  sfi_cdate;      /* file creation date */
    unsigned short  sfi_atime;      /* file access time */
    unsigned short  sfi_adate;      /* file access date */
    unsigned short  sfi_mtime;      /* file modification time */
    unsigned short  sfi_mdate;      /* file modification date */
    unsigned long   sfi_size;       /* size of file */
    unsigned long   sfi_position;   /* read/write pointer */
/* the following may be of use in sharing checks */
    unsigned short  sfi_UID;        /* user ID of initial opener */
    unsigned short  sfi_PID;        /* process ID of initial opener */
    unsigned short  sfi_PDB;        /* PDB (in 3.x box) of initial opener */
    unsigned short  sfi_selfsfn;    /* system file number of file instance */
    unsigned char   sfi_tstamp;     /* update/propagate time stamp flags */
                                    /* use with ST_Sxxx and ST_Pxxx */
    unsigned short  sfi_type;       /* use with STYPE_ */
    unsigned long   sfi_pPVDBFil;   /* performance counter data block pointer */
    unsigned char   sfi_DOSattr;    /* DOS file attributes  D/S/A/H/R  */
};  /* sffsi */

/* sfi_tstamps flags */
#define ST_SCREAT  1    /* stamp creation time */
#define ST_PCREAT  2    /* propagate creation time */
#define ST_SWRITE  4    /* stamp last write time */
#define ST_PWRITE  8    /* propagate last write time */
#define ST_SREAD  16    /* stamp last read time */
#define ST_PREAD  32    /* propagate last read time */

/* sfi_type flags */
#define STYPE_FILE      0       /* file */
#define STYPE_DEVICE    1       /* device */
#define STYPE_NMPIPE    2       /* named pipe */
#define STYPE_FCB       4       /* fcb sft */

/* file system dependent - file instance */
struct sffsd;

/* file system independent - file search parameters */
struct fsfsi {
  unsigned short  fsi_hVPB;
};

/* file system dependent - file search parameters */
struct fsfsd;

/*****
 *
 * Declarations for the FSD entry points.
 *
 */

/* bit values for the IOflag parameter in various FS_ entry points */
#define IOFL_WRITETHRU      0x10        /* Write through bit    */
#define IOFL_NOCACHE        0x20        /* No Cache bit         */

short int FSD_ATTACH(
    unsigned short,             /* flag         */
    char *,                 /* pDev         */
    struct vpfsd *,                 /* struct vpfsd * */
    struct cdfsd *,                 /* struct cdfsd * */
    void *,                 /* pParm        */
    unsigned short *        /* pLen         */
);

/* values for flag in FS_ATTACH */
#define FSA_ATTACH              0x00
#define FSA_DETACH              0x01
#define FSA_ATTACH_INFO         0x02


/* This was not defined */
struct filelock {
   unsigned long FileOffset;
   unsigned long RangeLength;
};


short int FS_CANCELLOCKREQUEST(
    struct sffsi *,         /* psffsi       */
    struct sffsd *,         /* psffsd       */
    struct filelock *       /* pLockRang    */
);


short int FS_CHDIR(
    struct vpfsd *,
    unsigned short,             /* flag         */
    struct cdfsi *,         /* pcdfsi       */
    struct cdfsd *,         /* pcdfsd       */
    char *,                 /* pDir         */
    unsigned short              /* iCurDirEnd   */
);

/* values for flag in FS_CHDIR */
#define CD_EXPLICIT             0x00
#define CD_VERIFY               0x01
#define CD_FREE                 0x02


short int FS_CHGFILEPTR(
    struct vpfsd *,
    struct sffsi *,         /* psffsi       */
    struct sffsd *,         /* psffsd       */
    long,                       /* offset       */
    unsigned short,             /* type         */
    unsigned short              /* IOflag       */
);

/* values for type in FS_CHGFILEPTR */
#define CFP_RELBEGIN            0x00
#define CFP_RELCUR              0x01
#define CFP_RELEND              0x02


short int FS_CLOSE(
    struct vpfsd *,
    unsigned short,             /* close type   */
    unsigned short,             /* IOflag       */
    struct sffsi *,         /* psffsi       */
    struct sffsd *          /* psffsd       */
);

#define FS_CL_ORDINARY  0
                /* ordinary close of file */
#define FS_CL_FORPROC   1
                /* final close of file for the process */
#define FS_CL_FORSYS    2
                /* final close of file for the system (for all processes) */


short int FS_COMMIT(
    struct vpfsd *,
    unsigned short,             /* commit type  */
    unsigned short,             /* IOflag       */
    struct sffsi *,         /* psffsi       */
    struct sffsd *          /* psffsd       */
);

/* values for commit type */
#define FS_COMMIT_ONE   1
        /* commit for a single file */
#define FS_COMMIT_ALL   2
        /* commit due to buf reset - for all files */


short int FS_COPY(
    struct vpfsd *,
    unsigned short,             /* copy mode    */
    struct cdfsi *,         /* pcdfsi       */
    struct cdfsd *,         /* pcdfsd       */
    char *,                 /* source name  */
    unsigned short,             /* iSrcCurrDirEnd       */
    char *,                 /* pDst         */
    unsigned short,             /* iDstCurrDirEnd       */
    unsigned short              /* nameType (flags)     */
);


short int FS_DELETE(
    struct vpfsd *,
    struct cdfsi *,         /* pcdfsi       */
    struct cdfsd *,         /* pcdfsd       */
    char *,                 /* pFile        */
    unsigned short              /* iCurDirEnd   */
);


short int FS_FILEATTRIBUTE(
    struct vpfsd *,
    unsigned short,             /* flag         */
    struct cdfsi *,         /* pcdfsi       */
    struct cdfsd *,         /* pcdfsd       */
    char *,                 /* pName        */
    unsigned short,             /* iCurDirEnd   */
    unsigned short *        /* pAttr        */
);

/* values for flag in FS_FILEATTRIBUTE */
#define FA_RETRIEVE             0x00
#define FA_SET                  0x01


short int FS_FILEINFO(
    struct vpfsd *,
    unsigned short,             /* flag         */
    struct sffsi *,         /* psffsi       */
    struct sffsd *,         /* psffsd       */
    unsigned short,             /* level        */
    char *,                 /* pData        */
    unsigned short,             /* cbData       */
    unsigned short,              /* IOflag       */
    unsigned long *oError
);

/* values for flag in FS_FILEINFO */
#define FI_RETRIEVE             0x00
#define FI_SET                  0x01


short int FS_FILELOCKS(
    struct sffsi *,         /* psffsi       */
    struct sffsd *,         /* psffsd       */
    struct filelock *,                 /* pUnLockRange */
    struct filelock *,                 /* pLockRange   */
    unsigned long,              /* timeout      */
    unsigned long               /* flags        */
);


short int FS_FINDCLOSE(
    struct vpfsd *,
    struct fsfsi *,         /* pfsfsi       */
    struct fsfsd *          /* pfsfsd       */
);

/* values for flag in FS_FindFirst, FS_FindFromName, FS_FindNext */
#define     FF_NOPOS            0x00
#define     FF_GETPOS           0X01


short int FS_FINDFIRST(
    struct vpfsd *,
    struct cdfsi *,         /* pcdfsi       */
    struct cdfsd *,         /* pcdfsd       */
    char *,                 /* pName        */
    unsigned short,             /* iCurDirEnd   */
    unsigned short,             /* attr         */
    struct fsfsi *,         /* pfsfsi       */
    struct fsfsd *,         /* pfsfsd       */
    char *,                 /* pData        */
    unsigned short,             /* cbData       */
    unsigned short *,           /* pcMatch      */
    unsigned short,             /* level        */
    unsigned short,             /* flags        */
    unsigned long *oError
);


short int FS_FINDFROMNAME(
    struct vpfsd *,
    struct fsfsi *,             /* pfsfsi       */
    struct fsfsd *,             /* pfsfsd       */
    char *,                     /* pData        */
    unsigned short,             /* cbData       */
    unsigned short *,           /* pcMatch      */
    unsigned short,             /* level        */
    unsigned long,              /* position     */
    char *,                     /* pName        */
    unsigned short,             /* flags        */
    unsigned long *oError
);


short int FS_FINDNEXT(
    struct vpfsd *,
    struct fsfsi *,             /* pfsfsi       */
    struct fsfsd *,             /* pfsfsd       */
    char *,                     /* pData        */
    unsigned short,             /* cbData       */
    unsigned short *,           /* pcMatch      */
    unsigned short,             /* level        */
    unsigned short,             /* flag         */
    unsigned long *oError
);


short int FS_FLUSHBUF(
    struct vpfsd *,             /* hVPB         */
    unsigned short              /* flag         */
);

/* values for flag in FS_FLUSH */
#define FLUSH_RETAIN            0x00
#define FLUSH_DISCARD           0x01


short int FS_FSINFO(
    unsigned short,             /* flag         */
    unsigned short hVPB,
    struct vpfsi *,
    struct vpfsd *,             /* hVPB         */
    char *,                     /* pData        */
    unsigned short,             /* cbData       */
    unsigned short              /* level        */
);

/* values for flag in FS_FSINFO */
#define INFO_RETRIEVE           0x00
#define INFO_SET                0x01


short int FS_MKDIR(
    struct vpfsd *,
    struct cdfsi *,         /* pcdfsi       */
    struct cdfsd *,         /* pcdfsd       */
    char *,                 /* pName        */
    unsigned short,             /* iCurDirEnd   */
    char *,                 /* pEABuf       */
    unsigned short,             /* flags        */
    unsigned long *oError
);

short int
FS_MOUNT(
    unsigned short,             /* flag         */
    struct vpfsi *,             /* pvpfsi       */
    struct vpfsd *,             /* pvpfsd       */
    unsigned short,             /* hVPB         */
    unsigned short,             /* VolDescriptor*/
    unsigned char *             /* pBoot        */
);
/* XLATON */

/* values for flag in FS_MOUNT */
#define MOUNT_MOUNT             0x00
#define MOUNT_VOL_REMOVED       0x01
#define MOUNT_RELEASE           0x02
#define MOUNT_ACCEPT            0x03

/* IFS specific error code */
#define ERROR_VOLUME_NOT_MOUNTED 0xee00

short int FS_MOVE(
    struct vpfsd *,
    struct cdfsi *,         /* pcdfsi       */
    struct cdfsd *,         /* pcdfsd       */
    char *,                 /* pSrc         */
    unsigned short,             /* iSrcCurDirEnd*/
    char *,                 /* pDst         */
    unsigned short,             /* iDstCurDirEnd*/
    unsigned short              /* flags        */
);


short int FS_NEWSIZE(
    struct vpfsd *,
    struct sffsi *,         /* psffsi       */
    struct sffsd *,         /* psffsd       */
    unsigned long,              /* len          */
    unsigned short              /* IOflag       */
);


short int FS_OPENCREATE(
    struct vpfsd *,
    struct cdfsi *,         /* pcdfsi               */
    struct cdfsd *,                 /* struct cdfsd * */
    char *,                 /* pName                */
    unsigned short,             /* iCurDirEnd           */
    struct sffsi *,         /* psffsi               */
    struct sffsd *,         /* psffsd               */
    unsigned long,              /* fhandflag/openmode   */
    unsigned short,             /* openflag             */
    unsigned short *,       /* pAction              */
    unsigned short,             /* attr                 */
    char *,                 /* pEABuf               */
    unsigned short *,       /* pfgenFlag            */
    unsigned long *oError
);

#define FOC_NEEDEAS     0x1     /*there are need eas for this file */


short int FS_PATHINFO(
    struct vpfsd *,
    unsigned short,             /* flag         */
    struct cdfsi *,         /* pcdfsi       */
    struct cdfsd *,         /* pcdfsd       */
    char *,                 /* pName        */
    unsigned short,             /* iCurDirEnd   */
    unsigned short,             /* level        */
    char *,                 /* pData        */
    unsigned short,             /* cbData       */
    unsigned long *oError
);

/* values for flag in FS_PATHINFO */
#define PI_RETRIEVE             0x00
#define PI_SET                  0x01


short int FS_PROCESSNAME(
    char *                  /* pNameBuf     */
);


short int FS_READ(
    struct vpfsd *,
    struct sffsi *,         /* psffsi       */
    struct sffsd *,         /* psffsd       */
    char *,                 /* pData        */
    unsigned short *,       /* pLen         */
    unsigned short              /* IOflag       */
);


short int FS_RMDIR(
    struct vpfsd *,
    struct cdfsi *,         /* pcdfsi       */
    struct cdfsd *,         /* pcdfsd       */
    char *,                 /* pName        */
    unsigned short              /* iCurDirEnd   */
);


short int FS_VERIFYUNCNAME(
    unsigned short,             /* flag         */
    char *                  /* pName        */
);

/* values for flag in FS_VERIFYUNCNAME */
#define VUN_PASS1                0x0000
#define VUN_PASS2                0x0001
#define ERROR_UNC_PATH_NOT_FOUND 0x0003   /* equals ERROR_PATH_NOT_FOUND */


short int FS_WRITE(
    struct vpfsd *,
    struct sffsi *,         /* psffsi       */
    struct sffsd *,         /* psffsd       */
    char *,                 /* pData        */
    unsigned short *,       /* pLen         */
    unsigned short              /* IOflag       */
);

#endif /* _FSD_H */
