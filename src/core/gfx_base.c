/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#include "sarien.h"
#include "gfx_base.h"
#include "console.h"

#ifdef USE_CONSOLE
extern struct sarien_console console;
#endif

extern UINT8 *font;

/* exported to the console drivers */
UINT8 palette[32 * 3]= {
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x2A,
	0x00, 0x2A, 0x00,
	0x00, 0x2A, 0x2A,
	0x2A, 0x00, 0x00,
	0x2A, 0x00, 0x2A,
	0x2A, 0x15, 0x00,
	0x2A, 0x2A, 0x2A,
	0x15, 0x15, 0x15,
	0x15, 0x15, 0x3F,
	0x15, 0x3F, 0x15,
	0x15, 0x3F, 0x3F,
	0x3F, 0x15, 0x15,
	0x3F, 0x15, 0x3F,
	0x3F, 0x3F, 0x15,
	0x3F, 0x3F, 0x3F
};

struct gfx_driver *gfx;			/* graphics driver */
UINT8		screen_mode;		/* 0=gfx mode, 1=text mode */
UINT8		txt_fg;			/* fg colour */
UINT8		txt_bg;			/* bg colour */
UINT8		txt_char;		/* input character */


/* for the blitter routine */
static int x_min = GFX_WIDTH, x_max = 0, y_min = GFX_HEIGHT, y_max = 0;
 

/* This works in the 320x200 visible screen, but receiving AGI 160x168
 * coordinates
 */
void put_pixel_buffer (int x, int y, int c)
{
#ifdef PALMOS
	y = y * PIC_HEIGHT / 168;	/* ick! */
#else
	x <<= 1;
	put_pixel (x + 1, y, c);
#endif
	put_pixel (x, y, c);
}

#ifdef USE_CONSOLE

/* driver wrapper */
/* put_pixel2 is console-aware and handles transparency
 * works on the 320x200 screen
 */
static void INLINE put_pixel2 (int x, int y, int c)
{
	int k;

	if (console.y <= y) {
		gfx->put_pixel (x, y, c);
		return;
	}

	if ((k = layer2_data[(y + 199 - console.y) * GFX_WIDTH + x]))
		c = k;
	else
		c += 16;	/* transparency */

	gfx->put_pixel (x, y, c);
}


/* put_pixel stores information in the screen buffer and calls put_pixel2
 * for actual screen drawing (put_pixel2 handles transparency)
 * works on the 320x200 screen
 */
void put_pixel (int x, int y, int c)
{
	layer1_data[y * GFX_WIDTH + x] = c;
	put_pixel2 (x, y, c);
}

#else

void put_pixel (int x, int y, int c)
{
	layer1_data[y * GFX_WIDTH + x] = c;
	gfx->put_pixel (x, y, c);
}

#define put_pixel2 put_pixel

#endif /* USE_CONSOLE */

/* flush block is used to put a block "behind" the console
 * works on the 320x200 screen
 */
void flush_block (int x1, int y1, int x2, int y2)
{
	int x, y;

	for (y = y1; y <= y2; y++)
		for (x = x1; x <= x2; x++)
			put_pixel2 (x, y, layer1_data[y * GFX_WIDTH + x]);

	gfx->put_block (x1, y1, x2, y2);
}


void clear_buffer ()
{
	memset (layer1_data, 0, GFX_WIDTH * GFX_HEIGHT);
	flush_block (0, 0, GFX_WIDTH - 1, GFX_HEIGHT - 1);
}


/*
 * Save/restore visible screen framebuffer
 * Never call these functions directly, use the gfx_agi.h wrappers
 * instead
 */

static UINT8 back_buffer[GFX_WIDTH * GFX_HEIGHT];

/* Kludge for nonblocking windows */
static int k_x1, k_y1, k_x2, k_y2;

void _save_screen ()
{
	memcpy (back_buffer, layer1_data, GFX_WIDTH * GFX_HEIGHT);
}

void _restore_screen ()
{
	memcpy (layer1_data, back_buffer, GFX_WIDTH * GFX_HEIGHT);
}

void _flush_screen ()
{
	flush_block (0, 0, GFX_WIDTH - 1, GFX_HEIGHT - 1);
}

void _restore_screen_area ()	/* Yuck! */
{
	int i;

	report ("Debug: restore_screen_area: %d %d %d %d\n",
		k_x1, k_y1, k_x2, k_y2);

	for (i = k_y1; i <= k_y2; i++)
		memcpy (&layer1_data[GFX_WIDTH * i + k_x1],
			&back_buffer[GFX_WIDTH * i + k_x1],
			k_x2 - k_x1 + 1);
}

void _flush_screen_area ()
{
	flush_block (k_x1, k_y1, k_x2, k_y2);
}



/* Based on LAGII 0.1.5 by XoXus */
/* Works on the 320x200 visible screen */
void shake_screen (int n)
{
#define MAG 3
	int i;
	UINT8 b[GFX_WIDTH * GFX_HEIGHT], c[GFX_WIDTH * GFX_HEIGHT];
	
	memset (c, 0, GFX_WIDTH * GFX_HEIGHT);
	memcpy (b, layer1_data, GFX_WIDTH * GFX_HEIGHT);
	for (i = 0; i < (200 - MAG); i++)
		memcpy (&c[GFX_WIDTH * (i + MAG) + MAG],
			&b[GFX_WIDTH * i], GFX_WIDTH - MAG);

	for (i = 0; i < (2 * n); i++) {
		main_cycle (TRUE);
		memcpy (layer1_data, c, GFX_WIDTH * GFX_HEIGHT);
		flush_block (0, 0, GFX_WIDTH - 1, GFX_HEIGHT - 1);
		main_cycle (TRUE);
		memcpy (layer1_data, b, GFX_WIDTH * GFX_HEIGHT);
		flush_block (0, 0, GFX_WIDTH - 1, GFX_HEIGHT - 1);
	}
#undef MAG
}


int init_video ()
{
	int i;

	fprintf (stderr, "Initializing graphics: resolution %dx%d (scale=%d)\n",
		GFX_WIDTH, GFX_HEIGHT, opt.scale);

	/* "Transparent" colors */
	for (i = 0; i < 48; i++)
		palette[i + 48] = (palette[i] + 0x30) >> 2;

#ifdef USE_CONSOLE
	/* Console */
	for (i = 0; i < CONSOLE_LINES_BUFFER; i++)
		console.line[i] = strdup ("\n");
#endif

	screen_mode = GFX_MODE;
	txt_fg = 0xF;
        txt_bg = 0x0;

	return gfx->init_video_mode ();
}


int deinit_video ()
{
	return gfx->deinit_video_mode ();
}


void set_block (int x1, int y1, int x2, int y2)
{
	if (x1 < x_min)
		x_min = x1;
	if (y1 < y_min)
		y_min = y1;
	if (x2 > x_max)
		x_max = x2;
	if (y2 > y_max)
		y_max = y2;
}


void put_text_character (int l, int x, int y, int c, int fg, int bg)
{
	int x1, y1, xx, yy, cc;
	UINT8 *p;

	p = font + (c * CHAR_LINES);
	for (y1 = 0; y1 < CHAR_LINES; y1++) {
		for (x1 = 0; x1 < CHAR_COLS; x1 ++) {
			xx = x + x1;
			yy = y + y1;
			cc = (*p & (1 << (7 - x1))) ? fg : bg;
#ifdef USE_CONSOLE
			if (l) {
				layer2_data[yy * GFX_WIDTH + xx] = cc;
			} else
#endif
			{
				put_pixel (xx, yy, cc);
			}
		}

		p++;
	}
}


void draw_box (int x1, int y1, int x2, int y2, int colour1, int colour2, int f, int offset)
{
	int x, y;

	if (x1 >= GFX_WIDTH)
		x1 = GFX_WIDTH - 1;
	if (y1 >= GFX_HEIGHT)
		y1 = GFX_HEIGHT - 1;
	if (x2 >= GFX_WIDTH)
		x2 = GFX_WIDTH - 1;
	if (y2 >= GFX_HEIGHT)
		y2 = GFX_HEIGHT - 1;

	k_x1 = x1; k_y1 = y1;	/* Yuck! Someone fix this */
	k_x2 = x2; k_y2 = y2;

	for (y = y1; y < y2; y++)
		for (x = x1; x < x2; x++)
			put_pixel (x, y, colour1);

	if (f & LINES) {
		/* draw lines */
		for (x = x1; x < x2 - 4; x++) {
			put_pixel (x+2, y1+2, colour2);
			put_pixel (x+2, y2-3, colour2);
		}

		for (y = y1; y <= y2 - 5; y++) {
			put_pixel (x1+2, y+2, colour2);
			put_pixel (x1+3, y+2, colour2);
			put_pixel (x2-3, y+2, colour2);
			put_pixel (x2-4, y+2, colour2);
		}
	}

	set_block (x1, y1 + offset, x2, y2 + offset);
}


void do_blit ()
{
	if (x_min < x_max && y_min < y_max) {
#ifdef PALMOS
		gfx->put_block (x_min, y_min * PIC_HEIGHT / 168, x_max + 1, y_max * PIC_HEIGHT / 168);
#else
		gfx->put_block (x_min << 1, y_min, (x_max << 1) + 1, y_max);
#endif
	}
	
	/* reset block variables */
	x_min = GFX_WIDTH;
	x_max = 0;
	y_min = GFX_HEIGHT;
	y_max = 0;
}


void print_character (int x, int y, char c, int fg, int bg)
{
	put_text_character (0, x, y, c, fg, bg);
	/* CM: the extra pixel in y is for the underline cursor */
	gfx->put_block (x, y, x + 7, y + 8); 
}


/* put_screen is a lower level function, not console-aware */
void put_screen ()
{
	gfx->put_block (0, 0, GFX_WIDTH - 1, GFX_HEIGHT - 1);
}


void put_block (int x1, int y1, int x2, int y2)
{
	gfx->put_block (x1, y1, x2, y2);
}


void poll_timer ()
{
	gfx->poll_timer ();
}


int get_key ()
{
	return gfx->get_key ();
}


int keypress ()
{
	return gfx->keypress ();
}

