/* os2func.c
 */

#define INCL_DOSMEMMGR
#define INCL_DOSERRORS
#include <os2.h>

#include <stdlib.h>

void *os2_malloc(size_t size)
{
  APIRET rc;
  SEL sel;
  char *p;

  rc = DosAllocSeg(size, &sel, SEG_NONSHARED);
  if(rc != NO_ERROR)
    return NULL;
  else
    return MAKEP(sel, 0);
}

void os2_free(void *p)
{
  DosFreeSeg(SELECTOROF(p));
}

void *os2_realloc(void *p, size_t size)
{
  DosReallocSeg(size, SELECTOROF(p));
  return p;
}

