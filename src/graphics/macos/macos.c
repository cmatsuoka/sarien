/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2002 Stuart George and Claudio Matsuoka
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
#include "agi.h"
#include "graphics.h"
#include "keyboard.h"
#include "resource.h"


extern struct gfx_driver *gfx;
extern struct sarien_options opt;
extern UINT8 ega_palette[], new_palette[];


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
static int rgb_palette[32];
static GWorldPtr gworld;
static PixMapHandle pix, wpix;
static UINT8 *screen_buffer;
static int depth = 16;
static int scale = 1;
static int fixratio = 1;
static int egapal = 0;

static int bpl;

#define KEY_QUEUE_SIZE 16

static int key_queue[KEY_QUEUE_SIZE];
static int key_queue_start = 0;
static int key_queue_end = 0;

#define key_enqueue(k) do { key_queue[key_queue_end++] = (k); \
	key_queue_end %= KEY_QUEUE_SIZE; } while (0)
#define key_dequeue(k) do { (k) = key_queue[key_queue_start++]; \
	key_queue_start %= KEY_QUEUE_SIZE; } while (0)

#define ASPECT_RATIO(x) ((x) * 6 / 5)
#define MAX_SCALE 2

/* ===================================================================== */
/* Optimized wrappers to access the offscreen pixmap directly.
 * From my raster star wars scroller screensaver.
 */

/* In the normal 8/15/16/24/32 bpp cases idx indexes the data item directly.
 * x and y are available for the other depths.
 */
static INLINE void putpixel_32 (UINT8 *img, int idx, int p)
{
	*(int *)&img[idx] = p;
}

static INLINE void putpixel_16 (UINT8 *img, int idx, int p)
{
	*(short *)&img[idx] = p;
}

static INLINE void putpixel_8 (UINT8 *img, int idx, int p)
{
	*(char *)&img[idx] = p;
}

/* ===================================================================== */

/* Standard put pixels handlers */

#define _putpixels_scale1(d) static void \
_putpixels_##d##bits_scale1 (int x, int y, int w, UINT8 *p) { \
	if (w == 0) return; \
	x *= ((d) / 8); x += y * bpl; \
	while (w--) { \
		putpixel_##d (screen_buffer, x, rgb_palette[*p++]); \
		x += ((d) / 8); \
	} \
}

#define _putpixels_scale2(d) static void \
_putpixels_##d##bits_scale2 (int x, int y, int w, UINT8 *p) { \
	register int c; if (w == 0) return; \
	x <<= 1; y <<= 1; x *= ((d) / 8); \
	x += y * bpl; \
	y = x + bpl; \
	while (w--) { \
		c = rgb_palette[*p++]; \
		putpixel_##d (screen_buffer, x, c); x += ((d) / 8); \
		putpixel_##d (screen_buffer, x, c); x += ((d) / 8); \
		putpixel_##d (screen_buffer, y, c); y += ((d) / 8); \
		putpixel_##d (screen_buffer, y, c); y += ((d) / 8); \
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
	x <<= 1; y <<= 1; x *= ((d) / 8); \
	if (y < ((GFX_WIDTH - 1) << 2) && ASPECT_RATIO (y) + 2 != ASPECT_RATIO (y + 2)) extra = w; \
	y = ASPECT_RATIO(y); \
	x += y * bpl; \
	y = x + bpl; \
	z = y + bpl; \
	for (p = p0; w--; ) { \
		c = rgb_palette[*p++]; \
		putpixel_##d (screen_buffer, x, c); x += ((d) / 8); \
		putpixel_##d (screen_buffer, x, c); x += ((d) / 8); \
		putpixel_##d (screen_buffer, y, c); y += ((d) / 8); \
		putpixel_##d (screen_buffer, y, c); y += ((d) / 8); \
	} \
	for (p = p0; extra--; ) { \
		c = rgb_palette[*p++]; \
		putpixel_##d (screen_buffer, z, c); z += ((d) / 8); \
		putpixel_##d (screen_buffer, z, c); z += ((d) / 8); \
	} \
}

_putpixels_fixratio_scale1 (8);
_putpixels_fixratio_scale1 (16);
_putpixels_fixratio_scale1 (32);

_putpixels_fixratio_scale2 (8);
_putpixels_fixratio_scale2 (16);
_putpixels_fixratio_scale2 (32);

/* ===================================================================== */

/*
 * Dialogs
 */
 
void about_sarien (void)
{
	DialogPtr dialog;
	short item;

	dialog = GetNewDialog (rAbout, NULL, (WindowPtr)-1);
	ModalDialog (nil, &item);
	DisposeDialog (dialog);
}


/*
 * AppleEvents
 */

static void load_game (FSSpec *fs)
{
	if (game.state >= STATE_LOADED) {
		report ("AGI game already loaded.\n");
		return;
	}

	if (agi_detect_game (fs->name) == err_OK) {
		game.state = STATE_RUNNING;
		report ("AGI game successfully loaded.\n");
		console_prompt ();
		return;
	}
	
	report ("AGI game load failed.\n");
	return;
}

static pascal OSErr open_app (AppleEvent *event, AppleEvent *reply, long refcon)
{
	return noErr;
}

static pascal OSErr quit_app (AppleEvent *event, AppleEvent *reply, long refcon)
{
	gfx->deinit_video_mode ();
	ExitToShell ();
	return noErr;
}

static pascal OSErr open_doc (AppleEvent *event, AppleEvent *reply, long refcon)
{
	OSErr err;
	FSSpec fileSpec;
	long i, numDocs;
	DescType returnedType;
	AEKeyword keywd;
	Size actualSize;
	AEDescList docList = { typeNull, nil };

	err = AEGetParamDesc (event, keyDirectObject, typeAEList, &docList);

	err = AECountItems (&docList, &numDocs);
   
	for (i = 1; i <= numDocs; i++) {
		err = AEGetNthPtr (&docList, i, typeFSS, &keywd, &returnedType,
			(Ptr)&fileSpec, sizeof (fileSpec), &actualSize);

		load_game (&fileSpec);
	}

	err = AEDisposeDesc (&docList);

	return err;
}

static void init_events ()
{
	AEInstallEventHandler (kCoreEventClass, kAEOpenApplication, 
		NewAEEventHandlerProc (open_app), 0, false);

	AEInstallEventHandler (kCoreEventClass, kAEOpenDocuments,
		NewAEEventHandlerProc (open_doc), 0, false);

	AEInstallEventHandler (kCoreEventClass, kAEQuitApplication,
		NewAEEventHandlerProc (quit_app), 0, false);
}

/* ===================================================================== */

/*
 * Toolbox & menu functions
 */

void (*ppix[2][3][MAX_SCALE])() = {
	{
		{ _putpixels_8bits_scale1, _putpixels_8bits_scale2 },
		{ _putpixels_16bits_scale1, _putpixels_16bits_scale2 },
		{ _putpixels_32bits_scale1, _putpixels_32bits_scale2 }
	},
	{
		{ _putpixels_fixratio_8bits_scale1, _putpixels_fixratio_8bits_scale2 },
		{ _putpixels_fixratio_16bits_scale1, _putpixels_fixratio_16bits_scale2 },
		{ _putpixels_fixratio_32bits_scale1, _putpixels_fixratio_32bits_scale2 }
	}
};


static void init_toolbox ()
{
	InitGraf (&qd.thePort);
	InitFonts ();
	InitWindows ();
	InitMenus ();
	TEInit ();
	InitDialogs (nil);
	InitCursor ();
}

static void init_menu ()
{
	Handle menubar;

	menubar = GetNewMBar (rMenuBar);
	SetMenuBar (menubar);
	DisposeHandle (menubar);
	AppendResMenu (GetMenuHandle (mApple), 'DRVR');
	DrawMenuBar();
}

static void resize_window ()
{
	SizeWindow (window, GFX_WIDTH * scale,
		fixratio ? ASPECT_RATIO (GFX_HEIGHT * scale) : GFX_HEIGHT * scale,
		true);
	InvalRect (&gworld->portRect);
	gfx->put_pixels = ppix[fixratio][depth / 8 - 1][scale - 1];
	flush_screen ();
}

static void adjust_menu ()
{
	MenuHandle menu;
	
	menu = GetMenuHandle (mView);
	
	if (egapal)
		SetMenuItemText (menu, iPal, "\nSarien colors");
	else
		SetMenuItemText (menu, iPal, "\nPC EGA colors");
	
	if (opt.hires)
		SetMenuItemText (menu, iHires, "\nLo-res mode");
	else
		SetMenuItemText (menu, iHires, "\nHi-res mode");
		
	if (scale == 1)
		SetMenuItemText (menu, iSize, "\pDouble size");
	else
		SetMenuItemText (menu, iSize, "\pNormal size");
	
	if (fixratio)
		SetMenuItemText (menu, iRatio, "\p8:5 aspect ratio");
	else
		SetMenuItemText (menu, iRatio, "\p4:3 aspect ratio");
}

static void process_menu (int mc)
{
	int id;
	int item;
	MenuHandle menu;
	Str255 name;
 
	id = HiWord (mc);
	item = LoWord (mc);
 
	switch (id) {
	case mApple:
		switch (item) {
		case iAbout:
			about_sarien ();
			break;
		default:
			menu = GetMenuHandle (mApple);
			GetMenuItemText (menu, item, name);
			/* OpenDeskAcc (name); */
			break;
		}
		break;
	case mFile:
		switch (item) {
		case iQuit:
			gfx->deinit_video_mode ();
			ExitToShell ();
			break;
		}
		break;
	case mView:
		switch (item) {
		case iPal:
			if (egapal) {
				init_palette (new_palette);
				egapal = 0;
			} else {
				init_palette (ega_palette);
				egapal = 1;
			}
			set_palette (palette, 0, 32);
			flush_screen ();
			adjust_menu ();
			break;
		case iHires:
			opt.hires = !opt.hires;
			erase_both ();
			show_pic ();
			blit_both ();
			adjust_menu ();
			break;
		case iSize:
			if (scale == 1)
				scale = 2;
			else
				scale = 1;
			adjust_menu ();
			resize_window ();
			break;
		case iRatio:
			fixratio = !fixratio;
			adjust_menu ();
			resize_window ();
			break;
		}
		break;
	}

	HiliteMenu (0);
}

static void process_events ()
{
	WindowPtr win;
	EventRecord event;

	SystemTask();

	if (WaitNextEvent (everyEvent, &event, 3, NULL)) {
		switch (event.what) {
		case mouseDown:
			switch (FindWindow (event.where, &win)) {
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
				process_menu (MenuSelect (event.where));
				break;
			case inDrag:
				/* title bar: call Window Manager to drag */
				DragWindow (win, event.where, &qd.screenBits.bounds);
				break;
			case inContent:
				/* body of application window:
				 * make it active if not
				 */
				if (win != FrontWindow())
					SelectWindow (win);
				break;
			case inGoAway:
				/* quit application */
				if (TrackGoAway (win, event.where)) {
					gfx->deinit_video_mode ();
					ExitToShell ();
				}
				break;
			}
			break;
		case updateEvt:		/* Update window. */
			if ((WindowPtr) event.message == window) {
				BeginUpdate((WindowPtr) event.message);
				/* repaint */
				gfx->put_block (0, 0, GFX_WIDTH - 1,
						GFX_HEIGHT - 1);
				EndUpdate((WindowPtr) event.message);
			}
			break;
		case keyDown:
		case autoKey:	/* key pressed once or held down to repeat */
			if (window == FrontWindow()) {
				int c = (event.message & charCodeMask);
				int k = (event.message & keyCodeMask);
				if (event.modifiers & cmdKey) {
					process_menu (MenuKey (c));
					break;
				}
				report ("%02x %02x\n", c, event.message);

				/* These key codes were obtained using the
				 * BasiliskII emulator. I'm assuming that
				 * the key codes are being correctly emulated,
				 * however it would be interesting to check
				 * it in a real Mac.
				 */
				switch (c) {
				case 0xa4:	/* Backquote in BasiliskII */
					c = 0x60;
					break;
				case 0xb1:	/* Tilde in BasiliskII */
					c = 0x7e;
					break;
				case 0x10:	/* Function keys */
					switch (k) {
					}
					break;
				case 0x1c:
					c = KEY_LEFT;
					break;
				case 0x1d:
					c = KEY_RIGHT;
					break;
				case 0x1e:
					c = KEY_UP;
					break;
				case 0x1f:
					c = KEY_DOWN;
					break;
				}
				key_enqueue (c);
			}
			break;
		}
	}
}


int init_machine (int argc, char **argv)
{
	gfx = &gfx_macos;
	return err_OK;
}


int deinit_machine ()
{
	return err_OK;
}



static int set_palette (UINT8 *pal, int scol, int numcols)
{
	int i;

	for (i = scol; i < scol + numcols; i++) {

		switch (depth) {
#if 0
		case 8:
			rgb_palette[i] = color[i].pixel;
			break;
#endif
		case 15:
		case 16:
			rgb_palette[i] =
				((int)(pal[i * 3] & 0x3e) << 9) |
				((int)(pal[i * 3 + 1] & 0x3e) << 4) |
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
		process_events ();
	}

	process_events ();
}

static int macos_init_vidmode ()
{
	OSErr err;
	Rect window_rect;
	Rect gworld_rect;
	GDHandle old_gd;
	GWorldPtr old_gw;
	SysEnvRec sysenv;
	
	err = SysEnvirons (1, &sysenv);
	if (sysenv.hasColorQD == false)
		return -1;
	
	init_toolbox ();
	init_events ();
	init_menu ();
	adjust_menu ();

	/* Set palette */
	set_palette (palette, 0, 32);

	/* Create offscreen pixmap */
	/* It looks inconsistent, but it seems that the rect here is
	 * defined by dimensions while in the window creation call it
	 * is defined by coordinates. --claudio
	 */
	SetRect (&gworld_rect, 0, 0, GFX_WIDTH * MAX_SCALE,
		ASPECT_RATIO (GFX_HEIGHT * MAX_SCALE));
	if (NewGWorld (&gworld, depth, &gworld_rect, NULL, NULL, 0) != noErr)
		return -1;

	/* Create window */
	SetRect (&window_rect, 50, 50, 50 + GFX_WIDTH * scale - 1,
		50 + (fixratio ? ASPECT_RATIO (GFX_HEIGHT * scale) :
		GFX_HEIGHT * scale) - 1);
	window = NewCWindow (NULL, &window_rect, "\pSarien " VERSION, true,
		noGrowDocProc, (WindowPtr) -1, true, NULL);
		
	/* Initialize pixmap pointers */
	pix = GetGWorldPixMap (gworld);
	LockPixels (pix);
	wpix = ((CGrafPort *)window)->portPixMap;
	LockPixels (wpix);
	bpl = (*pix)->rowBytes & 0x3fff;
	screen_buffer = (UINT8 *)GetPixBaseAddr(pix);
	
	/* set window to current graf port */
	SetPort (window);

	/* Clear offscreen gworld */
	GetGWorld (&old_gw, &old_gd);
	SetGWorld (gworld, NULL);
	BackColor (blackColor);
	EraseRect (&gworld->portRect);
	SetGWorld (old_gw, old_gd);
	
	/* CopyBits needs these */
	ForeColor (blackColor);
	BackColor (whiteColor);

	/* Set optimized put_pixels handler */
	gfx->put_pixels = ppix[fixratio][depth / 8 - 1][scale - 1];
		
	return err_OK;
}


static int macos_deinit_vidmode ()
{
	UnlockPixels (wpix);
	UnlockPixels (pix);
	DisposePtr ((char *) gworld);
	DisposeWindow (window);
	return err_OK;
}


/* blit a block onto the screen */
static void macos_put_block (int x1, int y1, int x2, int y2)
{
	Rect r;

	if (x1 >= GFX_WIDTH)  x1 = GFX_WIDTH  - 1;
	if (y1 >= GFX_HEIGHT) y1 = GFX_HEIGHT - 1;
	if (x2 >= GFX_WIDTH)  x2 = GFX_WIDTH  - 1;
	if (y2 >= GFX_HEIGHT) y2 = GFX_HEIGHT - 1;

	x1 *= scale;
	y1 *= scale;
	x2 = (x2 + 1) * scale;
	y2 = fixratio ? ASPECT_RATIO ((y2 + 1) * scale) : (y2 + 1) * scale;

	SetRect (&r, x1, y1, x2, y2);
	CopyBits ((BitMap *)*pix, (BitMap *)*wpix, &r, &r, srcCopy, 0L);
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

