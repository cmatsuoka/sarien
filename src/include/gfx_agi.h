/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999,2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#ifndef __AGI_GFX_AGI_H
#define __AGI_GFX_AGI_H

#ifdef __cplusplus
extern "C"{
#endif

#if 0
extern	UINT8		screen_mode;
extern	UINT8		txt_fg;
extern	UINT8		txt_bg;
extern	UINT8		txt_char;


/* Transparent layer */
extern UINT8	layer1_data[];
extern UINT8	layer2_data[];
#endif

void	get_bitmap	(UINT8 *, UINT8 *, int, int, int, int);
void	agi_put_bitmap	(UINT8 *, int, int, int, int, int, int);
void	agi_put_sprite	(UINT8 *, int, int, int, int, int, int);

void	erase_sprites	(void);
void	release_sprites	(void);
void	draw_sprites	(void);
void	redraw_sprites	(void);

void	print_character	(int, int, char, int, int);

int	init_video_mode	(void);
int	deinit_video_mode	(void);

#ifdef __cplusplus
};
#endif
#endif

