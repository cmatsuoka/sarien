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
#include "agi.h"
#include "gfx_base.h"
#include "gfx_agi.h"
#include "keyboard.h"
#include "view.h"
#include "picture.h"
#include "console.h"

extern struct sarien_options opt;
extern struct sarien_console console;
extern struct agi_view_table view_table[];

int greatest_kludge_of_all_time = 0;


/* This works in the 320x200 visible screen, but receiving AGI 160x168
 * coordinates
 */
void put_pixel_buffer (int x, int y, int c)
{
	if (game.line_min_print > 0)
		y += 8;
	x <<= 1;
	put_pixel (x, y, c);
	put_pixel (x + 1, y, c);
}



/* Works in the 160x168 AGI buffer */
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


/* Works in the 160x168 AGI buffer, priority only */
static void put_bitmap (UINT8 *dst, UINT8 *src, int x1, int y1,
	int w, int h, int trans, int prio)
{
	int x, y, xx, yy;
	int c;

	if (prio < 4)
		prio = 4;

	for (y1++, y = 0; y < h; y++) {
		for (yy = (y1 + y) * _WIDTH, x=0; x<w; x++) {
			if ((c=src[x + y * w]) == trans)
				continue;

			xx = x1 + x;
			if (y + y1 >= _HEIGHT || xx >= _WIDTH)
				continue;

			if (prio < dst[yy + xx])
				continue;

			dst[yy + xx] = c;
		}
	}

	y = game.line_min_print ? 8 : 0;
	set_block (x1, y1 + y, x1 + w, y1 + y + h);
}


/* Works in the 160x168 AGI buffer, priority and visible */
void agi_put_bitmap (UINT8 *src, int x1, int y1, int w, int h, int trans, int prio)
{
	int x, y, xx, yy;
	int c;

	if (prio < 4)
		prio = 4;

	/* FIXME: claudio's anti-crash test */
	if (!src) {
		_D ((": ERROR! src=0x00"));
		return;
	}

	for (y1++, y = 0; y < h; y++) {
		for (yy = (y1 + y) * _WIDTH, x = 0; x < w; x++) {
			if ((c = src[x + y * w]) == trans)
				continue;

			xx = x1 + x;
			if (y + y1 >= _HEIGHT || xx >= _WIDTH)
				continue;

			if (prio < priority_data[yy + xx])
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
	y = game.line_min_print ? 8 : 0;
	set_block (x1, y1 + y, x1 + w, y1 + h + y);
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


int init_video_mode ()
{
	return init_video ();
}


int deinit_video_mode ()
{
	return deinit_video ();
}



void reset_graphics(void)
{
	screen_mode = GFX_MODE;

	txt_fg = 0x0F;
	txt_bg = 0x00;

	memset (screen2, 0, _WIDTH * _HEIGHT);
	memset (screen_data, 0, _WIDTH * _HEIGHT);
	memset (priority_data, 0, _WIDTH * _HEIGHT);
	memset (control_data, 0, _WIDTH * _HEIGHT);
	memset (xdata_data, 0, _WIDTH * _HEIGHT);

	memset (layer1_data, 0, 320 * 200);
	memset (layer2_data, 0, 320 * 200);
}


void put_block_buffer (UINT8 *buff)
{
	int x, x1 = 0, y1 = 0, x2 = _WIDTH, y2 = _HEIGHT;

	for ( ; y1 < y2; y1++) {
		for(x = x1; x < x2; x++)
			put_pixel_buffer (x, y1, *(buff + (y1 * 160) + x));
	}
}


void save_screen ()
{
	_save_screen ();
}


void restore_screen ()
{
	_restore_screen ();
	redraw_sprites ();
	_flush_screen ();
	release_sprites ();
}


void restore_screen_area ()
{
	_restore_screen_area ();
	redraw_sprites ();
	_flush_screen_area ();
	release_sprites ();
}

