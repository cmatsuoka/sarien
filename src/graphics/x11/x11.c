/* Sarien - A Sierra AGI resource interpreter engine
 * Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  
 * $Id$
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; see docs/COPYING for further details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>
#include <unistd.h>

#include <signal.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#ifdef MITSHM
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
static XShmSegmentInfo shminfo;
#endif

#include "sarien.h"
#include "graphics.h"
#include "keyboard.h"

extern struct sarien_options opt;
extern struct gfx_driver *gfx;

static Display *display;
static Visual *visual;
static int screen;
static Screen *scrptr;
static Colormap colormap;
static XSetWindowAttributes attributes;
static unsigned long attribute_mask;
static int depth;
static GC gc;
static XImage *ximage;
static Window window, root;
static XColor color[32];

static int scale = 1;
static int key_control = 0;
static int key_alt = 0;
static unsigned int rgb_palette[32];

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
static void	x11_put_block	(int, int, int, int);
static void	_putpixels_anybits_scaleany
				(int, int, int, UINT8 *);
static int	x11_keypress	(void);
static int	x11_get_key	(void);
static void	x11_timer	(void);

static struct gfx_driver gfx_x11 = {
	init_vidmode,
	deinit_vidmode,
	x11_put_block,
	_putpixels_anybits_scaleany,
	x11_timer,
	x11_keypress,
	x11_get_key
};

#ifdef XF86DGA
#include "dga.c"
#endif


#define ASPECT_RATIO(x) ((x + 1) * 6 / 5 - 1)


/* ===================================================================== */
 
/* Optimized wrappers to access the X image directly.
 * From my raster star wars scroller screensaver.
 */

/* In the normal 8/15/16/24/32 bpp cases idx indexes the data item directly.
 * x and y are available for the other depths.
 */
static INLINE void putpixel_32 (XImage *img, int idx, int p)
{
	((int *)img->data)[idx] = p;
}

static INLINE void putpixel_16 (XImage *img, int idx, int p)
{
	((short *)img->data)[idx] = p;
}

static INLINE void putpixel_8 (XImage *img, int idx, int p)
{
	((char *)img->data)[idx] = p;
}

static void _putpixels_8bits_scale1 (int x, int y, int w, UINT8 *p)
{
	if (w == 0) return;
	x += y * GFX_WIDTH;
	while (w--) { putpixel_8 (ximage, x++, rgb_palette[*p++]); }
}

static void _putpixels_8bits_scale2 (int x, int y, int w, UINT8 *p)
{
	register int c;

	if (w == 0) return;

	x <<= 1; y <<= 1;
	x += y * (GFX_WIDTH << 1);
	y = x + (GFX_WIDTH << 1);

	while (w--) {
		c = rgb_palette[*p++];
		putpixel_8 (ximage, x++, c);
		putpixel_8 (ximage, x++, c);
		putpixel_8 (ximage, y++, c);
		putpixel_8 (ximage, y++, c);
	}
}

static void _putpixels_16bits_scale1 (int x, int y, int w, UINT8 *p)
{
	if (w == 0) return;
	x += y * GFX_WIDTH;
	while (w--) { putpixel_16 (ximage, x++, rgb_palette[*p++]); }
}

static void _putpixels_16bits_scale2 (int x, int y, int w, UINT8 *p)
{
	int c;

	if (w == 0) return;

	x <<= 1; y <<= 1;
	x += y * (GFX_WIDTH << 1);
	y = x + (GFX_WIDTH << 1);

	while (w--) {
		c = rgb_palette[*p++];
		putpixel_16 (ximage, x++, c);
		putpixel_16 (ximage, x++, c);
		putpixel_16 (ximage, y++, c);
		putpixel_16 (ximage, y++, c);
	}
}

static void _putpixels_32bits_scale1 (int x, int y, int w, UINT8 *p)
{
	if (w == 0) return;
	x += y * GFX_WIDTH;
	while (w--) { putpixel_32 (ximage, x++, rgb_palette[*p++]); }
}

static void _putpixels_32bits_scale2 (int x, int y, int w, UINT8 *p)
{
	int c;

	if (w == 0) return;

	x <<= 1; y <<= 1;
	x += y * (GFX_WIDTH << 1);
	y = x + (GFX_WIDTH << 1);

	while (w--) {
		c = rgb_palette[*p++];
		putpixel_32 (ximage, x++, c);
		putpixel_32 (ximage, x++, c);
		putpixel_32 (ximage, y++, c);
		putpixel_32 (ximage, y++, c);
	}
}

/* ===================================================================== */

#define _putpixels_fixratio(b,s) static void \
_putpixels_fixratio_##b##bits_scale##s## (int x, int y, int w, UINT8 *p) { \
	if (y > 0 && ASPECT_RATIO (y) - 1 != ASPECT_RATIO (y - 1)) \
		_putpixels_##b##bits_scale##s## (x, ASPECT_RATIO(y) - 1, w, p);\
	_putpixels_##b##bits_scale##s## (x, ASPECT_RATIO(y), w, p); \
}

_putpixels_fixratio (8,1);
_putpixels_fixratio (16,1);
_putpixels_fixratio (32,1);
_putpixels_fixratio (8,2);
_putpixels_fixratio (16,2);
_putpixels_fixratio (32,2);
_putpixels_fixratio (any,any);

/* ===================================================================== */


static void _putpixels_anybits_scaleany (int x, int y, int w, UINT8 *p)
{
	register int cp;
	register int i, j;

	if (w == 0)
		return;

	if (scale == 1) {
		while (w--) {
			cp = rgb_palette[*p++];
			x += y * GFX_WIDTH;
			if (depth == 8) putpixel_8 (ximage, x++, cp);
		}
	} else if (scale == 2) {
		x <<= 1;
		y <<= 1;
		while (w--) {
			cp = rgb_palette[*p++];
			XPutPixel (ximage, x, y, cp);
			XPutPixel (ximage, x++, y + 1, cp);
			XPutPixel (ximage, x, y, cp);
			XPutPixel (ximage, x++, y + 1, cp);
		}
	} else {
		x *= scale;
		y *= scale;
		while (w--) {
			cp = rgb_palette[*p++];
			for (i = 0; i < scale; i++)
				for (j = 0; j < scale; j++)
					XPutPixel (ximage, x + i, y + j, cp);
		}
	}
}


static void process_events ()
{
	static XEvent event;
	int key = 0;
	KeySym k;
	char buf[3];

	while (XEventsQueued (display, QueuedAfterReading)) {
		XNextEvent (display, &event);
		key = 0;
		switch (event.type) {
		case Expose:
#ifdef MITSHM
			if (opt.mitshm) {
				XShmPutImage (display, window, gc, ximage,
					event.xexpose.x, event.xexpose.y,
					event.xexpose.x, event.xexpose.y,
					event.xexpose.width,
					event.xexpose.height, 0);
			} else
#endif
			{
				XPutImage (display, window, gc, ximage,
					event.xexpose.x, event.xexpose.y,
					event.xexpose.x, event.xexpose.y,
					event.xexpose.width,
					event.xexpose.height);
			}
			break;
		case KeyPress:
			XLookupString (&event.xkey, buf, 1, &k, NULL);
			switch (key = k) {
			case XK_BackSpace:
				key = KEY_BACKSPACE;
				break;
			case XK_Shift_L:
			case XK_Shift_R:
				key = 0;
				break;
			case XK_Control_L:
				key_control |= 1;
				key = 0;
				break;
			case XK_Control_R:
				key_control |= 2;
				key = 0;
				break;
			case XK_Alt_L:
				key_alt |= 1;
				key = 0;
				break;
			case XK_Alt_R:
				key_alt |= 2;
				key = 0;
				break;
			case XK_Up:
			case XK_KP_Up:
			case XK_KP_8:
				key = KEY_UP;
				break;
			case XK_Left:
			case XK_KP_Left:
			case XK_KP_4:
				key = KEY_LEFT;
				break;
			case XK_Down:
			case XK_KP_Down:
			case XK_KP_2:
				key = KEY_DOWN;
				break;
			case XK_Right:
			case XK_KP_Right:
			case XK_KP_6:
				key = KEY_RIGHT;
				break;
			case XK_KP_Home:
			case XK_KP_7:
				key = KEY_UP_LEFT;
				break;
			case XK_KP_Page_Up:
			case XK_KP_9:
				key = KEY_UP_RIGHT;
				break;
			case XK_KP_Page_Down:
			case XK_KP_3:
				key = KEY_DOWN_RIGHT;
				break;
			case XK_KP_End:
			case XK_KP_1:
				key = KEY_DOWN_LEFT;
				break;
			case XK_KP_Enter:
				key = KEY_ENTER;
				break;
			case XK_KP_Add:
				key = '+';
				break;
			case XK_KP_Subtract:
				key = '-';
				break;
			case XK_F1:
				key = 0x3b00;
				break;
			case XK_F2:
				key = 0x3c00;
				break;
			case XK_F3:
				key = 0x3d00;
				break;
			case XK_F4:
				key = 0x3e00;
				break;
			case XK_F5:
				key = 0x3f00;
				break;
			case XK_F6:
				key = 0x4000;
				break;
			case XK_F7:
				key = 0x4100;
				break;
			case XK_F8:
				key = 0x4200;
				break;
			case XK_F9:
				key = 0x4300;
				break;
			case XK_F10:
				key = 0x4400;
				break;
			case XK_Return:
				key = 0x0d;
				break;
			case XK_Escape:
				key = 0x1b;
				break;
			default:
				if (!isalpha (key & 0xff))
					break;
				if (key_control)
					key = (key & ~0x20) - 0x40;
				else if (key_alt)
					key = scancode_table[(key & ~0x20)
						- 0x41] << 8;
				break;
			};
			_D ("key = 0x%02x ('%c')", key,
				isprint(key & 0xff) ? key & 0xff: '?');
			break;
		case KeyRelease:
			XLookupString (&event.xkey, buf, 1, &k, NULL);
			switch (k) {
			case XK_Control_L:
				key_control &= ~1;
				break;
			case XK_Control_R:
				key_control &= ~2;
				break;
			case XK_Alt_L:
				key_alt &= ~1;
				break;
			case XK_Alt_R:
				key_alt &= ~2;
				break;
			}
			break;
		}
		if (key)
			key_enqueue (key);
	}

	XFlush (display);
}



static int set_palette (UINT8 *pal, int scol, int numcols)
{
	int i;

	fprintf (stderr, "x11: visual is ");
	if (visual->class == PseudoColor && depth == 8)
		fprintf (stderr, "8 bpp pseudocolor\n");
	else if (visual->class == TrueColor && depth == 15)
		fprintf (stderr, "15 bpp true color\n");
	else if (visual->class == TrueColor && depth == 16)
		fprintf (stderr, "16 bpp true color\n");
	else if (visual->class == TrueColor && depth == 24)
		fprintf (stderr, "24 bpp true color\n");
	else {
		fprintf (stderr, "unknown\n");
		return err_Unk;
	}

	for (i = scol; i < scol + numcols; i++) {
		color[i].red = pal[i * 3] << 10;
		color[i].green = pal[i * 3 + 1] << 10;
		color[i].blue = pal[i * 3 + 2] << 10;
		if (!XAllocColor (display, colormap, &color[i]))
			fprintf (stderr, "x11: cannot allocte color cell\n");

		/* Palette color fixed by Sad Rejeb <sadrejeb@hotmail.com>
		 * Sat, 24 Jul 1999 01:23:33 CEST
		 */
		switch (depth) {
		case 8:
			rgb_palette[i] = color[i].pixel;
			break;
		case 15:
			rgb_palette[i] =
				((int)(pal[i * 3] & 0x3e) << 9) |
				((int)(pal[i * 3 + 1] & 0x3e) << 4) |
				((int)(pal[i * 3 + 2] & 0x3e) >> 1);
			break;
		case 16:
			rgb_palette[i] =
				((int)(pal[i * 3] & 0x3e) << 10) |
				((int)(pal[i * 3 + 1] & 0x3f) << 5) |
				((int)(pal[i * 3 + 2] & 0x3e) >> 1);
			break;
		case 24:
			rgb_palette[i] =
				((int) pal[i * 3] << 18) |
				((int) pal[i * 3 + 1] << 10) |
				((int)pal[i * 3 + 2]) << 2;
			break;
		}
	}

	return err_OK;
}


static int kill_flag = 0;	/* Yuck */
static void kill_mode (int i)
{
	if (kill_flag)
		exit (0);

	kill_flag = 1;
	fprintf (stderr, "Fatal: signal %d caught\n", i);
	gfx->deinit_video_mode ();

	exit (-1);
}


static int init_vidmode ()
{
	/*Pixmap icon; */
	XWMHints hints;
	XSizeHints sizehints;
	XClassHint classhint;
	XTextProperty appname, iconname;
	/*XpmAttributes attributes; */
	char *apptext = TITLE " " VERSION;
	char *icontext = TITLE;

	_D ("()");

	fprintf (stderr, "x11: X11 support by claudio@helllabs.org\n");

	if ((display = XOpenDisplay (NULL)) == NULL) {
		fprintf (stderr, "x11: no connection to server\n");
		return err_Unk;
	}
	screen = DefaultScreen (display);
	scrptr = DefaultScreenOfDisplay (display);
	visual = DefaultVisual (display, screen);
	root = DefaultRootWindow (display);
	depth = DefaultDepth (display, screen);
	colormap = DefaultColormap (display, screen);

#ifdef XF86DGA
	if (opt.fullscreen) {
		if (dga_init_vidmode () == err_OK)
			goto init_done;
		opt.fullscreen = FALSE;
	}
#endif

	attribute_mask = CWEventMask;
	attributes.event_mask |= ExposureMask | KeyPressMask | KeyReleaseMask;

	window = XCreateWindow (
		display, root, 0, 0, GFX_WIDTH * scale,
		(opt.fixratio ? ASPECT_RATIO(GFX_HEIGHT) : GFX_HEIGHT) * scale,
		1, depth, InputOutput, CopyFromParent,
		attribute_mask,&attributes);

	if (!window) {
		fprintf (stderr, "x11: can't create window\n");
		return err_Unk;
	}

	/*attributes.valuemask = XpmSize; */
	/*XpmCreatePixmapFromData (display,root,xmdp_icon,&icon,NULL,&attributes); */
	XStringListToTextProperty (&apptext, 1, &appname);
	XStringListToTextProperty (&icontext, 1, &iconname);
	sizehints.flags = PSize | PMinSize | PMaxSize;
	sizehints.min_width = sizehints.max_width = GFX_WIDTH * scale;
	sizehints.min_height = sizehints.max_height =
		(opt.fixratio ? ASPECT_RATIO(GFX_HEIGHT) : GFX_HEIGHT) * scale;
	/*hints.icon_pixmap=icon; */
	hints.flags = StateHint | IconPixmapHint | InputHint;
	hints.initial_state = NormalState;
	hints.input = 1;
	classhint.res_name = "sarien";
	classhint.res_class = "Sarien";
	XSetWMProperties (display, window, &appname, &iconname, __argv,
		__argc, &sizehints, &hints, &classhint);

	gc = XCreateGC (display, window, 0, NULL);

	signal (SIGSEGV, kill_mode);
	signal (SIGQUIT, kill_mode);
	signal (SIGFPE, kill_mode);
	signal (SIGTERM, kill_mode);
	signal (SIGINT, kill_mode);

#ifdef MITSHM
	if (opt.mitshm && !XShmQueryExtension (display)) {
		fprintf (stderr,
			"x11: shared memory extension not available\n");
		opt.mitshm = FALSE;
	}

	if (opt.mitshm) {
		int maj, min;
		Bool spix;

		XShmQueryVersion (display, &maj, &min, &spix);
		fprintf (stderr, "x11: using shared memory extension "
			"(version %d.%d)\n", maj, min);
		ximage = XShmCreateImage (display, visual, depth, ZPixmap, NULL,
			&shminfo, GFX_WIDTH * scale, (opt.fixratio ?
			ASPECT_RATIO(GFX_HEIGHT) : GFX_HEIGHT) * scale);

		shminfo.shmid = shmget (IPC_PRIVATE, ximage->bytes_per_line *
			ximage->height, IPC_CREAT | 0600);                              
		if (shminfo.shmid == -1) {
			fprintf (stderr,
				"x11: can't allocate X shared memory\n");
			return err_Unk;
		}
		shminfo.shmaddr = ximage->data = shmat (shminfo.shmid, 0, 0);
		shminfo.readOnly = 0;
		XShmAttach (display, &shminfo);
		fprintf (stderr, "x11: attached shared memory segment [%d]\n",
			shminfo.shmid);
	}
	else
#endif
	{
		ximage = XCreateImage (display, visual, depth, ZPixmap, 0,
			NULL, GFX_WIDTH * scale, (opt.fixratio ?
			ASPECT_RATIO(GFX_HEIGHT) : GFX_HEIGHT) * scale, 8, 0);

		if (ximage == NULL) {
			fprintf (stderr, "x11: can't create image\n");
			return err_Unk;
		}
		ximage->data = malloc (ximage->bytes_per_line * ximage->height);
	}

	if (!ximage) {
		fprintf (stderr, "x11: can't create image\n");
		return err_Unk;
	}

if (opt.fixratio) {
	gfx_x11.put_pixels = _putpixels_fixratio_anybits_scaleany;
	if (opt.gfxhacks) switch (scale) {
	case 1:
		switch (depth) {
		case 8:  gfx_x11.put_pixels = _putpixels_fixratio_8bits_scale1; break;
		case 16: gfx_x11.put_pixels = _putpixels_fixratio_16bits_scale1; break;
		case 24: /* fall-through */
		case 32: gfx_x11.put_pixels = _putpixels_fixratio_32bits_scale1; break;
		}
		break;
	case 2:
		switch (depth) {
		case 8:  gfx_x11.put_pixels = _putpixels_fixratio_8bits_scale2; break;
		case 16: gfx_x11.put_pixels = _putpixels_fixratio_16bits_scale2; break;
		case 24: /* fall-through */
		case 32: gfx_x11.put_pixels = _putpixels_fixratio_32bits_scale2; break;
		}
		break;
	}
} else {
	if (opt.gfxhacks) switch (scale) {
	case 1:
		switch (depth) {
		case 8:  gfx_x11.put_pixels = _putpixels_8bits_scale1; break;
		case 16: gfx_x11.put_pixels = _putpixels_16bits_scale1; break;
		case 24: /* fall-through */
		case 32: gfx_x11.put_pixels = _putpixels_32bits_scale1; break;
		}
		break;
	case 2:
		switch (depth) {
		case 8:  gfx_x11.put_pixels = _putpixels_8bits_scale2; break;
		case 16: gfx_x11.put_pixels = _putpixels_16bits_scale2; break;
		case 24: /* fall-through */
		case 32: gfx_x11.put_pixels = _putpixels_32bits_scale2; break;
		}
		break;
	}
}

	XMapWindow (display, window);
	XSetWindowBackground (display, window, BlackPixel (display, screen));
	XClearWindow (display, window);

#ifdef XF86DGA
init_done:
#endif
	set_palette (palette, 0, 32);
	XSync (display, False);

	return err_OK;
}


static int deinit_vidmode ()
{
	_D ("()");

	fprintf (stderr, "x11: deiniting video mode\n");
	XDestroyWindow (display, window);

#ifdef MITSHM
	if (opt.mitshm) {
		fprintf (stderr, "x11: detaching shared memory segment [%d]\n",
			shminfo.shmid);
		XShmDetach (display, &shminfo);
		shmctl (shminfo.shmid, IPC_RMID, NULL);
		shmdt (shminfo.shmaddr);
	}
#endif
	XDestroyImage (ximage);
	XCloseDisplay (display);

	return err_OK;
}


/* put a block onto the screen */
static void x11_put_block (int x1, int y1, int x2, int y2)
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
		x2 = x2 * scale + scale - 1;
		y2 = y2 * scale + scale - 1;
	}

#ifdef MITSHM
	if (opt.mitshm) {
		if (opt.fixratio) {
			XShmPutImage (display, window, gc, ximage, x1,
				ASPECT_RATIO(y1), x1, ASPECT_RATIO(y1), x2 -
				x1 + scale - 1, ASPECT_RATIO(y2 - y1 + 1), 0);
		} else {
			XShmPutImage (display, window, gc, ximage, x1, y1,
				x1, y1, x2 - x1 + scale - 1, y2 - y1 + 1, 0);
		}
	}
	else
#endif
	{
		if (opt.fixratio) {
			XPutImage (display, window, gc, ximage, x1,
				ASPECT_RATIO(y1), x1, ASPECT_RATIO(y1),
				x2 - x1 + scale - 1, ASPECT_RATIO(y2 - y1 + 1));
		} else {
			XPutImage (display, window, gc, ximage, x1, y1,
				x1, y1, x2 - x1 + scale - 1, y2 - y1 + 1);
		}
	}
	
	XSync (display, False);
}


static int x11_keypress ()
{
	process_events ();
	return key_queue_start != key_queue_end;
}


static int x11_get_key ()
{
	UINT16 k;

	while (key_queue_start == key_queue_end)	/* block */
		x11_timer ();

	key_dequeue(k);

	return k;
}


static void x11_timer ()
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


/*
 * Public functions
 */

int init_machine (int argc, char **argv)
{
	gfx = &gfx_x11;

	__argc = argc;
	__argv = argv;
	scale = opt.scale;

	return err_OK;
}


int deinit_machine (void)
{
	return err_OK;
}

