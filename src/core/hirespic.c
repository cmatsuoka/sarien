/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#ifdef USE_HIRES

static void put_hires_pixel (int x, int y)
{
	UINT8 *p;

	if (x < 0 || y < 0 || x >= (_WIDTH * 2) || y >= _HEIGHT)
		return;

	p = &game.hires[y * (_WIDTH * 2) + x];

	if (pri_on) *p = (pri_colour << 4) | (*p & 0x0f);
	if (scr_on) *p = scr_colour | (*p & 0xf0);
}

static void fix_pixel_bothsides (int x, int y)
{
	UINT8 *p;

	p = &game.hires[y * (_WIDTH * 2) + x];
	if ((*(p - 2) & 0x0f) == scr_colour)
		put_hires_pixel (x - 1, y);
	if ((*(p + 2) & 0x0f) == scr_colour)
		put_hires_pixel (x + 1, y);
}


/**************************************************************************
** drawline
** Draws an AGI line.
**
** line drawing routine sent to me by joshua neal
** modified by stuart george, fixed >>2 to >>1 and some other bugs
** like x1 instead of y1, etc
**************************************************************************/
static void draw_hires_line (int x1, int y1, int x2, int y2)
{
	int i, x, y, deltaX, deltaY, stepX, stepY, errorX, errorY, detdelta;

	/* CM: Do clipping */
#define clip(x, y) if((x)>=(y)) (x)=(y)
	clip (x1, (_WIDTH * 2) - 1);
	clip (x2, (_WIDTH * 2) - 1);
	clip (y1, _HEIGHT - 1);
	clip (y2, _HEIGHT - 1);

	/* Vertical line */

	if (x1 == x2) {
		if(y1 > y2) {
			y = y1;
			y1 = y2;
			y2 = y;
		}

		for ( ; y1 <= y2; y1++) {
			put_hires_pixel (x1, y1);
			fix_pixel_bothsides (x1, y1);	
		}

		return;
	}

	/* Horizontal line */

	if (y1 == y2) {
		if (x1 > x2) {
			x = x1;
			x1 = x2;
			x2 = x;
		}

		fix_pixel_bothsides (x1, y1);	

   		for( ; x1 < x2; x1++)
			put_hires_pixel (x1, y1);

		put_hires_pixel (x1, y1);
		fix_pixel_bothsides (x1, y1);	

		return;
	}

	y = y1;
  	x = x1;

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

	put_hires_pixel (x, y);
	fix_pixel_bothsides (x, y);	

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

		put_hires_pixel (x, y);
		fix_pixel_bothsides (x, y);	
		i--;
	} while (i > 0);

	put_hires_pixel (x, y);
	fix_pixel_bothsides (x, y);	
}

/**************************************************************************
** relativeDraw
**
** Draws short lines relative to last position.  (drawing action 0xF7)
**************************************************************************/
static void dynamic_hires_line ()
{
	int x1, y1, disp, dx, dy;

	x1 = 2 * next_byte;
	y1 = next_byte;

	put_hires_pixel (x1, y1);

	while (42) {
		if ((disp = next_byte) >= 0xf0)
			break;

		dx= ((disp & 0xf0) >> 4) & 0x0f;
		dy= (disp & 0x0f);

	      	if (dx & 0x08)
			dx = -(dx & 0x07);

	      	if (dy & 0x08)
			dy = -(dy & 0x07);

		dx *= 2;

		draw_hires_line (x1, y1, x1 + dx, y1 + dy);
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
static void absolute_hires_line ()
{
	int x1, y1, x2, y2;

	x1 = 2 * next_byte;
	y1 = next_byte;
	put_hires_pixel (x1, y1);

	while (42) {
		if ((x2 = next_byte) >= 0xf0)
			break;

		if ((y2 = next_byte) >= 0xf0)
			break;

		x2 *= 2;

		draw_hires_line (x1, y1, x2, y2);
		x1 = x2;
		y1 = y2;
	}
	foffs--;
}


/**************************************************************************
** okToFill
**************************************************************************/
static INLINE int hires_fill_here (int x, int y)
{
	UINT8 *p, *s;

	if (!scr_on && !pri_on)
		return FALSE;

	p = &game.hires[y * (_WIDTH * 2) + x * 2];
	s = &game.sbuf[y * _WIDTH + x];

	if (scr_on) {
		if (scr_colour == 0x0f)
			return FALSE;
		if ((*p & 0x0f) != 0x0f || (*(p + 1) & 0x0f) != 0x0f)
			return FALSE;
		if ((*s & 0x0f) != scr_colour)
			return FALSE;
	}

	if (pri_on) {
		if (pri_colour == 0x04)
			return FALSE;
		if ((*p >> 4) != 0x04 || (*(p + 1) >> 4) != 0x04)
			return FALSE;
		if ((*s >> 4) != pri_colour)
			return FALSE;
	}


	return TRUE;
}


static void fix_pixel_left (int x, int y)
{
	UINT8 *p;

	if (!scr_on)
		return;

	p = &game.hires[y * (_WIDTH * 2) + x * 2 + 1];
	if ((*p & 0x0f) == 0x0f)
		put_hires_pixel (2 * x + 1, y);
	else if ((*p & 0x0f) == (*(p - 1) & 0x0f))
		put_hires_pixel (2 * x + 1, y);
}

static void fix_pixel_right (int x, int y)
{
	UINT8 p;

	p = game.hires[y * (_WIDTH * 2) + x * 2];
	if (scr_on && (p & 0x0f) == 0x0f)
		put_hires_pixel (2 * x, y);
}

static void fix_pixel_here (int x, int y)
{
	UINT8 p;

	p = game.hires[y * (_WIDTH * 2) + x * 2 + 1];
	if (scr_on && (p & 0x0f) == 0x0f)
		put_hires_pixel (2 * x + 1, y);
}


/**************************************************************************
** agiFill
**************************************************************************/
static void hiresFill (int x, int y)
{
	struct point_xy c;

	c.x = x;
	c.y = y;
	_PUSH (&c);

	while (42) {
		_POP(&c);

		/* Exit if stack is empty */
		if (c.x == 0xffff || c.y == 0xffff)
			break;

		if (hires_fill_here (c.x, c.y)) {
			put_hires_pixel (2 * c.x, c.y);
			fix_pixel_here (c.x, c.y);

			if (c.x > 0) {
				if (hires_fill_here (c.x - 1, c.y)) {
					c.x--; _PUSH (&c); c.x++;
    				} else {
					fix_pixel_left (c.x - 1, c.y);
				}
			}
			if (c.x < _WIDTH - 1) {
				if (hires_fill_here (c.x + 1, c.y)) {
					c.x++; _PUSH (&c); c.x--;
 				} else {
					fix_pixel_right (c.x + 1, c.y);
				}
			}
			if (c.y < _HEIGHT - 1 && hires_fill_here (c.x, c.y + 1)) {
				c.y++; _PUSH (&c); c.y--;
    			}
			if (c.y > 0 && hires_fill_here (c.x, c.y - 1)) {
				c.y--; _PUSH (&c); c.y++;
    			}
		}
	}

	stack_ptr = 0;
	stack_seg = 0;
}

/**************************************************************************
** xCorner
**
** Draws an xCorner  (drawing action 0xF5)
**************************************************************************/
static void hires_x_corner ()
{
	int x1, x2, y1, y2;

	x1 = 2 * next_byte;
	y1 = next_byte;
   	put_hires_pixel (x1, y1);

	while (42) {
		x2 = next_byte;

		if (x2 >= 0xf0)
			break;

		x2 *= 2;

		draw_hires_line (x1, y1, x2, y1);
		x1 = x2;
		y2 = next_byte;

		if (y2 >= 0xf0)
			break;

		draw_hires_line (x1, y1, x1, y2);
		y1 = y2;
	}
	foffs--;
}


/**************************************************************************
** yCorner
**
** Draws an yCorner  (drawing action 0xF4)
**************************************************************************/
static void hires_y_corner ()
{
	int x1, x2, y1, y2;

	x1 = 2 * next_byte;
	y1 = next_byte;
	put_hires_pixel (x1, y1);

	while (42) {
		y2 = next_byte;

		if (y2 >= 0xF0)
			break;

		draw_hires_line (x1, y1, x1, y2);
		y1 = y2;
		x2 = next_byte;

		if (x2 >= 0xf0)
			break;

		x2 *= 2;

		draw_hires_line (x1, y1, x2, y1);
		x1 = x2;
	}

	foffs--;
}

/**************************************************************************
** fill
**
** AGI flood fill.  (drawing action 0xF8)
**************************************************************************/
static void hires_fill ()
{
	int x1, y1;

	while ((x1 = next_byte) < 0xf0 && (y1 = next_byte) < 0xf0) {
		hiresFill (x1, y1);
	}

	foffs--;
}


#define plotHiresPatternPoint() do {					\
	if (patCode & 0x20) {						\
		if ((splatterMap[bitPos>>3] >> (7-(bitPos&7))) & 1)	\
			put_hires_pixel(x1, y1);			\
		bitPos++;						\
		if (bitPos == 0xff)					\
			bitPos=0;					\
	} else put_hires_pixel(x1, y1);					\
} while (0)

/**************************************************************************
** plotPattern
**
** Draws pixels, circles, squares, or splatter brush patterns depending
** on the pattern code.
**************************************************************************/
static void plot_hires_pattern(UINT8 x, UINT8 y)
{
	static UINT8 circles[][15] = {		/* agi circle bitmaps */
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
		for (x1 = x * 2-(penSize+1); x1<=x * 2+penSize; x1++) {
			if (patCode & 0x10) {		/* Square */
				plotHiresPatternPoint();
			} else {			/* Circle */
				if ((circles[patCode&7][circlePos>>3] >> (7-(circlePos&7)))&1)
					plotHiresPatternPoint();
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
static void plot_hires_brush ()
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

		plot_hires_pattern (x1, y1);
   	}

   	foffs--;
}

/**
 * Show AGI picture.
 * This function copies a ``hidden'' AGI picture to the output device.
 */
void show_hires_pic ()
{
	int i, y;

	i = 0;
	for (y = 0; y < _HEIGHT; y++) {
		put_pixels_hires (0, y, _WIDTH * 2, &game.hires[i]);
		i += _WIDTH * 2;
	}

	flush_screen ();
}

void fix_hires_picture ()
{
	UINT8 *p, *b;

	p = game.hires;
	b = game.sbuf;

	while (b < &game.sbuf[_WIDTH * _HEIGHT]) {
		if ((*p & 0x0f) == 0x0f && (*b & 0x0f) != 0x0f)
			*p = *b;
		p++;
		if ((*p & 0x0f) == 0x0f && (*b & 0x0f) != 0x0f)
			*p = *b;
		if ((*p >> 4) == 4 && (*b >> 4) != 4)
			*p = (*p & 0x0f) | (*b & 0xf0);
		p++; b++;
	}
}

#endif /* USE_HIRES */

/* end: hirespic.c */

