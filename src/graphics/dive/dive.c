/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dive.h>
#include "sarien.h"
#include "graphics.h"


extern struct gfx_driver *gfx;
extern struct sarien_options opt;


static int	dive_init_vidmode	(void);
static int	dive_deinit_vidmode	(void);
static void	dive_put_block		(int, int, int, int);
static void	dive_put_pixels		(int, int, int, UINT8 *);
static void	dive_timer		(void);
static int	dive_get_key		(void);
static int	dive_keypress		(void);


static struct gfx_driver gfx_dive = {
	dive_init_vidmode,
	dive_deinit_vidmode,
	dive_put_block,
	dive_put_pixels,
	dive_timer,
	dive_keypress,
	dive_get_key
};


static HAB hab;
static HMQ hmq;
static HDIVE hdive;
static HWND hwndframe, hwndclient;
static HPS hpsclient;


int init_machine (int argc, char **argv)
{
	gfx = &gfx_dive;
	return err_OK;
}


int deinit_machine ()
{
	return err_OK;
}


static int dive_init_vidmode ()
{
	int reg;
	char *ClientClass;
	int x_screen, y_screen, x_border, y_border, title_size;
	static ULONG flags;

	ClientClass = "SarienWindow";
 	flags = FCF_TITLEBAR | FCF_SYSMENU | FCF_TASKLIST;

	x_screen = WinQuerySysValue(HWND_DESKTOP, SV_CXSCREEN);
	y_screen = WinQuerySysValue(HWND_DESKTOP, SV_CYSCREEN);
	x_border = WinQuerySysValue(HWND_DESKTOP, SV_CXSIZEBORDER);
	y_border = WinQuerySysValue(HWND_DESKTOP, SV_CYSIZEBORDER);
	title_size = WinQuerySysValue(HWND_DESKTOP, SV_CYTITLEBAR);

	hab = WinInitialize (0);
	hmq = WinCreateMsgQueue (hab, 0);

	if (InitializeDive() == FALSE)     /* bail out if no DIVE support */
		return -1;

	reg = WinRegisterClass (hab, ClientClass, ClientWndProc,
		CS_SIZEREDRAW, 0) ;

	hwndframe = WinCreateStdWindow (HWND_DESKTOP, 0, &flags, ClientClass,
		NULL, 0L, 0, 444, &hwndclient);

	hpsclient = WinGetPS (hwndclient);

	width = GFX_WIDTH * scale;
	height = GFX_HEIGHT * scale;

	if (hwndframe) {
		WinSetWindowPos (hwndframe, NULLHANDLE,
			(x_screen - width - 2 * xborderwidth) / 2,
			(y_screen - height - 2 * y_border -
				title_size) / 2,
			width + 2 * x_border,
			height + 2 * y_border + title_size,
			SWP_SIZE|SWP_MOVE|SWP_ACTIVATE|SWP_SHOW);
	}

	while (WinPeekMsg(hab, &qmsg, NULLHANDLE, 0, 0, PM_NOREMOVE)) {
		if (WinGetMsg(hab, &qmsg, NULLHANDLE, 0, 0))
			WinDispatchMsg (hab, &qmsg);
	}

	return (hwndframe) ? 0 : -1;
}


static int dive_deinit_vidmode ()
{
	DiveClose (hdive);
	WinDestroyWindow (hwnd);
	WinDestroyMsgQueue (hmq);
	WinTerminate (hab);

	return 0;
}


/* blit a block onto the screen */
static void dive_put_block (int x1, int y1, int x2, int y2)
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


static void dive_put_pixels(int x, int y, int w, UINT8 *p)
{
	UINT8 *s;
 	for (s = &screen_buffer[y * 320 + x]; w--; *s++ = *p++);
}


static int dive_keypress ()
{
}


static int dive_get_key ()
{
}

