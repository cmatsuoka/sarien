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

/*
 * Massively modified by Vasyl Tsvirkunov <vasyl@pacbell.net> for
 * Pocket PC/WinCE port
 */

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <commctrl.h>
#include <aygshell.h>
#include "gx.h"

#include "sarien.h"
#include "graphics.h"
#include "keyboard.h"
#include "console.h"
#include "text.h"
/* Prevent name conflict */
#undef snprintf
#include "win32.h"

#include "resource.h"

static LPTSTR szAppName = TEXT("Pocket Sarien");
static GXDisplayProperties gxdp;
static GXKeyList gxkl;
static bool active = true;

static HWND hwndMB = NULL;

typedef unsigned char UBYTE;
static UBYTE* screen;

static int bFilter = 1;
static UBYTE* palRed;
static UBYTE* palGreen;
static UBYTE* palBlue;
static unsigned short* pal;

static bool bmono;
static bool b565;
static bool b555;
static bool bpalette;

static UBYTE invert = 0;
static int colorscale = 0;

#define COLORCONV565(r,g,b) (((r&0xf8)<<(11-3))|((g&0xfc)<<(5-2))|((b&0xf8)>>3))
#define COLORCONV555(r,g,b) (((r&0xf8)<<(10-3))|((g&0xf8)<<(5-2))|((b&0xf8)>>3))
#define COLORCONVMONO(r,g,b) ((((3*r>>3)+(g>>1)+(b>>3))>>colorscale)^invert)

static int	wince_init_vidmode	(void);
static int	wince_deinit_vidmode(void);
static void	wince_put_block		(int, int, int, int);
static void	wince_put_pixels	(int, int, int, BYTE *);
static int	wince_keypress		(void);
static int	wince_get_key		(void);
static void	wince_new_timer		(void);

LRESULT CALLBACK WindowProc(HWND hwnd, UINT nMsg, WPARAM wParam, LPARAM lParam);

void setup_extra_windows(HWND hwndTop);
void adjust_extra_windows();
HWND hwndFKeys;

static struct gfx_driver gfx_wince =
{
	wince_init_vidmode,
	wince_deinit_vidmode,
	wince_put_block,
	wince_put_pixels,
	wince_new_timer,
	wince_keypress,
	wince_get_key
};


extern "C" struct gfx_driver *gfx;

int init_machine(int argc, char **argv)
{
	gfx = &gfx_wince;
	return err_OK;
}

int deinit_machine()
{
	return err_OK;
}

void set_palette(int ent, UBYTE r, UBYTE g, UBYTE b)
{
	if (ent >= 256)
		return;

	palRed[ent] = r;
	palGreen[ent] = g;
	palBlue[ent] = b;

	if(b565)
		pal[ent] = COLORCONV565(r,g,b);
	else if(b555)
		pal[ent] = COLORCONV555(r,g,b);
	else if(bmono)
		pal[ent] = COLORCONVMONO(r,g,b);
}

void palette_update()
{
	if(bpalette)
	{
		LOGPALETTE* ple = (LOGPALETTE*)malloc(sizeof(LOGPALETTE)+sizeof(PALETTEENTRY)*255);
		ple->palVersion = 0x300;
		ple->palNumEntries = 256;
		for(int i=0; i<236; i++) // first 10 and last ten belong to the system!
		{
			ple->palPalEntry[i+10].peBlue =  palBlue[i];
			ple->palPalEntry[i+10].peGreen = palGreen[i];
			ple->palPalEntry[i+10].peRed =   palRed[i];
			ple->palPalEntry[i+10].peFlags = PC_RESERVED;
		}
		HDC hDC = GetDC(hwndMain);
		GetSystemPaletteEntries(hDC, 0, 10, &(ple->palPalEntry[0]));
		GetSystemPaletteEntries(hDC, 246, 10, &(ple->palPalEntry[246]));
		HPALETTE hpal =	CreatePalette(ple);
		SelectPalette(hDC, hpal, FALSE);
		RealizePalette(hDC);
		DeleteObject((HGDIOBJ)hpal);
		ReleaseDC(hwndMain, hDC);
		free((void*)ple);
	}
}

static int wince_init_vidmode()
{
	WNDCLASS wc;
	
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hIcon = (HICON)LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_SARIEN));
	wc.hCursor = NULL;
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = szAppName;
	RegisterClass(&wc);
	
	hwndMain = CreateWindow(szAppName,
		szAppName,
		WS_VISIBLE,
		0,
		0,
		GetSystemMetrics(SM_CXSCREEN),
		GetSystemMetrics(SM_CYSCREEN),
		NULL,
		NULL,
		GetModuleHandle(NULL),
		NULL);
	
	if(!hwndMain)
		return err_Unk;

	SHMENUBARINFO smbi;
	smbi.cbSize = sizeof(smbi); 
	smbi.hwndParent = hwndMain; 
	smbi.dwFlags = 0; 
	smbi.nToolBarId = IDM_MENU; 
	smbi.hInstRes = GetModuleHandle(NULL); 
	smbi.nBmpId = 0; 
	smbi.cBmpImages = 0; 
	smbi.hwndMB = NULL;
	BOOL res = SHCreateMenuBar(&smbi);
	hwndMB = smbi.hwndMB;
	
	screen = new UBYTE[GFX_WIDTH*GFX_HEIGHT];

	palRed = new UBYTE[256];
	palGreen = new UBYTE[256];
	palBlue = new UBYTE[256];
	if(!palRed || !palGreen || !palBlue)
		return err_Unk;

	pal = new unsigned short[256];
	if(!pal)
		return err_Unk;

	if(!screen)
		return err_Unk;

	memset(screen, 0, GFX_WIDTH*GFX_HEIGHT);

	GXOpenDisplay(hwndMain, GX_FULLSCREEN);
	GXOpenInput();

	SetWindowPos(hwndMain, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
	SetForegroundWindow(hwndMain);
	SHFullScreen(hwndMain, SHFS_SHOWSIPBUTTON);
	SHFullScreen(hwndMain, SHFS_HIDETASKBAR);

	gxdp = GXGetDisplayProperties();
	gxkl = GXGetDefaultKeys(GX_NORMALKEYS);


	bmono = (gxdp.ffFormat & kfDirect) && (gxdp.cBPP <= 8);
	b565 = (gxdp.ffFormat & kfDirect565) != 0;
	b555 = (gxdp.ffFormat & kfDirect555) != 0;
	bpalette = (gxdp.ffFormat & kfPalette) != 0;
	if(bpalette)
	{
		bmono = false;
		bFilter = 0; // cannot do filtered image in palette mode, sorry
	}
	if(bmono)
	{
		if(gxdp.ffFormat & kfDirectInverted)
			invert = (1<<gxdp.cBPP)-1;
		colorscale = gxdp.cBPP < 8 ? 8-gxdp.cBPP : 0;
	}

	int i;
	for(i=0; i<16; i++)
		set_palette(i, palette[i*3+0]<<2, palette[i*3+1]<<2, palette[i*3+2]<<2);
	for(i=17; i<256; i++)
		set_palette(i, 0, 255, 0);

	palette_update();

	setup_extra_windows(hwndMain);

	return err_OK;
}

static void INLINE process_events()
{
	MSG msg;
	while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

static int wince_deinit_vidmode(void)
{
	GXCloseInput();
	GXCloseDisplay();

	delete[] screen;
	delete[] palRed;
	delete[] palGreen;
	delete[] palBlue;
	delete[] pal;

	PostMessage(hwndMain, WM_QUIT, 0, 0);
	return err_OK;
}


#define TICK_SECONDS		18
#define TICK_IN_MSEC		(1000 / (TICK_SECONDS))

static void wince_new_timer()
{
	DWORD	now;
	static DWORD last = 0;

	now = GetTickCount();

	while (now - last < TICK_IN_MSEC)
	{
		Sleep (TICK_IN_MSEC - (now - last));
		now = GetTickCount ();
	}
	last = now;

	process_events ();
}

#define REPEATED_KEYMASK	(1<<30)
#define RELEASED_KEYMASK	(1<<31)
#define KEY_QUEUE_SIZE		16

static struct{
	int start;
	int end;
	int queue[KEY_QUEUE_SIZE];
} g_key_queue = { 0, 0 };


#define key_enqueue(k) do {				\
	g_key_queue.queue[g_key_queue.end++] = (k);	\
	g_key_queue.end %= KEY_QUEUE_SIZE;		\
} while (0)

#define key_dequeue(k) do {				\
	(k) = g_key_queue.queue[g_key_queue.start++];	\
	g_key_queue.start %= KEY_QUEUE_SIZE;		\
} while (0)


static int wince_keypress(void)
{
	process_events();
	return (g_key_queue.start != g_key_queue.end);
}

static int wince_get_key(void)
{
	int k;
	while (!wince_keypress())
		wince_new_timer();
	key_dequeue(k);
	return k;
}


static void wince_put_pixels(int x, int y, int w, BYTE *p)
{
	memcpy(screen+x+y*GFX_WIDTH, p, w);
}

static void wince_put_block(int x1, int y1, int x2, int y2)
{
	if(!active)
		return;

/* Somehow this function gets called with unchecked bounds causing visual artefacts */
	if(x2 > 319)
		x2 = 319;
	if(y2 > 199)
		y2 = 199;

	static UBYTE *src;
	static UBYTE *dst;

	static UBYTE *scraddr;
	static UBYTE *scr_ptr;
	
	static UBYTE *scr_ptr_limit;
	static UBYTE *src_limit;
	
	static long pixelstep;
	static long linestep;

// Special code is used to deal with packed pixels in monochrome mode
	static UBYTE bitmask;
	static int   bitshift;

	x1 &= ~3;

	scr_ptr = screen + x1 + y1*GFX_WIDTH;
	scr_ptr_limit = screen + x2 + y2*GFX_WIDTH;

	linestep = gxdp.cbyPitch;
	pixelstep = gxdp.cbxPitch;

	if(bmono)
	{
		if(pixelstep == 0)
			return; // unsupported screen geometry
	// this will work on mono iPAQ and @migo, don't know about any others
		bitshift = 0;
		bitmask = (1<<gxdp.cBPP)-1;
		linestep = (pixelstep > 0) ? -1 : 1;
	}


	scraddr = (UBYTE*)GXBeginDraw();
	if(scraddr)
	{
		if(bmono)
		{
		// Some partial pixel voodoo. I don't know if this works in all cases.
			scraddr += (x1*3/4)*pixelstep;
			int full = (y1*gxdp.cBPP)>>3;
			int partial = (y1*gxdp.cBPP)&7;
			scraddr += full*linestep;
			bitshift += gxdp.cBPP*partial;
			bitmask <<= gxdp.cBPP*partial;
			if(linestep < 0)
				scraddr += (pixelstep-1);
		}
		else
			scraddr += (x1*3/4)*pixelstep + y1*linestep;
		src_limit = scr_ptr + x2-x1+1;

		while(scr_ptr < scr_ptr_limit)
		{
			src = scr_ptr;
			dst = scraddr;
			while(src < src_limit)
			{
				if(bFilter)
				{
				/* Let's see how fast that CPU is */
					UBYTE r, g, b;
					r = (3*palRed[*(src+0)] + palRed[*(src+1)])>>2;
					g = (3*palGreen[*(src+0)] + palGreen[*(src+1)])>>2;
					b = (3*palBlue[*(src+0)] + palBlue[*(src+1)])>>2;

					if(b565)
						*(unsigned short*)dst = COLORCONV565(r,g,b);
					else if(b555)
						*(unsigned short*)dst = COLORCONV555(r,g,b);
					else if(bmono)
						*dst = (*dst & ~bitmask) | (COLORCONVMONO(r,g,b)<<bitshift);

					dst += pixelstep;

					r = (palRed[*(src+1)] + palRed[*(src+2)])>>1;
					g = (palGreen[*(src+1)] + palGreen[*(src+2)])>>1;
					b = (palBlue[*(src+1)] + palBlue[*(src+2)])>>1;

					if(b565)
						*(unsigned short*)dst = COLORCONV565(r,g,b);
					else if(b555)
						*(unsigned short*)dst = COLORCONV555(r,g,b);
					else if(bmono)
						*dst = (*dst & ~bitmask) | (COLORCONVMONO(r,g,b)<<bitshift);

					dst += pixelstep;

					r = (palRed[*(src+2)] + 3*palRed[*(src+3)])>>2;
					g = (palGreen[*(src+2)] + 3*palGreen[*(src+3)])>>2;
					b = (palBlue[*(src+2)] + 3*palBlue[*(src+3)])>>2;

					if(b565)
						*(unsigned short*)dst = COLORCONV565(r,g,b);
					else if(b555)
						*(unsigned short*)dst = COLORCONV555(r,g,b);
					else if(bmono)
						*dst = (*dst & ~bitmask) | (COLORCONVMONO(r,g,b)<<bitshift);

					dst += pixelstep;

					src += 4;
				}
				else
				{
					if((unsigned long)src & 3)
					{
						if(bmono)
							*dst = ((*dst)&~bitmask)|(pal[*src]<<bitshift);
						else if(bpalette)
							*dst = *src+10; // YES!!!
						else
							*(unsigned short*)dst = pal[*src];
						dst += pixelstep;
					}
					src ++;
				}
			}
			if(bmono)
			{
				bitshift += gxdp.cBPP;
				if(bitshift >= 8)
				{
					bitshift = 0;
					bitmask = (1<<gxdp.cBPP)-1;
					scraddr += linestep;
				}
				else
					bitmask <<= gxdp.cBPP;
			}
			else
				scraddr += linestep;
			scr_ptr += GFX_WIDTH;
			src_limit += GFX_WIDTH;
		}

		GXEndDraw();
	}
}


LRESULT CALLBACK WindowProc(HWND hwnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	HDC          hDC;
	PAINTSTRUCT  ps;
	int          key = 0;
	static		 SHACTIVATEINFO sai;
	RECT		 rc;

	switch (nMsg)
	{
	case WM_CREATE:
		memset(&sai, 0, sizeof(sai));
		SHSipPreference(hwnd, SIP_INPUTDIALOG);
		return 0;
	case WM_DESTROY:
		wince_deinit_vidmode ();
		exit (-1);
		return 0;
	case WM_ERASEBKGND:
		{
			GetClientRect(hwnd, &rc);
			rc.top = 200;
			hDC = GetDC(hwnd);
			if(rc.top < rc.bottom)
				FillRect(hDC, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
			ReleaseDC(hwnd, hDC);
		}
		return 1;
	case WM_PAINT:
		hDC = BeginPaint (hwndMain, &ps);
		EndPaint (hwndMain, &ps);
		SHSipPreference(hwndMain, SIP_UP); /* Hack! */
		adjust_extra_windows();
		/* It does not happen often but I don't want to see tooltip traces */
		wince_put_block(0, 0, 319, 199);
		return 0;
	case WM_ACTIVATE:
		if(!active)
		{
			active = true;
			GXResume();
		}
		SHHandleWMActivate(hwnd, wParam, lParam, &sai, SHA_INPUTDIALOG);
		adjust_extra_windows();
		palette_update();
		return 0;
	case WM_HIBERNATE:
		if(active)
		{
			active = false;
			GXSuspend();
		}
		return 0;
	case WM_SETTINGCHANGE:
		SHHandleWMSettingChange(hwnd, wParam, lParam, &sai);
		adjust_extra_windows();
		return 0;
	case WM_COMMAND:
		switch(wParam)
		{
		case IDC_ABOUT:
			message_box("Pocket Sarien R4 (" __DATE__ ")\n\n"
						"Ported by Vasyl Tsvirkunov\n"
						"http://pocketatari.\n"
						"               retrogames.com\n\n"
						"Visit Sarien homepage at\n"
						"http://sarien.sourceforge.net");
			break;
		case IDC_EXIT:
			DestroyWindow(hwndMain);
			break;
		}
		return 0;

	/* Multimedia functions
	 * (Damn! The CALLBACK_FUNCTION parameter doesn't work!)
	 */
	/* VT: I think it works but I don't care to test at this moment */
	case MM_WOM_DONE:
		flush_sound ((PWAVEHDR) lParam);
		return 0;

	case WM_LBUTTONDOWN:
		if(HIWORD(lParam)<200)
		{
			key = BUTTON_LEFT;
			mouse.button = TRUE;
			mouse.x = LOWORD(lParam)*4/3;
			mouse.y = HIWORD(lParam);
		}
		break;

	case WM_RBUTTONDOWN:
		if(HIWORD(lParam)<200)
		{
			key = BUTTON_RIGHT;
			mouse.button = TRUE;
			mouse.x = LOWORD(lParam)*4/3;
			mouse.y = HIWORD(lParam);
		}
		break;

	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		mouse.button = FALSE;
		return 0;

	case WM_MOUSEMOVE:
		if(HIWORD(lParam)<200)
		{
			mouse.x = LOWORD(lParam)*4/3;
			mouse.y = HIWORD(lParam);
		}
		return 0;

	case WM_KEYDOWN:
		switch(wParam)
		{
		case VK_UP:
			if(!(lParam & REPEATED_KEYMASK))
				key = KEY_UP;
			break;
		case VK_LEFT:
			if(!(lParam & REPEATED_KEYMASK))
				key = KEY_LEFT;
			break;
		case VK_DOWN:
			if(!(lParam & REPEATED_KEYMASK))
				key = KEY_DOWN;
			break;
		case VK_RIGHT:
			if(!(lParam & REPEATED_KEYMASK))
				key = KEY_RIGHT;
			break;
		case VK_HOME:
			if(!(lParam & REPEATED_KEYMASK))
			key = KEY_UP_LEFT;
				break;
		case VK_PRIOR:
			if(!(lParam & REPEATED_KEYMASK))
				key = KEY_UP_RIGHT;
			break;
		case VK_NEXT:
			if(!(lParam & REPEATED_KEYMASK))
				key = KEY_DOWN_RIGHT;
			break;
		case VK_END:
			if(!(lParam & REPEATED_KEYMASK))
				key = KEY_DOWN_LEFT;
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
		case VK_SNAPSHOT:
			key = KEY_PRIORITY;
			break;
		default:
			if(key == gxkl.vkA)
			{
				key = KEY_ENTER;
				break;
			}
			else if(key == gxkl.vkB)
			{
				key = KEY_ESCAPE;
				break;
			}
#ifdef USE_CONSOLE
			else if(key == gxkl.vkC)
			{
				key = CONSOLE_ACTIVATE_KEY;
				break;
			}
#endif
		};
		break;

	case WM_CHAR:
		if(!(lParam & RELEASED_KEYMASK)) /* pressed? */
		{
			key = (int)wParam;
			if(key == '\\')
				key = KEY_ESCAPE; /* menu */
		}
		else
			return 0;
		break;
	};

	/* Keyboard message handled */
	if (key) {
		key_enqueue (key);
		return 0;
	}

	return DefWindowProc (hwnd, nMsg, wParam, lParam);
}

/* Window for F-key input. Hack but some people asked for it. */
LRESULT CALLBACK FWindowProc(HWND hwnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	switch (nMsg)
	{
	case WM_CREATE:
		return 0;
	case WM_PAINT:
		{
			HDC          hDC;
			PAINTSTRUCT  ps;
			RECT		 rc;
			hDC = BeginPaint(hwnd, &ps);
			GetClientRect(hwnd, &rc);
			HGDIOBJ oldPen = SelectObject(hDC, (HGDIOBJ)GetStockObject(BLACK_PEN));
			HGDIOBJ oldBr = SelectObject(hDC, (HGDIOBJ)GetStockObject(WHITE_BRUSH));
			HGDIOBJ oldFont = SelectObject(hDC, (HGDIOBJ)GetStockObject(SYSTEM_FONT));
			int rcWidth = rc.right-rc.left;
			RECT rcItem;
			rcItem.top = rc.top;
			rcItem.bottom = rc.bottom;
			POINT pts[2];
			pts[0].y = rc.top;
			pts[1].y = rc.bottom;
			TCHAR text[4];
			for(int i=0; i<10; i++)
			{
				wsprintf(text, TEXT("F%d"), i+1);
				rcItem.left = rc.left+rcWidth*i/10;
				rcItem.right = rc.left+rcWidth*(i+1)/10;
				pts[0].x = pts[1].x = rcItem.right;
				Polyline(hDC, pts, 2);
				DrawText(hDC, text, -1, &rcItem, DT_CENTER|DT_VCENTER);
			}
			SelectObject(hDC, oldPen);
			SelectObject(hDC, oldBr);
			SelectObject(hDC, oldFont);
			EndPaint(hwnd, &ps);
		}
		return 0;
	case WM_LBUTTONDOWN:
		{
			int x = LOWORD(lParam);
			RECT rc; GetWindowRect(hwnd, &rc);
			int fnum = x*10/(rc.right-rc.left);
			PostMessage(hwndMain, WM_KEYDOWN, VK_F1+fnum, 0);
		}
		return 0;
	}

	return DefWindowProc(hwnd, nMsg, wParam, lParam);
}

void setup_extra_windows(HWND hwndTop)
{
	LPTSTR fkeyname = TEXT("fkeys");

	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = FWindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hIcon = NULL;
	wc.hCursor = NULL;
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = fkeyname;
	RegisterClass(&wc);
	
	hwndFKeys = CreateWindow(fkeyname,
		fkeyname,
		WS_VISIBLE|WS_CHILD,
		0,
		200,
		GetSystemMetrics(SM_CXSCREEN),
		20,
		hwndTop,
		(HMENU)100,
		GetModuleHandle(NULL),
		NULL);
}

void adjust_extra_windows()
{
	SIPINFO si;
	si.cbSize = sizeof(SIPINFO);
	SHSipInfo(SPI_GETSIPINFO, 0, &si, 0);
	if(si.fdwFlags & SIPF_ON)
	{
		int bottom = si.rcVisibleDesktop.bottom;
		SetWindowPos(hwndFKeys, 0, 0, 200, GetSystemMetrics(SM_CXSCREEN), bottom-200,
			SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOZORDER);
	}
}
