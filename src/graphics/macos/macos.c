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
	macos_put_pixels,
	macos_timer,
	macos_keypress,
	macos_get_key
};


static Rect windRect, dragRect;
static QDGlobals qd;
static WindowPtr myWindow, whichWindow;
static EventRecord myEvent;
static char theChar;
static RGBColor rgb_color[32];


#define KEY_QUEUE_SIZE 16

static int key_queue[KEY_QUEUE_SIZE];
static int key_queue_start = 0;
static int key_queue_end = 0;

#define key_enqueue(k) do { key_queue[key_queue_end++] = (k); \
	key_queue_end %= KEY_QUEUE_SIZE; } while (0)
#define key_dequeue(k) do { (k) = key_queue[key_queue_start++]; \
	key_queue_start %= KEY_QUEUE_SIZE; } while (0)


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


static void process_events ()
{
	SystemTask();

	if (WaitNextEvent (everyEvent, &myEvent, 5L, NULL)) {
		switch (myEvent.what) {
		case mouseDown:
			switch (FindWindow(myEvent.where, &whichWindow)) {
			case inSysWindow:
				/* desk accessory window: call Desk Manager
				 * to handle it
				 */
				SystemClick (&myEvent, whichWindow);
				break;
			case inMenuBar:
				/* Menu bar: learn which command, then
				 * execute it.
				 */
				/*DoCommand(MenuSelect(myEvent.where));*/
				break;
			case inDrag:
				/* title bar: call Window Manager to drag */
				DragWindow (whichWindow, myEvent.where,
					&dragRect);
				break;
			case inContent:
				/* body of application window:
				 * make it active if not
				 */
				if (whichWindow != FrontWindow())
					SelectWindow (whichWindow);
				break;
			}
			break;
		case updateEvt:		/* Update window. */
			if ((WindowPtr) myEvent.message == myWindow) {
				BeginUpdate((WindowPtr) myEvent.message);
				/* repaint */
				EndUpdate((WindowPtr) myEvent.message);
			}
			break;
		case keyDown:
		case autoKey:	/* key pressed once or held down to repeat */
			if (myWindow == FrontWindow())
				theChar = (myEvent.message & charCodeMask);
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
		rgb_color[i].red   = (int)palette[i * 3] << 2;
		rgb_color[i].green = (int)palette[i * 3 + 1] << 2;
		rgb_color[i].blue  = (int)palette[i * 3 + 2] << 2;
	}

	windRect = qd.screenBits.bounds;
	InsetRect (&windRect, 10, 10);
	myWindow = NewCWindow (nil, &windRect, "Sarien", true, documentProc, 
		(WindowPtr) -1, false, 0);
		
	SetPort (myWindow);	/* set window to current graf port */

	return err_OK;
}


static int macos_deinit_vidmode ()
{
	DisposeWindow (myWindow);
	return err_OK;
}


/* blit a block onto the screen */
static void macos_put_block (int x1, int y1, int x2, int y2)
{
	unsigned int h, w, p;

	if (x1 >= GFX_WIDTH)  x1 = GFX_WIDTH  - 1;
	if (y1 >= GFX_HEIGHT) y1 = GFX_HEIGHT - 1;
	if (x2 >= GFX_WIDTH)  x2 = GFX_WIDTH  - 1;
	if (y2 >= GFX_HEIGHT) y2 = GFX_HEIGHT - 1;

	h = y2 - y1 + 1;
	w = x2 - x1 + 1;
	p = GFX_WIDTH * y1 + x1;
}


static void macos_put_pixels(int x, int y, int w, UINT8 *p)
{
	struct RGBColor *c;

	if (w == 0)
		return;

	while (w--) {
		c = &rgb_color[*p++];
		SetCPixel (x++, y, c);
	}
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

