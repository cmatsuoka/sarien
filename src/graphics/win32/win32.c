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
 * Win32 port by Felipe Rosinha <rosinha@helllabs.org>
 */
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <process.h>

#include "sarien.h"
#include "graphics.h"
#include "keyboard.h"
#include "console.h"
#include "win32.h"

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
	WM_PUT_BLOCK = WM_USER + 1,
};
static UINT16 g_err = err_OK;
static HPALETTE g_hPalette = NULL;
static const char g_szMainWndClass[] = "SarienWin";
static int  scale = 2;

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
static void	win32_put_pixels	(int, int, int, BYTE *);
static int	win32_keypress		(void);
static int	win32_get_key		(void);
static void	win32_new_timer		(void);

static void	gui_put_block		(int, int, int, int);
static int	set_palette		(UINT8 *, int, int);


static struct gfx_driver gfx_win32 = {
	init_vidmode,
	deinit_vidmode,
	win32_put_block,
	win32_put_pixels,
	win32_new_timer,
	win32_keypress,
	win32_get_key
};


extern struct sarien_options opt;
extern struct gfx_driver *gfx;

static char *apptext = TITLE " " VERSION;
static HDC  hDC;
static WNDCLASS wndclass;


static void INLINE gui_put_block (int x1, int y1, int x2, int y2)
{
	HDC hDC;

	if (x1 >= GFX_WIDTH)
		x1 = GFX_WIDTH - 1;

	if (y1 >= GFX_HEIGHT)
		y1 = GFX_HEIGHT - 1;

	if (x2 >= GFX_WIDTH)
		x2 = GFX_WIDTH - 1;

	if (y2 >= GFX_HEIGHT)
		y2 = GFX_HEIGHT - 1;

	hDC = GetDC( hwndMain );

	EnterCriticalSection (&g_screen.cs);
	StretchDIBits(
		hDC,
		x1 * scale,
		y1 * scale,
		(x2 - x1 + 1) * scale,
                (y2 - y1 + 1) * scale,
		x1,
                GFX_HEIGHT - y2 - 1,
		x2 - x1 + 1,
                y2 - y1 + 1,
		g_screen.screen_pixels,
		g_screen.binfo,
		DIB_RGB_COLORS,
		SRCCOPY);
	LeaveCriticalSection (&g_screen.cs);

	ReleaseDC (hwndMain, hDC);
}


LRESULT CALLBACK
MainWndProc (HWND hwnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	HDC          hDC;
	PAINTSTRUCT  ps;
	int          key = 0;

	switch (nMsg) {
	case WM_PUT_BLOCK: {
		xyxy *p = (xyxy *)lParam;
		gui_put_block (p->x1, p->y1, p->x2, p->y2);
		free(p);
		} break;
	case WM_DESTROY:
		deinit_vidmode ();
		exit (-1);
		return 0;
	case WM_PAINT:
		hDC = BeginPaint (hwndMain, &ps);
		EnterCriticalSection(&g_screen.cs);
		StretchDIBits(
			hDC,
			0,
			0,
			GFX_WIDTH * scale,
			GFX_HEIGHT * scale,
			0,
			0,
			GFX_WIDTH,
			GFX_HEIGHT,
			g_screen.screen_pixels,
			g_screen.binfo,
			DIB_RGB_COLORS,
			SRCCOPY);

		EndPaint (hwndMain, &ps);
		LeaveCriticalSection(&g_screen.cs);
		return 0;

	/* Multimedia functions
	 * (Damn! The CALLBACK_FUNCTION parameter doesn't work!)
	 */
	case MM_WOM_DONE:
		flush_sound ((PWAVEHDR) lParam);
		return 0;

	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
		if (lParam & REPEATED_KEYMASK)
			return 0;

		/* Keycode debug:
		 * report ("%02x\n", (int)wParam);
		 */
		switch (key = (int)wParam) {
		case VK_SHIFT:
			key = 0;
			break;
		case VK_UP:
			key = KEY_UP;
			break;
		case VK_LEFT:
			key = KEY_LEFT;
			break;
		case VK_DOWN:
			key = KEY_DOWN;
			break;
		case VK_RIGHT:
			key = KEY_RIGHT;
			break;
		case VK_HOME:
			key = KEY_UP_LEFT;
			break;
		case VK_PRIOR:
			key = KEY_UP_RIGHT;
			break;
		case VK_NEXT:
			key = KEY_DOWN_RIGHT;
			break;
		case VK_END:
			key = KEY_DOWN_LEFT;
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
		case 0xbc:
			key = GetAsyncKeyState (VK_SHIFT) & 0x8000 ? '<' : '.';
			break;
		case 0xbe:
			key = GetAsyncKeyState (VK_SHIFT) & 0x8000 ? '>' : '.';
			break;
		case 0xbf:
			key = GetAsyncKeyState (VK_SHIFT) & 0x8000 ? '?' : '/';
			break;
#ifdef USE_CONSOLE
		case 192:
			key = CONSOLE_ACTIVATE_KEY;
			break;
		case 193:
			key = CONSOLE_SWITCH_KEY;
			break;
#endif
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

		};

		_D (": key = 0x%02x ('%c')", key, isprint(key) ? key : '?');
		break;
	};
			
	/* Keyboard message handled */
	if (key) {
		key_enqueue (key);
		return 0;
	}

	return DefWindowProc (hwnd, nMsg, wParam, lParam);
}


int init_machine (int argc, char **argv)
{
	InitializeCriticalSection (&g_screen.cs);
	InitializeCriticalSection (&g_key_queue.cs);

	gfx = &gfx_win32;

	//scale = opt.scale;

	return err_OK;
}

int deinit_machine ()
{
	DeleteCriticalSection(&g_key_queue.cs);
	DeleteCriticalSection(&g_screen.cs);
	return err_OK;
}

static int init_vidmode ()
{
	int i;

#if 0
	/* FIXME: place this in an "About" box or something... */
	fprintf (stderr,
	"win32: Win32 DIB support by rosinha@dexter.damec.cefetpr.br\n");
#endif

	memset (&wndclass, 0, sizeof(WNDCLASS));
	wndclass.lpszClassName = g_szMainWndClass;
	wndclass.style         = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc   = MainWndProc;
	wndclass.hInstance     = GetModuleHandle(NULL);
	wndclass.hIcon         = LoadIcon (NULL, IDI_APPLICATION);
	wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);
	wndclass.hbrBackground = GetStockObject (BLACK_BRUSH);

	if (!RegisterClass(&wndclass)) {
		fprintf( stderr, "win32: can't register class\n");
		g_err = err_Unk;
		goto exx;
	}

	hwndMain = CreateWindow (
		g_szMainWndClass,
		apptext,
		WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		(GFX_WIDTH * scale) + GetSystemMetrics (SM_CXFRAME),
			(GFX_HEIGHT * scale) + GetSystemMetrics (SM_CYCAPTION) +
			GetSystemMetrics (SM_CYFRAME),
		NULL,
		NULL,
		NULL,
		NULL 
	);

	/* First create the palete */
	set_palette (palette, 0, 16); 

	/* Fill in the bitmap info header */
	g_screen.binfo = (BITMAPINFO *)malloc(sizeof(*g_screen.binfo) +
		256 * sizeof(RGBQUAD));

	if (g_screen.binfo == NULL) {
		fprintf (stderr, "win32: can't create DIB section\n");
		g_err =  err_Unk;
		goto exx;
	}

	g_screen.binfo->bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
	g_screen.binfo->bmiHeader.biWidth         = GFX_WIDTH;
	g_screen.binfo->bmiHeader.biHeight        = GFX_HEIGHT;
	g_screen.binfo->bmiHeader.biPlanes        = 1;
	g_screen.binfo->bmiHeader.biBitCount      = 8;   /* should be fine */
	g_screen.binfo->bmiHeader.biCompression   = BI_RGB;
	g_screen.binfo->bmiHeader.biSizeImage     = 0;
	g_screen.binfo->bmiHeader.biXPelsPerMeter = 0;
	g_screen.binfo->bmiHeader.biYPelsPerMeter = 0;
	g_screen.binfo->bmiHeader.biClrUsed       = 32;
	g_screen.binfo->bmiHeader.biClrImportant  = 0;

	for (i = 0; i < 32; i ++) {
		g_screen.binfo->bmiColors[i].rgbRed   = (palette[i*3    ]) << 2;
		g_screen.binfo->bmiColors[i].rgbGreen = (palette[i*3 + 1]) << 2;
		g_screen.binfo->bmiColors[i].rgbBlue  = (palette[i*3 + 2]) << 2;
		g_screen.binfo->bmiColors[i].rgbReserved = 0;
	}

	/* Create the offscreen bitmap buffer */
	hDC = GetDC (hwndMain);
	g_screen.screen_bmp = CreateDIBSection (hDC, g_screen.binfo,
		DIB_RGB_COLORS, (void **)(&g_screen.screen_pixels), NULL, 0);
	ReleaseDC (hwndMain, hDC);

	if (g_screen.screen_bmp == NULL || g_screen.screen_pixels == NULL) {
		fprintf( stderr, "win32: can't create DIB section\n");
		g_err = err_Unk;
	} else {
		ShowWindow (hwndMain, TRUE);
		UpdateWindow (hwndMain);
		g_err = err_OK;
	}

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

	p->x1 = x1;
	p->y1 = y1;
	p->x2 = x2;
	p->y2 = y2;

	PostMessage (hwndMain, WM_PUT_BLOCK, 0, (LPARAM)p);
}

/* put pixel routine */
/* Some errors! Handle color depth */
static void win32_put_pixels (int x, int y, int w, BYTE *p)
{
        BYTE *p0 = g_screen.screen_pixels; /* Word aligned! */
	EnterCriticalSection(&g_screen.cs);

        y = GFX_HEIGHT - y - 1;

	p0 += x + y * GFX_WIDTH;
	while (w--) { *p0++ = *p++; }

	LeaveCriticalSection(&g_screen.cs);

}
 
static int win32_keypress (void)
{
	int b;

	process_events ();
	EnterCriticalSection(&g_key_queue.cs);
	b = g_key_queue.start != g_key_queue.end;
	LeaveCriticalSection(&g_key_queue.cs);

	return b;
}

static int win32_get_key (void)
{
	int k;

	while (!win32_keypress())
		win32_new_timer ();

	key_dequeue(k);

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
	int          i, j;
	HDC          hDC;
	LOGPALETTE   *palette;
	PALETTEENTRY *entries;

	hDC = GetDC(hwndMain);

	if (GetDeviceCaps(hDC, PLANES) * GetDeviceCaps(hDC, BITSPIXEL) <= 8 ) {
		palette = malloc(sizeof(*palette) + 16 * sizeof(PALETTEENTRY));
		if (NULL == palette) {
			fprintf(stderr, "malloc failed for palette\n");
			return err_Unk;
		}

		palette->palVersion    = 0x300;
		palette->palNumEntries = 256;   /* Yikes! */

		GetSystemPaletteEntries(hDC, 0, 16, palette->palPalEntry);

		g_hPalette = CreatePalette(palette);

		entries = (PALETTEENTRY *)malloc(256 * sizeof(PALETTEENTRY));

		for (i = 0, j = 0; j < 256; j++) {
			entries[j].peRed   = pal[i*3    ] << 2;
			entries[j].peGreen = pal[i*3 + 1] << 2;
			entries[j].peBlue  = pal[i*3 + 2] << 2;
			entries[j].peFlags = PC_NOCOLLAPSE;

			i ++;
			if (i >= 32)
				i = 0;
		}

		SetPaletteEntries(g_hPalette, 0, 256, entries);
		SelectPalette(hDC, g_hPalette, FALSE);
		RealizePalette(hDC);
	}

	ReleaseDC( hwndMain, hDC );

	return err_OK;
}

