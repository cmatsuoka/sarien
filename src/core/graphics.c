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
#include <string.h>
#include "sarien.h"
#include "graphics.h"

#if defined PALMOS || defined FAKE_PALMOS
#  define DEV_X0(x) (x)
#  define DEV_X1(x) (x)
#  define DEV_Y(x) ((x) * PIC_HEIGHT / 168)
#else
#  define DEV_X0(x) ((x) << 1)
#  define DEV_X1(x) (((x) << 1) + 1)
#  define DEV_Y(x) (x)
#endif

#ifndef MAX_INT
#  define MAX_INT (int)((unsigned)~0 >> 1)
#endif


extern UINT8 cur_font[];

#ifdef __TURBOC__

static far UINT8 sarien_screen[64000];
#ifdef USE_CONSOLE
static far UINT8 console_screen[64000];
#endif

#else

static UINT8 sarien_screen[GFX_WIDTH * GFX_HEIGHT];
#ifdef USE_CONSOLE
static UINT8 console_screen[GFX_WIDTH * GFX_HEIGHT];
#endif

#endif

#ifdef USE_EGA_PALETTE
/**
 * 16 color RGB palette (plus 16 transparent colors).
 * This array contains the 6-bit RGB values of the EGA palette exported
 * to the console drivers.
 */
UINT8 palette [32 * 3]= {
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

#else

/**
 * 16 color amiga-ish palette.
 */
UINT8 palette [32 * 3]= {
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x3f,
	0x00, 0x2A, 0x00,
	0x00, 0x2A, 0x2A,
	0x33, 0x00, 0x00,
	0x2f, 0x1c, 0x37,
	0x23, 0x14, 0x00,
	0x2f, 0x2f, 0x2f,
	0x15, 0x15, 0x15,
	0x00, 0x2f, 0x3f,
	0x00, 0x33, 0x15,
	0x15, 0x3F, 0x3F,
	0x3f, 0x27, 0x23,
	0x3f, 0x15, 0x3f,
	0x3b, 0x3b, 0x00,
	0x3F, 0x3F, 0x3F
};

#endif



struct update_block {
	int x1, y1;
	int x2, y2;
};


static struct update_block update = {
	MAX_INT, MAX_INT, 0, 0
};

struct gfx_driver *gfx;			/* graphics driver */
 



/*
 *  Layer 4:  640x480?  ==================  User display
 *                              ^
 *                              |  do_update(), put_block()
 *                              |
 *  Layer 3:  640x480?  ==================  Framebuffer
 *                              ^
 *                              |  flush_block(), put_pixels()
 *                              |
 *  Layer 2:  320x200   ==================  Sarien screen (console), put_pixel()
 *                              |
 *  Layer 1:  160x168   ==================  AGI screen
 */

#ifdef USE_CONSOLE

/**
 * Draws a row of pixels in the output device framebuffer.
 * This function adds the console layer using transparent colors if
 * appropriate.
 */
static void put_pixels (const int x, const int y, const int w, UINT8 *p)
{
	int i;
	UINT8 _b[GFX_WIDTH] = { 0 };
	UINT8 *b, *c = NULL;

	if (console.y <= y) {
		gfx->put_pixels (x, y, w, p);
		return;
	}

	b = &_b[0];
	c = &console_screen[x + (y + GFX_HEIGHT - 1 - console.y) * GFX_WIDTH];

	for (i = 0; i < w; i++, c++, p++) {
		*b++ = *c ? *c : *p + 16;
	}

	gfx->put_pixels (x, y, w, _b);
}

static void init_console ()
{
	int i;

	/* "Transparent" colors */
	for (i = 0; i < 48; i++)
		palette[i + 48] = (palette[i] + 0x30) >> 2;

	/* Console */
	for (i = 0; i < CONSOLE_LINES_BUFFER; i++)
		console.line[i] = strdup ("\n");
}

#else

static void put_pixels (const int x, const int y, const int w, UINT8 *p)
{
	gfx->put_pixels (x, y, w, p);
}

static void init_console ()
{
}

#endif /* USE_CONSOLE */




/* Based on LAGII 0.1.5 by XoXus */
void shake_screen (int n)
{
#if !defined (__TURBOC__) && !defined (__DICE__)
#define MAG 3
	int i;
	UINT8 b[GFX_WIDTH * GFX_HEIGHT], c[GFX_WIDTH * GFX_HEIGHT];
	
	memset (c, 0, GFX_WIDTH * GFX_HEIGHT);
	memcpy (b, sarien_screen, GFX_WIDTH * GFX_HEIGHT);
	for (i = 0; i < (GFX_HEIGHT - MAG); i++)
		memcpy (&c[GFX_WIDTH * (i + MAG) + MAG],
			&b[GFX_WIDTH * i], GFX_WIDTH - MAG);

	for (i = 0; i < (2 * n); i++) {
		main_cycle ();
		memcpy (sarien_screen, c, GFX_WIDTH * GFX_HEIGHT);
		flush_block (0, 0, GFX_WIDTH - 1, GFX_HEIGHT - 1);
		main_cycle ();
		memcpy (sarien_screen, b, GFX_WIDTH * GFX_HEIGHT);
		flush_block (0, 0, GFX_WIDTH - 1, GFX_HEIGHT - 1);
	}
#undef MAG
#endif
}


void put_text_character (int l, int x, int y, unsigned int c, int fg, int bg)
{
	int x1, y1, xx, yy, cc;
	UINT8 *p;

#if defined PALMOS || defined FAKE_PALMOS
	if (c & 0x80)
		c = 1;
#endif

	p = cur_font + ((unsigned int)c * CHAR_LINES);
	for (y1 = 0; y1 < CHAR_LINES; y1++) {
		for (x1 = 0; x1 < CHAR_COLS; x1 ++) {
			xx = x + x1;
			yy = y + y1;
			cc = (*p & (1 << (7 - x1))) ? fg : bg;
#ifdef USE_CONSOLE
			if (l) {
				console_screen[xx + yy * GFX_WIDTH] = cc;
			} else
#endif
			{
				sarien_screen [xx + yy * GFX_WIDTH] = cc;
			}
		}

		p++;
	}
	/* FIXME: we don't want this when we're writing on the
	 *        console!
	 */
	flush_block (x, y, x + CHAR_COLS - 1, y + CHAR_LINES - 1);
}


void draw_rectangle (int x1, int y1, int x2, int y2, int c)
{
	int y, w, h;
	UINT8 *p0;

	if (x1 >= GFX_WIDTH)  x1 = GFX_WIDTH - 1;
	if (y1 >= GFX_HEIGHT) y1 = GFX_HEIGHT - 1;
	if (x2 >= GFX_WIDTH)  x2 = GFX_WIDTH - 1;
	if (y2 >= GFX_HEIGHT) y2 = GFX_HEIGHT - 1;

	w = x2 - x1 + 1;
	h = y2 - y1 + 1;
	p0 = &sarien_screen[x1 + y1 * GFX_WIDTH];
	for (y = 0; y < h; y++) {
		memset (p0, c, w);
		p0 += GFX_WIDTH;
	}
}


static void draw_frame (int x1, int y1, int x2, int y2, int c1, int c2)
{
	int y, w;
	UINT8 *p0;

	/* top line */
	w = x2 - x1 + 1;
	p0 = &sarien_screen[x1 + y1 * GFX_WIDTH];
	memset (p0, c1, w);

	/* bottom line */
	p0 = &sarien_screen[x1 + y2 * GFX_WIDTH];
	memset (p0, c2, w);

	/* side lines */
	for (y = y1; y <= y2; y++) {
		sarien_screen[x1 + y * GFX_WIDTH] = c1;
		sarien_screen[x2 + y * GFX_WIDTH] = c2;
	}
}


void draw_box (int x1, int y1, int x2, int y2, int colour1, int colour2, int m)
{
	x1 += m;
	y1 += m;
	x2 -= m;
	y2 -= m;

#ifdef FANCY_BOX
	draw_rectangle (x1, y1, x2, y2, 7);
	draw_frame (x1, y1, x2, y2, 15, 8);
	flush_block (x1, y1, x2, y2);
#else
	draw_rectangle (x1, y1, x2, y2, colour1);
	draw_frame (x1 + 2, y1 + 2, x2 - 2, y2 - 2, colour2, colour2);
	flush_block (x1, y1, x2, y2);
#endif
}


void print_character (int x, int y, char c, int fg, int bg)
{
	x *= CHAR_COLS;
	y *= CHAR_LINES;

	put_text_character (0, x, y, c, fg, bg);
	/* CM: the extra pixel in y is for the underline cursor */
	flush_block (x, y, x + CHAR_COLS - 1, y + CHAR_LINES); 
}


void put_block (int x1, int y1, int x2, int y2)
{
	gfx->put_block (x1, y1, x2, y2);
}

void put_screen ()
{
	put_block (0, 0, GFX_WIDTH - 1, GFX_HEIGHT - 1);
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


/*
 * Public functions
 */

/**
 * Initialize graphics device.
 *
 * @see deinit_video()
 */
int init_video ()
{
#ifndef _WIN32
	/* XXX: this causes the win32 debug build to crash for some reason */
	fprintf (stderr, "Initializing graphics: %dx%d (scale = %d)\n",
		GFX_WIDTH, GFX_HEIGHT, opt.scale);
#endif

	init_console ();
	return gfx->init_video_mode ();
}

/**
 * Deinitialize graphics device.
 *
 * @see init_video()
 */
int deinit_video ()
{
	return gfx->deinit_video_mode ();
}

/**
 * Write pixels on the output device.
 * This function writes a row of pixels on the output device. Only the
 * lower 4 bits of each pixel in the row will be used, making this
 * function suitable for use with rows from the AGI screen.
 * @param x x coordinate of the row start (AGI coord.)
 * @param y y coordinate of the row start (AGI coord.)
 * @param n number of pixels in the row
 * @param p pointer to the row start in the AGI screen
 */
void put_pixels_a (int x, int y, int n, UINT8 *p)
{
	/* y += CHAR_LINES; */
#ifdef FAKE_PALMOS
	for (; n--; p++, x ++) {
		*(UINT8 *)&sarien_screen[x + DEV_Y(y) * GFX_WIDTH] = *p & 0x0f;
	}
#else
	for (x *= 2; n--; p++, x += 2) {
		register UINT16 q = ((UINT16)*p << 8) | *p;
#ifdef USE_CONSOLE
		if (debug.priority) q >>= 4;
#endif
		*(UINT16 *)&sarien_screen[x + y * GFX_WIDTH] = q & 0x0f0f;
	}
#endif
}

#ifdef USE_HIRES

void put_pixels_hires (int x, int y, int n, UINT8 *p)
{
	/* y += CHAR_LINES; */
	for (; n--; p++, x++) {
		UINT8 q = *p;
		if (debug.priority) q >>= 4;
		sarien_screen[x + y * GFX_WIDTH] = q & 0x0f;
	}
}

#endif


/**
 * Schedule blocks for blitting on the output device.
 * This function gets the coordinates of a block in the AGI screen and
 * schedule it to be updated in the output device.
 * @param x1 x coordinate of the upper left corner of the block (AGI coord.)
 * @param y1 y coordinate of the upper left corner of the block (AGI coord.)
 * @param x2 x coordinate of the lower right corner of the block (AGI coord.)
 * @param y2 y coordinate of the lower right corner of the block (AGI coord.)
 *
 * @see do_update()
 */
void schedule_update (int x1, int y1, int x2, int y2)
{
	if (x1 < update.x1) update.x1 = x1;
	if (y1 < update.y1) update.y1 = y1;
	if (x2 > update.x2) update.x2 = x2;
	if (y2 > update.y2) update.y2 = y2;
}

/**
 * Update scheduled blocks on the output device.
 * This function exposes the blocks scheduled for updating to the output
 * device. Blocks can be scheduled at any point of the AGI cycle.
 *
 * @see schedule_update()
 */
void do_update ()
{
	if (update.x1 <= update.x2 && update.y1 <= update.y2) {
		gfx->put_block (update.x1, update.y1, update.x2, update.y2);
	}
	
	/* reset update block variables */
	update.x1 = MAX_INT;
	update.y1 = MAX_INT;
	update.x2 = 0;
	update.y2 = 0;
}


/**
 * Updates a block of the framebuffer with contents of the Sarien screen.
 * This function updates a block in the output device with the contents of
 * the Sarien screen, handling console transparency.
 * @param x1 x coordinate of the upper left corner of the block
 * @param y1 y coordinate of the upper left corner of the block
 * @param x2 x coordinate of the lower right corner of the block
 * @param y2 y coordinate of the lower right corner of the block
 *
 * @see flush_block_a()
 */
void flush_block (int x1, int y1, int x2, int y2)
{
	int y, w;
	UINT8 *p0;

	schedule_update (x1, y1, x2, y2);

	p0 = &sarien_screen[x1 + y1 * GFX_WIDTH];
	w = x2 - x1 + 1;

	for (y = y1; y <= y2; y++) {
		put_pixels (x1, y, w, p0);
		p0 += GFX_WIDTH;
	}
}

/**
 * Updates a block of the framebuffer receiving AGI picture coordinates.
 * @param x1 x AGI picture coordinate of the upper left corner of the block
 * @param y1 y AGI picture coordinate of the upper left corner of the block
 * @param x2 x AGI picture coordinate of the lower right corner of the block
 * @param y2 y AGI picture coordinate of the lower right corner of the block
 *
 * @see flush_block()
 */
void flush_block_a (int x1, int y1, int x2, int y2)
{
	/* y1 += 8;
	   y2 += 8; */
	flush_block (DEV_X0(x1), DEV_Y(y1), DEV_X1(x2), DEV_Y(y2));
}

/**
 * Updates the framebuffer with contents of the Sarien screen (console-aware).
 * This function updates the output device with the contents of the AGI
 * screen, handling console transparency.
 */
void flush_screen ()
{
	flush_block (0, 0, GFX_WIDTH - 1, GFX_HEIGHT - 1);
}

/**
 * Clear the output device screen (console-aware).
 * This function clears the output device screen and updates the
 * output device. Contents of the AGI screen are left untouched. This
 * function can be used to simulate a switch to a text mode screen in
 * a graphic-only device.
 * @param c  color to clear the screen
 */
void clear_screen (int c)
{
	memset (sarien_screen, c, GFX_WIDTH * GFX_HEIGHT);
	flush_screen ();
}

#ifdef USE_CONSOLE
/**
 * Clear the console screen.
 * This function clears the top n lines of the console screen.
 * @param n number of lines to clear (in pixels)
 */
void clear_console_screen (int n)
{
	memset (console_screen + n * GFX_WIDTH, 0, (200 - n) * GFX_WIDTH);
}
#endif

/**
 * Save a block of the Sarien screen
 */
void save_block (int x1, int y1, int x2, int y2, UINT8 *b)
{
	UINT8 *p0;
	int w, h;

	p0 = &sarien_screen[x1 + GFX_WIDTH * y1];
	w = x2 - x1 + 1;
	h = y2 - y1 + 1;
	while (h--) {
		memcpy (b, p0, w);
		b += w;
		p0 += GFX_WIDTH;
	}
}

/**
 * Restore a block of the Sarien screen
 */
void restore_block (int x1, int y1, int x2, int y2, UINT8 *b)
{
	UINT8 *p0;
	int w, h;

	p0 = &sarien_screen[x1 + GFX_WIDTH * y1];
	w = x2 - x1 + 1;
	h = y2 - y1 + 1;
	while (h--) {
		memcpy (p0, b, w);
		b += w;
		p0 += GFX_WIDTH;
	}
	flush_block (x1, y1, x2, y2);
}


UINT8 *get_sarien_screen ()
{
	return &sarien_screen[0];
}

/* end: graphics.c */
