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


static Rect windRect;
static QDGlobals qd;


int init_machine (int argc, char **argv)
{
	gfx = &gfx_macos;
	return err_OK;
}


int deinit_machine ()
{
	return err_OK;
}


static int macos_init_vidmode ()
{
	WindowPtr	mainPtr;
	OSErr		error;
	SysEnvRec	theWorld;
	
	error = SysEnvirons(1, &theWorld);
	if (theWorld.hasColorQD == false)
		return -1;
	
	/* Initialize all the needed managers. */
	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(nil);
	InitCursor();

	windRect = qd.screenBits.bounds;
	InsetRect (&windRect, 50, 50);
	mainPtr = NewCWindow(nil, &windRect, "Sarien", true, documentProc, 
		(WindowPtr) -1, false, 0);
		
	SetPort(mainPtr);	/* set window to current graf port */
}


static int macos_deinit_vidmode ()
{
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
#if 0
	UINT8 *s;
 	for (s = &screen_buffer[y * 320 + x]; w--; *s++ = *p++);
#endif
}


static int macos_keypress ()
{
}


static int macos_get_key ()
{
}


