/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999,2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#ifndef _WIN_32_H
#define _WIN_32_H 1

#ifdef	__cplusplus
extern "C" {
#endif

HWND     hwndMain;
void     (*felipes_kludge) (PWAVEHDR pWave);

#ifdef __cplusplus
} 
#endif

#endif  /* WIN_32_H */

