/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999,2001 Stuart George and Claudio Matsuoka
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
#include "agi.h"
#include "gfx.h"
#include "keyboard.h"
#include "view.h"
#include "picture.h"
#include "console.h"

extern struct sarien_options opt;
extern UINT8 *font;

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
static int x_min = 320, x_max = 0, y_min = 200, y_max = 0;

/* Ugly kludge for nonblocking windows */
static int k_x1, k_y1, k_x2, k_y2;

extern struct sarien_console console;
extern struct agi_view_table view_table[];

int greatest_kludge_of_all_time = 0;


/* driver wrapper */
static void INLINE put_pixel2 (int x, int y, int c)
{
	int k;

	if (console.y <= y) {
		gfx->put_pixel (x, y, c);
		return;
	}

	if ((k = layer2_data[(y + 199 - console.y) * 320 + x]))
		c = k;
	else
		c += 16;	/* transparency */

	gfx->put_pixel (x, y, c);
}


void put_pixel (int x, int y, int c)
{
	layer1_data[y * 320 + x] = c;
	put_pixel2 (x, y, c);
}


void flush_block (int x1, int y1, int x2, int y2)
{
	int x, y;

	for (y = y1; y <= y2; y++)
		for (x = x1; x <= x2; x++)
			put_pixel2 (x, y, layer1_data[y * 320 + x]);

	gfx->put_block (x1, y1, x2, y2);
}


void clear_buffer ()
{
	memset (layer1_data, 0, 320 * 200);
	flush_block (0, 0, 319, 199);
}


void put_screen ()
{
	gfx->put_block (0, 0, 319, 199);
}


static UINT8 back_buffer[320 * 200];

void save_screen ()
{
	memcpy (back_buffer, layer1_data, 320 * 200);
}


void restore_screen ()
{
	memcpy (layer1_data, back_buffer, 320 * 200);
	redraw_sprites ();
	flush_block (0, 0, 319, 199);
	release_sprites ();
}


void restore_screen_area ()	/* Yuck! */
{
	int i;

	report ("Debug: restore_screen_area: %d %d %d %d\n",
		k_x1, k_y1, k_x2, k_y2);

	for (i = k_y1; i <= k_y2; i++)
		memcpy (&layer1_data[320 * i + k_x1],
			&back_buffer[320 * i + k_x1],
			k_x2 - k_x1 + 1);

	redraw_sprites ();
	flush_block (k_x1, k_y1, k_x2, k_y2);
	release_sprites ();
}


/* Based on LAGII 0.1.5 by XoXus */
void shake_screen (int n)
{
#define MAG 3
	int i;
	UINT8 b[320 * 200], c[320 * 200];
	
	memset (c, 0, 320 * 200);
	memcpy (b, layer1_data, 320 * 200);
	for (i = 0; i < (200 - MAG); i++)
		memcpy (&c[320 * (i + MAG) + MAG], &b[320 * i], 320 - MAG);

	for (i = 0; i < (2 * n); i++) {
		main_cycle (TRUE);
		memcpy (layer1_data, c, 320 * 200);
		flush_block (0, 0, 319, 199);
		main_cycle (TRUE);
		memcpy (layer1_data, b, 320 * 200);
		flush_block (0, 0, 319, 199);
	}
#undef MAG
}


void put_pixel_buffer (int x, int y, int c)
{
	if (line_min_print > 0)
		y += 8;
	x <<= 1;
	put_pixel (x, y, c);
	put_pixel (x + 1, y, c);
}


int init_video ()
{
	int i;

	fprintf (stderr, "Initializing graphics: resolution %dx%d (scale=%d)\n",
		GFX_WIDTH, GFX_HEIGHT, opt.scale);

	/* "Transparent" colors */
	for (i = 0; i < 48; i++)
		palette[i + 48] = (palette[i] + 0x30) >> 2;

	/* Console */
	for (i = 0; i < CONSOLE_LINES_BUFFER; i++)
		console.line[i] = strdup ("\n");

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

	p = font + (c << 3);
	for (y1 = 0; y1 < 8; y1++) {
		for(x1 = 0; x1 < 8; x1 ++) {
			xx = x + x1;
			yy = y + y1;
			cc = (*p & (1 << (7 - x1))) ? fg : bg;
			if (l) {
				layer2_data[yy * 320 + xx] = cc;
			} else {
				put_pixel (xx, yy, cc);
			}
		}

		p++;
	}
}


void draw_box (int x1, int y1, int x2, int y2, int colour1, int colour2, int f)
{
	int x, y;

	if (x1 > 319)
		x1 = 319;
	if (y1 > 199)
		y1 = 199;
	if (x2 > 319)
		x2 = 319;
	if (y2 > 199)
		y2 = 199;

	k_x1 = x1; k_y1 = y1;	/* Yuck! Someone fix this */
	k_x2 = x2; k_y2 = y2;

	for (y=y1; y<y2; y++)
		for(x=x1; x<x2; x++)
			put_pixel(x, y, colour1);

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

	y = line_min_print ? 8 : 0;
	set_block (x1, y1 + y, x2, y2 + y);
}


void get_bitmap (UINT8 *dst, UINT8 *src, int x1, int y1, int w, int h)
{
	int y, x;

	for (y1++, y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			if (y1 + y < _HEIGHT && x1 + x < _WIDTH)
				dst[(y*w) + x] = src[((y1+y)*_WIDTH) + (x1+x)];
		}
	}
}


void put_bitmap (UINT8 *dst, UINT8 *src, int x1, int y1, int w, int h, int trans, int prio)
{
	int x, y, xx, yy;
	int c;

	if (prio < 4)
		prio = 4;

	for (y1++, y = 0; y < h; y++) {
		for (yy = (y1 + y) * _WIDTH, x=0; x<w; x++) {
			if ((c=src[x + y*w]) == trans)
				continue;

			xx = x1 + x;
			if (y + y1 >= _HEIGHT || xx >= _WIDTH)
				continue;

			if(prio < dst[yy + xx])
				continue;

			dst[yy + xx]=c;
		}
	}

	y = line_min_print ? 8 : 0;
	set_block (x1, y1 + y, x1 + w, y1 + y + h);
}


void agi_put_bitmap (UINT8 *src, int x1, int y1, int w, int h, int trans, int prio)
{
	int x, y, xx, yy;
	int c;

	/* _D (("(%p, %d, %d, %d, %d, %d, %d)",
		src, x1, y1, w, h, trans, prio)); */

	if (prio < 4)
		prio = 4;

	/* FIXME: claudio's anti-crash test */
	if (!src) {
		_D ((": ERROR! src=0x00"));
		return;
	}

	for(y1++, y = 0; y < h; y++) {
		for (yy = (y1 + y) * _WIDTH, x = 0; x < w; x++) {
			if ((c = src[x + y * w]) == trans)
				continue;

			xx = x1 + x;
			if(y + y1 >= _HEIGHT || xx >= _WIDTH)
				continue;

			if(prio < priority_data[yy + xx])
				continue;

			priority_data[yy+xx] = prio;
			if (!greatest_kludge_of_all_time) {
				screen_data[yy+xx] = c;
			}
				/* Should be in the if, but it breaks SQ2 */
				put_pixel_buffer (xx, y + y1, c);
			screen2[yy+xx] = c;
		}
	}
	y = line_min_print ? 8 : 0;
	set_block (x1, y1 + y, x1 + w, y1 + h + y);
}


void do_blit ()
{
	/* _D (("()")); */
	if (x_min < x_max && y_min < y_max) {
		/* _D ((": %d %d %d %d", x_min, y_min, x_max, y_max)); */
		gfx->put_block (x_min << 1, y_min, (x_max << 1) + 1, y_max);
	}
	
	x_min = 320;
	x_max = 0;
	y_min = 200;
	y_max = 0;
}


static void release_sprite (int i)
{
	if (view_table[i].bg_scr) {
		agi_put_bitmap (view_table[i].bg_scr, view_table[i].bg_x,
			view_table[i].bg_y, view_table[i].bg_x_size,
			view_table[i].bg_y_size, 0xff, 0xff);
		free (view_table[i].bg_scr);
		view_table[i].bg_scr = NULL;
	}

	if (view_table[i].bg_pri) {
		put_bitmap (priority_data, view_table[i].bg_pri,
			view_table[i].bg_x, view_table[i].bg_y,
			view_table[i].bg_x_size,
			view_table[i].bg_y_size, 0xff, 0xff);
		free(view_table[i].bg_pri);
		view_table[i].bg_pri = NULL;
	}
}


static int cmp_pri (const void *a, const void *b)
{
	int x = *(int *)a, y = *(int *)b;
	
	if (view_table[x].priority < view_table[y].priority)
		return -1;

	if (view_table[x].priority > view_table[y].priority)
		return 1;

	if (view_table[x].y_pos < view_table[y].y_pos)
		return -1;

	if (view_table[x].y_pos > view_table[y].y_pos)
		return 1;

	return 0;
}


/* UPDATE has precedence over CYCLING -- balloons in MUMG fail when or'ing */

static void _release_sprites (int all)
{
	int i, a;
	int list[MAX_VIEWTABLE];

	for (i = 0; i < MAX_VIEWTABLE; i++)
		list[i] = i;

	qsort (list, MAX_VIEWTABLE, sizeof (int), cmp_pri);

	/* CM: remove sprites from top to bottom */
 	for (i = MAX_VIEWTABLE - 1; i >= 0; i--) {
		a = list[i];

		if (~view_table[a].flags & DRAWN)
			continue;

		if (~view_table[a].flags & ANIMATED)
			continue;

		if (!all) {
			if (~view_table[a].flags & UPDATE)
				continue;
		}

		release_sprite (a);
	}
}


/* Erase all sprites, including non-updating */
void erase_sprites ()
{
	_release_sprites (1);
}

void release_sprites ()
{
	_release_sprites (0);
}


/* Draw all sprites, including non-updating etc */
static void _draw_sprites (int all)
{
	int i, a;
	int list[MAX_VIEWTABLE];

	for (i = 0; i < MAX_VIEWTABLE; i++)
		list[i] = i;

	for(i = 0; i < MAX_VIEWTABLE; i++) {
		/* Calculate priority bands */
		if (~view_table[i].flags & FIXED_PRIORITY) {
			view_table[i].priority = 
				view_table[i].y_pos < 48 ? 4 :
				view_table[i].y_pos / 12 + 1;
		}
	}

	qsort (list, MAX_VIEWTABLE, sizeof (int), cmp_pri);

	/* CM: redraw sprites from bottom to top */
	for(i = 0; i < MAX_VIEWTABLE; i++) {
		a = list[i];

		if (~view_table[a].flags & DRAWN)
			continue;

		if (~view_table[a].flags & ANIMATED)
			continue;

		if (!all) {
			if (~view_table[a].flags & UPDATE)
				continue;
		}

		draw_obj (a);
	}
}


void draw_sprites ()
{
	_draw_sprites (1);
}


void redraw_sprites ()
{
	_draw_sprites (0);
}


void print_character (int x, int y, char c, int fg, int bg)
{
	if (allow_kyb_input) {
		put_text_character( 0, x, y, c, fg, bg );
		/* CM: the extra pixel in y is for the underline cursor */
		gfx->put_block (x, y, x + 7, y + 8); 
	}
}

