/* hfsflt.h
   Miscellaneous declarations.
   */

#ifndef _HFSFLT_H
#define _HFSFLT_H

#define ADDNAME                "HFS_FILTER"
#define ADDFLAGS               0

#define MAX_UNITS               28
#define MAX_DEVICETABLE_SIZE    (1*1024)
#define SECTOR_SIZE             512

typedef VOID (FAR* ADDEntryPoint)(PIORB);
typedef PIORB (FAR* NotifyAddr)(PIORB);

typedef struct _UNITENTRY {
  ADDEntryPoint pADDEntry;        /* Original ADD entry point */
  USHORT UnitHandle;              /* Original unit handle */
  USHORT Allocated;               /* Set if unit is allocated */
  UNITINFO newUnitInfo;           /* New unit info */
} UNITENTRY, *PUNITENTRY;

/* From filter.c */
VOID FAR FilterIORBEntry(PIORB);
/*void FilterIORB(PIORB);*/
void CommandDone(PIORB pIORB);
void CallOriginalADD(PIORB pIORB);

/* From init.c */
VOID Init(PRPINITIN pRPI);

/* From io.c */
VOID FilterRead(PIORB pIORB);
USHORT read_sector(ULONG RBA, USHORT count, ULONG ppBuf,
		   ADDEntryPoint pADDEntry, USHORT UnitHandle);
USHORT SendIORB(PIORB pIORB, ADDEntryPoint ADDEntry);

/* Compiler-specific stuff */

#if defined(__MSC__)
#define ENABLE  _enable()
#define DISABLE _disable()
#else if defined(__BORLANDC__)
#define ENABLE	asm { sti }
#define DISABLE	asm { cli }
#endif


#endif /* _HFSFLT_H */
