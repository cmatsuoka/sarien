/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 *
 *  Photon graphics module by Jeremy Penner, jeremy@astra.mb.ca
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>
#include <unistd.h>

#include <signal.h>
#include <sys/neutrino.h>
#include <errno.h>
#include <pthread.h>
#include <Pt.h>
#include <Ph.h>

#include "sarien.h"
#include "graphics.h"
#include "keyboard.h"

extern struct sarien_options opt;
extern struct gfx_driver *gfx;

static int scale = 1;
static int key_control = 0;
static int key_alt = 0;
static PgColor_t ph_pal[32];

static int __argc;
static char **__argv;


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
static void	put_block	(int, int, int, int);
static void	put_pixels	(int, int, int, UINT8*);
static int	keypress	(void);
static int	get_key		(void);
static void	new_timer	(void);

void		photon_thread	(void *);
static void	ph_raw_draw_cb	(PtWidget_t *, PhTile_t *);
static int	ph_keypress_cb	(PtWidget_t *, void *, PtCallbackInfo_t *);
static int	ph_cause_damage	(void *, int, void *, size_t);


static pthread_t ph_tid;
static PhImage_t *phimage;
static pthread_barrier_t barrier;
static pthread_mutex_t mut_image;

static PtWidget_t *rawwidget;

static int ph_coid;
static int ph_chid;

static struct gfx_driver GFX_ph = {
	init_vidmode,
	deinit_vidmode,
	put_block,
	put_pixels,
	new_timer,
	keypress,
	get_key
};


int init_machine (int argc, char **argv)
{
	gfx = &GFX_ph;

	__argc = argc;
	__argv = argv;
	scale = optScale;

	return err_OK;
}


int deinit_machine (void)
{
	return err_OK;
}


static int init_vidmode (void)
{
	int i;

	fprintf (stderr, "ph: Photon support by jeremy@astra.mb.ca\n");

	if (optFullScreen) { // We can implement later.
		optFullScreen = FALSE;
	}

	for (i = 0; i < 32; i ++)
		ph_pal[i] = (palette[i * 3] << 18) + 
			    (palette[i * 3 + 1] << 10) +
			    palette[i * 3 + 2] << 2;

	pthread_barrier_init(&barrier, NULL, 2);

	pthread_create (&ph_tid, NULL, photon_thread, (void*)getpid());

	pthread_barrier_wait (&barrier); // Wait for thread to create a channel to send to
	
	ph_coid = ConnectAttach (0, 0, ph_chid, _NTO_SIDE_CHANNEL, 0);
	return err_OK;
}


static int deinit_vidmode ()
{

	fprintf (stderr, "ph: deiniting video mode\n");

	pthread_cancel (ph_tid);

	PhReleaseImage(phimage);

	return err_OK;
}


/* put a block onto the screen */
static void put_block (int x1, int y1, int x2, int y2)
{
	PhRect_t rect;

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
		x2 = x2 * scale + scale - 1;
		y2 = y2 * scale + scale - 1;
	}

	rect.ul.x = x1;
	rect.ul.y = y1;
	rect.lr.x = x2;
	rect.lr.y = y2;

	MsgSend (ph_coid, &rect, sizeof (rect), NULL, 0);
}


/* put pixel routine */
static void put_pixels (int x, int y, int w, char *p)
{
	register int i, j;

	pthread_mutex_lock (&mut_image);

	/* CM: this is only a kludge to use the new interface.
	 *     please fix it properly to take advantage of the
	 *     driver interface change.
	 */
while (w--) {
	int c = *p++;

	if (scale == 1) {
		PiSetPixel (phimage, x, y, c);
	} else if (scale == 2) {
		x <<= 1;
		y <<= 1;
		PiSetPixel (phimage, x, y, c);
		PiSetPixel (phimage, x, y + 1, c);
		PiSetPixel (phimage, x + 1, y, c);
		PiSetPixel (phimage, x + 1, y + 1, c);
	} else {
		x *= scale;
		y *= scale;
		for (i = 0; i < scale; i++)
			for (j = 0; j < scale; j++)
				PiSetPixel (phimage, x + i, y + j, c);
	}
}
	pthread_mutex_unlock (&mut_image);
}


static int keypress ()
{
	int retcode;

	pthread_sleepon_lock(); // Watch those threads!  This ain't atomic.
	retcode = key_queue_start != key_queue_end ? TRUE : FALSE;
	pthread_sleepon_unlock();

	return (retcode);
}


static int get_key (void)
{
	int k;

	pthread_sleepon_lock ();
	
	while (key_queue_start == key_queue_end)	/* block */
		pthread_sleepon_wait(key_queue);
	key_dequeue (k);

	pthread_sleepon_unlock ();

	return k;
}

static void new_timer ()
{
	struct timeval tv;
	struct timezone tz;
	static double msec = 0.0;
	double m;
	
	gettimeofday (&tv, &tz);
	m = 1000.0 * tv.tv_sec + tv.tv_usec / 1000.0;

	while (m - msec < 42)
	{
		usleep (5000);
		gettimeofday (&tv, &tz);
		m = 1000.0 * tv.tv_sec + tv.tv_usec / 1000.0;
	}
	msec = m; 

}

// PHOTON THREAD //

void photon_thread(void *pidarg)
{
	PtArg_t args[4];
	PtWidget_t *window;
	PhDim_t dim;
	PhArea_t area;
	PtRawCallback_t keycb[] = {{ Ph_EV_KEY , ph_keypress_cb, NULL }};
	int recpid = pidarg;

	dim.w = GFX_WIDTH * scale;
	dim.h = GFX_HEIGHT * scale;

	PtSetArg(&args[0], Pt_ARG_WINDOW_TITLE, "Sarien for Photon", 0);
	PtSetArg(&args[1], Pt_ARG_WINDOW_RENDER_FLAGS, Pt_FALSE, Ph_WM_RENDER_RESIZE);
	PtSetArg(&args[2], Pt_ARG_DIM, &dim, sizeof (dim));
	window = PtAppInit( NULL, NULL, NULL, 3, args);

	if (!window)
	{
		fprintf (stderr, "ph: Unable to create main widget!");
		exit (-1);
	}

	ph_chid = PhChannelAttach (0, -1, NULL);

	phimage = PhCreateImage(NULL, GFX_WIDTH * scale, GFX_HEIGHT *
		scale, Pg_IMAGE_PALETTE_BYTE, ph_pal, 32, 1);

	if (phimage == NULL)
	{
		fprintf (stderr, "ph: can't create image\n");
		exit (-1);
	}

	PtAppAddInput (NULL, recpid, ph_cause_damage, NULL);

	area.size = dim;
	area.pos.x = 0;
	area.pos.y = 0;

	PtSetArg(&args[0], Pt_ARG_AREA, &area, 0);
	PtSetArg(&args[1], Pt_ARG_RAW_DRAW_F, ph_raw_draw_cb, 0);
	PtSetArg(&args[2], Pt_CB_RAW, keycb, 0);
	PtSetArg(&args[3], Pt_ARG_FLAGS, Pt_TRUE, Pt_GETS_FOCUS);
	rawwidget = PtCreateWidget (PtRaw, window, 4, args);

	pthread_barrier_wait (&barrier); // sync with original thread

	PtRealizeWidget(window);
	PtMainLoop();

	return;
}


static void ph_raw_draw_cb (PtWidget_t *widget, PhTile_t *damage)
{
	PhRect_t raw_canvas;
	PhRect_t draw_area;

	PtCalcCanvas (widget, &raw_canvas);

	PtClipAdd (widget, &raw_canvas);
	
	draw_area = damage->rect;

	if (draw_area.ul.x < raw_canvas.ul.x)
		draw_area.ul.x = raw_canvas.ul.x;
	if (draw_area.ul.y < raw_canvas.ul.y)
		draw_area.ul.y = raw_canvas.ul.y;
	if (draw_area.lr.x > raw_canvas.lr.x)
		draw_area.lr.x = raw_canvas.lr.x;
	if (draw_area.lr.x > raw_canvas.lr.x)
		draw_area.lr.x = raw_canvas.lr.x;

	PgSetPalette( ph_pal, 0, 0, 32, Pg_PALSET_SOFT, 0);

	pthread_mutex_lock (&mut_image);

	PgDrawPhImageRectmx (&draw_area.ul, phimage, &draw_area, 0);

	pthread_mutex_unlock (&mut_image);

	PtClipRemove();

}

static int ph_keypress_cb (PtWidget_t *widget, void *data, PtCallbackInfo_t *cb)
{
	PhKeyEvent_t *key_event;
	int key = 0;

	key_event = PhGetData(cb->event);
	if (key_event->key_flags & Pk_KF_Key_Down)
	{	
		switch (key = key_event->key_cap) 
		{
			case Pk_Shift_L:
			case Pk_Shift_R:
				key = 0;
				break;
			case Pk_Control_L:
				key_control |= 1;
				key = 0;
				break;
			case Pk_Control_R:
				key_control |= 2;
				key = 0;
				break;
			case Pk_Alt_L:
				key_alt |= 1;
				key = 0;
				break;
			case Pk_Alt_R:
				key_alt |= 2;
				key = 0;
				break;
			case Pk_Up:
//			case Pk_KP_Up:
			case Pk_KP_8:
				key = KEY_UP;
				break;
			case Pk_Left:
//			case Pk_KP_Left:
			case Pk_KP_4:
				key = KEY_LEFT;
				break;
			case Pk_Down:
//			case Pk_KP_Down:
			case Pk_KP_2:
				key = KEY_DOWN;
				break;
			case Pk_Right:
//			case Pk_KP_Right:
			case Pk_KP_6:
				key = KEY_RIGHT;
				break;
			case Pk_Home:
			case Pk_KP_7:
				key = KEY_UP_LEFT;
				break;
			case Pk_Pg_Up:
			case Pk_KP_9:
				key = KEY_UP_RIGHT;
				break;
			case Pk_Pg_Down:
			case Pk_KP_3:
				key = KEY_DOWN_RIGHT;
				break;
			case Pk_End:
			case Pk_KP_1:
				key = KEY_DOWN_LEFT;
				break;
			case Pk_KP_Enter:
				key = KEY_ENTER;
				break;
			case Pk_KP_Add:
				key = '+';
				break;
			case Pk_KP_Subtract:
				key = '-';
				break;
			case Pk_F1:
				key = 0x3b00;
				break;
			case Pk_F2:
				key = 0x3c00;
				break;
			case Pk_F3:
				key = 0x3d00;
				break;
			case Pk_F4:
				key = 0x3e00;
				break;
			case Pk_F5:
				key = 0x3f00;
				break;
			case Pk_F6:
				key = 0x4000;
				break;
			case Pk_F7:
				key = 0x4100;
				break;
			case Pk_F8:
				key = 0x4200;
				break;
			case Pk_F9:
				key = 0x4300;
				break;
			case Pk_F10:
				key = 0x4400;
				break;
			case Pk_Return:
				key = 0x0d;
				break;
			case Pk_Escape:
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
		}
	}
	else
	{
		switch (key_event->key_cap) {
			case Pk_Control_L:
				key_control &= ~1;
				break;
			case Pk_Control_R:
				key_control &= ~2;
				break;
			case Pk_Alt_L:
				key_alt &= ~1;
				break;
			case Pk_Alt_R:
				key_alt &= ~2;
				break;
			}
	}


	if (key)
	{
		pthread_sleepon_lock();

		key_enqueue (key);

		pthread_sleepon_signal (key_queue);
		pthread_sleepon_unlock();

	}

	return (Pt_CONSUME);
}
	
static int ph_cause_damage (void *data, int rcvid, void *message, size_t mbsize)
{
	PhRect_t *rect;

	rect = message;
	PtDamageExtent (rawwidget, rect);

	MsgReply(rcvid, EOK, NULL, 0);

	return (Pt_CONTINUE);
}

