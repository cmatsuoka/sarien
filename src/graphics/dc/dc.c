/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  Dreamcast files Copyright (C) 2002 Brian Peek/Ganksoft Entertainment
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#include <kos.h>
#include <mp3/sndstream.h>
#include <kos/thread.h>

#include "sarien.h"
#include "graphics.h"
#include "keyboard.h"
#include "console.h"
#include "agi.h"
#include "image.h"
#include "gsi.h"

#define TICK_SECONDS		18
#define TICK_IN_MSEC		(1000 / (TICK_SECONDS))
#define YOFFS				(24.0f)

static int	init_vidmode	(void);
static int	deinit_vidmode	(void);
static void	dc_put_block	(int, int, int, int);
static void dc_put_pixels	(int, int, int, UINT8 *);
static int	dc_keypress		(void);
static int	dc_get_key		(void);
static void	dc_new_timer	(void);

struct sarien_options opt;

static struct gfx_driver gfx_dc =
{
	init_vidmode,
	deinit_vidmode,
	dc_put_block,
	dc_put_pixels,
	dc_new_timer,
	dc_keypress,
	dc_get_key
};

#define MODE_TA	0
#define MODE_FB	1

extern struct gfx_driver *gfx;

static uint8 screen_buffer[GFX_WIDTH*GFX_HEIGHT];
static uint16 *texture;
static uint8 mkb;
static int filter = 0;
static int mode = MODE_TA;
static uint32 tex_addr;
static uint32 mouse_addr;
static IMAGE imgMouse;
static uint8 mmouse;
static int last_mouse = 0;
static int key;
static int mouse_key;

void frate_update()
{
	static int maxfrate=0; // Top of graph, auto scaled
	static int ftimes[16]; // last 16 RAW frame times
	static int frate[60]; // last 60 averaged frame times
	static int frames=0; // frames since we shifted the chart
	static uint32 jifs=0; // number of jiffies processed
	static int graphspeed=15;// [0-15] graph speed/accuracy
	int i;

	frames++;
	if(jiffies-jifs > 100/graphspeed)
	{
		// Shift the data back one spot
		memcpy(&frate[0], &frate[1], 59*sizeof(int));
		memcpy(&ftimes[0], &ftimes[1], 15*sizeof(int));
	
		// Update the counters
		ftimes[15]=frames;
		frate[59]=0;
		for(i=0; i<graphspeed; i++)
		{
	        frate[59]+=ftimes[15-i];
		}
		if(frate[59]>maxfrate)
			maxfrate=frate[59];
		frames=0;
		jifs=jiffies;
		printf("fps=%d\n", maxfrate);
	}
}

static void update_mouse()
{
	mouse_cond_t mcond;

	if (!mmouse)
		return;

	if (mouse_get_cond(mmouse, &mcond))
	{
		printf("Error checking mouse status\r\n");
		return;
	}

	mouse.button = FALSE;
	
	if(mcond.buttons != last_mouse)
	{
		last_mouse = mcond.buttons;

		if(!(mcond.buttons & MOUSE_LEFTBUTTON))
		{
			mouse.button = TRUE;
			mouse_key = BUTTON_LEFT;
		}

		if(!(mcond.buttons & MOUSE_RIGHTBUTTON))
		{
			mouse.button = TRUE;
			mouse_key = BUTTON_RIGHT;
		}
	}

	if(mcond.dx || mcond.dy)
	{
		if((int)mouse.x + mcond.dx > 640)
			mouse.x = 640;
		else if((int)mouse.x + mcond.dx < 0)
			mouse.x = 0;
		else
			mouse.x += mcond.dx;

		if((int)mouse.y + mcond.dy > 480)
			mouse.y = 480;
		else if((int)mouse.y + mcond.dy < 0)
			mouse.y = 0;
		else
			mouse.y += mcond.dy;
	}
}

int init_machine (int argc, char **argv)
{
	mkb = maple_first_kb();
	if(!mkb)
	{
		char *s = "Dreamcast Keyboard required.";
		ta_shutdown();
		bfont_draw_str(vram_s + (160 * 640) + 16*((40 - strlen(s))/2), 640, 0, s);
		printf("No keyboard attached!");
		return -1;
	}

	mmouse = maple_first_mouse();
	if(!mmouse)
		printf("No mice attached!\r\n");

	gfx = &gfx_dc;
	return err_OK;
}

int deinit_machine ()
{
	return err_OK;
}

static int init_vidmode ()
{
	uint32 size;

	ta_txr_release_all();
	tex_addr = ta_txr_allocate(1024*1024*2);
	texture = ta_txr_map(tex_addr);
	memset(texture, 0, 1024*1024*2);

	if(load_gsi(&imgMouse, "/rd/cursor.gsi") == -1)
	{
		printf("Couldn't open mouse cursor\n");
		return -1;
	}
	size = imgMouse.w * imgMouse.h * 2;
	mouse_addr = ta_txr_allocate(size);
	ta_txr_load(mouse_addr, imgMouse.data, size);

	return err_OK;
}

static int deinit_vidmode (void)
{
	ta_txr_release_all();
	return err_OK;
}

#define RGBTO565(r,g,b) (((r >> 3) & 0x1f) << 11) | (((g >> 2) & 0x3f) << 5) | (((b >> 3) & 0x1f) << 0)

/* put a block onto the screen */
static void dc_put_block (int x1, int y1, int x2, int y2)
{
	int h;
	int i, j;
	UINT8 *p;

	h = y2 - y1 + 1;

	if (x1 >= GFX_WIDTH)
		x1 = GFX_WIDTH - 1;
	if (y1 >= GFX_HEIGHT)
		y1 = GFX_HEIGHT - 1;
	if (x2 >= GFX_WIDTH)
		x2 = GFX_WIDTH - 1;
	if (y2 >= GFX_HEIGHT)
		y2 = GFX_HEIGHT - 1;

	if(mode == 0)
	{
		vertex_ot_t	v;
		poly_hdr_t	hdr;

		for (i = 0; i < h; i++)
		{
			for(j = 0; j < x2 - x1 + 1; j++)
			{
				p = &palette[(screen_buffer[(GFX_WIDTH * (y1 + i) + x1)+j]) * 3];
				texture[(512 * (y1 + i) + x1)+j] = RGBTO565(p[0] << 2, p[1] << 2, p[2] << 2);
			}
		}
		
		/* draw it on both buffers */
		for(i = 0; i < 2; i++)
		{
			ta_begin_render();
		
			ta_poly_hdr_txr(&hdr, TA_OPAQUE, TA_RGB565, 512, 512, tex_addr, filter);
			ta_commit_poly_hdr(&hdr);
			
			/* - -
			   + - */
			v.flags = TA_VERTEX_NORMAL;
			v.x = 0.0f;
			v.y = 1024.0f + YOFFS;
			v.z = 1.0f;
			v.u = 0.0f; v.v = 1.0f;
			v.a = v.r = v.g = v.b = 1.0f;
			v.oa = v.or = v.og = v.ob = 0.0f;
			ta_commit_vertex(&v, sizeof(v));
			
			/* + -
			   - - */
			v.y = 0.0f + YOFFS;
			v.v = 0.0f;
			ta_commit_vertex(&v, sizeof(v));
			
			/* - -
			   - + */
			v.x = 1024.0f;
			v.y = 1024.0f + YOFFS;
			v.u = 1.0f; v.v = 1.0f;//420.0f / 512.0f;
			ta_commit_vertex(&v, sizeof(v));
		
			/* - +
			   - - */
			v.flags = TA_VERTEX_EOL;
			v.y = 0.0f + YOFFS;
			v.v = 0.0f;
			ta_commit_vertex(&v, sizeof(v));
		
			ta_commit_eol();
			draw_bm_scale(mouse_addr, 1.0f, 1.0f, 16, 16, (float)(mouse.x * opt.scale), (float)((mouse.y * opt.scale) + YOFFS), 0);
			ta_commit_eol();
			ta_finish_frame();
		}
	}
	else
	{
		int h;
		int i, j;
		UINT8 *p;
	
		h = y2 - y1 + 1;
	
		for (i = 0; i < h; i++)
		{
			for(j = 0; j < x2 - x1 + 1; j++)
			{
				p = &palette[(screen_buffer[(GFX_WIDTH * (y1 + i) + x1)+j]) * 3];
				vram_s[(320 * (y1 + i) + x1)+j] = RGBTO565(p[0] << 2, p[1] << 2, p[2] << 2);
			}
		}
	}
}

/* put pixel routine */
/* Some errors! Handle color depth */
static void dc_put_pixels (int x, int y, int w, UINT8 *p)
{
    UINT8 *p0 = screen_buffer;
	p0 += x + y * GFX_WIDTH;
	while (w--) { *p0++ = *p++; }
	return;
}

static int dc_keypress (void)
{
	snd_stream_poll();
	update_mouse();

	/* Get queued keys */
	key = kbd_get_key();

	if(key != -1 || mouse_key)
		return 1;
	else
		return 0;
}

static int dc_get_key (void)
{
	static int numlock = 0;
	kbd_state_t *state;
	int p, u;
	uint8 addr;
	int ret;

	addr = maple_first_kb();
	if(!addr)
		return;

	maple_raddr(addr, &p, &u);
	state = kbd_get_state(p, u);

	if(mouse.button)
	{
		mouse.button = FALSE;
		ret = mouse_key;
		mouse_key = 0;
		return ret;
	}

	if(state->shift_keys & KBD_MOD_LALT || state->shift_keys & KBD_MOD_RALT)
		key = scancode_table[(key & ~0x20) - 0x41] << 8;

	if(state->shift_keys & KBD_MOD_LCTRL || state->shift_keys & KBD_MOD_RCTRL)
		key = (key & ~0x20) - 0x40;

	switch(key >> 8)
	{
		case KBD_KEY_UP:
			return KEY_UP;

		case KBD_KEY_DOWN:
			return KEY_DOWN;

		case KBD_KEY_LEFT:
			return KEY_LEFT;

		case KBD_KEY_RIGHT:
			return KEY_RIGHT;

		case KBD_KEY_F1:
		case KBD_KEY_F2:
		case KBD_KEY_F3:
		case KBD_KEY_F4:
		case KBD_KEY_F5:
		case KBD_KEY_F6:
		case KBD_KEY_F7:
		case KBD_KEY_F8:
		case KBD_KEY_F9:
		case KBD_KEY_F10:
		case KBD_KEY_F11:
		case KBD_KEY_F12:
			return key + 0x0100;

		case KBD_KEY_HOME:
			return KEY_UP_LEFT;

		case KBD_KEY_END:
			return KEY_DOWN_LEFT;

		case KBD_KEY_PGUP:
			return KEY_UP_RIGHT;

		case KBD_KEY_PGDOWN:
			return KEY_DOWN_RIGHT;

		case KBD_KEY_PAD_NUMLOCK:
			numlock = !numlock;
			break;

		/* dead keys */
		case KBD_KEY_INSERT:
		case KBD_KEY_DEL:
		case KBD_KEY_PRINT:
		case KBD_KEY_SCRLOCK:
		case KBD_KEY_PAUSE:
			return 0;
	}

	if(key != -1)
		return key;
	else
		return 0;
}

static void dc_new_timer ()
{
	static int i = 0;
	int num_vbl;
	static uint32 last;
	uint32 now, secs;

	timer_ms_gettime(&secs, &now);

	while (now - last < TICK_IN_MSEC)
	{
		thd_sleep(TICK_IN_MSEC - (now - last));
		timer_ms_gettime(&secs, &now);
	}

	last = now;

//	frate_update();
}

