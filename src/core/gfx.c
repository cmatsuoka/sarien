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

__GFX_DRIVER	*gfx;			/* graphics driver */
UINT8		screen_mode;		/* 0=gfx mode, 1=text mode */
UINT8		txt_fg;			/* fg colour */
UINT8		txt_bg;			/* bg colour */
UINT8		txt_char;		/* input character */


/* for the blitter routine */
static UINT16 x_min = 320, x_max = 0, y_min = 200, y_max = 0;

/* Ugly kludge for nonblocking windows */
static int k_x1, k_y1, k_x2, k_y2;

extern struct sarien_console console;
extern struct agi_view_table view_table[];

int greatest_kludge_of_all_time = 0;


/* driver wrapper */
/*static*/ void INLINE put_pixel2 (UINT16 x, UINT16 y, UINT16 c)
{
	UINT16 k;

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


void put_pixel (UINT16 x, UINT16 y, UINT16 c)
{
	layer1_data[y * 320 + x] = c;
	put_pixel2 (x, y, c);
}


void flush_block (UINT16 x1, UINT16 y1, UINT16 x2, UINT16 y2)
{
	UINT32 x, y;

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
void shake_screen (UINT8 n)
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
}


void put_pixel_buffer (UINT16 x, UINT16 y, UINT16 c)
{
	if (line_min_print > 0)
		y += 8;
	x <<= 1;
	put_pixel (x, y, c);
	put_pixel (x + 1, y, c);
}


UINT16 init_video ()
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


UINT16 deinit_video ()
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


void message_box (UINT8 *message, ...)
{
	char	x[512];
	va_list	args;

	_D (("(message, ...)"));
	va_start (args, message);

	#ifdef HAVE_VSNPRINTF
	vsnprintf (x, 510, (char*)message, args);
	#else
	vsprintf (x, (char*)message, args);
	#endif

	va_end (args);

	save_screen ();
	redraw_sprites ();

	/* FR:
	 * Messy...
	 */
	allow_kyb_input = FALSE;

	textbox((UINT8*)x, -1, -1, -1);
	message_box_key = wait_key();

	allow_kyb_input = TRUE;

	release_sprites ();
	restore_screen ();
}


/* len = len in characters, not pixels!!
 */
void textbox(UINT8 *message, SINT16 x, SINT16 y, SINT16 len)
{
	/* if x | y = -1, then centre the box */
	SINT16	xoff, yoff, lin;
	UINT8	*msg, *m;

	_D (("(\"%s\", %d, %d, %d)", message, x, y, len));

	if (len <= 0 || len >= 40)
		//len = 29;		/* FIXME: 29 or 39?? */
		len=30;

	xoff = x;
	yoff = y;
	len--;

	m = msg = word_wrap_string (message, (UINT16*)&len);

	for (lin = 1; *m; m++)
		if (*m == '\n')
			lin++;

	_D ((": lin=%d", lin));

	if (lin * 8 > GFX_HEIGHT)
		lin = (GFX_HEIGHT / 8);

	if (xoff == -1)
		xoff = (GFX_WIDTH - ((len + 2) * 8)) / 2;

	if (yoff == -1)
		yoff = (GFX_HEIGHT - 16 - ((lin + 2) * 8)) / 2;

	draw_box (xoff, yoff, xoff + ((len + 2) * 8), yoff + ((lin + 2) * 8),
		MSG_BOX_COLOUR, MSG_BOX_LINE, LINES);
	print_text2 (2, msg, 0, 8 + xoff, 8 + yoff, len + 1,
		MSG_BOX_TEXT, MSG_BOX_COLOUR);
	gfx->put_block (xoff, yoff, xoff + ((len + 2) * 8), yoff + ((lin + 2) * 8));

	free (msg);
}


void print_text (UINT8 *msg, UINT16 foff, UINT16 xoff, UINT16 yoff, UINT16 len, UINT8 fg, UINT8 bg)
{
	print_text2 (0, msg, foff, xoff, yoff, len, fg, bg);
}


void print_text_layer (UINT8 *msg, UINT16 foff, UINT16 xoff, UINT16 yoff, UINT16 len, UINT8 fg, UINT8 bg)
{
	print_text2 (1, msg, foff, xoff, yoff, len, fg, bg);
}


void print_text2 (UINT8 l, UINT8 *msg, UINT16 foff, UINT16 xoff, UINT16 yoff, UINT16 len, UINT8 fg, UINT8 bg)
{
	UINT8  *m;
	UINT8  x1, y1;
	UINT16 maxx, minx, ofoff;
	UINT8 update;

	/* kludge! */
	update = 1;
	if (l == 2) {
		update = l = 0;
	}

	/*_D (("(\"%s\", %d, %d, %d, %d, %d, %d, %d",
		msg, foff, xoff, yoff, len, fg, bgc, f));*/

	/* FR :
	 * Changed here
	 * The string with len == 1 wasn't being printed...
	 */
	if (len == 1)
	{
		put_text_character (l, xoff + foff,	yoff, *msg, fg, bg);
		maxx  = 1;
		minx  = 0;
		ofoff = foff;
		y1 = 0;		/* Check this */
	}
	else
	{
		maxx  = 0;
		minx  = 320;
		ofoff = foff;
		for (m = msg, x1 = y1 = 0; *m; m++)
		{
			if (*m >= 0x20 || *m == 1 || *m == 2 || *m == 3) {
				/* FIXME */

				if((x1!=(len-1) || x1==39) && ((y1*8)+yoff <= 192)) {
					put_text_character (l, (x1 * 8) + xoff + foff,
						(y1 * 8) + yoff, *m, fg, bg);
					if (x1>maxx)
						maxx=x1;
					if (x1<minx)
						minx=x1;
				}
				x1++;
				//if(x1 == len - 1 && m[1] != '\n')
				/* DF: removed the len-1 to len... */
				if(x1 == len && m[1] != '\n')
					y1++, x1 = foff = 0;
			} else {
				y1++;
				x1=foff=0;
			}
		}
	}

	if (l)
		return;

	if (maxx < minx)
		return;

	maxx <<= 3;
	minx <<= 3;
	if (update)
		gfx->put_block (foff+xoff+minx, yoff, ofoff+xoff+maxx+7, yoff+y1*8+9);
}


/* CM: Ok, this is my attempt to make a good line wrapping algorithm.
 *     Sierra like, that is.
 */
UINT8* word_wrap_string(UINT8 *mesg, UINT16 *len)
{
	char *msg, *v, *e;
	UINT16 maxc, c, l = *len;

	_D (("(\"%s\", %d)", mesg, *len));
	v = msg = strdup ((char*)mesg);
	e = msg + strlen ((char*)msg);
	maxc = 0;

	while (42)
	{
		while ((c = strcspn (v, "\n")) <= l)
		{
			if (c > maxc)
				maxc = c;
			if ((v += c + 1) >= e)
				goto end;
		}
		c = l;
		if ((v += l) >= e)
			break;
		if (*v != ' ')
			for (; *v != ' '; v--, c--);
		if (c > maxc)
			maxc = c;
		*v++ = '\n';
	}
end:
	*len = maxc;
	return (UINT8*)msg;
}


void put_text_character (UINT8 l, UINT16 x, UINT16 y, UINT8 c, UINT8 fg, UINT8 bg)
{
	UINT16	x1, y1, xx, yy, cc;
	UINT8	*p;

	p = font + (c << 3);
	for (y1 = 0; y1 < 8; y1++)
	{
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


void draw_box(UINT16 x1, UINT16 y1, UINT16 x2, UINT16 y2, UINT8 colour1, UINT8 colour2, UINT8 f)
{
	UINT16	x, y;

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

	for(y=y1; y<y2; y++)
		for(x=x1; x<x2; x++)
			put_pixel(x, y, colour1);

	if((f&LINES)==LINES)
	{
		/* draw lines */
		for(x=x1; x<x2-4; x++)
		{
			put_pixel(x+2, y1+2, colour2);
			put_pixel(x+2, y2-3, colour2);
		}

		for(y=y1; y<=y2-5; y++)
		{
			put_pixel(x1+2, y+2, colour2);
			put_pixel(x1+3, y+2, colour2);

			put_pixel(x2-3, y+2, colour2);
			put_pixel(x2-4, y+2, colour2);
		}
	}

	y = line_min_print ? 8 : 0;
	set_block (x1, y1 + y, x2, y2 + y);
}


void get_bitmap(UINT8 *dst, UINT8 *src, UINT16 x1, UINT16 y1, UINT16 w, UINT16 h)
{
	UINT16	y, x;

	for(y1++, y=0; y<h; y++)
	{
		for(x=0; x<w; x++)
		{
			if(y1+y<_HEIGHT && x1+x<_WIDTH)
				dst[(y*w)+x]=src[((y1+y)*_WIDTH)+(x1+x)];
		}
	}
}


void put_bitmap(UINT8 *dst, UINT8 *src, UINT16 x1, UINT16 y1, UINT16 w, UINT16 h, UINT16 trans, UINT16 prio)
{
	UINT32 x, y, xx, yy;
	UINT8 c;

	/* _D (("(%p, %p, %d, %d, %d, %d, %d, %d)",
		dst, src, x1, y1, w, h, trans, prio)); */

	if (prio < 4)
		prio = 4;

	for (y1++, y=0; y<h; y++)
	{
		for (yy = (y1 + y) * _WIDTH, x=0; x<w; x++)
		{
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


void agi_put_bitmap(UINT8 *src, UINT16 x1, UINT16 y1, UINT16 w, UINT16 h, UINT16 trans, UINT16 prio)
{
	UINT32 x, y, xx, yy;
	UINT8	c;

	/* _D (("(%p, %d, %d, %d, %d, %d, %d)",
		src, x1, y1, w, h, trans, prio)); */

	if (prio < 4)
		prio = 4;

	/* FIXME: claudio's anti-crash test */
	if (!src) {
		_D ((": ERROR! src=0x00"));
		return;
	}

	for(y1++, y=0; y<h; y++)
	{
		for (yy = (y1 + y) * _WIDTH, x = 0; x < w; x++)
		{
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


static void release_sprite (UINT16 i)
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

