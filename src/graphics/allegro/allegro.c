/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2003 Stuart George and Claudio Matsuoka
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
#include <allegro.h>
#include "sarien.h"
#include "graphics.h"
#include "keyboard.h"

UINT8		*exec_name;
static BITMAP	*screen_buffer;

static int	init_vidmode	(void);
static int	deinit_vidmode	(void);
static void	gfx_put_block	(int, int, int, int);
static void	gfx_put_pixels	(int x, int y, int w, UINT8 *c);
static void	gfx_timer	(void);
static int	gfx_get_key	(void);
static int	gfx_keypress	(void);

#define TICK_SECONDS 20

static struct gfx_driver gfx_allegro = {
	init_vidmode,
	deinit_vidmode,
	gfx_put_block,
	gfx_put_pixels,
	gfx_timer,
	gfx_keypress,
	gfx_get_key
};

extern struct gfx_driver *gfx;

static void gfx_timer (void)
{
	static UINT32 cticks = 0;

	while (cticks == clock_ticks);
	cticks = clock_ticks;
}


static void tick_increment (void)
{
	clock_ticks++;
}
END_OF_FUNCTION (tick_increment);


int init_machine (int argc, char **argv)
{
	gfx = &gfx_allegro;
	return err_OK;
}


int deinit_machine ()
{
	return err_OK;
}


static int init_vidmode ()
{
	int i;
	RGB p;

	install_keyboard();
	install_timer();

	LOCK_VARIABLE (clock_ticks);
	LOCK_FUNCTION (tick_increment);

	install_int_ex (tick_increment, BPS_TO_TIMER (TICK_SECONDS));

	screen_buffer = create_bitmap (GFX_WIDTH, GFX_HEIGHT);
	clear(screen_buffer);

	clock_count = 0;
	clock_ticks = 0;

	set_gfx_mode(GFX_VGA, GFX_WIDTH, GFX_HEIGHT, 0, 0);

	for(i = 0; i < 32; i++) {
		p.r = palette[(i*3)+0];
		p.g = palette[(i*3)+1];
		p.b = palette[(i*3)+2];
		set_color(i, &p);
	}

	return err_OK;
}


static int deinit_vidmode (void)
{
	set_gfx_mode (GFX_TEXT, 0, 0, 0, 0);
	destroy_bitmap (screen_buffer);
	remove_int (tick_increment);

	allegro_exit();

	return err_OK;
}


/* blit a block onto the screen */
static void gfx_put_block (int x1, int y1, int x2, int y2)
{
	int h;
	int w;

	if (x1 >= GFX_WIDTH)
		x1 = GFX_WIDTH - 1;
	if (y1 >= GFX_HEIGHT)
		y1 = GFX_HEIGHT - 1;
	if (x2 >= GFX_WIDTH)
		x2 = GFX_WIDTH - 1;
	if (y2 >= GFX_HEIGHT)
		y2 = GFX_HEIGHT - 1;

	h = y2 - y1 + 1;
	w = x2 - x1 + 1;

	blit (screen_buffer, screen, x1, y1, x1, y1, w, h);
}


static void gfx_put_pixels (int x, int y, int w, UINT8 *p)
{
	UINT8 *s = &screen_buffer->line[y][x];
	while (w--) *s++ = *p++;
}


static int gfx_keypress ()
{
	return keypressed();
}


static int gfx_get_key ()
{
	UINT16 key;

	key = readkey();

	if ((key & 0x00FF) == 0)
		key &= 0xFF00;
	else
		key &= 0x00FF;

	switch (key) {
	case 0x5200:
		key = KEY_LEFT;
		break;
	case 0x5300:
		key = KEY_RIGHT;
		break;
	case 0x5400:
		key = KEY_UP;
		break;
	case 0x5500:
		key = KEY_DOWN;
		break;
	case 0x2f00:	/* F1 */
	case 0x3000:	/* F2 */
	case 0x3100:	/* F3 */
	case 0x3200:	/* F4 */
	case 0x3300:	/* F5 */
	case 0x3400:	/* F6 */
	case 0x3500:	/* F7 */
	case 0x3600:	/* F8 */
	case 0x3700:	/* F9 */
	case 0x3800:	/* F10 */
		key += 0x0c00;
		break;
	case 0x3900:	/* F11 */
		key = KEY_STATUSLN;
		break;
	case 0x3a00:	/* F12 */
		key = KEY_PRIORITY;
		break;
	}

	return key;
}

