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
 * Massively modified by Vasyl Tsvirkunov <vasyl@pacbell.net> for Pocket PC/WinCE port
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
#include "win32.h"

#include "resource.h"

static LPTSTR szAppName = TEXT("Pocket Sarien");
static GXDisplayProperties gxdp;
static GXKeyList gxkl;
static bool active = true;

static HWND hwndMB = NULL;

typedef unsigned char UBYTE;
static UBYTE* screen;
static unsigned short* pal;

static int	wince_init_vidmode	(void);
static int	wince_deinit_vidmode(void);
static void	wince_put_block		(int, int, int, int);
static void	wince_put_pixels	(int, int, int, BYTE *);
static int	wince_keypress		(void);
static int	wince_get_key		(void);
static void	wince_new_timer		(void);

LRESULT CALLBACK WindowProc(HWND hwnd, UINT nMsg, WPARAM wParam, LPARAM lParam);


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
	if(gxdp.ffFormat & kfDirect565)
		pal[ent] = ((r&0xf8)<<(11-3))|((g&0xfc)<<(5-2))|((b&0xf8)>>3);
	else if(gxdp.ffFormat & kfDirect555)
		pal[ent] = ((r&0xf8)<<(10-3))|((g&0xf8)<<(5-2))|((b&0xf8)>>3);
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
	pal = new unsigned short[256];

	if(!screen || !pal)
		return err_Unk;

	memset(screen, 0, GFX_WIDTH*GFX_HEIGHT);

	GXOpenDisplay(hwndMain, GX_FULLSCREEN);
	GXOpenInput();

	SetWindowPos(hwndMain, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
	SetForegroundWindow(hwndMain);
	SHFullScreen(hwndMain, SHFS_SHOWSIPBUTTON);
	SHFullScreen(hwndMain, SHFS_HIDETASKBAR);
	SHSipPreference(hwndMain, SIP_UP);

	gxdp = GXGetDisplayProperties();
	gxkl = GXGetDefaultKeys(GX_NORMALKEYS);

	int i;
	for(i=0; i<16; i++)
		set_palette(i, palette[i*3+0]<<2, palette[i*3+1]<<2, palette[i*3+2]<<2);
	for(i=17; i<256; i++)
		set_palette(i, 0, 255, 0);


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

/* Somehow this function get called with unchecked bounds causing visual artefacts */
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

	scr_ptr = screen + x1 + y1*GFX_WIDTH;
	scr_ptr_limit = screen + x2 + y2*GFX_WIDTH;

	linestep = gxdp.cbyPitch;
	pixelstep = gxdp.cbxPitch;

	scraddr = (UBYTE*)GXBeginDraw();
	if(scraddr)
	{
		scraddr += (x1*3/4)*pixelstep + y1*linestep;
		src_limit = scr_ptr + x2-x1+1;

		while(scr_ptr < scr_ptr_limit)
		{
			src = scr_ptr;
			dst = scraddr;
			while(src < src_limit)
			{
				if((unsigned long)src & 3)
				{
					*(unsigned short*)dst = pal[*src];
					dst += pixelstep;
				}
				src ++;
			}
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
	int x, y;
	RECT		 rc;

	switch (nMsg) {
	case WM_CREATE:
		memset(&sai, 0, sizeof(sai));
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
		return 0;
	case WM_ACTIVATE:
		if(!active)
		{
			active = true;
			GXResume();
			SetForegroundWindow(hwndMain);
			SHFullScreen(hwndMain, SHFS_SHOWSIPBUTTON);
			SHFullScreen(hwndMain, SHFS_HIDETASKBAR);
		}
		SHHandleWMActivate(hwnd, wParam, lParam, &sai, 0);
		SHSipPreference(hwndMain, SIP_UP);
		return 0;
	case WM_HIBERNATE:
		if(active)
		{
			active = false;
			GXSuspend();
		}
		return 0;
//	case WM_SETTINGCHANGE:
//		SHHandleWMSettingChange(hwnd, wParam, lParam, &sai);
//		return 0;
	case WM_COMMAND:
		switch(wParam)
		{
		case IDC_ABOUT:
			SHSipPreference(hwndMain, SIP_DOWN);
			GXSuspend();
			active = false;
			MessageBox(hwndMain, TEXT("Pocket Sarien rev.2\n\nPorted by Vasyl Tsvirkunov\nhttp://pocketatari.retrogames.com"),
				TEXT("Pocket Sarien"), MB_OK|MB_APPLMODAL);
			active = true;
			GXResume();
			wince_put_block(0, 0, 319, 199);
			SetForegroundWindow(hwndMain);
			SHFullScreen(hwndMain, SHFS_SHOWSIPBUTTON);
			SHFullScreen(hwndMain, SHFS_HIDETASKBAR);
			SHSipPreference(hwndMain, SIP_UP);
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
		key = BUTTON_LEFT;
		mouse.button = TRUE;
		mouse.x = LOWORD(lParam)*4/3;
		mouse.y = HIWORD(lParam);
		break;

	case WM_RBUTTONDOWN:
		key = BUTTON_RIGHT;
		mouse.button = TRUE;
		mouse.x = LOWORD(lParam)*4/3;
		mouse.y = HIWORD(lParam);
		break;

	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		mouse.button = FALSE;
		return 0;

	case WM_MOUSEMOVE:
		mouse.x = LOWORD(lParam)*4/3;
		mouse.y = HIWORD(lParam);
		return 0;

	case WM_KEYDOWN:
		/* Keycode debug:
		 * report ("%02x\n", (int)wParam);
		 */
		switch (key = (int)wParam) {
		case VK_LWIN:
		case VK_RWIN: /* don't ask */
		case VK_SHIFT:
			key = 0;
			break;
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
		case VK_RETURN:
			key = KEY_ENTER;
			break;
		case VK_BACK:
			key = KEY_BACKSPACE;
			break;
		case VK_ADD:
			key = '+';
			break;
		case VK_SUBTRACT:
			key = '-';
			break;
/*
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
*/
		case VK_BACKSLASH:
		case VK_ESCAPE:
			key = KEY_ESCAPE;
			break;
		case VK_COMMA:
			key = GetKeyState (VK_SHIFT) & 0x8000 ? '<' : ',';
			break;
		case VK_PERIOD:
			key = GetKeyState (VK_SHIFT) & 0x8000 ? '>' : '.';
			break;
		case VK_SLASH:
			key = GetKeyState (VK_SHIFT) & 0x8000 ? '?' : '/';
			break;
		case VK_SPACE:
			key = ' ';
			break;
/*
#ifdef USE_CONSOLE
		case 192:
			key = CONSOLE_ACTIVATE_KEY;
			break;
		case 193:
			key = CONSOLE_SWITCH_KEY;
			break;
#endif
*/
		default:
			if(key == gxkl.vkA)
			{
				key = KEY_ENTER;
				break;
			}
			else if(key == gxkl.vkB)
			{
				key = 0x1b;
				break;
			}
			else if(key == gxkl.vkC)
			{
				key = CONSOLE_ACTIVATE_KEY;
				break;
			}
			if (!isprint(key))
				break;


			/* Must exist a better way to do that! */
			if (GetKeyState (VK_CAPITAL) & 0x1) {
				if (GetKeyState (VK_SHIFT) & 0x8000)
					key = key + 32;
			} else {
				if (!(GetKeyState (VK_SHIFT) & 0x8000))
					key = key + 32;
			}
#if 0
			/* Control and Alt modifier */
			if (GetAsyncKeyState (VK_CONTROL) & 0x8000)
				key = (key & ~0x20) - 0x40;
			else 
				if (GetAsyncKeyState (VK_MENU) & 0x8000)
					key = scancode_table[(key & ~0x20) - 0x41] << 8;
#endif

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
