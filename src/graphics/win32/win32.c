/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

/* Win32 port by Felipe Rosinha <rosinha@helllabs.org>
 * Fixes and hacks by Igor Nesterov <nest@rtsnet.ru>
 * Mouse support by Ryan Gordon <icculus@clutteredmind.org>
 * Extra fixes and hacks by Matt Hargett <matt@use.net>
 * More fixes and hacks by Claudio Matsuoka <claudio@helllabs.org>
 */

/*
 * Scale2x support added by Matt Hargett
 * Scale2x Copyright (C) 2001-2002 Andrea Mazzoleni
 */

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <stdio.h>
#include <process.h>

#include "sarien.h"
#include "sprite.h"
#include "picture.h"
#include "graphics.h"
#include "keyboard.h"
#include "console.h"
#include "win32.h"
#include "../../core/win32/resource.h"

#include "scale2x.h"

#define TICK_SECONDS		18
#define TICK_IN_MSEC		(1000 / (TICK_SECONDS))
#define REPEATED_KEYMASK	(1<<30)
#define EXTENDED_KEYMASK	(1<<24)
#define KEY_QUEUE_SIZE		16

#define key_enqueue(k) do {				\
	EnterCriticalSection(&g_key_queue.cs);		\
	g_key_queue.queue[g_key_queue.end++] = (k);	\
	g_key_queue.end %= KEY_QUEUE_SIZE;		\
	LeaveCriticalSection(&g_key_queue.cs);		\
} while (0)

#define key_dequeue(k) do {				\
	EnterCriticalSection(&g_key_queue.cs);		\
	(k) = g_key_queue.queue[g_key_queue.start++];	\
	g_key_queue.start %= KEY_QUEUE_SIZE;		\
	LeaveCriticalSection(&g_key_queue.cs);		\
} while (0)


typedef struct {
	UINT16 x1, y1;
	UINT16 x2, y2;
} xyxy;

enum {
	WM_PUT_BLOCK = WM_USER + 1
};
static UINT16 g_err = err_OK;
static HPALETTE g_hPalette = NULL;
static const char g_szMainWndClass[] = "SarienWin";
static int scale = 2;

static struct{
	HBITMAP    screen_bmp;
	CRITICAL_SECTION  cs;
	BITMAPINFO *binfo;
	void       *screen_pixels;
} g_screen;

static struct{
	int start;
	int end;
	int queue[KEY_QUEUE_SIZE];
	CRITICAL_SECTION cs;
} g_key_queue = { 0, 0 };


static int	init_vidmode		(void);
static int	deinit_vidmode		(void);
static void	win32_put_block		(int, int, int, int);
static int	win32_keypress		(void);
static int	win32_get_key		(void);
static void	win32_new_timer		(void);

static int	set_palette		(UINT8 *, int, int);


struct gfx_driver gfx_win32 = {
	init_vidmode,
	deinit_vidmode,
	win32_put_block,
	NULL,
	win32_new_timer,
	win32_keypress,
	win32_get_key
};


extern struct sarien_options opt;
extern struct gfx_driver *gfx;
extern UINT8 ega_palette[], new_palette[];

static char *apptext = TITLE " " VERSION;
static HDC  hdc;
static WNDCLASSEX wc;
static int xsize, ysize;
	

#define ASPECT_RATIO(x) ((x) * 6 / 5)

/* ====================================================================*/

/* Some optimized put_pixel routines for the most common cases */

static void _putpixels_scale1 (int x, int y, int w, BYTE *p)
{
	BYTE *p0 = g_screen.screen_pixels; /* Word aligned! */

	/* Always consider large, ratio-corrected screen for runtime resize */
	y = ASPECT_RATIO(GFX_HEIGHT) * 2 - y - 1;
	p0 += x + y * xsize;

	EnterCriticalSection(&g_screen.cs);
	while (w--) *p0++ = *p++;
	LeaveCriticalSection(&g_screen.cs);
}

static void _putpixels_scale2 (int x, int y, int w, BYTE *p)
{
	BYTE *p0 = g_screen.screen_pixels, *p1; /* Word aligned! */

	/* Using ASPECT_RATIO to allow runtime window resize */
	y = ASPECT_RATIO(GFX_HEIGHT) - y - 1;
	x <<= 1; y <<= 1; 
	p0 += x + y * xsize;
	p1 = p0 + xsize;

	EnterCriticalSection(&g_screen.cs);
	if (!opt.hires || y > 0 || y < (GFX_HEIGHT - 1)) {
		while (w--) {
			*p0++ = *p; *p0++ = *p;
			*p1++ = *p; *p1++ = *p;
			p++;
		}
	} else {
		scale2x_8_def(p0, p1, p - GFX_WIDTH, p, p + GFX_WIDTH, w);
	}
	LeaveCriticalSection (&g_screen.cs);
}


/* ====================================================================*/

/* Aspect ratio correcting put pixels handlers */

static void _putpixels_fixratio_scale1 (int x, int y, int w, UINT8 *p)
{
	if (y > 0 && ASPECT_RATIO (y) - 1 != ASPECT_RATIO (y - 1))
		_putpixels_scale1 (x, ASPECT_RATIO(y) - 1, w, p);
	_putpixels_scale1 (x, ASPECT_RATIO(y), w, p);
}

static void _putpixels_fixratio_scale2 (int x, int y, int w, BYTE *p)
{
	BYTE *p0 = g_screen.screen_pixels, *p1, *p2, *_p; /* Word aligned! */
	int extra = 0;

	if (0 == w)
		return;

	y = GFX_HEIGHT - y - 1;
	x <<= 1; y <<= 1; 

	if (y < ((GFX_WIDTH - 1) << 2) && ASPECT_RATIO (y) + 2 != ASPECT_RATIO (y + 2)) {
		extra = w;
	}

	y = ASPECT_RATIO(y);

	p0 += x + y * xsize;
	p1 = p0 + xsize;
	p2 = p1 + xsize;

	EnterCriticalSection(&g_screen.cs);
	if (!opt.hires || y > 0 || y < ASPECT_RATIO(GFX_HEIGHT - 1)) {
		for (_p = p; w--; p++) {
			*p0++ = *p;
			*p0++ = *p;
			*p1++ = *p;
			*p1++ = *p;
		}
		for (p = _p; extra--; p++) {
			*p2++ = *p;
			*p2++ = *p;
		}
	} else {
		scale2x_8_def(p0, p1, p - GFX_WIDTH * 2, p, p + GFX_WIDTH * 2, w);
		scale2x_8_def(p1, p2, p - GFX_WIDTH * 2, p, p + GFX_WIDTH * 2, extra);
	}
	LeaveCriticalSection (&g_screen.cs);
}

static void set_putpixels_method ()
{
	if (!opt.fixratio) {
		switch (scale) {
		case 1:
			gfx_win32.put_pixels = _putpixels_scale1;
			break;
		case 2:
			gfx_win32.put_pixels = _putpixels_scale2;
			break;
		}
	} else {
		switch (scale) {
		case 1:
			gfx_win32.put_pixels = _putpixels_fixratio_scale1;
			break;
		case 2:
			gfx_win32.put_pixels = _putpixels_fixratio_scale2;
			break;
		}
	}
}

/* ====================================================================*/

static void update_mouse_pos(int x, int y)
{
	mouse.x = x;
	mouse.y = y;
	if (scale != 0) {
		mouse.x /= scale;
		mouse.y /= scale;
	}

	/* for mouse we make the inverse transform of ASPECT_RATIO */
	if (opt.fixratio)
		mouse.y = mouse.y * 5 / 6;
}


static void resize_window(HWND hwnd)
{
	int xsize, ysize;

	xsize = GFX_WIDTH * scale;
	ysize = (opt.fixratio ? ASPECT_RATIO(GFX_HEIGHT) : GFX_HEIGHT) * scale;

	SetWindowPos(
		hwnd, HWND_TOP,
		0, 0,
		xsize + GetSystemMetrics (SM_CXFRAME) * 2,
		ysize + GetSystemMetrics (SM_CYCAPTION) +
			GetSystemMetrics (SM_CYFRAME) * 2 +
			GetSystemMetrics (SM_CYMENU),
		SWP_NOMOVE | SWP_NOZORDER
	);
}

static void refresh_screen()
{
	erase_both ();
	show_pic ();
	blit_both ();
	commit_both ();
	put_screen ();
}

LRESULT CALLBACK
MainWndProc (HWND hwnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	int h, w, key = 0, shift;
	static old_res = TRUE;
	xyxy *p = (xyxy *)lParam;

	switch (nMsg) {
	case WM_COMMAND:
		/* Handle menu options */
		switch (LOWORD(wParam)) {
		case ITEM_OPTIONS_RES_LOW:
			opt.hires = FALSE;
			refresh_screen();
			break;
		case ITEM_OPTIONS_RES_HIGH:
			opt.cgaemu = FALSE;
			opt.hires = TRUE;
			refresh_screen();
			break;
		case ITEM_OPTIONS_PAL_CGA:
			opt.cgaemu = TRUE;
			old_res = opt.hires;
			opt.hires = FALSE;
			refresh_screen();
			break;
		case ITEM_OPTIONS_PAL_EGA:
			if (opt.cgaemu) opt.hires = old_res;
			opt.cgaemu = FALSE;
			init_palette (ega_palette);
			set_palette (palette, 0, 32); 
			refresh_screen();
			break;
		case ITEM_OPTIONS_PAL_NEW:
			if (opt.cgaemu) opt.hires = old_res;
			opt.cgaemu = FALSE;
			init_palette (new_palette);
			set_palette (palette, 0, 32); 
			refresh_screen();
			break;
		case ITEM_OPTIONS_SIZE_SMALL:
			scale = 1;
			resize_window(hwnd);
			set_putpixels_method();
			refresh_screen();
			break;
		case ITEM_OPTIONS_SIZE_LARGE:
			scale = 2;
			resize_window (hwnd);
			set_putpixels_method();
			refresh_screen();
			break;
		case ITEM_OPTIONS_RATIO_43:
			opt.fixratio = 1;
			resize_window(hwnd);
			set_putpixels_method();
			refresh_screen();
			break;
		case ITEM_OPTIONS_RATIO_85:
			opt.fixratio = 0;
			resize_window(hwnd);
			set_putpixels_method();
			refresh_screen();
			break;
		case ITEM_FILE_OPEN:
			open_file (hwnd);
			break;
		case ITEM_HELP_ABOUT:
			DialogBox (GetModuleHandle(NULL),
				MAKEINTRESOURCE(DIALOG_ABOUT), hwnd,
				AboutDlgProc);
			break;
		case ITEM_FILE_EXIT:
			deinit_vidmode ();
			exit (0);
		}
		break;

	case WM_PUT_BLOCK:
		hdc = GetDC (hwnd);
		w = p->x2 - p->x1 + 1;
		h = p->y2 - p->y1 + 1;
		EnterCriticalSection (&g_screen.cs);
		StretchDIBits (
			hdc,
			p->x1, p->y1, w, h,
			p->x1, ysize - p->y2 - 1, w, h,
			g_screen.screen_pixels,
			g_screen.binfo,
			DIB_RGB_COLORS,
			SRCCOPY);
		LeaveCriticalSection (&g_screen.cs);
		ReleaseDC (hwnd, hdc);
		break;

	case WM_DESTROY:
		deinit_vidmode ();
		exit (-1);

	case WM_PAINT:
		hdc = BeginPaint (hwnd, &ps);
		EnterCriticalSection(&g_screen.cs);
		StretchDIBits(
			hdc,
			0, 0, xsize, ysize,
			0, 0, xsize, ysize,
			g_screen.screen_pixels,
			g_screen.binfo,
			DIB_RGB_COLORS,
			SRCCOPY);
		EndPaint (hwnd, &ps);
		LeaveCriticalSection(&g_screen.cs);
		return 0;

	/* Multimedia functions
	 * (Damn! The CALLBACK_FUNCTION parameter doesn't work!)
	 */
	case MM_WOM_DONE:
		flush_sound ((PWAVEHDR) lParam);
		return 0;

	case WM_LBUTTONDOWN:
		key = BUTTON_LEFT;
		mouse.button = 1;
		update_mouse_pos(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;

	case WM_RBUTTONDOWN:
		key = BUTTON_RIGHT;
		mouse.button = 2;
		update_mouse_pos(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;

	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		mouse.button = FALSE;
		return 0;

	case WM_MOUSEMOVE:
		update_mouse_pos(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;

	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
		/*  report ("%02x\n", (int)wParam); */
		key = (int)wParam;
		shift = GetAsyncKeyState (VK_SHIFT) & 0x8000;

		switch (key) {
		case VK_SHIFT:
		case VK_CONTROL:
		case VK_MENU:
			key = 0;
			break;
		case VK_UP:
		case VK_NUMPAD8:
			if (lParam & REPEATED_KEYMASK) 
				return 0;
			key = KEY_UP;
			break;
		case VK_LEFT:
		case VK_NUMPAD4:
			if (lParam & REPEATED_KEYMASK) 
				return 0;
			key = KEY_LEFT;
			break;
		case VK_DOWN:
		case VK_NUMPAD2:
			if (lParam & REPEATED_KEYMASK) 
				return 0;
			key = KEY_DOWN;
			break;
		case VK_RIGHT:
		case VK_NUMPAD6:
			if (lParam & REPEATED_KEYMASK) 
				return 0;
			key = KEY_RIGHT;
			break;
		case VK_HOME:
		case VK_NUMPAD7:
			if (lParam & REPEATED_KEYMASK) 
				return 0;
			key = KEY_UP_LEFT;
			break;
		case VK_PRIOR:
		case VK_NUMPAD9:
			if (lParam & REPEATED_KEYMASK) 
				return 0;
			key = KEY_UP_RIGHT;
			break;
		case VK_NEXT:
		case VK_NUMPAD3:
			if (lParam & REPEATED_KEYMASK) 
				return 0;
			key = KEY_DOWN_RIGHT;
			break;
		case VK_END:
		case VK_NUMPAD1:
			if (lParam & REPEATED_KEYMASK) 
				return 0;
			key = KEY_DOWN_LEFT;
			break;
		case VK_CLEAR:
		case VK_NUMPAD5:
			key = KEY_STATIONARY;
			break;
		case VK_RETURN:
			key = KEY_ENTER;
			break;
		case VK_ADD:
			key = '+';
			break;
		case VK_SUBTRACT:
			key = '-';
			break;
		case VK_TAB:
			key = 0x0009;
			break;
		case VK_F1:
			key = 0x3b00;
			break;
		case VK_F2:
			key = 0x3c00;
			break;
		case VK_F3:
			key = 0x3d00;
			break;
		case VK_F4:
			key = 0x3e00;
			break;
		case VK_F5:
			key = 0x3f00;
			break;
		case VK_F6:
			key = 0x4000;
			break;
		case VK_F7:
			key = 0x4100;
			break;
		case VK_F8:
			key = 0x4200;
			break;
		case VK_F9:
			key = 0x4300;
			break;
		case VK_F10:
			key = 0x4400;
			break;
		case VK_F11:
			key = KEY_STATUSLN;
			break;
		case VK_F12:
			key = KEY_PRIORITY;
			break;
		case VK_ESCAPE:
			key = 0x1b;
			break;
		case 0xba:
			key = shift ? ':' : ';';
			break;
		case 0xbb:
			key = shift ? '+' : '=';
			break;
		case 0xbc:
			key = shift ? '<' : ',';
			break;
		case 0xbd:
			key = shift ? '_' : '-';
			break;
		case 0xbe:
			key = shift ? '>' : '.';
			break;
		case 0xbf:
			key = shift ? '?' : '/';
			break;
		case 0xdb:
			key = shift ? '{' : '[';
			break;
		case 0xdc:
			key = shift ? '|' : '\\';
			break;
		case 0xdd:
			key = shift ? '}' : ']';
			break;
		case 0xde:
			key = shift ? '"' : '\'';
			break;
		case 192:
			key = shift ? '~' : '`';
			break;
		default:
			if (!isalpha (key))
				break;

			/* Must exist a better way to do that! */
			if (GetKeyState (VK_CAPITAL) & 0x1) {
				if (GetAsyncKeyState (VK_SHIFT) & 0x8000)
					key = key + 32;
			} else {
				if (!(GetAsyncKeyState (VK_SHIFT) & 0x8000))
					key = key + 32;
			}

			/* Control and Alt modifier */
			if (GetAsyncKeyState (VK_CONTROL) & 0x8000)
				key = (key & ~0x20) - 0x40;
			else 
				if (GetAsyncKeyState (VK_MENU) & 0x8000)
					key = scancode_table[(key & ~0x20) - 0x41] << 8;

			break;

		}
		
		// this causes assertions in VS.NET
		// _D (": key = 0x%02x ('%c')", key, isprint(key) ? key : '?');

		break;
	};
			
	/* Keyboard message handled */
	if (key) {
		key_enqueue (key);
		return 0;
	}

	return DefWindowProc (hwnd, nMsg, wParam, lParam);
}

static int init_vidmode ()
{
	InitializeCriticalSection (&g_screen.cs);
	InitializeCriticalSection (&g_key_queue.cs);

	/* Don't change! The off-screen bitmap must contain the largest
	 * allowable scale for runtime window resizing!
	 */
	scale = 2;

	xsize = GFX_WIDTH * scale;
	ysize = (opt.fixratio ? ASPECT_RATIO(GFX_HEIGHT) : GFX_HEIGHT) * scale;

	memset (&wc, 0, sizeof(WNDCLASS));
	wc.cbSize        = sizeof(WNDCLASSEX);
	wc.lpszClassName = g_szMainWndClass;
	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = MainWndProc;
	wc.hInstance     = GetModuleHandle(NULL);
	wc.hIcon         = LoadIcon (GetModuleHandle(NULL),
			   MAKEINTRESOURCE(ICON_MAIN));
	wc.hIconSm       = (HICON)LoadImage (GetModuleHandle(NULL),
			   MAKEINTRESOURCE(ICON_MAIN), IMAGE_ICON, 16, 16, 0);
	wc.hCursor       = LoadCursor (NULL, IDC_ARROW);
	wc.hbrBackground = GetStockObject (BLACK_BRUSH);
	wc.lpszMenuName  = MAKEINTRESOURCE(MENU_MAIN);

	if (!RegisterClassEx(&wc)) {
		OutputDebugString("win32.c: init_vidmode(): can't register class");
		g_err = err_Unk;
		goto exx;
	}

	hwndMain = CreateWindowEx (
		WS_EX_CLIENTEDGE,
		g_szMainWndClass,
		apptext,
		WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		xsize + GetSystemMetrics (SM_CXFRAME) * 2,
		ysize + GetSystemMetrics (SM_CYCAPTION) +
			GetSystemMetrics (SM_CYFRAME) * 2 +
			GetSystemMetrics (SM_CYMENU),
		NULL,
		NULL,
		NULL,
		NULL 
	);

	if (NULL == hwndMain) {
		OutputDebugString("win32.c: init_vidmode(): can't register class");
		g_err = err_Unk;
		goto exx;
	}

	/* Fill in the bitmap info header */
	g_screen.binfo = (BITMAPINFO *)malloc(sizeof(*g_screen.binfo) +
		32 * sizeof(RGBQUAD));

	if (g_screen.binfo == NULL) {
		OutputDebugString("win32.c: init_vidmode(): malloc of g_screen.binfo failed");
		g_err =  err_Unk;
		goto exx;
	}

	g_screen.binfo->bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
	g_screen.binfo->bmiHeader.biWidth         = xsize;
	g_screen.binfo->bmiHeader.biHeight        = ysize;
	g_screen.binfo->bmiHeader.biPlanes        = 1;
	g_screen.binfo->bmiHeader.biBitCount      = 8;   /* should be fine */
	g_screen.binfo->bmiHeader.biCompression   = BI_RGB;
	g_screen.binfo->bmiHeader.biSizeImage     = 0;
	g_screen.binfo->bmiHeader.biXPelsPerMeter = 0;
	g_screen.binfo->bmiHeader.biYPelsPerMeter = 0;
	g_screen.binfo->bmiHeader.biClrUsed       = 32;
	g_screen.binfo->bmiHeader.biClrImportant  = 0;

	/* Create the palete */
	set_palette (palette, 0, 32); 

	/* Create the offscreen bitmap buffer */
	hdc = GetDC (hwndMain);
	g_screen.screen_bmp = CreateDIBSection (hdc, g_screen.binfo,
		DIB_RGB_COLORS, (void **)(&g_screen.screen_pixels), NULL, 0);
	ReleaseDC (hwndMain, hdc);

	if (g_screen.screen_bmp == NULL || g_screen.screen_pixels == NULL) {
		OutputDebugString ("win32.c: init_vidmode(): "
			"CreateDIBSection failed");
		g_err = err_Unk;
	} else {
		ShowWindow (hwndMain, TRUE);
		UpdateWindow (hwndMain);
		g_err = err_OK;
	}

	set_putpixels_method ();

exx:
	return g_err;	
}

static void INLINE process_events ()
{
	MSG msg;
	
	while (PeekMessage (&msg, NULL, 0, 0, PM_NOREMOVE)) {
		GetMessage (&msg, NULL, 0, 0);
		TranslateMessage (&msg);
		DispatchMessage (&msg);
	}
}

static int deinit_vidmode (void)
{
	DeleteCriticalSection(&g_key_queue.cs);
	DeleteCriticalSection(&g_screen.cs);
	
	PostMessage (hwndMain, WM_QUIT, 0, 0);
	DeleteObject (g_screen.screen_bmp);

	return err_OK;
}

/* put a block onto the screen */
static void win32_put_block (int x1, int y1, int x2, int y2)
{
	xyxy *p;

	if ((p = malloc (sizeof(xyxy))) == NULL)
		return;

	if (x1 >= GFX_WIDTH)  x1 = GFX_WIDTH - 1;
	if (y1 >= GFX_HEIGHT) y1 = GFX_HEIGHT - 1;
	if (x2 >= GFX_WIDTH)  x2 = GFX_WIDTH - 1;
	if (y2 >= GFX_HEIGHT) y2 = GFX_HEIGHT - 1;

	p->x1 = x1 * scale;
	p->y1 = y1 * scale;
	p->x2 = (x2 + 1) * scale - 1;
	p->y2 = (y2 + 1) * scale - 1;

	if (opt.fixratio) {
		p->y1 = ASPECT_RATIO(p->y1);
		p->y2 = ASPECT_RATIO(p->y2 + 1);
	}

	PostMessage (hwndMain, WM_PUT_BLOCK, 0, (LPARAM)p);
}

static int win32_keypress (void)
{
	int b;

	process_events ();
	EnterCriticalSection(&g_key_queue.cs);
	b = (g_key_queue.start != g_key_queue.end);
	LeaveCriticalSection(&g_key_queue.cs);

	return b;
}

static int win32_get_key (void)
{
	int k;

	while (!win32_keypress())
		win32_new_timer ();

	key_dequeue (k);

	return k;
}

static void win32_new_timer ()
{
	DWORD	now;
	static DWORD last = 0;

	now = GetTickCount();

	while (now - last < TICK_IN_MSEC) {
		Sleep (TICK_IN_MSEC - (now - last));
		now = GetTickCount ();
	}
	last = now;

	process_events ();
}

/* Primitive palette functions */
static int set_palette (UINT8 *pal, int scol, int numcols)
{
	int i;

#if 0
	HDC hdc;
	LOGPALETTE *palette;
	PALETTEENTRY *entries;

	hdc = GetDC(hwndMain);

	if (GetDeviceCaps(hdc, PLANES) * GetDeviceCaps(hdc, BITSPIXEL) <= 8) {
		entries = (PALETTEENTRY *)malloc(32 * sizeof(PALETTEENTRY));

		for (i = scol; i < numcols; i++) {
			entries[i].peRed   = pal[i*3    ] << 2;
			entries[i].peGreen = pal[i*3 + 1] << 2;
			entries[i].peBlue  = pal[i*3 + 2] << 2;
			entries[i].peFlags = PC_NOCOLLAPSE;
		}

		palette = malloc(sizeof(*palette) + 32 * sizeof(PALETTEENTRY));
		if (palette == NULL) {
			OutputDebugString("malloc failed for palette");
			return err_Unk;
		}
		palette->palVersion = 0x300;
		palette->palNumEntries = 32;
		g_hPalette = CreatePalette(palette);
		free(palette);

		SetPaletteEntries(g_hPalette, 0, 32, entries);
		SelectPalette(hdc, g_hPalette, FALSE);
		RealizePalette(hdc);
	} else
#endif

	for (i = scol; i < numcols; i ++) {
		g_screen.binfo->bmiColors[i].rgbRed  = (pal[i*3    ]) << 2;
		g_screen.binfo->bmiColors[i].rgbGreen= (pal[i*3 + 1]) << 2;
		g_screen.binfo->bmiColors[i].rgbBlue = (pal[i*3 + 2]) << 2;
		g_screen.binfo->bmiColors[i].rgbReserved = 0;
	}


	ReleaseDC(hwndMain, hdc);

	return err_OK;
}


