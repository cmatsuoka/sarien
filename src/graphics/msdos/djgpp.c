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
#include <dos.h>
#include <conio.h>

#include <allegro.h>

#include "sarien.h"
#include "gfx.h"

BITMAP *screen_buffer;

UINT16	IBM_init_vidmode(void);
UINT16	IBM_deinit_vidmode(void);
void	IBM_blit_block(UINT16 x1, UINT16 y1, UINT16 x2, UINT16 y2);
void	IBM_put_pixel(UINT16 x, UINT16 y, UINT16 c);
void	IBM_dummy(void);
UINT16	IBM_get_key(void);
UINT8	IBM_keypress(void);


#define TICK_SECONDS 20

__GFX_DRIVER	GFX_ibm=
	{
		IBM_init_vidmode,
		IBM_deinit_vidmode,
		IBM_blit_block,
		IBM_put_pixel,
		IBM_dummy,
		IBM_keypress,
		IBM_get_key
	};

void IBM_dummy(void)
{
	static UINT32	cticks=(SINT32)-1;

	while(cticks==clock_ticks);
	cticks=clock_ticks;
}

void new_timer(void)
{
	clock_ticks++;
}
END_OF_FUNCTION(new_timer);



int init_machine(int argc, char **argv)
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

int deinit_machine(void)
{
	destroy_bitmap(screen_buffer);
	remove_int(IBM_dummy);

	allegro_exit();

	return err_OK;
}

UINT16 IBM_init_vidmode(void)
{
	int i;
	RGB p;

	set_gfx_mode(GFX_VGA, 320, 200, 0, 0);

	for(i=0; i<16; i++)
	{
		p.r=palette[(i*3)+0];
		p.g=palette[(i*3)+1];
		p.b=palette[(i*3)+2];
		set_color(i, &p);
	}

	screen_mode=GFX_MODE;

	return err_OK;
}

UINT16 IBM_deinit_vidmode(void)
{
	set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
	screen_mode=TXT_MODE;

	return err_OK;
}


/* blit a block onto the screen */
void IBM_blit_block(UINT16 x1, UINT16 y1, UINT16 x2, UINT16 y2)
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

void IBM_put_pixel(UINT16 x, UINT16 y, UINT16 c)
{
	//screen_buffer[y * 320 + x] = (c & 0xFF);
	screen_buffer->line[y][x]=(c&0xFF);
}


UINT8 IBM_keypress(void)
{
	return !!keypressed();
}


UINT16 IBM_get_key(void)
{
	UINT16 key;

	key=readkey();

	if((key&0x00FF)==0)
		key=key&0xFF00;
	else
		key=key&0x00FF;

	return key;
}
