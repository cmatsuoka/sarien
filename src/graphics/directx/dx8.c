/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.  */

/*
 * DirectX8 driver by Michael Montague <mikem@tarix.dhs.org>
 */

/*
 * Starting with DirectX 8 there is no 2D only version, so we draw to a 
 * texture and then project that texture orthographically to fill the 
 * entire screen.  This also paves the way for an OpenGL version which may 
 * be useful on many platforms.
 */

/* this allows us to compile without DX8 libs */
#ifdef USE_DIRECTX

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <stdio.h>
#include <process.h>
#include <d3d8.h>

#include "sarien.h"
#include "graphics.h"
#include "keyboard.h"
#include "console.h"
#include "win32.h"

#define TICK_SECONDS		36
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


enum {
	WM_PUT_BLOCK = WM_USER + 1
};

static UINT16 g_err = err_OK;
static const char g_szMainWndClass[] = "SarienWin";
static IDirect3D8*		pD3D = NULL;
static IDirect3DDevice8*	d3dDevice = NULL;
static IDirect3DTexture8*	texScreen = NULL;
static IDirect3DSurface8*	texSurface = NULL;
static IDirect3DVertexBuffer8*	vertBuf = NULL;
static int screen_x, screen_y;
static int tex_x, tex_y;

static struct{
	int start;
	int end;
	int queue[KEY_QUEUE_SIZE];
	CRITICAL_SECTION cs;
} g_key_queue = { 0, 0 };

static int	init_d3d		(void);
static int	deinit_d3d		(void);
static void	win32_put_block	(int, int, int, int);
static void _putpixels_d3d  (int x, int y, int w, BYTE *p);
static int	win32_keypress	(void);
static int	win32_get_key	(void);
static void	win32_new_timer	(void);
static int	set_palette		(UINT8 *, int, int);

struct gfx_driver gfx_d3d = {
	init_d3d,
	deinit_d3d,
	win32_put_block,
	_putpixels_d3d,
	win32_new_timer,
	win32_keypress,
	win32_get_key
};

static void wm_paint();
static void wm_put_block();

extern struct sarien_options opt;
extern struct gfx_driver *gfx;

static char *apptext = TITLE " " VERSION;
static HDC  hDC;
static WNDCLASS wndclass;
static int xsize, ysize;
	
#define ASPECT_RATIO(x) ((x) * 6 / 5)


static void update_mouse_pos(int x, int y)
{
	mouse.x = x;
	mouse.y = y;
	if (opt.scale != 0) {
		mouse.x /= opt.scale;
		mouse.y /= opt.scale;
	}

	/* for mouse we make the inverse transform of ASPECT_RATIO */
	if (opt.fixratio)
		mouse.y = mouse.y * 5 / 6;
}

LRESULT CALLBACK DirectXProc (HWND hwnd, UINT nMsg, WPARAM wParam, LPARAM lParam) {
	int          key = 0;

	switch (nMsg) {
	case WM_PUT_BLOCK:
		wm_put_block();
		break;

	case WM_DESTROY:
		deinit_d3d ();
		exit (-1);
		return 0;

	case WM_PAINT:
		wm_paint();
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
		switch (key = (int)wParam) {
		case VK_SHIFT:
			key = 0;
			break;
		case VK_CONTROL:
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
			key = GetAsyncKeyState (VK_SHIFT) & 0x8000 ? ':' : ';';
			break;
		case 0xbb:
			key = GetAsyncKeyState (VK_SHIFT) & 0x8000 ? '+' : '=';
			break;
		case 0xbc:
			key = GetAsyncKeyState (VK_SHIFT) & 0x8000 ? '<' : ',';
			break;
		case 0xbd:
			key = GetAsyncKeyState (VK_SHIFT) & 0x8000 ? '_' : '-';
			break;
		case 0xbe:
			key = GetAsyncKeyState (VK_SHIFT) & 0x8000 ? '>' : '.';
			break;
		case 0xbf:
			key = GetAsyncKeyState (VK_SHIFT) & 0x8000 ? '?' : '/';
			break;
		case 0xdb:
			key = GetAsyncKeyState (VK_SHIFT) & 0x8000 ? '{' : '[';
			break;
		case 0xdc:
			key = GetAsyncKeyState (VK_SHIFT) & 0x8000 ? '|' : '\\';
			break;
		case 0xdd:
			key = GetAsyncKeyState (VK_SHIFT) & 0x8000 ? '}' : ']';
			break;
		case 0xde:
			key = GetAsyncKeyState (VK_SHIFT) & 0x8000 ? '"' : '\'';
			break;
		case 192:
			key = GetAsyncKeyState (VK_SHIFT) & 0x8000 ? '~' : '`';
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

		_D (": key = 0x%02x ('%c')", key, isprint(key) ? key : '?');

		/* Cancel "alt" keybind to toggle.monitor (huh?) */
		if (key == 0x12)
			key = 0;

		break;
	};
			
	/* Keyboard message handled */
	if (key) {
		key_enqueue (key);
		return 0;
	}

	return DefWindowProc (hwnd, nMsg, wParam, lParam);
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

/* put a block onto the screen */
static void win32_put_block (int x1, int y1, int x2, int y2)
{
	PostMessage (hwndMain, WM_PUT_BLOCK, 0, 0);
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


typedef struct {
    FLOAT x, y, z, rhw; /* The transformed position for the vertex. */
    DWORD color;        /* The vertex color. */
    FLOAT u, v;		/* The texture coordinates. */
} CUSTOMVERTEX;

#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1)

CUSTOMVERTEX ScreenVerts[] = {
	/* x, y, z, rhw, color, tu, tv */
	{   0.0f,   0.0f, 0.0f, 1.0f, 0xffffffff, 0.0f, 0.0f },
	{ 640.0f,   0.0f, 0.0f, 1.0f, 0xffffffff, 1.0f, 0.0f },
	{   0.0f, 480.0f, 0.0f, 1.0f, 0xffffffff, 0.0f, 1.0f },
	{ 640.0f, 480.0f, 0.0f, 1.0f, 0xffffffff, 1.0f, 1.0f },
};

/* ====================================================================*/

static void _putpixels_d3d (int x, int y, int w, BYTE *p)
{
	int posx;
	int pitch;
	BYTE* pix;
	HRESULT res;
	D3DLOCKED_RECT rect;

	/* set our start position in the buffer */
	posx = x; 
	/* lock our buffer */
	res = IDirect3DSurface8_LockRect(texSurface, &rect, NULL, 0);
	if (D3D_OK != res) {
		OutputDebugString ("win32.c: putpixels_d3d(): "
			"failed to lock texture surface\n");
		return;
	}
	/* grab the bits pointer */
	pix = (BYTE*)rect.pBits + (y*rect.Pitch) + x;
	/* calculate the excess pitch */
	pitch = rect.Pitch - tex_x;
	/* write all the pixels */
	while (w--)
	{
		*pix++ = *p++;
		if (++posx >= tex_x)
		{
			pix += pitch;
			posx = 0;
		}
	}
	/* unlock the buffer */
	IDirect3DSurface8_UnlockRect(texSurface);
}

static void wm_put_block ()
{
	/* clear the back buffer */
	IDirect3DDevice8_Clear(d3dDevice, 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0,0,0), 1.0f, 0);
	/* Begin the scene */
	IDirect3DDevice8_BeginScene(d3dDevice);
	/* set our current palette (0=Amiga,1=EGA) */
	IDirect3DDevice8_SetCurrentTexturePalette(d3dDevice, 0);
	/* setup texture to render */
	IDirect3DDevice8_SetTexture(d3dDevice, 0, (IDirect3DBaseTexture8*)texScreen);
	IDirect3DDevice8_SetTextureStageState(d3dDevice, 0, D3DTSS_COLOROP,   D3DTOP_MODULATE);
	IDirect3DDevice8_SetTextureStageState(d3dDevice, 0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	IDirect3DDevice8_SetTextureStageState(d3dDevice, 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	IDirect3DDevice8_SetTextureStageState(d3dDevice, 0, D3DTSS_ALPHAOP,   D3DTOP_DISABLE);
	/* render a quad with our texture */
	IDirect3DDevice8_SetStreamSource(d3dDevice, 0, vertBuf, sizeof(CUSTOMVERTEX));
	IDirect3DDevice8_SetVertexShader(d3dDevice, D3DFVF_CUSTOMVERTEX );
	IDirect3DDevice8_DrawPrimitive(d3dDevice, D3DPT_TRIANGLESTRIP, 0, 2);
	/* End the scene */
	IDirect3DDevice8_EndScene(d3dDevice);
	/* flip the buffer */
	IDirect3DDevice8_Present(d3dDevice, NULL, NULL, NULL, NULL);
}

static void wm_paint ()
{
	/* make sure windows knows the window is updated */
	ValidateRect(hwndMain, NULL);
	/* draw the AGI screen to our screen */
	wm_put_block();
}

static int init_d3d ()
{
	int i, p;
	VOID* ptr;
	HRESULT res;
	WNDCLASSEX wc;
	D3DDISPLAYMODE d3ddm;
	D3DPRESENT_PARAMETERS d3dpp;
	PALETTEENTRY* pal;

	InitializeCriticalSection (&g_key_queue.cs);

	tex_x = GFX_WIDTH;
	tex_y = GFX_HEIGHT;

	/* for fullscreen we always set 640x480 */
	if (opt.fullscreen) {
		screen_x = 640;
		screen_y = 480;
	} else {

		screen_x = GFX_WIDTH * opt.scale;
		screen_y = (opt.fixratio ? ASPECT_RATIO(GFX_HEIGHT) : GFX_HEIGHT) * opt.scale;
	}

	/* Register the window class. */
	memset(&wc, 0, sizeof(WNDCLASSEX));
	wc.cbSize			   = sizeof(WNDCLASSEX);
	wc.style			   = CS_CLASSDC;
	wc.lpfnWndProc		   = DirectXProc;
	wc.hInstance		   = GetModuleHandle(NULL);
	wc.lpszClassName	   = g_szMainWndClass;
	wndclass.hIcon         = LoadIcon (NULL, IDI_APPLICATION);
	wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);
	wndclass.hbrBackground = GetStockObject (BLACK_BRUSH);
    if (!RegisterClassEx(&wc)) {
		OutputDebugString("win32.c: init_d3d(): can't register class\n");
		g_err = err_Unk;
		goto exx;
	}

	/* Create the application's window. */
	if (opt.fullscreen){
		/* fullscreen */
		hwndMain  = CreateWindow(
			g_szMainWndClass,
			apptext,
			WS_OVERLAPPED,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			screen_x,
			screen_y,
			GetDesktopWindow(), 
			NULL, 
			wc.hInstance, 
			NULL 
		);
	} else {
		/* windowed */
		hwndMain  = CreateWindow(
			g_szMainWndClass,
			apptext,
			WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			screen_x + GetSystemMetrics (SM_CXFRAME),
			screen_y + GetSystemMetrics (SM_CYCAPTION) + GetSystemMetrics (SM_CYFRAME), 
			GetDesktopWindow(), 
			NULL, 
			wc.hInstance, 
			NULL 
		);
	}

	if (NULL == hwndMain) {
		OutputDebugString("win32.c: init_d3d(): can't register class\n");
		g_err = err_Unk;
		goto exx;
	}

	/* get point to d3d interface */
	pD3D = Direct3DCreate8(D3D_SDK_VERSION);
	if (NULL == pD3D) {
		OutputDebugString("win32.c: init_d3d(): can't create dx8 interface\n");
		g_err = err_Unk;
		goto exx;
	}

	res = IDirect3D8_GetAdapterDisplayMode(pD3D, D3DADAPTER_DEFAULT, &d3ddm);
	if (D3D_OK != res) {
		OutputDebugString("win32.c: init_d3d(): can't get adapter display mode\n");
		g_err = err_Unk;
		goto exx;
	}

	/* setup the render properties we want */
	memset(&d3dpp, 0, sizeof(d3dpp));
	if (opt.fullscreen) {
		/* fullscreen */
		d3dpp.Windowed		   = FALSE;
		d3dpp.SwapEffect	   = D3DSWAPEFFECT_FLIP;
		d3dpp.BackBufferWidth  = 640;
		d3dpp.BackBufferHeight = 480;
		d3dpp.BackBufferFormat = d3ddm.Format;
		d3dpp.FullScreen_RefreshRateInHz = 60;
		/* back buffer format */
		d3ddm.Width		  = 640;
		d3ddm.Height	  = 480;
		d3ddm.RefreshRate = 60;
		d3ddm.Format	  = D3DFMT_A8R8G8B8;
	} else {
		/* windowed */
		d3dpp.Windowed   = TRUE;
		d3dpp.SwapEffect = D3DSWAPEFFECT_FLIP;
		d3dpp.BackBufferFormat = d3ddm.Format;
	}

	/* create our actual rendering device */
	res = IDirect3D8_CreateDevice(pD3D, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwndMain, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &d3dDevice);
	if (D3D_OK != res) {
		OutputDebugString("win32.c: init_d3d(): can't create device\n");
		g_err = err_Unk;
		goto exx;
	}

	/* create a buffer for our palettes */
	pal = (PALETTEENTRY*)malloc(sizeof(PALETTEENTRY)*256);
	memset(pal, 0, (sizeof(PALETTEENTRY)*256) );
	/* set the amiga palette */
	for (i=0, p=0; i<32; i++)
	{
		pal[i].peRed   = palette[p++] << 2;
		pal[i].peGreen = palette[p++] << 2;
		pal[i].peBlue  = palette[p++] << 2;
		pal[i].peFlags = 0;
	}
	/* give it to direct 3d */
	res = IDirect3DDevice8_SetPaletteEntries(d3dDevice, 0, pal);
	if (D3D_OK != res) {
		OutputDebugString("win32.c: init_d3d(): failed to set amiga palette\n");
		g_err = err_Unk;
		goto exx;
	}
	/* clear it */
	memset(pal, 0, (sizeof(PALETTEENTRY)*256) );
	/* set the ega palette */
	for (i=0, p=0; i<16; i++)
	{
		pal[i].peRed   = palette[p++] << 2;
		pal[i].peGreen = palette[p++] << 2;
		pal[i].peBlue  = palette[p++] << 2;
		pal[i].peFlags = 0;
	}
	/* give it to direct 3d */
	res = IDirect3DDevice8_SetPaletteEntries(d3dDevice, 1, pal);
	if (D3D_OK != res) {
		OutputDebugString("win32.c: init_d3d(): failed to set ega palette\n");
		g_err = err_Unk;
		goto exx;
	}
	/* free the memory */
	free(pal);

	/* create our texture, we make it 512x256 so that it's a power of 2 */
	res = IDirect3DDevice8_CreateTexture(d3dDevice, 512, 256, 1, 0, D3DFMT_P8, D3DPOOL_MANAGED, &texScreen);
	if (D3D_OK != res) {
		OutputDebugString("win32.c: init_dx8(): failed to create screen\n");
		g_err = err_Unk;
		goto exx;
	}

	/* get a surface to our texture */
	res = IDirect3DTexture8_GetSurfaceLevel(texScreen, 0, &texSurface);
	if (D3D_OK != res) {
		OutputDebugString("win32.c: init_dx8(): failed to get surface texture\n");
		g_err = err_Unk;
		goto exx;
	}

	/* setup the verts to draw the scene and stretch the texture appropriately */
	ScreenVerts[1].x = (float)screen_x;						/* right */
	ScreenVerts[1].u = ((float)tex_x) / 512.0f;
	ScreenVerts[2].y = (float)screen_y;						/* bottom */
	ScreenVerts[2].v = ((float)tex_y) / 256.0f;
	ScreenVerts[3].x = (float)screen_x;						/* bottom right */
	ScreenVerts[3].y = (float)screen_y;
	ScreenVerts[3].u = ((float)tex_x) / 512.0f;
	ScreenVerts[3].v = ((float)tex_y) / 256.0f;

	/* setup our vertex buffers */
	res = IDirect3DDevice8_CreateVertexBuffer(d3dDevice, 3*sizeof(CUSTOMVERTEX), 0 /* Usage */, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &vertBuf);
	if (D3D_OK != res) {
		OutputDebugString("win32.c: init_dx8(): failed to create vertex buffer\n");
		g_err = err_Unk;
		goto exx;
	}
	/* copy our verts in */
	res = IDirect3DVertexBuffer8_Lock(vertBuf, 0, sizeof(ScreenVerts), (BYTE**)&ptr, 0);
	if (D3D_OK != res) {
		OutputDebugString("win32.c: init_dx8(): failed lock vertex buffer\n");
		g_err = err_Unk;
		goto exx;
	}
	memcpy(ptr, ScreenVerts, sizeof(ScreenVerts));
	IDirect3DVertexBuffer8_Unlock(vertBuf);

	/* if we got this far we're good to go */
	ShowWindow (hwndMain, TRUE);
	UpdateWindow (hwndMain);
	g_err = err_OK;

exx:
	return g_err;	
}

static int deinit_d3d (void)
{
	/* remove critial section */
	DeleteCriticalSection(&g_key_queue.cs);
	/* release d3d resources */
	IDirect3DVertexBuffer8_Release(vertBuf);
	IDirect3DSurface8_Release(texSurface);
	IDirect3DTexture8_Release(texScreen);
	/* FIXME: these two currently crash */
/*	IDirect3DDevice8_Release(d3dDevice); */
/*	IDirect3D8_Release(pD3D); */
	/* get us out */
	PostMessage (hwndMain, WM_QUIT, 0, 0);

	return err_OK;
}

#endif /* USE_DIRECTX */