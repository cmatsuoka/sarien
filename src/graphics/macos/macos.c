/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#include <Types.h>
#include <Memory.h>
#include <Quickdraw.h>
#include <Fonts.h>
#include <Events.h>
#include <Menus.h>
#include <Windows.h>
#include <TextEdit.h>
#include <Dialogs.h>
#include <ToolUtils.h>
#include <Processes.h>
#include <Timer.h>
#include "sarien.h"
#include "graphics.h"


extern struct gfx_driver *gfx;
extern struct sarien_options opt;


static int	macos_init_vidmode	(void);
static int	macos_deinit_vidmode	(void);
static void	macos_put_block		(int, int, int, int);
static void	macos_put_pixels	(int, int, int, UINT8 *);
static void	macos_timer		(void);
static int	macos_get_key		(void);
static int	macos_keypress		(void);


static struct gfx_driver gfx_macos = {
	macos_init_vidmode,
	macos_deinit_vidmode,
	macos_put_block,
	NULL,				/* put_pixels */
	macos_timer,
	macos_keypress,
	macos_get_key
};


static QDGlobals qd;
static WindowPtr window;
static RGBColor rgb_color[32];
static GWorldPtr gworld;
static PixMapHandle pix, wpix;
static UINT8 *screen_buffer;


#define KEY_QUEUE_SIZE 16

static int key_queue[KEY_QUEUE_SIZE];
static int key_queue_start = 0;
static int key_queue_end = 0;

#define key_enqueue(k) do { key_queue[key_queue_end++] = (k); \
	key_queue_end %= KEY_QUEUE_SIZE; } while (0)
#define key_dequeue(k) do { (k) = key_queue[key_queue_start++]; \
	key_queue_start %= KEY_QUEUE_SIZE; } while (0)

#define ASPECT_RATIO(x) ((x) * 6 / 5)


#ifdef __MPW__

char *strdup (char *s)
{
	char *r = malloc (strlen (s) + 1);
	strcpy (r, s);
	return r;
}

#endif


int init_machine (int argc, char **argv)
{
	gfx = &gfx_macos;
	return err_OK;
}


int deinit_machine ()
{
	return err_OK;
}


/* ===================================================================== */
 
/* Optimized wrappers to access the offscreen pixmap directly.
 * From my raster star wars scroller screensaver.
 */

/* In the normal 8/15/16/24/32 bpp cases idx indexes the data item directly.
 * x and y are available for the other depths.
 */
static INLINE void putpixel_32 (XImage *img, int idx, int p)
{
	((int *)screen_buffer)[idx] = p;
}

static INLINE void putpixel_16 (XImage *img, int idx, int p)
{
	((short *)screen_buffer)[idx] = p;
}

static INLINE void putpixel_8 (XImage *img, int idx, int p)
{
	((char *)screen_buffer)[idx] = p;
}

/* ===================================================================== */

/* Standard put pixels handlers */

#define _putpixels_scale1(d) static void \
_putpixels_##d##bits_scale1 (int x, int y, int w, UINT8 *p) { \
	if (w == 0) return; \
	x += y * GFX_WIDTH; \
	while (w--) { putpixel_##d## (ximage, x++, rgb_palette[*p++]); } \
}

#define _putpixels_scale2(d) static void \
_putpixels_##d##bits_scale2 (int x, int y, int w, UINT8 *p) { \
	register int c; if (w == 0) return; \
	x <<= 1; y <<= 1; \
	x += y * (GFX_WIDTH << 1); \
	y = x + (GFX_WIDTH << 1); \
	while (w--) { \
		c = rgb_palette[*p++]; \
		putpixel_##d## (ximage, x++, c); \
		putpixel_##d## (ximage, x++, c); \
		putpixel_##d## (ximage, y++, c); \
		putpixel_##d## (ximage, y++, c); \
	} \
}

_putpixels_scale1(8);
_putpixels_scale1(16);
_putpixels_scale1(32);
_putpixels_scale2(8);
_putpixels_scale2(16);
_putpixels_scale2(32);

/* ===================================================================== */

/* Aspect ratio correcting put pixels handlers */

#define _putpixels_fixratio_scale1(d) static void \
_putpixels_fixratio_##d##bits_scale1 (int x, int y, int w, UINT8 *p) { \
	if (y > 0 && ASPECT_RATIO (y) - 1 != ASPECT_RATIO (y - 1)) \
		_putpixels_##d##bits_scale1 (x, ASPECT_RATIO(y) - 1, w, p);\
	_putpixels_##d##bits_scale1 (x, ASPECT_RATIO(y), w, p); \
}

#define _putpixels_fixratio_scale2(d) static void \
_putpixels_fixratio_##d##bits_scale2 (int x, int y, int w, UINT8 *p0) { \
	register int c; int extra = 0, z; UINT8 *p; \
	if (w == 0) return; \
	x <<= 1; y <<= 1; \
	if (y < ((GFX_WIDTH - 1) << 2) && ASPECT_RATIO (y) + 2 != ASPECT_RATIO (y + 2)) extra = w; \
	y = ASPECT_RATIO(y); \
	x += y * (GFX_WIDTH << 1); \
	y = x + (GFX_WIDTH << 1); \
	z = x + (GFX_WIDTH << 2); \
	for (p = p0; w--; ) { \
		c = rgb_palette[*p++]; \
		putpixel_##d## (ximage, x++, c); \
		putpixel_##d## (ximage, x++, c); \
		putpixel_##d## (ximage, y++, c); \
		putpixel_##d## (ximage, y++, c); \
	} \
	for (p = p0; extra--; ) { \
		c = rgb_palette[*p++]; \
		putpixel_##d## (ximage, z++, c); \
		putpixel_##d## (ximage, z++, c); \
	} \
}

_putpixels_fixratio_scale1 (8);
_putpixels_fixratio_scale1 (16);
_putpixels_fixratio_scale1 (32);
_putpixels_fixratio_scale2 (8);
_putpixels_fixratio_scale2 (16);
_putpixels_fixratio_scale2 (32);

/* ===================================================================== */


static void process_events ()
{
	WindowPtr win;
	EventRecord event;
	Rect drag_rect;

	SystemTask();

	if (WaitNextEvent (everyEvent, &event, 5L, NULL)) {
		switch (event.what) {
		case mouseDown:
			switch (FindWindow(event.where, &win)) {
			case inSysWindow:
				/* desk accessory window: call Desk Manager
				 * to handle it
				 */
				SystemClick (&event, win);
				break;
			case inMenuBar:
				/* Menu bar: learn which command, then
				 * execute it.
				 */
				/*DoCommand(MenuSelect(event.where));*/
				break;
			case inDrag:
				/* title bar: call Window Manager to drag */
				DragWindow (win, event.where,
					&drag_rect);
				break;
			case inContent:
				/* body of application window:
				 * make it active if not
				 */
				if (win != FrontWindow())
					SelectWindow (win);
				break;
			}
			break;
		case updateEvt:		/* Update window. */
			if ((WindowPtr) event.message == window) {
				BeginUpdate((WindowPtr) event.message);
				/* repaint */
				EndUpdate((WindowPtr) event.message);
			}
			break;
		case keyDown:
		case autoKey:	/* key pressed once or held down to repeat */
			if (window == FrontWindow()) {
				int c = (event.message & charCodeMask);
				key_enqueue (c);
			}
			break;
		}
	}
}


static unsigned long delta ()
{ 
	unsigned long dt;
	UnsignedWide a;
	static UnsignedWide b;
 
	Microseconds (&a);

	if (a.hi > b.hi) {
		dt = (0xffffffff - (b.lo - a.lo)) / 1000;
	} else {
		dt = (a.lo - b.lo) / 1000;
	}
 
	b.lo = a.lo;
	b.hi = a.hi;

	return dt;
}


static void macos_timer ()
{
	while (delta () < 42) {
		/* usleep (5000); */
	}

	process_events ();
}


static int macos_init_vidmode ()
{
	OSErr error;
	Rect window_rect;
	Rect gworld_rect;
	GDHandle old_gd;
	GWorldPtr old_gw;
	SysEnvRec theWorld;
	int i;
	
	error = SysEnvirons (1, &theWorld);
	if (theWorld.hasColorQD == false)
		return -1;
	
	/* Initialize all the needed managers. */
	InitGraf (&qd.thePort);
	InitFonts ();
	InitWindows ();
	InitMenus ();
	TEInit ();
	InitDialogs (nil);
	InitCursor ();

	/* Set palette */
	for (i = 0; i < 32; i++) {
		rgb_color[i].red   = (int)palette[i * 3] << 10;
		rgb_color[i].green = (int)palette[i * 3 + 1] << 10;
		rgb_color[i].blue  = (int)palette[i * 3 + 2] << 10;
	}

	/* Create offscreen pixmap */
	SetRect (&gworld_rect, 0, 0, GFX_WIDTH, GFX_HEIGHT);
	GetGWorld (&old_gw, &old_gd);
	if (NewGWorld (&gworld, 16, &gworld_rect, NULL, NULL, 0) != noErr)
		return -1;
	LockPixels (gworld->portPixMap);
	SetGWorld (gworld, NULL);
	FillCRect (&gworld_rect, gEmptyPat);
	SetGWorld (old_gw, old_gd);
	UnlockPixels (gworld->portPixMap);

	/* Set optimized put_pixels handler */
	gfx_macos.put_pixels = _putpixels_16bits_scale1;

	/* Initialize pixmap pointers */
	pix = GetGWorldPixMap (gworld);
	wpix = ((CGrafPort *)window)->portPixMap;
	screen_buffer = (UINT8 *)GetPixBaseAddr(pix);

	/* Create window */
	SetRect (&window_rect, 50, 50, 50 + 320, 50 + 200);
	window = NewCWindow (NULL, &window_rect, "\pSarien", true,
		noGrowDocProc, (WindowPtr) -1, false, NULL);
	SetPort (window);	/* set window to current graf port */

	return err_OK;
}


static int macos_deinit_vidmode ()
{
	DisposePtr (gworld);
	DisposeWindow (window);
	return err_OK;
}


/* blit a block onto the screen */
static void macos_put_block (int x1, int y1, int x2, int y2)
{
	Rect sr, dr;

	if (x1 >= GFX_WIDTH)  x1 = GFX_WIDTH  - 1;
	if (y1 >= GFX_HEIGHT) y1 = GFX_HEIGHT - 1;
	if (x2 >= GFX_WIDTH)  x2 = GFX_WIDTH  - 1;
	if (y2 >= GFX_HEIGHT) y2 = GFX_HEIGHT - 1;

	SetRect (&sr, x1, y1, x2, y2);
	SetRect (&dr, x1, y1, x2, y2);

	CopyBits ((BitMap *)*pix, (BitMap *)*wpix, &sr, &dr, srcCopy, 0L);
}


static int macos_keypress ()
{
	process_events ();
	return key_queue_start != key_queue_end;
}


static int macos_get_key ()
{
	UINT16 k;

	while (key_queue_start == key_queue_end)	/* block */
		macos_timer ();

	key_dequeue(k);

	return k;
}

