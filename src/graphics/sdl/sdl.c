/*
 *  Sarien AGI :: Copyright (C) 1998 Dark Fiber
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

#ifdef _UNIX_DEBUG
#include <ctype.h>
#endif

#include "sarien.h"
#include "gfx.h"
#include "keyboard.h"

extern struct sarien_options opt;

static int scale = 1;
static int key_control = 0;
static int key_alt = 0;
static SDL_Surface *screen;

SDL_Color color[16];

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

static int init_vidmode (void);
static int deinit_vidmode (void);
static void put_block (UINT16, UINT16, UINT16, UINT16);
static void inline _put_pixel (UINT16, UINT16, UINT16);

static void new_timer (void);
static Uint32 timer_function (Uint32);

static volatile UINT32 tick_timer = 0;


#define TICK_SECONDS 20

UINT8 is_keypress(void);
UINT16 get_keypress(void);

static __GFX_DRIVER GFX_sdl =
{
	init_vidmode,
	deinit_vidmode,
	put_block,
	_put_pixel,
	new_timer,
	is_keypress,
	get_keypress
};


static void process_events ()
{
	SDL_Event event;

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

			_D ((": key = 0x%02x ('%c')", key, isprint (key) ? key : '?'));
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
	gfx = &GFX_sdl;
	scale = opt.scale;
	screen_mode = GFX_MODE;
	clock_count = 0;
	clock_ticks = 0;

	return err_OK;
}


int deinit_machine (void)
{
	return err_OK;
}


static int init_vidmode (void)
{
	int i, mode;

	fprintf (stderr, "sdl: SDL support by claudio@helllabs.org\n");

	/* Initialize SDL */
	if (SDL_Init (SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0)
	{
		fprintf (stderr, "sdl: can't initialize: %s\n",
			SDL_GetError ());
		return err_Unk;
	}

	mode = SDL_SWSURFACE | SDL_ANYFORMAT;
	if (opt.fullscreen)
		mode |= SDL_FULLSCREEN;
	if ((screen = SDL_SetVideoMode (320 * scale, 200 * scale, 8,
		mode)) == NULL)
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
	}
	SDL_SetColors (screen, color, 0, 32);

	screen_mode = GFX_MODE;

	return err_OK;
}


static int deinit_vidmode (void)
{
	_D (("()"));
	SDL_Quit ();
	screen_mode = TXT_MODE;

	return err_OK;
}


/* put a block onto the screen */
static void put_block (UINT16 x1, UINT16 y1, UINT16 x2, UINT16 y2)
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
		x2 *= scale;
		y2 *= scale;
	}
	SDL_UpdateRect (screen, x1, y1, x2 - x1 + 1, y2 - y1 + 1);
}


static void inline sdl_put_pixel (UINT16 x, UINT16 y, UINT16 c)
{
	UINT32 pixel;
	UINT8 *bits, bpp;

	pixel = SDL_MapRGB (screen->format, color[c].r, color[c].g, color[c].b);

	if (SDL_MUSTLOCK (screen))
	{
		if (SDL_LockSurface (screen) < 0)
			return;
	}
	bpp = screen->format->BytesPerPixel;
	bits = ((UINT8 *) screen->pixels) + y * screen->pitch + x * bpp;

	/* Set the pixel */
	switch (bpp) {
	case 1:
		*((UINT8 *) (bits)) = (Uint8) pixel;
		break;
	case 2:
		*((UINT16 *) (bits)) = (Uint16) pixel;
		break;
	case 3:{
		UINT8 r, g, b;
		r = (pixel >> screen->format->Rshift) & 0xFF;
		g = (pixel >> screen->format->Gshift) & 0xFF;
		b = (pixel >> screen->format->Bshift) & 0xFF;
		*((bits) + screen->format->Rshift / 8) = r;
		*((bits) + screen->format->Gshift / 8) = g;
		*((bits) + screen->format->Bshift / 8) = b;
		}
		break;
	case 4:
		*((UINT32 *) (bits)) = (Uint32) pixel;
		break;
	}

	if (SDL_MUSTLOCK (screen))
	{
		SDL_UnlockSurface (screen);
	}
}


/* put pixel routine */
static void inline _put_pixel (UINT16 x, UINT16 y, UINT16 c)
{
	register int i, j;

	if (scale == 1)
	{
		sdl_put_pixel (x, y, c);
	}
	else
	{
		for (i = 0; i < scale; i++)
			for (j = 0; j < scale; j++)
				sdl_put_pixel (x * scale + i, y * scale + j, c);
	}
}


UINT8 is_keypress (void)
{
	process_events ();

	return key_queue_start != key_queue_end;
}


UINT16 get_keypress (void)
{
	UINT16 k;

	while (key_queue_start == key_queue_end)	/* block */
		new_timer ();

	key_dequeue(k);

	return k;
}


static Uint32 timer_function (Uint32 i)
{
	tick_timer++;

	return i;
}


static void new_timer ()
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
