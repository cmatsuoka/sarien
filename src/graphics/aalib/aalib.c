/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include <aalib.h>
#include "sarien.h"
#include "graphics.h"
#include "keyboard.h"


extern struct gfx_driver *gfx;
extern struct sarien_options opt;


static int	aalib_init_vidmode	(void);
static int	aalib_deinit_vidmode	(void);
static void	aalib_put_block		(int, int, int, int);
static void	aalib_put_pixels	(int, int, int, UINT8 *);
static void	aalib_timer		(void);
static int	aalib_get_key		(void);
static int	aalib_keypress		(void);


static struct gfx_driver gfx_aalib = {
	aalib_init_vidmode,
	aalib_deinit_vidmode,
	aalib_put_block,
	aalib_put_pixels,
	aalib_timer,
	aalib_keypress,
	aalib_get_key
};

static aa_context *context;
static aa_palette pal;
static char *framebuffer;


#define KEY_QUEUE_SIZE 16

static int key_queue[KEY_QUEUE_SIZE];
static int key_queue_start = 0;
static int key_queue_end = 0;

#define key_enqueue(k) do { key_queue[key_queue_end++] = (k); \
	key_queue_end %= KEY_QUEUE_SIZE; } while (0)
#define key_dequeue(k) do { (k) = key_queue[key_queue_start++]; \
	key_queue_start %= KEY_QUEUE_SIZE; } while (0)


int init_machine (int argc, char **argv)
{
	gfx = &gfx_aalib;
	return err_OK;
}


int deinit_machine ()
{
	return err_OK;
}


static void kill_mode (int i)
{
	static int kill_flag = 0;       /* Yuck */

	if (kill_flag)
		exit (0);

	kill_flag = 1;
	gfx->deinit_video_mode ();
	fprintf (stderr, "Fatal: signal %d caught\n", i);

	exit (-1);
}


static void process_events ()
{
	int k;

	switch (k = aa_getevent (context, 0)) {
	case AA_NONE:
		return;
	case AA_UP:
		k = KEY_UP;
		break;
	case AA_DOWN:
		k = KEY_DOWN;
		break;
	case AA_LEFT:
		k = KEY_LEFT;
		break;
	case AA_RIGHT:
		k = KEY_RIGHT;
		break;
	case AA_BACKSPACE:
		k = 0x08;
		break;
	case AA_ESC:
		k = 0x1b;
		break;
	case AA_RESIZE:
		if (aa_resize (context))
			flush_screen ();
	}

	if (k >= AA_RELEASE)
		return;

	key_enqueue (k);
}

static void aalib_timer ()
{
	struct timeval tv;
	struct timezone tz;
	static double msec = 0.0;
	double m;
	
	gettimeofday (&tv, &tz);
	m = 1000.0 * tv.tv_sec + tv.tv_usec / 1000.0;

	while (m - msec < 42) {
		usleep (5000);
		gettimeofday (&tv, &tz);
		m = 1000.0 * tv.tv_sec + tv.tv_usec / 1000.0;
	}
	msec = m; 

	process_events ();
}


static int aalib_init_vidmode ()
{
	int i;

	aa_parseoptions (NULL, NULL, NULL, NULL);

	if ((context = aa_autoinit (&aa_defparams)) == NULL) {
		printf ("Failed to initialize aalib\n");
		exit (1);
	}

	framebuffer = aa_image (context);

	for (i = 0; i < 32; i++) {
		aa_setpalette (pal, i, palette[i * 3 + 0] << 2,
			palette[i * 3 + 1] << 2, palette[i * 3 + 2] << 2);
	}

	aa_autoinitkbd (context, 0);

	signal (SIGSEGV, kill_mode);
	signal (SIGQUIT, kill_mode);
	signal (SIGFPE, kill_mode);
	signal (SIGTERM, kill_mode);
	signal (SIGINT, kill_mode);

	return 0;
}


static int aalib_deinit_vidmode ()
{
	aa_uninitkbd (context);
	aa_close (context);
	return 0;
}


/* blit a block onto the screen */
static void aalib_put_block (int x1, int y1, int x2, int y2)
{
	if (x1 >= GFX_WIDTH)  x1 = GFX_WIDTH  - 1;
	if (y1 >= GFX_HEIGHT) y1 = GFX_HEIGHT - 1;
	if (x2 >= GFX_WIDTH)  x2 = GFX_WIDTH  - 1;
	if (y2 >= GFX_HEIGHT) y2 = GFX_HEIGHT - 1;

	x1 = x1 * aa_scrwidth (context) / GFX_WIDTH;
	y1 = y1 * aa_scrheight (context) / GFX_HEIGHT;
	x2 = (x2 + 1) * aa_scrwidth (context) / GFX_WIDTH;
	y2 = (y2 + 1) * aa_scrheight (context) / GFX_HEIGHT;

	/* AA bug(?) workaround */ x1 = 0; x2++; y2++;

	aa_renderpalette (context, pal, &aa_defrenderparams, x1, y1, x2, y2);

	aa_flush (context);
}


static void aalib_put_pixels (int x, int y, int w, UINT8 *p)
{
	int x1, x2, y1, y2, i, j, ww;
	UINT8 *pp;

	y1 = y * aa_imgheight (context) / GFX_HEIGHT;
	y2 = (y + 1) * aa_imgheight (context) / GFX_HEIGHT;

	for (i = y1; i < y2; i++) {
		pp = p; ww = w; while (ww--) {
			x1 = x * aa_imgwidth (context) / GFX_WIDTH;
			x2 = (x + 1) * aa_imgwidth (context) / GFX_WIDTH;
			for (j = x1; j < x2; j++) {
				aa_putpixel (context, j, i, *pp);
			}
			pp++;
			x++;
		}
	}
}


static int aalib_keypress ()
{
	process_events ();
	return key_queue_start != key_queue_end;
}


static int aalib_get_key ()
{
	UINT16 k;

	while (key_queue_start == key_queue_end)	/* block */
		aalib_timer ();

	key_dequeue(k);

	return k;
}

