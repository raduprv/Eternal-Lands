#ifdef PAWN

#ifndef AMXCONS_H_INCLUDED
#define AMXCONS_H_INCLUDED

#include "amx.h"

#if defined _UNICODE
# include <tchar.h>
#else
  typedef char          TCHAR;
#endif

typedef struct tagFMTINFO {
  const cell *params;
  int numparams;
  int skip;     /* number of characters to skip from the beginning */
  int length;   /* number of characters to print */
  /* helper functions */
  int (*f_putstr)(void *dest,const TCHAR *);
  int (*f_putchar)(void *dest,TCHAR);
  void *user;   /* user data */
} AMX_FMTINFO;

int amx_printstring(AMX *amx,cell *cstr,AMX_FMTINFO *info);

// Grum: added for initializing AMX machine
int AMXEXPORT amx_ConsoleInit(AMX *amx);
int AMXEXPORT amx_ConsoleCleanup(AMX *amx);

#endif /* AMXCONS_H_INCLUDED */

#endif // PAWN
