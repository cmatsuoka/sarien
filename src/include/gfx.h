/*
 *  Sarien AGI :: Copyright (C) 1999 Dark Fiber 
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __AGI_GFX
#define __AGI_GFX

#ifdef __cplusplus
extern "C"{
#endif

#define GFX_WIDTH	320
#define GFX_HEIGHT	200

typedef struct __GFX_DRIVER
{
	UINT16	(*init_video_mode)(void);
	UINT16	(*deinit_video_mode)(void);
	void	(*put_block)(UINT16 x1, UINT16 y1, UINT16 x2, UINT16 y2);
	void	(*put_pixel)(UINT16 x, UINT16 y, UINT16 c);
	void	(*poll_timer)(void);
	UINT8	(*keypress)(void);
	UINT16	(*get_key)(void);
} __GFX_DRIVER;

extern	__GFX_DRIVER	*gfx;
extern	UINT8		palette[];
extern	UINT8		screen_mode;
extern	UINT8		txt_fg;
extern	UINT8		txt_bg;
extern	UINT8		txt_char;


/* Transparent layer */
extern UINT8	layer1_data[];
extern UINT8	layer2_data[];


void	message_box	(UINT8 *, ...);
void	textbox		(UINT8 *, SINT16, SINT16, SINT16);
UINT8	*word_wrap_string (UINT8 *, UINT16 *);
void	get_bitmap	(UINT8 *, UINT8 *, UINT16, UINT16, UINT16, UINT16);
void	put_bitmap	(UINT8 *, UINT8 *, UINT16, UINT16, UINT16, UINT16,
			 UINT16, UINT16);
void	draw_box	(UINT16, UINT16, UINT16, UINT16, UINT8, UINT8, UINT8);
void	print_text	(UINT8 *, UINT16, UINT16, UINT16, UINT16, UINT8,
			 UINT8);
void	print_text2	(UINT8, UINT8 *, UINT16, UINT16, UINT16, UINT16, UINT8,
			 UINT8);
void	print_text_layer(UINT8 *, UINT16, UINT16, UINT16, UINT16, UINT8,
			 UINT8);
void 	put_text_character(UINT8, UINT16, UINT16, UINT8, UINT8, UINT8);
void	agi_put_bitmap	(UINT8 *, UINT16, UINT16, UINT16, UINT16, UINT16,
			 UINT16);
void	agi_put_bitmap_save(UINT8 *, UINT16, UINT16, UINT16, UINT16, UINT16,
			 UINT16);
void	agi_put_sprite	(UINT8 *, UINT16, UINT16, UINT16, UINT16, UINT16,
			 UINT16);
void	do_blit		(void);
UINT16	init_video	(void);
UINT16	deinit_video	(void);
void	put_pixel_buffer	(UINT16, UINT16, UINT16);
void	shake_screen	(UINT8);
void	save_screen	(void);
void	restore_screen	(void);
void	restore_screen_area	(void);
void	put_screen	(void);
void	clear_buffer	(void);

void	put_pixel	(UINT16, UINT16, UINT16);	/* driver wrapper */
void	flush_block	(UINT16, UINT16, UINT16, UINT16);
void	build_console_layer (void);

void	erase_sprites	(void);
void	release_sprites	(void);
void	draw_sprites	(void);
void	redraw_sprites	(void);

#ifdef __cplusplus
};
#endif
#endif

