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
int savegame_simple (void);
int loadgame_simple (void);


/* Image stack support */
#define ADD_PIC 1
#define ADD_VIEW 2

void clear_image_stack(void);
void record_image_stack_call(UINT8 type, SINT16 p1, SINT16 p2, SINT16 p3, SINT16 p4, SINT16 p5, SINT16 p6, SINT16 p7);
void replay_image_stack_call(UINT8 type, SINT16 p1, SINT16 p2, SINT16 p3, SINT16 p4, SINT16 p5, SINT16 p6, SINT16 p7);
void release_image_stack(void);

#ifdef __cplusplus
};
#endif

#endif
