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
 *
 * Win32 port by Felipe Rosinha <rosinha@helllabs.org>
 *
 */
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <process.h>

#include "sarien.h"
#include "gfx_base.h"
#include "keyboard.h"
#include "console.h"

#include "win32.h"

#define TICK_SECONDS	18
#define TICK_IN_MSEC	(1000 / (TICK_SECONDS))
#define REPEATED_KEYMASK  (1<<30)
#define EXTENDED_KEYMASK  (1<<24)
#define KEY_QUEUE_SIZE    16

static struct{
	int start;
	int end;
	int queue[KEY_QUEUE_SIZE];
	CRITICAL_SECTION cs;
} g_key_queue = {0, 0};

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

enum {
	WM_PUT_BLOCK = WM_USER + 1,
};


typedef struct {
	UINT16 x1, y1;
	UINT16 x2, y2;
} xyxy;

static HANDLE	  g_hThread;
static UINT16 g_err = err_OK;
static HANDLE g_hExchEvent = NULL;

static struct{
	HBITMAP    screen_bmp;
	CRITICAL_SECTION  cs;
	BITMAPINFO *binfo;
	void       *screen_pixels;
} g_screen;
static HPALETTE   g_hPalette = NULL;
static const char*      g_szMainWndClass = "SarienWin";

static int  scale = 1;

//static LONGLONG g_update_freq, g_counts_per_msec; /* counts in tick */


volatile UINT32 c_ticks;
volatile UINT32 c_count;

static int	init_vidmode	(void);
static int	deinit_vidmode	(void);
static void	put_block	(int, int, int, int);
static void	_put_pixel	(int, int, int);
static int	keypress	(void);
static int	get_key		(void);
static void	new_timer	(void);
static void	gui_put_block	(int, int, int, int);

static int	set_palette	(UINT8 *, int, int);

static unsigned int __stdcall GuiThreadProc(void *);

static struct gfx_driver GFX_WIN32 = {
	init_vidmode,
	deinit_vidmode,
	put_block,
	_put_pixel,
	new_timer,
	keypress,
	get_key
};

extern struct sarien_options opt;
extern struct gfx_driver *gfx;


LRESULT CALLBACK
MainWndProc (HWND hwnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	HDC          hDC;
	PAINTSTRUCT  ps;
	int          key = 0;

	switch (nMsg)
	{
		case WM_PUT_BLOCK:
			{
				xyxy *p = (xyxy *)lParam;
				gui_put_block(p->x1, p->y1, p->x2, p->y2);
				free(p);
			}
			break;
		case WM_DESTROY:
			fprintf (stderr, "Fatal: message WM_DESTROY caught\n" );
			deinit_vidmode ();
			exit (-1);
			return 0;

		case WM_PAINT:
			hDC = BeginPaint( hwndMain, &ps );
			EnterCriticalSection(&g_screen.cs);
			StretchDIBits(
					hDC,
					0,
					0,
					GFX_WIDTH * scale,
					GFX_HEIGHT * scale,
					0,
					0,
					GFX_WIDTH * scale,
					GFX_HEIGHT * scale,
					g_screen.screen_pixels,
					g_screen.binfo,
					DIB_RGB_COLORS,
					SRCCOPY);

			EndPaint( hwndMain, &ps );
			LeaveCriticalSection(&g_screen.cs);
			return 0;

		/* Multimedia functions (Damn! The CALLBACK_FUNCTION parameter don't work...) */
		case MM_WOM_DONE:
			felipes_kludge ((PWAVEHDR) lParam);
			return 0;

		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
			if (lParam & REPEATED_KEYMASK )
				return(0);

			switch ( key = (int) wParam )
			{
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
				case VK_ESCAPE:
					key = 0x1b;
					break;
				case 192:
					key = CONSOLE_ACTIVATE_KEY;
					break;
//				case 193:
//					key = CONSOLE_SWITCH_KEY;
//					break;
				default:
					if (!isalpha (key))
						break;

					//Must exist a better way to do that!
					if ( GetKeyState( VK_CAPITAL  ) & 0x1 ) 
					{
						if ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 )
							key = key + 32;
					}
					else
					{
						if ( (GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) == 0 )
							key = key + 32;
					}

					//Control and Alt modifier
					if ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 )
						key = (key & ~0x20) - 0x40;
					else 
						if ( GetAsyncKeyState( VK_MENU ) &0x8000 )
							key = scancode_table[(key & ~0x20) - 0x41] << 8;

					break;

			};

			_D (": key = 0x%02x ('%c')", key, isprint(key) ? key : '?');
			break;
	};
			
	/* Keyboard message handled */
	if (key)
	{
		key_enqueue (key);
		return 0;
	}

	return DefWindowProc (hwnd, nMsg, wParam, lParam);
}

int init_machine (int argc, char **argv)
{
	InitializeCriticalSection(&g_screen.cs);
	InitializeCriticalSection(&g_key_queue.cs);

	gfx = &GFX_WIN32;

	scale = opt.scale;

	c_ticks = 0;
	c_count = 0;
/*
	if ( !QueryPerformanceFrequency((LARGE_INTEGER*)&g_update_freq) ) {
		fprintf (stderr, "win32: high resoultion timer needed\n");
		return err_Unk;
	}
*/
//        update_freq = 45 * (update_freq / 1000);   /* X11 speed 
//	g_update_freq = g_update_freq / TICK_SECONDS; /* Sierra speed 
//	g_counts_per_msec = g_update_freq / 1000;
//	g_update_freq /= TICK_SECONDS; /* Original speed */
	
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
	unsigned id;

	fprintf (stderr,
	"win32: Win32 DIB support by rosinha@dexter.damec.cefetpr.br\n");
	g_hExchEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	ResetEvent(g_hExchEvent);
	g_hThread = (HANDLE)_beginthreadex(NULL, 0, GuiThreadProc, NULL, 0, &id);
	if (g_hThread == INVALID_HANDLE_VALUE)
		return err_Unk;
	WaitForSingleObject(g_hExchEvent, INFINITE);
	ResetEvent(g_hExchEvent);
	return g_err;	
}

static unsigned int __stdcall GuiThreadProc(void *param)
{
	char *apptext = TITLE " " VERSION;
	MSG msg;
	int i;
	HDC  hDC;
	WNDCLASS wndclass;

	memset (&wndclass, 0, sizeof(WNDCLASSEX));
	wndclass.lpszClassName = g_szMainWndClass;
//	wndclass.cbSize        = sizeof(WNDCLASSEX);
	wndclass.style         = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc   = MainWndProc;
	wndclass.hInstance     = NULL;
	wndclass.hIcon         = LoadIcon (NULL, IDI_APPLICATION);
//	wndclass.hIconSm       = LoadIcon (NULL, IDI_APPLICATION);
	wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH) GetStockObject (BLACK_BRUSH);

	RegisterClass(&wndclass);

	hwndMain = CreateWindow (
		g_szMainWndClass,
		apptext,
		WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		(GFX_WIDTH*scale) + 2*GetSystemMetrics(SM_CXFRAME),
		(GFX_HEIGHT*scale) + GetSystemMetrics(SM_CYCAPTION) + 2*GetSystemMetrics(SM_CYFRAME),
		NULL,
		NULL,
		NULL,
		NULL 
	);

	/* First create the palete */
	set_palette (palette, 0, 16); 

	/* Fill in the bitmap info header */
	g_screen.binfo = (BITMAPINFO *)malloc( sizeof(*g_screen.binfo) + 256*sizeof(RGBQUAD) );

	if ( g_screen.binfo == NULL ) {
                fprintf (stderr, "win32: can't create DIB section\n");
                g_err =  err_Unk;
				goto exx;
	}

	g_screen.binfo->bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
	g_screen.binfo->bmiHeader.biWidth         = GFX_WIDTH * scale;
	g_screen.binfo->bmiHeader.biHeight        = GFX_HEIGHT * scale;
	g_screen.binfo->bmiHeader.biPlanes        = 1;
	g_screen.binfo->bmiHeader.biBitCount      = 8;   /* 8 Bits per pixel should work fine! */
	g_screen.binfo->bmiHeader.biCompression   = BI_RGB;
	g_screen.binfo->bmiHeader.biSizeImage     = 0;
	g_screen.binfo->bmiHeader.biXPelsPerMeter = 0;
	g_screen.binfo->bmiHeader.biYPelsPerMeter = 0;
	g_screen.binfo->bmiHeader.biClrUsed       = 32;
	g_screen.binfo->bmiHeader.biClrImportant  = 0;

	for (i = 0; i < 32; i ++) {
		g_screen.binfo->bmiColors[i].rgbRed      = (palette[i*3    ]) << 2;
		g_screen.binfo->bmiColors[i].rgbGreen    = (palette[i*3 + 1]) << 2;
		g_screen.binfo->bmiColors[i].rgbBlue     = (palette[i*3 + 2]) << 2;
		g_screen.binfo->bmiColors[i].rgbReserved = 0;
	}

	/* Create the offscreen bitmap buffer */
	hDC = GetDC( hwndMain );
	g_screen.screen_bmp = CreateDIBSection(hDC, g_screen.binfo, DIB_RGB_COLORS, (void **)(&g_screen.screen_pixels), NULL, 0);
	ReleaseDC( hwndMain, hDC );

	if ( (g_screen.screen_bmp == NULL) || (g_screen.screen_pixels == NULL) ) 
	{
		fprintf( stderr, "win32: can't create DIB section\n");
		g_err = err_Unk;
	}else{
		ShowWindow (hwndMain, TRUE);
		UpdateWindow (hwndMain);

		screen_mode = GFX_MODE;      
		g_err = err_OK;
	}

exx:
	SetEvent(g_hExchEvent); // notify main thread to continue

	while ( GetMessage( &msg, NULL, 0, 0) ) {
		TranslateMessage (&msg);
		DispatchMessage  (&msg);
	}
	return 0;	
}

static int deinit_vidmode (void)
{
	PostMessage(hwndMain, WM_QUIT, 0, 0);
	CloseHandle(g_hThread);
	DeleteObject( g_screen.screen_bmp );
	screen_mode = TXT_MODE;

	return err_OK;
}

#if 0
/* Based on LAGII 0.1.5 by XoXus */
static void shake_screen (int times)
{
#define MAG 4
	int  i, alt;
	RECT rcClient;

	GetWindowRect( hwndMain, &rcClient );

	alt = -MAG;
	for (i = 0; i < (2 * times); i++) {
		alt = -alt;

		SetWindowPos(
			hwndMain,
			NULL,
			rcClient.left + alt,
			rcClient.top  + alt,
			0,
			0,
			SWP_NOZORDER | SWP_NOSIZE );

		/* Yikes! */
		Sleep(25);

		SetWindowPos(
			hwndMain,
			NULL,
			rcClient.left - alt,
			rcClient.top  - alt,
			0,
			0,
			SWP_NOZORDER | SWP_NOSIZE );

		/* Yikes! */
		Sleep(25);
	}

	/* Return to original position */
	SetWindowPos(
		hwndMain,
		NULL,
		rcClient.left,
		rcClient.top,
		0,
		0,
		SWP_NOZORDER | SWP_NOSIZE );
}
#endif

/* put a block onto the screen */
void put_block (int x1, int y1, int x2, int y2) //th0
{
	xyxy *p = (xyxy*)malloc(sizeof(xyxy));
	if (p == NULL) // no way
		return;

	p->x1 = x1;
	p->x2 = x2;
	p->y1 = y1;
	p->y2 = y2;

	PostMessage(hwndMain, WM_PUT_BLOCK, 0, (LPARAM)p);
}

static void gui_put_block (int x1, int y1, int x2, int y2) //1
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

	if (scale > 1) {
		x1 *= scale;
		y1 *= scale;
		x2 *= scale;
		y2 *= scale;
	}

	hDC = GetDC( hwndMain );

	EnterCriticalSection(&g_screen.cs);
	StretchDIBits(
		hDC,
		x1,
		y1,
		x2 - x1 + 1,
                y2 - y1 + 1,
		x1,
                (GFX_HEIGHT * scale - 1) - y2,
		x2 - x1 + 1,
                (GFX_HEIGHT * scale - y1 - 1) - (GFX_HEIGHT * scale - y2 - 1) + 1,
		g_screen.screen_pixels,
		g_screen.binfo,
		DIB_RGB_COLORS,
		SRCCOPY);
	LeaveCriticalSection(&g_screen.cs);

	ReleaseDC( hwndMain, hDC );
}

/* put pixel routine */
/* Some errors! Handle color depth */
static void _put_pixel (int x, int y, int c)
{
	register int i, j;
        int offset;
        BYTE *screen_surface = g_screen.screen_pixels; /* Word aligned! */
	EnterCriticalSection(&g_screen.cs);

        y = GFX_HEIGHT - y - 1;

	if (scale == 1) {
		*(screen_surface + x + (y * GFX_WIDTH)) = c;
	} else {
		x = x * scale; 
		y = y * scale;

		for (i = 0; i < scale; i++) {
                        for (j = 0; j < scale; j++) {
                                offset = (x + i) + (y + j) * GFX_WIDTH * 2;
                                *(screen_surface +  offset) = c;
                        }
		}
	}
	LeaveCriticalSection(&g_screen.cs);

}
 
static int keypress (void)
{
	int b;

	EnterCriticalSection(&g_key_queue.cs);
	b = g_key_queue.start != g_key_queue.end;
	LeaveCriticalSection(&g_key_queue.cs);

	return b;
}

static int get_key (void)
{
	int k;

	while (!keypress()){
		new_timer ();
	}
	key_dequeue(k);

	return k;
}

static void new_timer ()
{
	DWORD	now;
	static DWORD end_of_last_tick = 0;
//	LONGLONG        end;
//	static LONGLONG start = 0;
//	static int      msg_box_ticks = 0;

	/* This is the original code written by Felipe. It works but boosts
	 * the system load to 100% when running
	 */
	/* 20 fps */
	now = GetTickCount();
	if ((end_of_last_tick != 0) && ((now - end_of_last_tick) < TICK_IN_MSEC)){
		Sleep(TICK_IN_MSEC - (now - end_of_last_tick));
	}
//	QueryPerformanceCounter( (LARGE_INTEGER*) &end );
//	if (( g_update_freq > (end - start) )){
//		Sleep((g_update_freq - (end - start))/ g_counts_per_msec);
//	}
	end_of_last_tick = GetTickCount();

	
	/* This is a highly indecent method to make it work, but it seems
	 * to work better than the two previous versions. Urght. I hate
	 * win32.
	 */
	/* Sleep (5); */

}

/* Primitive palette functions */
static int set_palette (UINT8 *pal, int scol, int numcols)
{
	int          i, j;
	HDC          hDC;
	LOGPALETTE   *palette;
	PALETTEENTRY *entries;

	hDC = GetDC(hwndMain);

	if ( GetDeviceCaps(hDC, PLANES) * GetDeviceCaps(hDC, BITSPIXEL) <= 8 )
	{

		palette                = (LOGPALETTE *)malloc(sizeof(*palette) + 16 * sizeof(PALETTEENTRY));
		palette->palVersion    = 0x300;
		palette->palNumEntries = 256;   /* Yckes! */

		GetSystemPaletteEntries(hDC, 0, 16, palette->palPalEntry);

		g_hPalette = CreatePalette(palette);

		entries = (PALETTEENTRY *)malloc(256 * sizeof(PALETTEENTRY));

		for (i = 0, j = 0; j < 256; j++)
		{
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

