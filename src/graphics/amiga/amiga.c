/* Sarien - A Sierra AGI resource interpreter engine
 * Amiga files Copyright (C) 1999-2001 Paul Hill
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
#include <devices/timer.h>
#include <sys/time.h>
#ifndef __DICE__
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/gadtools.h>
#endif
#include <graphics/gfxbase.h>
#include <graphics/rastport.h>
#include <intuition/intuition.h>
#include "amiga_keys.h"

#include "sarien.h"
#include "graphics.h"
#include "keyboard.h"

#define DEPTH 6

extern struct sarien_options opt;
extern struct gfx_driver *gfx;

static struct Screen *screen;
struct Window *window;
static UBYTE *ximage;

static struct RastPort video_temprp;
static struct BitMap video_tmp_bm = {
	0, 0, 0, 0, 0,
	{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
};
APTR VisualInfo = NULL;

static int scale = 1;
static int key_control = 0;
static int key_alt = 0;
static unsigned int rgb_palette[32];

static int __argc;
static char **__argv;

#ifdef __DICE__
extern struct GfxBase *GfxBase;
#endif

#define KEY_QUEUE_SIZE 16
static int key_queue[KEY_QUEUE_SIZE];
static int key_queue_start = 0;
static int key_queue_end = 0;
#define key_enqueue(k) do { key_queue[key_queue_end++] = (k); \
	key_queue_end %= KEY_QUEUE_SIZE; } while (0)
#define key_dequeue(k) do { (k) = key_queue[key_queue_start++]; \
	key_queue_start %= KEY_QUEUE_SIZE; } while (0)

static int Amiga_init_vidmode (void);
static int Amiga_deinit_vidmode (void);
static UINT16 set_palette (UINT8 *, UINT16, UINT16);
static void Amiga_blit_block (int, int, int, int);
static void Amiga_put_pixel (int x, int y, int w, UINT8 *p);
static int Amiga_keypress (void);
static int Amiga_get_key (void);
static void Amiga_new_timer (void);


/* From timer.c */
int opentimer();
void closetimer();

static void die_die_die()
{
	gfx->deinit_video_mode ();
	deinit_sound();
	deinit_machine();
	exit(0);
}


static struct gfx_driver GFX_amiga = {
	Amiga_init_vidmode,
	Amiga_deinit_vidmode,
	Amiga_blit_block,
	Amiga_put_pixel,
	Amiga_new_timer,
	Amiga_keypress,
	Amiga_get_key
};

void __chkabort(void){}


static void process_events ()
{
	UWORD Code;
	UWORD Qualifier;
	ULONG Class;
	struct IntuiMessage *imsg;
	int goingup;
	int key = 0;

	while ((imsg = (struct IntuiMessage *)GT_GetIMsg(window->UserPort)))
	{
		Class = imsg->Class;
		Code = imsg->Code;
		Qualifier = imsg->Qualifier;

		/* Get the message that's waiting for us */
		GT_ReplyIMsg(imsg);

		switch (Class)
		{
		case IDCMP_MOUSEBUTTONS:
			switch (Code)
			{
				case IECODE_RBUTTON:
				case IECODE_LBUTTON:
					key = Code == IECODE_LBUTTON ?
						BUTTON_LEFT : BUTTON_RIGHT;
					mouse.button = TRUE;
					mouse.x = (imsg->MouseX - window->BorderLeft) / opt.scale;
					mouse.y = (imsg->MouseY - window->BorderTop) / opt.scale;
					break;
				case IECODE_LBUTTON | IECODE_UP_PREFIX:
				case IECODE_RBUTTON | IECODE_UP_PREFIX:
					mouse.button = FALSE;
					break;
			}
			break;

		case IDCMP_CLOSEWINDOW:
			die_die_die();
			break;

		case IDCMP_VANILLAKEY:
			if ((Code == 13) && (Qualifier == 0x8010))
			{
				/*
				 * [alt + enter] = switch between
				 *	fullscreen and window mode
				 */
				Amiga_deinit_vidmode();
				opt.fullscreen = !opt.fullscreen;
				Amiga_init_vidmode();
				/* re-draw the screen */
printf("..................................................\n");
				Amiga_blit_block (0, 0,
					GFX_WIDTH * scale, GFX_HEIGHT * scale);
printf("..................................................\n");
			} else {
				key = Code;
			}
			break;

		case IDCMP_RAWKEY:
			goingup = Code & 0x080;
			Code = (Code & 0x07F);

			if(!goingup)
			{
				switch (Code)
				{
				/* Key press */
				case XK_Up:
					key = KEY_UP;
					break;
				case XK_Left:
					key = KEY_LEFT;
					break;
				case XK_Down:
					key = KEY_DOWN;
					break;
				case XK_Right:
					key = KEY_RIGHT;
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

				case XK_Control_L:
					key_control |= 1;
					key = 0;
					break;
#if 0
				case XK_Control_R:
					key_control |= 2;
					key = 0;
					break;
#endif
				case XK_Shift_L:
				case XK_Shift_R:
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
#if 0
				/* Amiga numpad doesn't return rawkeys... */
				/* We'll have to make up with joystick support */
				case XK_KP_Home:
					key = KEY_UP_LEFT;
					break;
				case XK_KP_Page_Up:
					key = KEY_UP_RIGHT;
					break;
				case XK_KP_Page_Down:
					key = KEY_DOWN_RIGHT;
					break;
				case XK_KP_End:
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
#endif

#if 0
#ifdef USE_CONSOLE
				case XK_Help:
					key = CONSOLE_ACTIVATE_KEY;
					break;
				case XK_Help:
					key = CONSOLE_SWITCH_KEY;
					break;
#endif
#endif
				}
			} else
			{
				/* Key Release */
				switch (Code)
				{
				case XK_Control_L:
					key_control &= ~1;
					break;
#if 0
				case XK_Control_R:
					key_control &= ~2;
					break;
#endif
				case XK_Alt_L:
					key_alt &= ~1;
					break;
				case XK_Alt_R:
					key_alt &= ~2;
					break;
				default:
					key = 0;
				}
			}

			break;
		}
		if (key)
			key_enqueue (key);
	}
}


int init_machine (int argc, char **argv)
{
	gfx = &GFX_amiga;

	/* Open the Amiga's timer.device */
	opentimer();			/* !!! check result! */

	__argc = argc;
	__argv = argv;
	scale = opt.scale;

	/* ximage will be used to hold the chunky gfx data */
	ximage = malloc ((GFX_WIDTH * scale) * (GFX_HEIGHT * scale));
	if (!ximage) return err_Unk;

	return err_OK;
}


int deinit_machine(void)
{
	if (ximage) free (ximage);
	ximage = NULL;
	closetimer();
	return err_OK;
}


static UINT16 set_palette (UINT8 *pal, UINT16 scol, UINT16 numcols)
{
	ULONG r,g,b;
	int i;

	for (i = scol; i < scol + numcols; i++)
	{
		r = pal[i * 3] << 26;
		g = pal[i * 3 + 1] << 26;
		b = pal[i * 3 + 2] << 26;

		if (opt.fullscreen)
		{
			rgb_palette[i] = i;
			SetRGB4 (&screen->ViewPort, i,
				pal[i * 3] >> 2,
				pal[i * 3 + 1] >> 2,
				pal[i * 3 + 2] >> 2);
		}
		else
		{
			if (GfxBase->LibNode.lib_Version >= 39)
				rgb_palette[i] = ObtainBestPenA (
					screen->ViewPort.ColorMap,
					r, g, b, NULL);
		}
	}

	return err_OK;
}


static int Amiga_init_vidmode (void)
{
	char *PubScreen=NULL;
	int i;

	if (opt.fullscreen)
	{
		ULONG AmigaModeID=-1;	/* Amiga screen ID    */
		/* Running full screen */
		AmigaModeID = BestModeID (
			BIDTAG_DesiredWidth,	GFX_WIDTH * scale,
			BIDTAG_DesiredHeight,	GFX_HEIGHT * scale,
			BIDTAG_NominalWidth,	GFX_WIDTH * scale,
			BIDTAG_NominalHeight,	GFX_HEIGHT * scale,
			BIDTAG_Depth,		DEPTH,
			TAG_END);
		if (AmigaModeID==INVALID_ID) AmigaModeID=0;

		screen = OpenScreenTags(NULL,
			SA_Left,	0,
			SA_Top,		0,
			SA_Width,	GFX_WIDTH * scale,
			SA_Height,	GFX_HEIGHT * scale,
			SA_Depth,	DEPTH,
			SA_Title,	(ULONG) "Amiga Sarien v" VERSION,
			SA_DisplayID,	AmigaModeID,
			TAG_DONE);

		if (screen) {
			window = OpenWindowTags(NULL,
				WA_Width,	GFX_WIDTH * scale,
				WA_Height,	GFX_HEIGHT * scale,
				WA_CloseGadget,	FALSE,
				WA_DepthGadget,	FALSE,
				WA_DragBar,	FALSE,
				WA_SizeGadget,	FALSE,
				WA_Activate,	TRUE,
				WA_Borderless,	TRUE,
				WA_IDCMP,	IDCMP_CLOSEWINDOW |
						IDCMP_RAWKEY |
						IDCMP_VANILLAKEY |
						IDCMP_MOUSEBUTTONS |
						WFLG_RMBTRAP,
				WA_Activate,	TRUE,
				WA_CustomScreen,(ULONG) screen,
				TAG_END);
		}
	}
	else
	{
		/* Running in a window */
		screen = LockPubScreen(PubScreen);

		if (screen)
		{
			window = OpenWindowTags(NULL,
				WA_InnerWidth,	GFX_WIDTH * scale,
				WA_InnerHeight,	GFX_HEIGHT * scale,
				WA_Title,	(ULONG)"Amiga Sarien v" VERSION,
				WA_CloseGadget,	TRUE,
				WA_DepthGadget,	TRUE,
				WA_DragBar,	TRUE,
				WA_SizeGadget,	FALSE,
				WA_Activate,	TRUE,
				WA_RMBTrap,	TRUE,
				WA_PubScreenName,(ULONG) PubScreen,
				WA_IDCMP,	IDCMP_CLOSEWINDOW |
						IDCMP_RAWKEY |
						IDCMP_VANILLAKEY |
						IDCMP_MOUSEBUTTONS,
				TAG_END);
		}
	}

	if (!screen)
	{
		fprintf(stderr, "Error opening/locking screen\n");
		return err_Unk;
	}

	if (!window)
	{
		fprintf(stderr, "Error opening window\n");
		if (screen)
		{
			if (opt.fullscreen)
				CloseScreen(screen);
			else
				UnlockPubScreen(NULL,screen);
			screen = NULL;
		}
		return err_Unk;
	}

	/* Setup the temporary rastport required for kickstarts < 3.1 */
	if (GfxBase->LibNode.lib_Version < 39)
	{
		int depth;
		InitBitMap (&video_tmp_bm, 1, (GFX_WIDTH * scale), 1);

		for (depth = 0; depth < DEPTH; depth++)
		{
			if ((video_tmp_bm.Planes[depth] = (PLANEPTR)AllocRaster (GFX_WIDTH * scale, 1)) == NULL)
			{
				fprintf(stderr,"AllocRaster failed");
				return 0;
			}
		}

		video_temprp = *window->RPort;
		video_temprp.Layer = NULL;
		video_temprp.BitMap = &video_tmp_bm;
	}

	/* clear_buffer(); */

	for (i=0;i<16;i++) {
		rgb_palette[i] = -1;
	}

	set_palette (palette, 0, 32);

	/* clear screen */
	memset (ximage, rgb_palette[0], GFX_HEIGHT * scale * GFX_WIDTH * scale);
	return err_OK;
}

static int Amiga_deinit_vidmode (void)
{
	fprintf (stderr, "amiga: deiniting video mode\n");

	/* Free the temporary rastport required for kickstarts < 3.1 */
	if (GfxBase->LibNode.lib_Version < 39)
	{
		int depth;
		for (depth = 0; depth < DEPTH; depth++)
		{
			if (video_tmp_bm.Planes[depth] != NULL)
			{
				FreeRaster (video_tmp_bm.Planes[depth],
					(GFX_WIDTH * scale), 1);
				video_tmp_bm.Planes[depth] = NULL;
			}
		}
	}

	if (opt.fullscreen)
	{
		/* release allocated colour pens */
		if (screen)
		{
			int i;
			for (i=0;i<16;i++)
			{
				if (rgb_palette[i] >= 0) {
					ReleasePen (screen->ViewPort.ColorMap,
						rgb_palette[i]);
				}
				rgb_palette[i] = -1;
			}
		}
	}

	if (window)
	{
		if (VisualInfo)
			FreeVisualInfo(VisualInfo);
		CloseWindow(window);
		window = NULL;
	}

	if (screen)
	{
		if (opt.fullscreen)
			CloseScreen(screen);
		else
			UnlockPubScreen(NULL,screen);
		screen = NULL;
	}

	return err_OK;
}


/* put a block onto the screen */
static void Amiga_blit_block(int x1, int y1, int x2, int y2)
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
		x2 = (x2 + 1) * scale - 1;
		y2 = (y2 + 1) * scale - 1;
	}

	if (GfxBase->LibNode.lib_Version >= 39)
	{
		/* Kickstart 3.1+ */
		static UBYTE *image;

		if (scale == 1) {
			image = ximage + (x1 + (y1 * GFX_WIDTH));
		} else if (scale == 2) {
			image = ximage + (x1 + (y1 * (GFX_WIDTH << 1)));
		} else {
			image = ximage + (x1 + (y1 * (GFX_WIDTH * scale)));
		}
		WriteChunkyPixels(window->RPort,
			x1 + window->BorderLeft,
			y1 + window->BorderTop,
			x2 + window->BorderLeft,
			y2 + window->BorderTop,
			(UBYTE *) image,
			GFX_WIDTH * scale);
	}
	else
	{
		/* Kickstart 2.0 -> 3.0 */
		/* I don't bother to support these any more :-) */
	}
}


/* put pixel routine */
static void Amiga_put_pixel (int x, int y, int w, UINT8 *p)
{
	int cp, z;
	register int i, j;

	switch (scale) {
	case 1:
		while (w--) {
			ximage[x + (y * GFX_WIDTH)] = rgb_palette[*p++];
			x++;
		}
		break;
	case 2:
		x <<= 1;
		y <<= 1;
		z = y + 1;
		while (w--) {
			cp = rgb_palette[*p++];
			ximage[(x  ) + (y * (GFX_WIDTH << 1))] = cp;
			ximage[(x++) + (z * (GFX_WIDTH << 1))] = cp;
			ximage[(x  ) + (y * (GFX_WIDTH << 1))] = cp;
			ximage[(x++) + (z * (GFX_WIDTH << 1))] = cp;
		}
		break;
	default:

		x *= scale;
		y *= scale;
		while (w--) {
			cp = rgb_palette[*p++];
			for (i = 0; i < scale; i++) {
				for (j = 0; j < scale; j++)
					ximage[x + i + ((y + j) * (GFX_WIDTH << scale))] = cp;
			}
			x += scale;
		}
		break;
	}
}


static int Amiga_keypress (void)
{
	process_events ();
	return key_queue_start != key_queue_end;
}


static int Amiga_get_key (void)
{
	int k;

	while (key_queue_start == key_queue_end)	/* block */
		Amiga_new_timer ();

	key_dequeue(k);

	return k;
}


static void Amiga_new_timer ()
{
	struct timeval tv;
	/* struct timezone tz; */
	static double msec = 0.0;
	double m;
	
	gettimeofday (&tv, NULL);
	m = 1000.0 * tv.tv_sec + tv.tv_usec / 1000.0;

	while (m - msec < 42)
	{
		gettimeofday (&tv, NULL);
		m = 1000.0 * tv.tv_sec + tv.tv_usec / 1000.0;
	}
	msec = m; 

	process_events ();
}

