/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

/*
 * Based on the Amiga graphics output for v2600 Atari 2600 emulator,
 * version 2.2 (September 4, 1997) written by Matthew Stroup
 */

#include <stdlib.h>
#include <string.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <graphics/gfxmacros.h>
#include <graphics/display.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <hardware/custom.h>
#include <hardware/cia.h>
#include "sarien.h"
#include "graphics.h"


extern struct gfx_driver *gfx;
extern struct sarien_options opt;


static int	amiga_init_vidmode	(void);
static int	amiga_deinit_vidmode	(void);
static void	amiga_put_block		(int, int, int, int);
static void	amiga_put_pixels	(int, int, int, UINT8 *);
static void	amiga_timer		(void);
static int	amiga_get_key		(void);
static int	amiga_keypress		(void);


static struct gfx_driver gfx_amiga = {
	amiga_init_vidmode,
	amiga_deinit_vidmode,
	amiga_put_block,
	amiga_put_pixels,
	amiga_timer,
	amiga_keypress,
	amiga_get_key
};

static PLANEPTR bp[7];

static struct Window *win;
static struct Screen *scr;
static struct RastPort *rp;
static PLANEPTR raster = 0;
static struct BitMap bitmap_bm;
static UBYTE *vscreen;

extern struct GfxBase *GfxBase;
extern struct IntuitionBase *IntuitionBase;

static char perm[8] = {0, 1, 2, 3, 4, 5, 6, 7};	// bitplane order



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
	struct IntuiMessage *message;

	message = (struct IntuiMessage *)GetMsg (win->UserPort);
	if (message) {
		if (message->Class == CLOSEWINDOW) {
			exit(0);
		}
	}
}

int init_machine (int argc, char **argv)
{
	gfx = &gfx_amiga;
	return err_OK;
}

int deinit_machine ()
{
	return err_OK;
}

static void amiga_timer ()
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

static int amiga_init_vidmode ()
{
	struct RastPort temprp;
	struct BitMap tempbm;
	int i;

	/* Make sure necessary ROM libraries are open */

	IntuitionBase = (struct IntuitionBase *)
		OpenLibrary ("intuition.library", 36L);
	if (IntuitionBase == NULL)
		return -1;

	GfxBase = (struct GfxBase *) OpenLibrary ("graphics.library", 36L);
	if (GfxBase == NULL)
		return -1;

	vscreen = (UBYTE *)AllocMem (81920, MEMF_PUBLIC | MEMF_CLEAR);

	win = OpenWindowTags (NULL,
		WA_Title, "Sarien" ##VERSION,
		WA_AutoAdjust, TRUE,
		WA_InnerWidth, GFX_WIDTH, 
		WA_InnerHeight, GFX_HEIGHT,
		WA_DragBar, TRUE,
		WA_CloseGadget, TRUE,
		WA_DepthGadget,TRUE,
		WA_SimpleRefresh, TRUE,
		WA_IDCMP,IDCMP_CLOSEWINDOW,
		TAG_DONE);
	if (win == NULL)
		return -1;

	rp = win->RPort;
	bleft = win->BorderLeft;
	btop=win->BorderTop;

	for (i = 0; i < NUMCOLS; i++) {
		colors[i]=ObtainBestPen(win->WScreen->ViewPort.ColorMap,
			(colortable[i*2] & 0xff0000)<<8,
			(colortable[i*2] & 0x00ff00)<<16,
			(colortable[i*2] & 0x0000ff)<<24,
			OBP_Precision,PRECISION_EXACT,
			TAG_DONE);
		wcolors[i]=(colors[i]<<8)|colors[i];
	}

	InitBitMap(&tempbm, 7, vwidth+16, tv_height);

	raster = (PLANEPTR)AllocRaster (GFX_WIDTH + 16, 7 * GFX_HEIGHT);
	if (raster == NULL)
		return -1;

	for (i = 0; i < 7; i++) {
		tempbm.Planes[i] = raster + (i * RASSIZE (GFX_WIDTH + 16,
			GFX_HEIGHT));
		bp[i] = rp->BitMap->Planes[i];
	}
	InitRastPort(&temprp);
	temprp.BitMap=&tempbm;

	return 0;
}

static int amiga_deinit_vidmode ()
{
	CloseWindow (win);
	CloseScreen (scr);
	FreeRaster (raster, GFX_WIDTH + 16, 7 * GFX_HEIGHT);
	FreeMem (vscreen, 81920);
	CloseLibrary ((struct Library *)GfxBase);
	CloseLibrary ((struct Library *)IntuitionBase);

	return 0;
}


/* blit a block onto the screen */
static void amiga_put_block (int x1, int y1, int x2, int y2)
{
	unsigned int h, w;

	if (x1 >= GFX_WIDTH)  x1 = GFX_WIDTH  - 1;
	if (y1 >= GFX_HEIGHT) y1 = GFX_HEIGHT - 1;
	if (x2 >= GFX_WIDTH)  x2 = GFX_WIDTH  - 1;
	if (y2 >= GFX_HEIGHT) y2 = GFX_HEIGHT - 1;

	h = y2 - y1 + 1;
	w = x2 - x1 + 1;

	WritePixelArray8 (rp, x1, y1, x2, y2, vscreen, &temprp);
}


static void amiga_put_pixels(int x, int y, int w, UINT8 *p)
{
	while (w--) { scrdev.DrawPixel (mempsd, x, y++, *p++); }
}


static int amiga_keypress ()
{
	process_events ();
	return key_queue_start != key_queue_end;
}


static int amiga_get_key ()
{
	UINT16 k;

	while (key_queue_start == key_queue_end)	/* block */
		amiga_timer ();

	key_dequeue(k);

	return k;
}

#if 0
---------
Leftovers from the original driver
---------


UBYTE colors[NUMCOLS];
UWORD wcolors[NUMCOLS];

extern int moudrv_x, moudrv_y, moudrv_but;

#define CIAAPRA 0xBFE001

struct CIA *ciamou = (struct CIA *) CIAAPRA;

/*****************************************************************************/

/* The Event code. */
void tv_event (void)
{
	read_keyboard();
}

/*****************************************************************************/

void moudrv_read(void)
{
	moudrv_x=win->MouseX;
	moudrv_y=win->MouseY;
	moudrv_but=!(ciamou->ciapra & 0x0040);
}

/*****************************************************************************/

#endif
