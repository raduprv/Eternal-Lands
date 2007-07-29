/* Console input/output functions
 *
 * (c) Copyright 1998-2005, ITB CompuPhase
 * This file is provided as is (no warranties).
 */
/* Grum: Edited to only include the functions used in EL
 */
#if defined _console_included
  #endinput
#endif
#define _console_included

native printf (const format[], {Float,Fixed,_}:...);
