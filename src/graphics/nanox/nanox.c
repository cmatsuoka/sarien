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
#define MWINCLUDECOLORS
#include <nano-X.h>
#include <device.h>
#include "sarien.h"
#include "graphics.h"


extern struct gfx_driver *gfx;
extern struct sarien_options opt;


static int	nanox_init_vidmode	(void);
static int	nanox_deinit_vidmode	(void);
static void	nanox_put_block		(int, int, int, int);
static void	nanox_put_pixels	(int, int, int, UINT8 *);
static void	nanox_timer		(void);
static int	nanox_get_key		(void);
static int	nanox_keypress		(void);


static struct gfx_driver gfx_nanox = {
	nanox_init_vidmode,
	nanox_deinit_vidmode,
	nanox_put_block,
	nanox_put_pixels,
	nanox_timer,
	nanox_keypress,
	nanox_get_key
};

static GR_WINDOW_ID main_window;
static GR_EVENT nevent;
static UINT8 *screen_buffer;
static PSD mempsd;


#define KEY_QUEUE_SIZE 16

static int key_queue[KEY_QUEUE_SIZE];
static int key_queue_start = 0;
static int key_queue_end = 0;

#define key_enqueue(k) do { key_queue[key_queue_end++] = (k); \
	key_queue_end %= KEY_QUEUE_SIZE; } while (0)
#define key_dequeue(k) do { (k) = key_queue[key_queue_start++]; \
	key_queue_start %= KEY_QUEUE_SIZE; } while (0)


static void handle_exposure_event ()
{
	/*GR_EVENT_EXPOSURE *event = &nevent.exposure;*/
}


void handle_keyboard_event ()
{
	switch (nevent.keystroke.ch) {
		case 'P':
			break;
	}
}

static void process_events ()
{
	GrGetNextEventTimeout (&nevent, 10);
	switch (nevent.type) {
	case GR_EVENT_TYPE_EXPOSURE:
		handle_exposure_event ();
		break;
	case GR_EVENT_TYPE_KEY_DOWN:
		handle_keyboard_event ();
		break;
	case GR_EVENT_TYPE_CLOSE_REQ:
		break;
	case GR_EVENT_TYPE_TIMEOUT:
		break;
	default:
		fprintf(stderr, "Unhandled event type %d\n", nevent.type);
		break;
	}
}

int init_machine (int argc, char **argv)
{
	gfx = &gfx_nanox;
	return err_OK;
}


int deinit_machine ()
{
	return err_OK;
}


static void nanox_timer ()
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


static int nanox_init_vidmode ()
{
	PSD	mempsd;
	int	linelen, size;

	if(GrOpen() < 0) {
		fprintf(stderr, "Couldn't connect to Nano-X server\n");
		return -1;
	}

	main_window = GrNewWindow (GR_ROOT_WINDOW_ID, 0, 0, 320, 200, 0, 0, 0);
	GrSelectEvents (main_window, GR_EVENT_MASK_EXPOSURE |
		GR_EVENT_MASK_CLOSE_REQ | GR_EVENT_MASK_KEY_DOWN |
		GR_EVENT_MASK_TIMEOUT);

	GrMapWindow (main_window);

	mempsd = scrdev.AllocateMemGC (&scrdev);
	GdCalcMemGCAlloc (mempsd, 320, 200, 0, 0, &size, &linelen);
	screen_buffer = malloc(size);
	mempsd->flags |= PSF_ADDRMALLOC;
	mempsd->MapMemGC(mempsd, 320, 200, scrdev.planes, scrdev.bpp,
		linelen, size, screen_buffer);

	return 0;
}


static int nanox_deinit_vidmode ()
{
	GrClose();
	return 0;
}


/* blit a block onto the screen */
static void nanox_put_block (int x1, int y1, int x2, int y2)
{
	unsigned int h, w, p;

	if (x1 >= GFX_WIDTH)  x1 = GFX_WIDTH  - 1;
	if (y1 >= GFX_HEIGHT) y1 = GFX_HEIGHT - 1;
	if (x2 >= GFX_WIDTH)  x2 = GFX_WIDTH  - 1;
	if (y2 >= GFX_HEIGHT) y2 = GFX_HEIGHT - 1;

	h = y2 - y1 + 1;
	w = x2 - x1 + 1;
	p = GFX_WIDTH * y1 + x1;

	GdBlit (&scrdev, x1, y1, w, h, mempsd, 0, 0, 0);
}


static void nanox_put_pixels(int x, int y, int w, UINT8 *p)
{
	while (w--) { scrdev.DrawPixel (mempsd, x, y++, *p++); }
}


static int nanox_keypress ()
{
	process_events ();
	return key_queue_start != key_queue_end;
}


static int nanox_get_key ()
{
	UINT16 k;

	while (key_queue_start == key_queue_end)	/* block */
		nanox_timer ();

	key_dequeue(k);

	return k;
}

