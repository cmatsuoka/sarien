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
#include <dos.h>
#include <conio.h>

#include <allegro.h>

#include "sarien.h"
#include "gfx_base.h"

static BITMAP *screen_buffer;

static int	init_vidmode	(void);
static int	deinit_vidmode	(void);
static void	blit_block	(int, int, int, int);
static void	put_pixel	(int, int, int);
static void	dummy		(void);
static int	get_key		(void);
static int	keypress	(void);


#define TICK_SECONDS 20

static struct gfx_driver GFX_ibm= {
	init_vidmode,
	deinit_vidmode,
	blit_block,
	put_pixel,
	dummy,
	keypress,
	get_key
};


static void dummy(void)
{
	static UINT32 cticks=(SINT32)-1;

	while (cticks == clock_ticks);
	cticks=clock_ticks;
}


static void new_timer(void)
{
	clock_ticks++;
}
END_OF_FUNCTION(new_timer);


int init_machine (int argc, char **argv)
{
	gfx=&GFX_ibm;

	install_keyboard();
	install_timer();

	LOCK_VARIABLE(clock_ticks);
	LOCK_FUNCTION(new_timer);

	install_int_ex(new_timer, BPS_TO_TIMER(TICK_SECONDS));

	screen_mode=GFX_MODE;
	screen_buffer=create_bitmap(320, 200);
	clear_buffer();

	clock_count=0;
	clock_ticks=0;

	return err_OK;
}


int deinit_machine ()
{
	destroy_bitmap(screen_buffer);
	remove_int(dummy);

	allegro_exit();

	return err_OK;
}


static int init_vidmode ()
{
	int i;
	RGB p;

	set_gfx_mode(GFX_VGA, 320, 200, 0, 0);

	for(i=0; i<16; i++) {
		p.r=palette[(i*3)+0];
		p.g=palette[(i*3)+1];
		p.b=palette[(i*3)+2];
		set_color(i, &p);
	}

	screen_mode=GFX_MODE;

	return err_OK;
}


static int deinit_vidmode (void)
{
	set_gfx_mode (GFX_TEXT, 0, 0, 0, 0);
	screen_mode = TXT_MODE;

	return err_OK;
}


/* blit a block onto the screen */
static void blit_block (int x1, int y1, int x2, int y2)
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

	blit(screen_buffer, screen, x1, y1, x1, y1, w, h);
	//blit(screen_buffer, screen, 0, 0, 0, 0, 320, 200);
}


static void put_pixel (int x, int y, int c)
{
	//screen_buffer[y * 320 + x] = (c & 0xFF);
	screen_buffer->line[y][x]=(c&0xFF);
}


static int keypress ()
{
	return !!keypressed();
}


static int get_key ()
{
	UINT16 key;

	key=readkey();

	if ((key & 0x00FF) == 0)
		key &= 0xFF00;
	else
		key &= 0x00FF;

	return key;
}

