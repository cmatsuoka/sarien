/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999 Dark Fiber, (C) 1999,2001 Claudio Matsuoka
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

#include "sarien.h"
#include "agi.h"
#include "gfx.h"
#include "picture.h"

#define next_byte data[foffs++]

struct point {
	struct point *next;
	int x, y;
};

struct agi_picture pictures[MAX_DIRS];

UINT8	*data;
UINT32	flen;
UINT32	foffs;

UINT8	patCode;
UINT8	patNum;
UINT8	we_are_drawing;
UINT8	pri_on;
UINT8	scr_on;
UINT8	scr_colour;
UINT8	pri_colour;

UINT8	screen2[_WIDTH * _HEIGHT];
UINT8	screen_data[_WIDTH * _HEIGHT];
UINT8	priority_data[_WIDTH * _HEIGHT];
UINT8	control_data[_WIDTH * _HEIGHT];
UINT8	xdata_data[_WIDTH * _HEIGHT];

UINT8	layer1_data[320 * 200];
UINT8	layer2_data[320 * 200];

UINT8	pic_clear_flag = TRUE;



extern UINT8 old_prio;		/* Used in add_to_pic() */

extern struct sarien_options opt;


void dump_screen (int resnum)
{
	put_block_buffer (pictures[resnum].sdata, 0, 0, _WIDTH, _HEIGHT);
	put_screen ();
}


void dump_screen2 (void)
{
	put_block_buffer (screen_data, 0, 0, _WIDTH, _HEIGHT);
	put_screen();
}


void dump_pri (int resnum)
{
	put_block_buffer (pictures[resnum].pdata, 0, 0, _WIDTH, _HEIGHT);
	put_screen ();
}


void dump_con (int resnum)
{
	put_block_buffer (pictures[resnum].cdata, 0, 0, _WIDTH, _HEIGHT);
	put_screen();
}


void dump_x (int resnum)
{
	put_block_buffer (pictures[resnum].xdata, 0, 0, _WIDTH, _HEIGHT);
	put_screen ();
}


void dump_screenX ()
{
	memmove (screen_data, screen2, _WIDTH*_HEIGHT);
	put_block_buffer (screen_data, 0, 0, _WIDTH, _HEIGHT);
	put_screen ();
}


void dump_screen3 ()
{
	put_block_buffer (screen2, 0, 0, _WIDTH, _HEIGHT);
	put_screen ();
}


void dump_pri_screen ()
{
	put_block_buffer (priority_data, 0, 0, _WIDTH, _HEIGHT);
	put_screen ();
}


void dump_con_screen ()
{
	put_block_buffer (control_data, 0, 0, _WIDTH, _HEIGHT);
	put_screen ();
}


void dump_x_screen ()
{
	put_block_buffer (xdata_data, 0, 0, _WIDTH, _HEIGHT);
	put_screen ();
}


static void clear_base ()
{
	memset (&screen2, 0x0F, _WIDTH * _HEIGHT);
}


static void clear_priority ()
{
	memset (&priority_data, 0x4, _WIDTH * _HEIGHT);
	memset (&control_data, 0x4, _WIDTH * _HEIGHT);
	memset (&xdata_data, 0x4, _WIDTH * _HEIGHT);
}


static void put_virt_screen_pixel (UINT16 x, UINT16 y)
{
	screen2[y * _WIDTH + x] = scr_colour;
}


static void put_virt_pri_pixel (UINT16 x, UINT16 y)
{
	xdata_data[y * _WIDTH + x] = pri_colour;

	if (pri_colour >= 0x4)
		priority_data[y * _WIDTH + x] = pri_colour;
#if 0
	/* This can't be done here. It doesn't understand control lines
	 * painted over with priority data.
	 */
	if (pri_colour >= 0x4)
		priority_data[y * _WIDTH + x] = pri_colour;
	else
		control_data[y * _WIDTH + x] = pri_colour;
#endif
}


static UINT8 get_scr_pixel (UINT16 x, UINT16 y)
{
	return screen2[y * _WIDTH + x];
}


static UINT8 get_pri_pixel (UINT16 x, UINT16 y)
{
	return xdata_data[y * _WIDTH + x];
}


static void put_virt_pixel(UINT16 x, UINT16 y)
{
	if (x >= _WIDTH || y >= _HEIGHT)
		return;

	if (pri_on)
		put_virt_pri_pixel (x, y);
	if (scr_on)
		put_virt_screen_pixel (x, y);
}


/* For the flood fill routines */
#define STACK_SEG_SIZE 0x1000
#define MAX_STACK_SEGS 16
static struct point *stack[MAX_STACK_SEGS];
static int stack_num_segs;
static int stack_seg;
static int stack_ptr;

INLINE void _PUSH(struct point *c)
{
	if (stack_ptr >= STACK_SEG_SIZE) {
		/* Allocate new stack segment */
		if (stack_num_segs >= MAX_STACK_SEGS) {
			fprintf (stderr, "flood fill stack overflow\n");
			abort ();
		}
		if (stack_num_segs <= ++stack_seg) {
			_D ((": new stack (#%ld)", stack_num_segs));
			stack[stack_num_segs++] = malloc (sizeof (struct point) *
				STACK_SEG_SIZE);
		}
		stack_ptr = 0;
	}

	stack[stack_seg][stack_ptr].x = c->x;
	stack[stack_seg][stack_ptr].y = c->y;
	stack_ptr++;
}

INLINE void _POP (struct point *c)
{
	if (stack_ptr == 0) {
		if (stack_seg == 0)
			c->x = c->y = 0xffff;
		else
			stack_seg--, stack_ptr = STACK_SEG_SIZE - 1;

		return;
	}

	stack_ptr--;
	c->x = stack[stack_seg][stack_ptr].x;
	c->y = stack[stack_seg][stack_ptr].y;
}


/**************************************************************************
** drawline
** Draws an AGI line.
**
** line drawing routine sent to me by joshua neal
** modified by stuart george, fixed >>2 to >>1 and some other bugs
** like x1 instead of y1, etc
**************************************************************************/
static void draw_line (int x1, int y1, int x2, int y2)
{
	int i, x, y, deltaX, deltaY, stepX, stepY, errorX, errorY, detdelta;

	/* CM: Do clipping */
#define clip(x, y) if((x)>=(y)) (x)=(y)
	clip (x1, _WIDTH-1);
	clip (x2, _WIDTH-1);
	clip (y1, _HEIGHT-1);
	clip (y2, _HEIGHT-1);

	/* Vertical line */

	if (x1 == x2) {
		if(y1 > y2) {
			y = y1;
			y1 = y2;
			y2 = y;
		}

		for( ; y1 <= y2; y1++)
			put_virt_pixel (x1, y1);

		return;
	}

	/* Horizontal line */

	if (y1 == y2) {
		if (x1 > x2) {
			x = x1;
			x1 = x2;
			x2 = x;
		}

   		for( ; x1 <= x2; x1++)
   			put_virt_pixel (x1, y1);

		return;
	}

	y = y1;
  	x = x1;
	put_virt_pixel (x1, y1);

	stepY = 1;
	deltaY = y2-y1;
	if (deltaY < 0) {
		stepY = -1;
		deltaY = -deltaY;
	}

	stepX = 1;
	deltaX = x2 - x1;
	if (deltaX < 0) {
		stepX = -1;
		deltaX = -deltaX;
	}

	if(deltaY > deltaX) {
		i=deltaY;
		detdelta=deltaY;
		errorX=deltaY/2;
		errorY=0;
	} else {
		i=deltaX;
		detdelta=deltaX;
		errorX=0;
		errorY=deltaX/2;
	}

	put_virt_pixel (x, y);

	do {
		errorY += deltaY;
		if (errorY >= detdelta) {
			errorY -= detdelta;
			y += stepY;
		}

		errorX += deltaX;
		if (errorX >= detdelta) {
			errorX -= detdelta;
			x += stepX;
		}

		put_virt_pixel(x, y);
		i--;
	} while (i > 0);

	put_virt_pixel (x, y);
}

/**************************************************************************
** relativeDraw
**
** Draws short lines relative to last position.  (drawing action 0xF7)
**************************************************************************/
static void dynamic_draw_line ()
{
	int x1, y1, disp, dx, dy;

	x1 = next_byte;
	y1 = next_byte;

	put_virt_pixel (x1, y1);

	while (42) {
		if ((disp = next_byte) >= 0xf0)
			break;

		dx= ((disp & 0xf0) >> 4) & 0x0f;
		dy= (disp & 0x0f);

	      	if (dx & 0x08)
			dx = -(dx & 0x07);

	      	if (dy & 0x08)
			dy = -(dy & 0x07);

		draw_line (x1, y1, x1 + dx, y1 + dy);
		x1 += dx;
		y1 += dy;
	}
	foffs--;
}

/**************************************************************************
** absoluteLine
**
** Draws long lines to actual locations (cf. relative) (drawing action 0xF6)
**************************************************************************/
static void absolute_draw_line ()
{
	UINT8	x1, y1, x2, y2;

	x1= next_byte;
	y1= next_byte;
	put_virt_pixel (x1, y1);

	while (42) {
		if ((x2 = next_byte) >= 0xf0)
			break;

		if ((y2 = next_byte) >= 0xf0)
			break;

		draw_line (x1, y1, x2, y2);
		x1 = x2;
		y1 = y2;
	}
	foffs--;
}


/**************************************************************************
** okToFill
**************************************************************************/
static INLINE int is_ok_fill_here (int x, int y)
{
	if (!scr_on && !pri_on)
		return FALSE;

	if (!pri_on && scr_on && scr_colour != 15)
		return get_scr_pixel (x, y) == 15;

	if (pri_on && !scr_on && pri_colour != 4)
		return get_pri_pixel (x, y) == 4;

	return (get_scr_pixel (x, y) == 15 && scr_colour != 15);
}

/**************************************************************************
** agiFill
**************************************************************************/
static void agiFill (int x, int y)
{
	struct point c;

	/* _D (("(%d, %d)", x, y)); */
	c.x = x;
	c.y = y;
	_PUSH (&c);

	while (42) {
		_POP(&c);

		/* Exit if stack is empty */
		if ((c.x == 0xFFFF) || (c.y == 0xFFFF))
			break;

		if (is_ok_fill_here (c.x,c.y)) {
			put_virt_pixel (c.x, c.y);
			if (c.x > 0 && is_ok_fill_here (c.x-1, c.y)) {
				c.x--; _PUSH (&c); c.x++;
    			}
			if (c.x < _WIDTH - 1 && is_ok_fill_here (c.x+1, c.y)) {
				c.x++; _PUSH (&c); c.x--;
 			}
			if (c.y < _HEIGHT - 1 && is_ok_fill_here (c.x, c.y+1)) {
				c.y++; _PUSH (&c); c.y--;
    			}
			if (c.y > 0 && is_ok_fill_here (c.x, c.y-1)) {
				c.y--; _PUSH (&c); c.y++;
    			}
		}
#ifdef DUMPFILL
		if (opt.showscreendraw)
			dump_screen3 ();
#endif
	}

	stack_ptr = 0;
	stack_seg = 0;
}

/**************************************************************************
** xCorner
**
** Draws an xCorner  (drawing action 0xF5)
**************************************************************************/
static void x_corner ()
{
	int x1, x2, y1, y2;

	x1 = next_byte;
	y1 = next_byte;
   	put_virt_pixel (x1,y1);

	while (42) {
		x2=next_byte;

		if (x2 >= 0xf0)
			break;

		draw_line (x1, y1, x2, y1);
		x1 = x2;
		y2 = next_byte;

		if(y2>=0xF0)
			break;

		draw_line (x1, y1, x1, y2);
		y1 = y2;
	}
	foffs--;
}


/**************************************************************************
** yCorner
**
** Draws an yCorner  (drawing action 0xF4)
**************************************************************************/
static void y_corner ()
{
	int x1, x2, y1, y2;

	x1 = next_byte;
	y1 = next_byte;
	put_virt_pixel (x1, y1);

	while (42) {
		y2 = next_byte;

		if (y2 >= 0xF0)
			break;

		draw_line (x1, y1, x1, y2);
		y1 = y2;
		x2 = next_byte;

		if (x2 >= 0xF0)
			break;

		draw_line (x1, y1, x2, y1);
		x1 = x2;
	}

	foffs--;
}

/**************************************************************************
** fill
**
** AGI flood fill.  (drawing action 0xF8)
**************************************************************************/
static void fill ()
{
	int x1, y1;

	while ((x1 = next_byte) < 0xF0 && (y1 = next_byte) < 0xF0)
		agiFill (x1, y1);

	foffs--;
}


#define plotPatternPoint() do {						\
	if (patCode & 0x20) {						\
		if ((splatterMap[bitPos>>3] >> (7-(bitPos&7))) & 1)	\
			put_virt_pixel(x1, y1);				\
		bitPos++;						\
		if (bitPos == 0xff)					\
			bitPos=0;					\
	} else put_virt_pixel(x1, y1);					\
} while (0)

/**************************************************************************
** plotPattern
**
** Draws pixels, circles, squares, or splatter brush patterns depending
** on the pattern code.
**************************************************************************/
void plotPattern(UINT8 x, UINT8 y)
{
	static char circles[][15] = {		/* agi circle bitmaps */
		{ 0x80 },
		{ 0xfc },
		{ 0x5f, 0xf4 },
		{ 0x66, 0xff, 0xf6, 0x60 },
		{ 0x23, 0xbf, 0xff, 0xff, 0xee, 0x20 },
		{ 0x31, 0xe7, 0x9e, 0xff, 0xff, 0xde, 0x79, 0xe3, 0x00 },
		{ 0x38, 0xf9, 0xf3, 0xef, 0xff, 0xff,
		  0xff, 0xfe, 0xf9, 0xf3, 0xe3, 0x80 },
		{ 0x18, 0x3c, 0x7e, 0x7e, 0x7e, 0xff, 0xff,
		  0xff, 0xff, 0xff, 0x7e, 0x7e, 0x7e, 0x3c, 0x18 }
	};

	static UINT8 splatterMap[32] = {	/* splatter brush bitmaps */
		0x20, 0x94, 0x02, 0x24, 0x90, 0x82, 0xa4, 0xa2,
		0x82, 0x09, 0x0a, 0x22, 0x12, 0x10, 0x42, 0x14,
		0x91, 0x4a, 0x91, 0x11, 0x08, 0x12, 0x25, 0x10,
		0x22, 0xa8, 0x14, 0x24, 0x00, 0x50, 0x24, 0x04
	};

	static UINT8 splatterStart[128] = {	/* starting bit position */
		0x00, 0x18, 0x30, 0xc4, 0xdc, 0x65, 0xeb, 0x48,
		0x60, 0xbd, 0x89, 0x05, 0x0a, 0xf4, 0x7d, 0x7d,
		0x85, 0xb0, 0x8e, 0x95, 0x1f, 0x22, 0x0d, 0xdf,
		0x2a, 0x78, 0xd5, 0x73, 0x1c, 0xb4, 0x40, 0xa1,
		0xb9, 0x3c, 0xca, 0x58, 0x92, 0x34, 0xcc, 0xce,
		0xd7, 0x42, 0x90, 0x0f, 0x8b, 0x7f, 0x32, 0xed,
		0x5c, 0x9d, 0xc8, 0x99, 0xad, 0x4e, 0x56, 0xa6,
		0xf7, 0x68, 0xb7, 0x25, 0x82, 0x37, 0x3a, 0x51,
		0x69, 0x26, 0x38, 0x52, 0x9e, 0x9a, 0x4f, 0xa7,
		0x43, 0x10, 0x80, 0xee, 0x3d, 0x59, 0x35, 0xcf,
		0x79, 0x74, 0xb5, 0xa2, 0xb1, 0x96, 0x23, 0xe0,
		0xbe, 0x05, 0xf5, 0x6e, 0x19, 0xc5, 0x66, 0x49,
		0xf0, 0xd1, 0x54, 0xa9, 0x70, 0x4b, 0xa4, 0xe2,
		0xe6, 0xe5, 0xab, 0xe4, 0xd2, 0xaa, 0x4c, 0xe3,
		0x06, 0x6f, 0xc6, 0x4a, 0xa4, 0x75, 0x97, 0xe1
	};

	SINT32 circlePos = 0;
	UINT32 x1, y1, penSize, bitPos = splatterStart[patNum];

	penSize = (patCode & 7);

	if (x < penSize)
		x = penSize-1;
	if (y < penSize)
		y = penSize;

	for (y1 = y - penSize; y1 <= y + penSize; y1++) {
		for (x1 = x-(penSize+1)/2; x1<=x+penSize/2; x1++) {
			if (patCode & 0x10) {		/* Square */
				plotPatternPoint();
			} else {			/* Circle */
				if ((circles[patCode&7][circlePos>>3] >> (7-(circlePos&7)))&1)
					plotPatternPoint();
				circlePos++;
			}
		}
	}
}

/**************************************************************************
** plotBrush
**
** Plots points and various brush patterns.
**************************************************************************/
static void plot_brush ()
{
	int x1, y1;

	while (42) {
		if (patCode & 0x20) {
			if ((patNum = next_byte) >= 0xF0)
				break;
			patNum = (patNum >> 1) & 0x7f;
		}

		if ((x1 = next_byte) >= 0xf0)
			break;

		if ((y1 = next_byte) >= 0xf0)
			break;

		plotPattern (x1, y1);
   	}

   	foffs--;
}


UINT8* convert_v2_v3_pic (UINT8 *data, UINT32 len)
{
	UINT8	d, old = 0, x, *in, *xdata, *out, mode = 0;
	UINT32	i, ulen;

	xdata = (UINT8*)malloc (len + len / 2);

	out = xdata;
	in = data;

	for (i = ulen = 0; i < len; i++, ulen++) {
		d = *in++;

		*out++ = x = mode ? ((d & 0xF0) >> 4) + ((old & 0x0F) << 4) : d;

		if (x == 0xFF) {
			ulen++;
			break;
		}

		if (x == 0xf0 || x == 0xf2) {
			if (mode) {
				*out++ = d & 0x0F;
				ulen++;
			} else {
				d = *in++;
				*out++ = (d & 0xF0) >> 4;
				i++, ulen++;
			}

			mode = !mode;
		}

		old = d;
	}

	free (data);
	xdata = realloc (xdata, ulen);

	return xdata;
}


/**************************************************************************
** splitPriority
**
** Purpose: To split the priority colours from the control colours. It
** determines the priority information for pixels that are overdrawn by
** control lines by the same method used in Sierras interpreter (at this
** stage). This could change later on.
**************************************************************************/
void splitPriority (int resnum)
{
	UINT16 x, y;
	register UINT8 *p, *c;

	_D (("()"));

	p = pictures[resnum].xdata;
	c = pictures[resnum].cdata;

	for (x = 0; x < _WIDTH; x++) {
		for (y = 0; y < _HEIGHT; y++, p++, c++) {
			if (*p == 3) {
				*c = *p;
#if 0
				*p = 4;
#endif
		 	}

			if (*p >= 3)
				continue;

			*c = *p;
#if 0
			for (pp = p + _WIDTH, dy = y + 1;
				dy < _HEIGHT; dy++, pp += _WIDTH) {
				if (*pp > 3) {
					*p = *pp;
					break;
				}
			}
#endif
		}
	}
}


/* FIXME */
extern UINT8 show_screen_mode;

void draw_picture ()
{
	UINT8	act;
	int i;

	_D (("()"));
 	patCode = 0;
 	patNum = 0;
 	pri_on = scr_on = FALSE;
 	scr_colour = 0xf;
 	pri_colour = 0x4;
	old_prio = 4;

	if (opt.showkeypress == 3)
		opt.showkeypress = TRUE;
	if (opt.showscreendraw == 3)
		opt.showscreendraw = TRUE;

	if (pic_clear_flag == TRUE) {
		clear_base ();
		clear_priority ();
	}

	we_are_drawing = 1;

	stack[0] = calloc (sizeof (struct point), STACK_SEG_SIZE);
	stack_ptr = stack_seg = 0;
	stack_num_segs = 1;

	for (we_are_drawing = 1; we_are_drawing && foffs < flen; ) {
		act = next_byte;

		switch(act) {
		case 0xf0:			/* set colour on screen */
			scr_colour = next_byte;
			scr_colour &= 0xF;	/* for v3 drawing diff */
			scr_on = TRUE;
			break;
		case 0xf1:			/* disable screen drawing */
			scr_on = FALSE;
			break;
		case 0xf2:			/* set colour on priority */
			pri_colour = next_byte;
			pri_colour &= 0xf;	/* for v3 drawing diff */
			pri_on = TRUE;
			break;
		case 0xf3:			/* disable priority screen */
			pri_on = FALSE;
			break;
		case 0xf4:			/* y-corner */
			y_corner ();
			break;
		case 0xf5:			/* x-corner */
			x_corner ();
			break;
		case 0xf6:			/* absolute draw lines */
			absolute_draw_line ();
			break;
		case 0xf7:			/* dynamic draw lines */
			dynamic_draw_line ();
			break;
		case 0xf8:			/* fill */
			fill ();
			break;
		case 0xf9:			/* set pattern */
			patCode = next_byte;
			break;
		case 0xfA:			/* plot brush */
			plot_brush ();
			break;
		case 0xFF:			/* end of pic data */
		default:
			we_are_drawing = 0;
			break;
		}

		if (opt.showscreendraw) {
			switch (show_screen_mode) {
			case 'x':
				put_block_buffer (xdata_data, 0, 0,
					_WIDTH, _HEIGHT);
				break;
			case 'p':
				put_block_buffer (priority_data, 0, 0,
					_WIDTH, _HEIGHT);
				break;
			case 'c':
				put_block_buffer (control_data, 0, 0,
					_WIDTH, _HEIGHT);
				break;
			default:
				dump_screen3 ();
				break;
			}

			put_screen ();
		}

		/* FIXME: ugh */
		if (opt.showscreendraw && opt.showkeypress) {
			act = gfx->get_key() & 0xFF;
			/*if (act == 0) getchar ();*/
			if(act == 'c')
				opt.showkeypress = 3;
			if(act == 'q' || act == 'Q')
				opt.showscreendraw = 3;
		}
	}

	for (i = 0; i < stack_num_segs; i++)
		free (stack[i]);

	/* splitPriority (); */
}


void put_block_buffer (UINT8 *buff, int x1, int y1, int x2, int y2)
{
	int x;

	for ( ; y1 < y2; y1++) {
		for(x = x1; x < x2; x++)
			put_pixel_buffer (x, y1, *(buff + (y1 * 160) + x));
	}
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


/* load a pic and decode it into the correct slot */
int decode_picture (int resnum)
{
	int ec = err_OK;

	_D (("(%d)", resnum));

	patCode = 0;
	patNum = 0;
	we_are_drawing = pri_on = scr_on = FALSE;
	scr_colour = 0xF;
	pri_colour = 0x4;

	data = pictures[resnum].rdata;
	flen = dir_pic[resnum].len;
	foffs = 0;

	pictures[resnum].sdata = malloc (_WIDTH * _HEIGHT);
	pictures[resnum].pdata = malloc (_WIDTH * _HEIGHT);
	pictures[resnum].cdata = malloc (_WIDTH * _HEIGHT);
	pictures[resnum].xdata = malloc (_WIDTH * _HEIGHT);

	draw_picture ();

	memcpy (pictures[resnum].sdata, &screen_data, _WIDTH * _HEIGHT);
	memcpy (pictures[resnum].pdata, &priority_data, _WIDTH * _HEIGHT);
	memcpy (pictures[resnum].cdata, &control_data, _WIDTH * _HEIGHT);
	memcpy (pictures[resnum].xdata, &xdata_data, _WIDTH * _HEIGHT);

	splitPriority (resnum);

	/* FIXME: Ugh! */
	memcpy (&priority_data, pictures[resnum].pdata, _WIDTH * _HEIGHT);
	memcpy (&control_data, pictures[resnum].cdata, _WIDTH * _HEIGHT);

	return ec;
}


int unload_picture (int resnum)
{
	/* remove visual buffer & priority buffer if they exist */
	if (dir_pic[resnum].flags & RES_LOADED) {
		if (~dir_pic[resnum].flags & 0x80) {
			free (pictures[resnum].pdata);	/* free priority image */
			free (pictures[resnum].sdata);	/* free screen image */
			free (pictures[resnum].cdata);	/* free control image */
			free (pictures[resnum].xdata);	/* free p+c image */
		}
		free (pictures[resnum].rdata);	
		dir_pic[resnum].flags &= ~RES_LOADED;
	}

	return err_OK;
}

