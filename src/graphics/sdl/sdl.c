/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

/* SDL support written by Claudio Matsuoka <claudio@helllabs.org>
 */

/* Sat, 31 Jul 1999 10:32:01 +0200 (MET DST)
 * SDL full screen mode added by Robert Bihlmeyer <robbe@orcus.priv.at>
 *
 * Wed Dec 22 15:51:22 EDT 1999
 * Compile errors fixed by Ryan Gordon <ryan_gordon@hotmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <SDL/SDL.h>
#include "sarien.h"
#include "graphics.h"
#include "keyboard.h"

extern struct sarien_options opt;

static int scale = 1;
static int key_control = 0;
static int key_alt = 0;
static SDL_Surface *screen;

SDL_Color color[32];
Uint32 mapped_color[32];

UINT32 clock_ticks;
UINT32 clock_count;

#define KEY_QUEUE_SIZE 16

static int key_queue[KEY_QUEUE_SIZE];
static int key_queue_start = 0;
static int key_queue_end = 0;

#define key_enqueue(k) do { key_queue[key_queue_end++] = (k); \
	key_queue_end %= KEY_QUEUE_SIZE; } while (0)
#define key_dequeue(k) do { (k) = key_queue[key_queue_start++]; \
	key_queue_start %= KEY_QUEUE_SIZE; } while (0)

static int	init_vidmode	(void);
static int	deinit_vidmode	(void);
static void	sdl_put_block	(int, int, int, int);
static void	_putpixels	(int, int, int, Uint8 *);
static void	sdl_timer	(void);
static Uint32	timer_function	(Uint32);
int		sdl_is_keypress	(void);
int		sdl_get_keypress(void);

static volatile UINT32 tick_timer = 0;


#define TICK_SECONDS 20


static struct gfx_driver gfx_sdl = {
	init_vidmode,
	deinit_vidmode,
	sdl_put_block,
	_putpixels,
	sdl_timer,
	sdl_is_keypress,
	sdl_get_keypress
};

extern struct gfx_driver *gfx;


#define ASPECT_RATIO(x) ((x) * 6 / 5)

/* ====================================================================*/

/* Some optimized put_pixel routines for the most common cases */

#define _putpixels_scale1(d) static void INLINE				\
_putpixels_##d##bits_scale1 (int x, int y, int w, UINT8 *p) {		\
	Uint##d## *s;							\
	if (w == 0) return;						\
	s = (Uint##d## *)screen->pixels + x + y * screen->w;		\
	if (SDL_MUSTLOCK (screen)) {					\
		if (SDL_LockSurface (screen) < 0)			\
			return;						\
	}								\
	while (w--) { *s++ = mapped_color[*p++]; }			\
	if (SDL_MUSTLOCK (screen)) SDL_UnlockSurface (screen);		\
}

#define _putpixels_scale2(d) static void INLINE				\
_putpixels_##d##bits_scale2 (int x, int y, int w, UINT8 *p) {		\
	Uint##d## *s, *t;						\
	if (w == 0) return;						\
	x <<= 1; y <<= 1;						\
	s = (Uint##d## *)screen->pixels + x + y * screen->w;		\
	t = s + screen->w;						\
	if (SDL_MUSTLOCK (screen)) {					\
		if (SDL_LockSurface (screen) < 0)			\
			return;						\
	}								\
	while (w--) {							\
		int c = mapped_color[*p];				\
		*s++ = c; *s++ = c; *t++ = c; *t++ = c; p++;		\
	}								\
	if (SDL_MUSTLOCK (screen)) SDL_UnlockSurface (screen);		\
}

_putpixels_scale1(8);
_putpixels_scale1(16);
_putpixels_scale1(32);
_putpixels_scale2(8);
_putpixels_scale2(16);
_putpixels_scale2(32);


/* ====================================================================*/

/* Aspect ratio correcting put pixels handlers */

#define _putpixels_fixratio_scale1(d) static void			\
_putpixels_fixratio_##d##bits_scale1 (int x, int y, int w, UINT8 *p) {	\
	if (y > 0 && ASPECT_RATIO (y) - 1 != ASPECT_RATIO (y - 1))	\
		_putpixels_##d##bits_scale1 (x, ASPECT_RATIO(y) - 1, w, p);\
	_putpixels_##d##bits_scale1 (x, ASPECT_RATIO(y), w, p);		\
}

#define _putpixels_fixratio_scale2(d) static void INLINE		\
_putpixels_fixratio_##d##bits_scale2 (int x, int y, int w, Uint8 *p0) {	\
	Uint##d## *s, *t, *u; Uint8 *p; int extra = 0;			\
	if (w == 0) return;						\
	x <<= 1; y <<= 1;						\
	if (y < ((GFX_WIDTH - 1) << 2) && ASPECT_RATIO (y) + 2 != ASPECT_RATIO (y + 2)) extra = w; \
	y = ASPECT_RATIO(y);						\
	s = (Uint##d## *)screen->pixels + x + y * screen->w;		\
	t = s + screen->w;						\
	u = t + screen->w;						\
	if (SDL_MUSTLOCK (screen)) {					\
		if (SDL_LockSurface (screen) < 0)			\
			return;						\
	}								\
	for (p = p0; w--; p++) {					\
		int c = mapped_color[*p];				\
		*s++ = c; *s++ = c; *t++ = c; *t++ = c;			\
	}								\
	for (p = p0; extra--; p++) {					\
		int c = mapped_color[*p];				\
		*u++ = c; *u++ = c;					\
	}								\
	if (SDL_MUSTLOCK (screen)) SDL_UnlockSurface (screen);		\
}

_putpixels_fixratio_scale1(8);
_putpixels_fixratio_scale1(16);
_putpixels_fixratio_scale1(32);
_putpixels_fixratio_scale2(8);
_putpixels_fixratio_scale2(16);
_putpixels_fixratio_scale2(32);

/* ====================================================================*/

/* Slow, non-optimized put pixel routine */

static void inline _put_pixel (int x, int y, int c)
{
	Uint32 pixel;
	Uint8 *bits, bpp;

	pixel = mapped_color[c];

	bpp = screen->format->BytesPerPixel;
	bits = ((UINT8 *) screen->pixels) + y * screen->pitch + x * bpp;

	/* Set the pixel */
	switch (bpp) {
	case 1:
		*(Uint8 *)(bits) = pixel;
		break;
	case 2:
		*((UINT16 *) (bits)) = (Uint16) pixel;
		break;
	case 3:{
		UINT8 r, g, b;
		r = (pixel >> screen->format->Rshift) & 0xff;
		g = (pixel >> screen->format->Gshift) & 0xff;
		b = (pixel >> screen->format->Bshift) & 0xff;
		*((bits) + screen->format->Rshift / 8) = r;
		*((bits) + screen->format->Gshift / 8) = g;
		*((bits) + screen->format->Bshift / 8) = b;
		}
		break;
	case 4:
		*((UINT32 *) (bits)) = (Uint32) pixel;
		break;
	}
}

static void _putpixels (int x, int y, int w, Uint8 *p)
{
	register int c;
	register int i, j;

	if (w == 0) return;

	if (SDL_MUSTLOCK (screen)) {
		if (SDL_LockSurface (screen) < 0)
			return;
	}

	switch (scale) {
	case 1:
		while (w--) {
			_put_pixel (x++, y, *p++);
		}
		break;
	case 2:
		x <<= 1;
		y <<= 1;
		while (w--) {
			c = *p++;
			_put_pixel (x, y, c);
			_put_pixel (x++, y + 1, c);
			_put_pixel (x, y, c);
			_put_pixel (x++, y + 1, c);
		}
		break;
	default:
		x *= scale;
		y *= scale;
		while (w--) {
			c = *p++;
			for (i = 0; i < scale; i++) {
				for (j = 0; j < scale; j++)
					_put_pixel (x + i, y + j, c);
			}
			x += scale;
		}
		break;
	}

	if (SDL_MUSTLOCK (screen))
		SDL_UnlockSurface (screen);
}

static void _putpixels_fixratio (int x, int y, int w, UINT8 *p)
{
	if (y > 0 && ASPECT_RATIO (y) - 1 != ASPECT_RATIO (y - 1))
		_putpixels (x, ASPECT_RATIO(y) - 1, w, p);
	_putpixels (x, ASPECT_RATIO(y), w, p);
} 

/* ====================================================================*/

static void process_events ()
{
	SDL_Event event;
	int key;

	while (SDL_PollEvent (&event) > 0) {
		switch (event.key.type) {
		case SDL_KEYDOWN:
			switch (key = event.key.keysym.sym) {
			case SDLK_LSHIFT:
			case SDLK_RSHIFT:
				key = 0;
				break;
			case SDLK_LCTRL:
				key_control |= 1;
				key = 0;
				break;
			case SDLK_RCTRL:
				key_control |= 2;
				key = 0;
				break;
			case SDLK_LALT:
				key_alt |= 1;
				key = 0;
				break;
			case SDLK_RALT:
				key_alt |= 2;
				key = 0;
				break;
			case SDLK_UP:
			case SDLK_KP8:
				key = KEY_UP;
				break;
			case SDLK_LEFT:
			case SDLK_KP4:
				key = KEY_LEFT;
				break;
			case SDLK_DOWN:
			case SDLK_KP2:
				key = KEY_DOWN;
				break;
			case SDLK_RIGHT:
			case SDLK_KP6:
				key = KEY_RIGHT;
				break;
			case SDLK_KP7:
				key = KEY_UP_LEFT;
				break;
			case SDLK_KP9:
				key = KEY_UP_RIGHT;
				break;
			case SDLK_KP3:
				key = KEY_DOWN_RIGHT;
				break;
			case SDLK_KP1:
				key = KEY_DOWN_LEFT;
				break;
			case SDLK_KP_ENTER:
				key = KEY_ENTER;
				break;
			case SDLK_KP_PLUS:
				key = '+';
				break;
			case SDLK_KP_MINUS:
				key = '-';
				break;
			case SDLK_F1:
				key = 0x3b00;
				break;
			case SDLK_F2:
				key = 0x3c00;
				break;
			case SDLK_F3:
				key = 0x3d00;
				break;
			case SDLK_F4:
				key = 0x3e00;
				break;
			case SDLK_F5:
				key = 0x3f00;
				break;
			case SDLK_F6:
				key = 0x4000;
				break;
			case SDLK_F7:
				key = 0x4100;
				break;
			case SDLK_F8:
				key = 0x4200;
				break;
			case SDLK_F9:
				key = 0x4300;
				break;
			case SDLK_F10:
				key = 0x4400;
				break;
			case SDLK_ESCAPE:
				key = 0x1b;
				break;
			case SDLK_PRINT:
				key = KEY_PRIORITY;
				break;
			default:
				if (!isalpha (key))
					break;
				if (key_control)
					key = (key & ~0x20) - 0x40;
				else if (key_alt)
					key = scancode_table[(key & ~0x20)
						- 0x41] << 8;
				break;
			};
			if (key)
				key_enqueue (key);

			break;
		case SDL_KEYUP:
			switch (event.key.keysym.sym) {
			case SDLK_LCTRL:
				key_control &= ~1;
				break;
			case SDLK_RCTRL:
				key_control &= ~2;
				break;
			case SDLK_LALT:
				key_alt &= ~1;
				break;
			case SDLK_RALT:
				key_alt &= ~2;
				break;
			default:
				break;
			}
			break;
		}
	}
}


int init_machine (int argc, char **argv)
{
	gfx = &gfx_sdl;
	scale = opt.scale;
	clock_count = 0;
	clock_ticks = 0;

	return err_OK;
}


int deinit_machine ()
{
	return err_OK;
}


static int init_vidmode ()
{
	int i, mode;

	fprintf (stderr, "sdl: SDL support by claudio@helllabs.org\n");

	/* Initialize SDL */
	if (SDL_Init (SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0) {
		fprintf (stderr, "sdl: can't initialize: %s\n",
			SDL_GetError ());
		return err_Unk;
	}

	mode = SDL_SWSURFACE | SDL_ANYFORMAT;

	if (opt.fullscreen)
		mode |= SDL_FULLSCREEN;

	if ((screen = SDL_SetVideoMode (320 * scale,
		(opt.fixratio ? ASPECT_RATIO(GFX_HEIGHT) : GFX_HEIGHT) * scale,
		8, mode)) == NULL)
	{
		fprintf (stderr, "sdl: can't set video mode: %s\n",
			SDL_GetError ());
		return err_Unk;
	}
	atexit (SDL_Quit);

	SDL_WM_SetCaption (TITLE " " VERSION, TITLE);

	tick_timer = 0;
	SDL_SetTimer (10, timer_function);

	for (i = 0; i < 32; i++) {
		color[i].r = palette[i * 3] << 2;
		color[i].g = palette[i * 3 + 1] << 2;
		color[i].b = palette[i * 3 + 2] << 2;
		mapped_color[i] = SDL_MapRGB (screen->format,
			color[i].r, color[i].g, color[i].b);
	}
	SDL_SetColors (screen, color, 0, 32);

#define handle_case(d,s) case (d/8): \
	gfx_sdl.put_pixels = _putpixels_##d##bits_scale##s##; break;
#define handle_fixratio_case(d,s) case (d/8): \
	gfx_sdl.put_pixels = _putpixels_fixratio_##d##bits_scale##s##; break;

	/* Use an optimized put_pixels if available */
	if (!opt.fixratio) {
		gfx_sdl.put_pixels = _putpixels_fixratio;
		if (opt.gfxhacks) switch (scale) {
		case 1:
			switch (screen->format->BytesPerPixel) {
			handle_case(8,1);
			handle_case(16,1);
			handle_case(32,1);
			}
			break;
		case 2:
			switch (screen->format->BytesPerPixel) {
			handle_case(8,2);
			handle_case(16,2);
			handle_case(32,2);
			}
			break;
		}
	} else {
		if (opt.gfxhacks) switch (scale) {
		case 1:
			switch (screen->format->BytesPerPixel) {
			handle_fixratio_case(8,1);
			handle_fixratio_case(16,1);
			handle_fixratio_case(32,1);
			}
			break;
		case 2:
			switch (screen->format->BytesPerPixel) {
			handle_fixratio_case(8,2);
			handle_fixratio_case(16,2);
			handle_fixratio_case(32,2);
			}
			break;
		}
	}

	return err_OK;
}


static int deinit_vidmode ()
{
	_D ("()");
	SDL_Quit ();
	return err_OK;
}


/* put a block onto the screen */
static void sdl_put_block (int x1, int y1, int x2, int y2)
{
	if (x1 >= GFX_WIDTH)
		x1 = GFX_WIDTH - 1;
	if (y1 >= GFX_HEIGHT)
		y1 = GFX_HEIGHT - 1;
	if (x2 >= GFX_WIDTH)
		x2 = GFX_WIDTH - 1;
	if (y2 >= GFX_HEIGHT)
		y2 = GFX_HEIGHT - 1;

	if (scale > 1) {
		x1 *= scale;
		y1 *= scale;
		x2 = (x2 + 1) * scale - 1;
		y2 = (y2 + 1) * scale - 1;
	}

	if (opt.fixratio) {
		SDL_UpdateRect (screen, x1, ASPECT_RATIO(y1), x2 - x1 + 1,
			ASPECT_RATIO (y2 + 1) - ASPECT_RATIO(y1));
	} else {
		SDL_UpdateRect (screen, x1, y1, x2 - x1 + 1, y2 - y1 + 1);
	}
}


int sdl_is_keypress ()
{
	process_events ();
	return key_queue_start != key_queue_end;
}


int sdl_get_keypress ()
{
	int k;

	while (key_queue_start == key_queue_end)	/* block */
		sdl_timer ();
	key_dequeue(k);

	return k;
}


static Uint32 timer_function (Uint32 i)
{
	tick_timer++;
	return i;
}


static void sdl_timer ()
{
	static UINT32 m = 0;
	UINT32 dm;

	if (tick_timer < m)
		m = 0;

	while ((dm = tick_timer - m) < 5) {
		SDL_Delay (5);
	}
	m = tick_timer;

	process_events ();
}

