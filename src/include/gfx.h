/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999,2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#ifndef __AGI_GFX_H
#define __AGI_GFX_H

#ifdef __cplusplus
extern "C"{
#endif

#define GFX_WIDTH	320
#define GFX_HEIGHT	200

struct gfx_driver {
	int	(*init_video_mode)(void);
	int	(*deinit_video_mode)(void);
	void	(*put_block)(int x1, int y1, int x2, int y2);
	void	(*put_pixel)(int x, int y, int c);
	void	(*poll_timer)(void);
	int	(*keypress)(void);
	int	(*get_key)(void);
};

extern	UINT8		palette[];
extern	UINT8		screen_mode;
extern	UINT8		txt_fg;
extern	UINT8		txt_bg;
extern	UINT8		txt_char;


/* Transparent layer */
extern UINT8	layer1_data[];
extern UINT8	layer2_data[];

void	get_bitmap	(UINT8 *, UINT8 *, int, int, int, int);
void	draw_box	(int, int, int, int, int, int, int);
void 	put_text_character(int, int, int, int, int, int);
void	agi_put_bitmap	(UINT8 *, int, int, int, int, int, int);
void	agi_put_sprite	(UINT8 *, int, int, int, int, int, int);
void	do_blit		(void);
int	init_video	(void);
int	deinit_video	(void);
void	put_pixel_buffer(int, int, int);
void	shake_screen	(int);
void	save_screen	(void);
void	restore_screen	(void);
void	restore_screen_area	(void);
void	put_screen	(void);
void	clear_buffer	(void);

void	put_pixel	(int, int, int);	/* driver wrapper */
void	flush_block	(int, int, int, int);
void	build_console_layer (void);

void	erase_sprites	(void);
void	release_sprites	(void);
void	draw_sprites	(void);
void	redraw_sprites	(void);

void	print_character	(int, int, char, int, int);

#ifdef __cplusplus
};
#endif
#endif

