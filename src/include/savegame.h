/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999,2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#ifndef __AGI_SAVEGAME_H
#define __AGI_SAVEGAME_H

#ifdef __cplusplus
extern "C"{
#endif

int savegame_dialog (void);
int loadgame_dialog (void);

int save_game (char *, char *);
int load_game (char *);

#ifdef __cplusplus
};
#endif

#endif
