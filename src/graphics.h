/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999,2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#ifndef __AGI_GRAPHICS_H
#define __AGI_GRAPHICS_H

#ifdef __cplusplus
extern "C"{
#endif

#define GFX_WIDTH	320
#define GFX_HEIGHT	200
#define CHAR_COLS	8
#define CHAR_LINES	8

struct gfx_driver {
	int	(*init_video_mode)(void);
	int	(*deinit_video_mode)(void);
	void	(*put_block)(int x1, int y1, int x2, int y2);
	void	(*put_pixels)(int x, int y, int w, UINT8 *p);
	void	(*poll_timer)(void);
	int	(*keypress)(void);
	int	(*get_key)(void);
};

extern UINT8 palette[];


/* Transparent layer */
extern UINT8	layer1_data[];
extern UINT8	layer2_data[];


void 	put_text_character(int, int, int, unsigned int, int, int);
void	shake_screen	(int);
void	shake_start	(void);
void	shake_end	(void);
void	save_screen	(void);
void	restore_screen	(void);

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
void	clear_console_screen	(int);
void	draw_box	(int, int, int, int, int, int, int);
void	draw_button	(int, int, char *, int, int);
int	test_button	(int, int, char *);
void	draw_rectangle	(int, int, int, int, int);
void	save_block	(int, int, int, int, UINT8 *);
void	restore_block	(int, int, int, int, UINT8 *);
void	init_palette	(UINT8 *);

void	put_pixel	(int, int, int);	


#ifdef USE_HIRES
void put_pixels_hires (int x, int y, int n, UINT8 *p);
#endif
int	keypress	(void);
int	get_key		(void);
void	print_character	(int, int, char, int, int);
void	poll_timer	(void);

#ifdef __cplusplus
};
#endif

#endif /* __AGI_GRAPHICS_H */

