/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999,2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#ifndef __GRAPHICS_H
#define __GRAPHICS_H

#ifdef __cplusplus
extern "C"{
#endif

#if defined PALMOS || defined FAKE_PALMOS
#define GFX_WIDTH	160
#define GFX_HEIGHT	160
#define CHAR_COLS	4
#define CHAR_LINES	6
#define PIC_HEIGHT	(22 * CHAR_LINES)
#else
#define GFX_WIDTH	320
#define GFX_HEIGHT	200
#define CHAR_COLS	8
#define CHAR_LINES	8
#endif

struct gfx_driver {
	int	(*init_video_mode)(void);
	int	(*deinit_video_mode)(void);
	void	(*put_block)(int x1, int y1, int x2, int y2);
	void	(*put_pixels)(int x, int y, int w, UINT8 *p);
	void	(*poll_timer)(void);
	int	(*keypress)(void);
	int	(*get_key)(void);
};

extern	UINT8		palette[];
extern	UINT8		txt_char;


/* Transparent layer */
extern UINT8	layer1_data[];
extern UINT8	layer2_data[];


void 	put_text_character(int, int, int, int, int, int);
void	shake_screen	(int);
void	save_screen	(void);
void	restore_screen	(void);

/* documented */
int	init_video	(void);
int	deinit_video	(void);
void	schedule_update	(int, int, int, int);
void	do_update	(void);
void	put_screen	(void);
void	flush_block	(int, int, int, int);
void	flush_block_a	(int, int, int, int);
void	put_pixels_a	(int, int, int, UINT8 *);
void	flush_screen	(void);
void	clear_screen	(int);
void	draw_box	(int, int, int, int, int, int, int);
void	draw_rectangle	(int, int, int, int, int);

void	put_pixel	(int, int, int);	

int	keypress	(void);
int	get_key		(void);

void	print_character	(int, int, char, int, int);

void	poll_timer	(void);

#ifdef __cplusplus
};
#endif

#endif /* __GRAPHICS_H */
