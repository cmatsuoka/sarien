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

static void fix_pixel_bothsides (int x, int y)
{
	UINT8 *p, *s;

	/* Sometimes a solid color area in the lo-res pic is made
	 * with lines, and we want to keep this  effect in the
	 * hi-res pic.
	 */
	p = &game.hires[y * (_WIDTH * 2) + x];
	if ((*(p - 2) & 0x0f) == scr_colour)
		put_virt_pixel (x - 1, y, 2);
	if ((*(p + 2) & 0x0f) == scr_colour)
		put_virt_pixel (x + 1, y, 2);

	/* If two lines are contiguous in the lo-res pic, make them
	 * contiguous in the hi-res pic. This condition is needed
	 * in some scenes like in front of Lefty's in LSL1, to draw
	 * the pole. Note: it adds artifacts in some cases.
	 */
	s = &game.sbuf[y * _WIDTH + x / 2];
	if ((*(p - 1) & 0x0f) != (*(s - 1) & 0x0f))
		put_virt_pixel (x - 1, y, 2);
}

/**************************************************************************
** okToFill
**************************************************************************/
static INLINE int hires_fill_here (int x, int y)
{
	UINT8 *p, *s;

	if (x < 0 || x >= _WIDTH || y < 0 || y >= _HEIGHT)
		return FALSE;

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
		put_virt_pixel (2 * x + 1, y, 2);
	else if ((*p & 0x0f) == (*(p - 1) & 0x0f))
		put_virt_pixel (2 * x + 1, y, 2);
}

static void fix_pixel_right (int x, int y)
{
	UINT8 p;

	p = game.hires[y * (_WIDTH * 2) + x * 2];
	if (scr_on && (p & 0x0f) == 0x0f)
		put_virt_pixel (2 * x, y, 2);
}

static void fix_pixel_here (int x, int y)
{
	UINT8 p;

	p = game.hires[y * (_WIDTH * 2) + x * 2 + 1];
	if (scr_on && (p & 0x0f) == 0x0f)
		put_virt_pixel (2 * x + 1, y, 2);
}


/**************************************************************************
** agiFill
**************************************************************************/
static void hires_fill_scanline (int x, int y)
{
	int c;
	int newspan_up, newspan_down;

	if (!hires_fill_here (x, y))
		return;

	/* Scan for left border */
	for (c = x - 1; hires_fill_here (c, y); c--);
	fix_pixel_left (c, y);
	
	newspan_up = newspan_down = 1;
	for (c++; hires_fill_here (c, y); c++) {
		put_virt_pixel (c * 2, y, 2);
		fix_pixel_here (c, y);

		if (hires_fill_here (c, y - 1)) {
			if (newspan_up) {
				_PUSH (c + 320 * (y - 1));
				newspan_up = 0;
			}
		} else {
			newspan_up = 1;
		}

		if (hires_fill_here (c, y + 1)) {
			if (newspan_down) {
				_PUSH (c + 320 * (y + 1));
				newspan_down = 0;
			}
		} else {
			newspan_down = 1;
		}
	}
	fix_pixel_right (c, y);
}

static void hiresFill (int x, int y)
{
	_PUSH (x + 320 * y);

	while (42) {
		int c = _POP();

		/* Exit if stack is empty */
		if (c == 0xffff)
			break;

		x = c % 320;
		y = c / 320;

		hires_fill_scanline (x, y);
	}

	stack_ptr = 0;
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

/* Extra randomness added to brush fill, and double width to single
 * pixels (makes MUMG and others look a lot nicer). 
 */

#define plotHiresPatternPoint() do {					\
	if (patCode & 0x20) {						\
		if ((splatterMap[bitPos>>3] >> (7-(bitPos&7))) & 1) {	\
			if (rnd(4)) put_virt_pixel(x1*2, y1, 2);	\
			if (!rnd(4))put_virt_pixel(x1*2+1, y1, 2);	\
		}							\
		bitPos++;						\
		if (bitPos == 0xff)					\
			bitPos=0;					\
	} else { put_virt_pixel(x1*2,y1,2); put_virt_pixel(x1*2+1,y1,2); }\
} while (0)

/**************************************************************************
** plotPattern
**
** Draws pixels, circles, squares, or splatter brush patterns depending
** on the pattern code.
**************************************************************************/
static void plot_hires_pattern(UINT8 x, UINT8 y)
{
	SINT32 circlePos = 0;
	UINT32 x1, y1, penSize, bitPos = splatterStart[patNum];

	penSize = (patCode & 7);

	if (x < penSize)
		x = penSize-1;
	if (y < penSize)
		y = penSize;

	for (y1 = y - penSize; y1 <= y + penSize; y1++) {
		for (x1 = x - (penSize+1)/2; x1<=x + penSize/2; x1++) {
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
	UINT8 x1, y1;

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
	int offset;

	i = 0;
	offset = game.line_min_print * CHAR_LINES;
	for (y = 0; y < _HEIGHT; y++) {
		put_pixels_hires (0, y + offset, _WIDTH * 2,
			&game.hires[i]);
		i += _WIDTH * 2;
	}

	flush_screen ();
}

void fix_hires_picture ()
{
	UINT8 *p, *b;
	int i;

	p = game.hires;
	b = game.sbuf;

	for (i = 0; p < &game.hires[_WIDTH * _HEIGHT * 2] - 1; p++, i++) {
		if ((*p & 0x0f) == 0x0f && (*b & 0x0f) != 0x0f) {
			if ((*(p + 1) & 0x0f) != 0x0f)
				*p = *(p + 1);
			else
				*p = *b;
		}
		if ((*p >> 4) == 4 && (*b >> 4) != 4 &&
			(*(b + 1) >> 4) != 4)
		{
			*p = (*p & 0x0f) | (*b & 0xf0);
		}
		b += (i & 1);
	}
}

#endif /* USE_HIRES */

/* end: hirespic.c */

