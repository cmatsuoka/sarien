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
#include <photon/Pf.h>
#include <photon/PhRender.h>

#include "sarien.h"
#include "graphics.h"
#include "keyboard.h"
#include "agi.h"

typedef struct
{
	PhImage_t *behind_img;
	PhPoint_t pos;

	struct mouse sarien_mouse;
} ph_mouse_t;

#define PH_MOUSE_CURSOR_W 10
#define PH_MOUSE_CURSOR_H 16
#define PH_MOUSE_TRANS_COL 2

extern struct sarien_options opt;
extern struct gfx_driver *gfx;

static int scale = 1;
static int key_control = 0;
static int key_alt = 0;
static PgColor_t ph_pal[32];

static int __argc;
static char **__argv;

static PgDisplaySettings_t oldvidmode;
static PdDirectContext_t *dc;

static ph_mouse_t ph_mouse;

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
static void	ph_put_block	(int, int, int, int);
static void	ph_put_pixels	(int, int, int, UINT8*);
static int	ph_keypress	(void);
static int	ph_get_key	(void);
static void	ph_new_timer	(void);
static void	ph_update_mouse	(void);

void *		photon_thread	(void *);
static void	ph_raw_draw_cb	(PtWidget_t *, PhTile_t *);
static int	ph_keypress_cb	(PtWidget_t *, void *, PtCallbackInfo_t *);
static int	ph_cause_damage	(void *, int, void *, size_t);
static int	ph_mouse_cb	(PtWidget_t *widget, void *data, PtCallbackInfo_t *cb);
static int	ph_close_cb	(PtWidget_t *widget, void *data, PtCallbackInfo_t *cb);
static void	ph_setup_mouse_image (void);
static void	ph_show_mouse_image (void);
static void	ph_hide_mouse_image (void);

static pthread_t ph_tid;
static PhImage_t *phimage;
static pthread_barrier_t barrier;
static pthread_mutex_t mut_image = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mut_mouse = PTHREAD_MUTEX_INITIALIZER;

static PtWidget_t *rawwidget;

static int ph_coid;
static int ph_chid;

static char ph_cursor_img_data[] = {
 0, 0, 2, 2, 2, 2, 2, 2, 2, 2,
 0,15, 0, 2, 2, 2, 2, 2, 2, 2,
 0,15,15, 0, 2, 2, 2, 2, 2, 2,
 0,15,15,15, 0, 2, 2, 2, 2, 2,
 0,15,15,15,15, 0, 2, 2, 2, 2,
 0,15,15,15,15,15, 0, 2, 2, 2,
 0,15,15,15,15,15,15, 0, 2, 2,
 0,15,15,15,15,15,15,15, 0, 2,
 0,15,15,15,15,15,15,15,15, 0,
 0,15,15,15,15,15, 0, 0, 0, 0,
 0,15,15, 0,15,15, 0, 2, 2, 2,
 0,15, 0, 2, 0,15,15, 0, 2, 2,
 0, 0, 2, 2, 0,15,15, 0, 2, 2,
 0, 2, 2, 2, 2, 0,15,15, 0, 2,
 2, 2, 2, 2, 2, 0,15,15, 0, 2,
 2, 2, 2, 2, 2, 2, 0, 0, 2, 2 };

static struct gfx_driver GFX_ph = {
	init_vidmode,
	deinit_vidmode,
	ph_put_block,
	ph_put_pixels,
	ph_new_timer,
	ph_keypress,
	ph_get_key
};


int init_machine (int argc, char **argv)
{
	gfx = &GFX_ph;

	__argc = argc;
	__argv = argv;
	scale = opt.scale;

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

	for (i = 0; i < 32; i ++) {
		ph_pal[i] = (palette[i * 3] << 18) + 
			    (palette[i * 3 + 1] << 10) +
			    (palette[i * 3 + 2] << 2);
	}

	pthread_barrier_init(&barrier, NULL, 2);

	pthread_create (&ph_tid, NULL, photon_thread, (void*)getpid());

	pthread_barrier_wait (&barrier); /* Wait for thread to create a
                                            channel to send to */
	
	ph_coid = ConnectAttach (0, 0, ph_chid, _NTO_SIDE_CHANNEL, 0);
	return err_OK;
}


static int deinit_vidmode ()
{
	fprintf (stderr, "ph: deiniting video mode\n");

	PtEnter(0);

	pthread_mutex_lock (&mut_image);

	PhReleaseImage(phimage);
	phimage = NULL;

	pthread_mutex_unlock (&mut_image);

	if (opt.fullscreen)
	{
		PdDirectStop (dc);
		PdReleaseDirectContext(dc);
		PgSetVideoMode (&oldvidmode);
	}

	PtLeave(0);
	
	return err_OK;
}


/* put a block onto the screen */
static void ph_put_block (int x1, int y1, int x2, int y2)
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
static void ph_put_pixels (int x, int y, int w, UINT8 *p)
{
	register int i, j;

	pthread_mutex_lock (&mut_image);

	if (!phimage)
		return;

	switch (scale) {
	case 1:
		while (w--) PiSetPixel (phimage, x++, y, *p++);
		break;
	case 2:
		x <<= 1;
		y <<= 1;
		while (w--) {
			int c = *p++;
			PiSetPixel (phimage, x, y, c);
			PiSetPixel (phimage, x++, y + 1, c);
			PiSetPixel (phimage, x, y, c);
			PiSetPixel (phimage, x++, y + 1, c);
		}
		break;
	default:
		x *= scale;
		y *= scale;
		while (w--) {
			int c = *p++;
			for (i = 0; i < scale; i++) {
				for (j = 0; j < scale; j++)
					PiSetPixel (phimage, x + i, y + j, c);
			}
			x += scale;
		}
	}

	pthread_mutex_unlock (&mut_image);
}


static int ph_keypress ()
{
	int retcode;

	pthread_sleepon_lock(); /* Watch those threads!  This ain't atomic. */
	retcode = key_queue_start != key_queue_end ? TRUE : FALSE;
	pthread_sleepon_unlock();

	ph_update_mouse();

	return (retcode);
}


static int ph_get_key (void)
{
	int k;

	pthread_sleepon_lock ();
	
	while (key_queue_start == key_queue_end)	/* block */
		pthread_sleepon_wait(key_queue);
	key_dequeue (k);

	pthread_sleepon_unlock ();

	ph_update_mouse();

	return k;
}

static void ph_new_timer ()
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

static void ph_update_mouse(void)
{
	pthread_mutex_lock (&mut_mouse);

	mouse = ph_mouse.sarien_mouse;

	pthread_mutex_unlock (&mut_mouse);
}

/* PHOTON THREAD */

void *photon_thread (void *pidarg)
{
	PtArg_t args[8];
	PtWidget_t *window;
	PhRect_t rect;
	PhDim_t dim;
	PhArea_t area;
	PhRid_t rid;
	PgVideoModes_t modelist;
	PgVideoModeInfo_t modeinfo;
	PgDisplaySettings_t modesettings;
	PhDrawContext_t *olddc;
	PtRawCallback_t keycb[] = {{ Ph_EV_KEY , ph_keypress_cb, NULL },
                                   { Ph_EV_BUT_PRESS, ph_mouse_cb, NULL },
                                   { Ph_EV_BUT_RELEASE, ph_mouse_cb, NULL },
                                   { Ph_EV_PTR_MOTION_NOBUTTON, ph_mouse_cb, NULL },
                                   { Ph_EV_PTR_MOTION_BUTTON, ph_mouse_cb, NULL }};
	PtCallback_t closecb[] = { { ph_close_cb, NULL } };

	int recpid = (int)pidarg;
	int index;
	char found;

	dim.w = GFX_WIDTH * scale;
	dim.h = GFX_HEIGHT * scale;

	PtSetArg(&args[0], Pt_ARG_WINDOW_TITLE, "Sarien for Photon", 0);
	PtSetArg(&args[1], Pt_ARG_WINDOW_RENDER_FLAGS, Pt_FALSE, Ph_WM_RENDER_RESIZE | Ph_WM_RENDER_MAX);
	PtSetArg(&args[2], Pt_ARG_DIM, &dim, sizeof (dim));
	PtSetArg(&args[3], Pt_ARG_WINDOW_MANAGED_FLAGS, Pt_FALSE, Ph_WM_CLOSE);
	PtSetArg(&args[4], Pt_ARG_WINDOW_NOTIFY_FLAGS, Pt_TRUE, Ph_WM_CLOSE);
	PtSetArg(&args[5], Pt_CB_WINDOW, closecb, 1);
	window = PtAppInit( NULL, NULL, NULL, 6, args);

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
	if (!opt.fullscreen)
	{
		area.pos.x = 0;
		area.pos.y = 0;
	}
	else
	{
		PhWindowQueryVisible (Ph_QUERY_INPUT_GROUP | Ph_QUERY_EXACT, 0, 0, &rect);
		area.pos.x = rect.ul.x;
		area.pos.y = rect.ul.y;
	}

	PtSetArg(&args[0], Pt_ARG_AREA, &area, 0);
	PtSetArg(&args[1], Pt_ARG_FLAGS, Pt_TRUE, Pt_GETS_FOCUS);
	if (!opt.fullscreen)
	{
		PtSetArg(&args[2], Pt_ARG_RAW_DRAW_F, ph_raw_draw_cb, 0);
		rawwidget = PtCreateWidget (PtRaw, window, 3, args);
	}
	else
	{
		PtSetArg(&args[2], Pt_ARG_REGION_INFRONT, Ph_DEV_RID, 0);
		PtSetArg(&args[3], Pt_ARG_REGION_OPAQUE, Pt_TRUE, Ph_EV_KEY | 
			Ph_EV_BUT_PRESS | Ph_EV_BUT_RELEASE | Ph_EV_PTR_MOTION_NOBUTTON |
			Ph_EV_PTR_MOTION_BUTTON);
		PtSetArg(&args[4], Pt_ARG_REGION_FLAGS, Pt_TRUE,
			Ph_FOLLOW_IG_SIZE | Ph_FORCE_FRONT);
		PtSetArg(&args[5], Pt_ARG_REGION_FIELDS, Pt_TRUE,
			Ph_REGION_FLAGS | Ph_REGION_EV_OPAQUE |
			Ph_REGION_IN_FRONT);
		rawwidget = PtCreateWidget (PtRegion, Pt_NO_PARENT, 6, args);
		PtRealizeWidget (rawwidget);
	}
	PtAddEventHandlers (rawwidget, keycb, 5);

	if (opt.fullscreen)
	{
		ph_setup_mouse_image();

		olddc = PhDCGetCurrent();
		PdGetDevices(&rid, 1);
		PdSetTargetDevice (olddc, rid);

		PgGetVideoMode(&oldvidmode);

		PgGetVideoModeList (&modelist);
		found = 0;
		for (index = 0; index < modelist.num_modes; index ++)
		{
			PgGetVideoModeInfo(modelist.modes[index], &modeinfo);
			printf("%ux%ux%u\n", modeinfo.width,
				modeinfo.height, modeinfo.bits_per_pixel);
#if 0
			if (modeinfo.width == GFX_WIDTH * scale &&
				modeinfo.height == GFX_HEIGHT * scale &&
				modeinfo.bits_per_pixel == 8)
#endif
			if (modeinfo.width == 640 &&
				modeinfo.height == 480 &&
				modeinfo.bits_per_pixel == 8)
			{
				modesettings.refresh = 0;
				modesettings.mode = modelist.modes[index];
				found = 1;
			}
		}
		if (!found)
		{
			fprintf (stderr, "Unable to initialize %ux%ux8\n", 
				GFX_WIDTH * scale, GFX_HEIGHT * scale);
			exit(0);
		}
		PgSetVideoMode(&modesettings);			
		dc = PdCreateDirectContext();
		PdDirectStart(dc);
	}
	pthread_barrier_wait (&barrier); /* sync with original thread */

	PtRealizeWidget(window);
	PtMainLoop();

	return (0);
}

static void ph_draw_image (PhRect_t *area)
{
	PgSetPalette( ph_pal, 0, 0, 32, Pg_PALSET_SOFT, 0);

	pthread_mutex_lock (&mut_image);
	pthread_mutex_lock (&mut_mouse);

	if (!phimage)
		return;

	if (opt.fullscreen)
	{
		if (area->ul.x > ph_mouse.pos.x)
			area->ul.x = ph_mouse.pos.x;
		if (area->ul.y > ph_mouse.pos.y)
			area->ul.y = ph_mouse.pos.y;
		if (area->lr.x < ph_mouse.pos.x + PH_MOUSE_CURSOR_W)
			area->lr.x = ph_mouse.pos.x + PH_MOUSE_CURSOR_W;
		if (area->lr.y < ph_mouse.pos.y + PH_MOUSE_CURSOR_H)
			area->lr.y = ph_mouse.pos.y + PH_MOUSE_CURSOR_H;
	}

	ph_show_mouse_image();

	PgDrawPhImageRectmx (&(area->ul), phimage, area, 0);

        PgFlush();

	ph_hide_mouse_image();

	pthread_mutex_unlock (&mut_mouse);
	pthread_mutex_unlock (&mut_image);
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

	ph_draw_image (&draw_area);

	PtClipRemove();

}

static int ph_keypress_cb (PtWidget_t *widget, void *data, PtCallbackInfo_t *cb)
{
	PhKeyEvent_t *key_event;
	int key = 0;

	key_event = PhGetData(cb->event);
	if (key_event->key_flags & Pk_KF_Key_Down)
	{	
		if (key_control || key_alt)
			key = key_event->key_cap;
		else
			key = key_event->key_sym;

		switch (key) 
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
			case Pk_KP_8:
				key = KEY_UP;
				break;
			case Pk_Left:
			case Pk_KP_4:
				key = KEY_LEFT;
				break;
			case Pk_Down:
			case Pk_KP_2:
				key = KEY_DOWN;
				break;
			case Pk_Right:
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
			case Pk_Tab:
			case Pk_KP_Tab:
				key = 0x0009;
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
			case Pk_F11:
				key = KEY_STATUSLN;
				break;
			case Pk_F12:
				key = KEY_PRIORITY;
				break;
			case Pk_Return:
				key = 0x0d;
				break;
			case Pk_Escape:
				key = 0x1b;
				break;
			case Pk_BackSpace:
				key = KEY_BACKSPACE;
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
	} else {
		key = 0;
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


	if (key) {
		pthread_sleepon_lock();

		key_enqueue (key);

		pthread_sleepon_signal (key_queue);
		pthread_sleepon_unlock();

	}

	return (Pt_CONSUME);
}

static int ph_mouse_cb (PtWidget_t *widget, void *data, PtCallbackInfo_t *cb)
{
	PhPointerEvent_t *mouse_event;
	PhRect_t area;

	mouse_event = PhGetData(cb->event);

	/* Don't modify global variables from the Photon thread! */
	pthread_mutex_lock (&mut_mouse);

	if (cb->event->type == Ph_EV_BUT_PRESS)
	{
		ph_mouse.sarien_mouse.button = (mouse_event->buttons == Ph_BUTTON_SELECT) ? 1 : 2;
		pthread_sleepon_lock();

		key_enqueue ((mouse_event->buttons == Ph_BUTTON_SELECT) 
						? BUTTON_LEFT : BUTTON_RIGHT);
		pthread_sleepon_signal (key_queue);
		pthread_sleepon_unlock();
	}

        /* If all buttons released */
	if (cb->event->type == Ph_EV_BUT_RELEASE && mouse_event->button_state == 0)
		ph_mouse.sarien_mouse.button = FALSE;

	ph_mouse.sarien_mouse.x = PhGetRects(cb->event)->ul.x / opt.scale;
	ph_mouse.sarien_mouse.y = PhGetRects(cb->event)->ul.y / opt.scale;

#if 0
	/* FIXME: Photon driver doesn't support this flag yet */
        if (opt.fixratio)
		ph_mouse.sarien_mouse.y = ph_mouse.sarien_mouse.y * 5 / 6;
#endif

	if (opt.fullscreen)
	{
		// Include old mouse position when updating screen.
		area.ul = ph_mouse.pos;
		area.lr.x = ph_mouse.pos.x + PH_MOUSE_CURSOR_W;
		area.lr.y = ph_mouse.pos.y + PH_MOUSE_CURSOR_H;
		ph_mouse.pos = PhGetRects(cb->event)->ul;
		pthread_mutex_unlock (&mut_mouse);

		ph_draw_image(&area);
	}
	else
	{
		pthread_mutex_unlock (&mut_mouse);
	}

        return (Pt_CONSUME);
}	

static int ph_close_cb (PtWidget_t *widget, void *data, PtCallbackInfo_t *cb)
{
	deinit_machine();
	deinit_vidmode();
	PtExit(0);

	return (Pt_CONTINUE);
}

static int ph_cause_damage (void *data, int rcvid, void *message, size_t mbsize)
{
	PhRect_t *rect;

	rect = message;
	if (opt.fullscreen)
		ph_draw_image (rect);
	else
		PtDamageExtent (rawwidget, rect);

	MsgReply(rcvid, EOK, NULL, 0);

	return (Pt_CONTINUE);
}

static void ph_setup_mouse_image(void)
{
	PhDim_t imgsize = { PH_MOUSE_CURSOR_W, PH_MOUSE_CURSOR_H };

	ph_mouse.behind_img = PhCreateImage (NULL, imgsize.w, imgsize.h,
		Pg_IMAGE_PALETTE_BYTE, ph_pal, 32, 1);
}

void ph_show_mouse_image(void)
{
	static PmMemoryContext_t *behindmc = NULL;
	int i, j, apos;

	PhDim_t imgsize = { PH_MOUSE_CURSOR_W, PH_MOUSE_CURSOR_H };
	PhPoint_t pos = { 0, 0 };
	PhRect_t area;

	if (!opt.fullscreen)
		return;

	if (behindmc == NULL)
		behindmc = PmMemCreateMC(ph_mouse.behind_img, &imgsize, &pos);

	area.ul = ph_mouse.pos;
	area.lr.x = ph_mouse.pos.x + PH_MOUSE_CURSOR_W;
	area.lr.y = ph_mouse.pos.y + PH_MOUSE_CURSOR_H;

	PmMemStart (behindmc);
	PgDrawPhImageRectmx (&pos, phimage, &area, 0);
	PmMemFlush (behindmc, ph_mouse.behind_img);
	PmMemStop (behindmc);	

	for (i = 0; i < PH_MOUSE_CURSOR_W; i ++)
	{
		for (j = 0; j < PH_MOUSE_CURSOR_H; j ++)
		{
			apos = i + (j * PH_MOUSE_CURSOR_W);
			if (ph_cursor_img_data[apos] != PH_MOUSE_TRANS_COL)
				PiSetPixel (phimage, i + ph_mouse.pos.x, 
					j + ph_mouse.pos.y, ph_cursor_img_data[apos]);
		}
	}
}

void ph_hide_mouse_image (void)
{
	static PmMemoryContext_t *imgmc = NULL;
	PhPoint_t pos = { 0, 0 };

	if (!opt.fullscreen)
		return;

	if (imgmc == NULL)
		imgmc = PmMemCreateMC (phimage, &(phimage->size), &pos);

	PmMemStart (imgmc);
	PgDrawPhImage (&(ph_mouse.pos), ph_mouse.behind_img, 0);
	PmMemFlush (imgmc, phimage);
	PmMemStop (imgmc);
}
