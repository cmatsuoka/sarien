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
 * Launcher window added by Ian Hanschen <ian@stardock.com>
 */

#include <windows.h>
#include <shlobj.h>
#include <stdio.h>
#include "sarien.h"
#include "agi.h"
#include "text.h"
#include "graphics.h"
#include "sprite.h"


volatile UINT32 clock_ticks;
volatile UINT32 clock_count;

struct sarien_options opt;
struct game_id_list game_info;
struct agi_game game;

extern struct gfx_driver *gfx;
extern struct gfx_driver gfx_win32;
#ifdef USE_DIRECTX
extern struct gfx_driver gfx_d3d;
#endif

#ifndef __GNUC__
void _D (char *x, ...)
{
#ifdef _TRACE
	OutputDebugString(x);
#endif
}
#endif


BOOL CheckForGame (char *szDir)
{
	WIN32_FIND_DATA ffd;
	HANDLE hFind;
	CHAR szDirLocal[MAX_PATH] = { 0 };
	CHAR szFullPath[MAX_PATH] = { 0 };
	BOOL bFound = FALSE;
	DWORD dwFileInfo;

	sprintf(szDirLocal, "%s\\words.tok", szDir);
	ZeroMemory(&ffd, sizeof(WIN32_FIND_DATA));
	hFind = FindFirstFile(szDirLocal, &ffd);

	if (hFind == INVALID_HANDLE_VALUE)
		return FALSE;

	do {
		sprintf (szFullPath, "%s\\%s", szDir, ffd.cFileName);
		dwFileInfo = GetFileAttributes(szFullPath);
		
		if (dwFileInfo != 0xFFFFFFFF &&
			dwFileInfo & ~FILE_ATTRIBUTE_DIRECTORY)
		{
			bFound = TRUE;
		}
	} while (0 != FindNextFile(hFind, &ffd) && bFound != TRUE);

	FindClose (hFind);

	return bFound;
}

int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg,LPARAM lp, LPARAM pData)
{
	CHAR szDir[MAX_PATH] = "";
	HKEY hKey            = NULL;
	DWORD dwDisposition  = 0;
	DWORD cbData         = MAX_PATH;

	switch(uMsg) {
	case BFFM_INITIALIZED: 
		if (ERROR_SUCCESS == RegCreateKeyEx (HKEY_CURRENT_USER,
			"SOFTWARE\\FreeAGI", 0, NULL, REG_OPTION_NON_VOLATILE,
			KEY_QUERY_VALUE, NULL, &hKey, &dwDisposition))
		{
			/* if the key exists, read the value from it */
			if (REG_OPENED_EXISTING_KEY == dwDisposition) {
				if (ERROR_SUCCESS != RegQueryValueEx (hKey,
					"LastFolder", NULL, NULL, 
					(unsigned char *)szDir, &cbData))
				{
					OutputDebugString ("winmain.c: "
					"BrowseCallbackProc(): "
					"RegQueryValueEx != ERROR_SUCESS");
				}
			}
		}

		SendMessage(hwnd,BFFM_SETSELECTION,TRUE,(LPARAM)szDir);

		if (ERROR_SUCCESS != RegCloseKey(hKey)) {
			OutputDebugString ("winmain.c: open_file(): "
				"RegCloseKey != ERROR_SUCESS");
		}
		break;

	case BFFM_SELCHANGED: 
		if (SHGetPathFromIDList((LPITEMIDLIST) lp ,szDir)) {
			if(CheckForGame(szDir)) {
				SendMessage (hwnd, BFFM_ENABLEOK, 0, TRUE);
				SendMessage (hwnd, BFFM_SETSTATUSTEXT, 0,
					(LPARAM)szDir);
			} else {
				SendMessage(hwnd, BFFM_ENABLEOK, 0, FALSE);
			}
		}
		break;
	}

	return 0;
}

void open_file (HWND hwnd)
{
	BROWSEINFO bi        = { 0 };
	CHAR szDir[MAX_PATH] = { 0 };
	LPITEMIDLIST pidl    = { 0 };
	LPMALLOC pMalloc     = NULL;
	HKEY hKey            = NULL;
	DWORD dwDisposition  = 0;
	char s[MAX_PATH]     = { 0 };

	if (game.state >= STATE_LOADED) {
		MessageBox(hwnd, "AGI game already loaded.", "Error", MB_OK);
		return;
	}

	if (!SUCCEEDED (SHGetMalloc(&pMalloc))) {
		OutputDebugString("open_file(): SHGetMalloc failed");
		exit(1);
	}

	bi.hwndOwner = NULL;
	bi.lpszTitle = "Select AGI game folder:"; 
	bi.pszDisplayName = 0;
	bi.pidlRoot = 0;
	/* USENEWUI flag has weird problem where "My Computer" is
	 * seen as a valid folder by CheckForGame. TODO: figure out
	 * why and fix.
	 */
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT;
	bi.lpfn = BrowseCallbackProc;

	if ((pidl = SHBrowseForFolder(&bi)) != 0) {
		if (SHGetPathFromIDList(pidl,szDir)) 
			strncpy(s, szDir, MAX_PATH);

		if (ERROR_SUCCESS != RegCreateKeyEx (HKEY_CURRENT_USER,
			"SOFTWARE\\FreeAGI", 0, NULL, REG_OPTION_NON_VOLATILE,
			KEY_SET_VALUE, NULL, &hKey, &dwDisposition))
		{
			OutputDebugString("winmain.c: open_file(): "
				"RegCreateKeyEx != ERROR_SUCCESS");
		}

		if (ERROR_SUCCESS != RegSetValueEx(hKey, "LastFolder", 0,
			REG_SZ, (const unsigned char *)szDir, MAX_PATH))
		{
			OutputDebugString ("winmain.c: open_file(): "
				"RegSetValueEx != ERROR_SUCCESS");
		}

		if (ERROR_SUCCESS != RegCloseKey(hKey)) {
			OutputDebugString("winmain.c: open_file(): "
			"RegCloseKey != ERROR_SUCCESS");
		}
	}

	/* not c++ so vtbl */
	pMalloc->lpVtbl->Free(pMalloc,pidl);
	pMalloc->lpVtbl->Release(pMalloc);

	stop_sound ();

	if (agi_detect_game (s) == err_OK) {
		game.state = STATE_RUNNING;
		return;
	}
}


BOOL CALLBACK
AboutDlgProc (HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch(Message) {
	case WM_INITDIALOG:
		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDOK:
			EndDialog(hwnd, IDOK);
			break;
		case IDCANCEL:
			EndDialog(hwnd, IDCANCEL);
			break;
		}
		break;
	default:
		return FALSE;
	}

	return TRUE;
}


int WINAPI
WinMain (HINSTANCE hThisInst, HINSTANCE hPrevInst, LPSTR lpszArgs, int nWinMode)
{
	int ec = err_OK;
	char *c, *t;
	char filename[MAX_PATH]	= "";
	
	game.clock_enabled = FALSE;
	game.state = STATE_INIT;

	/* Set defaults */
	memset (&opt, 0, sizeof (struct sarien_options));
	opt.gamerun = GAMERUN_RUNGAME;
	opt.scale = 2;
	opt.fixratio = TRUE;
	opt.gfxhacks = TRUE;
#ifdef USE_HIRES
	opt.hires = TRUE;
#endif
	opt.soundemu = SOUND_EMU_NONE;

	c = GetCommandLine();

	for (t = strtok(c, " "); t != NULL; t = strtok(NULL, " ")) {
#ifdef OPT_PICTURE_VIEWER
		if (!strcmp(t, "/p"))
			opt.gamerun = GAMERUN_PICVIEW;
#endif

		c = t;
	}

	if (*c)
		strcpy (filename, c);

	init_machine (1, 0);

	game.color_fg = 15;
	game.color_bg = 0;

	if ((game.sbuf = calloc (_WIDTH, _HEIGHT)) == NULL)
		goto bail_out;
#ifdef USE_HIRES
	if ((game.hires = calloc (_WIDTH * 2, _HEIGHT)) == NULL)
		goto bail_out2;
#endif
	if (init_sprites () != err_OK)
		goto bail_out3;

	if (init_video () != err_OK)
		goto bail_out4;
	
#ifdef OPT_PICTURE_VIEWER
	if (opt.gamerun == GAMERUN_PICVIEW) {
		console.y = 0;
		if (agi_detect_game (filename ? filename :
			get_current_directory ()) == err_OK)
		{
			game.state = STATE_LOADED;
		}
		do { main_cycle (); } while (game.state < STATE_LOADED);
		
		agi_init ();
		view_pictures ();

		goto bail_out;
	}
#endif

	console_init ();
	report ("--- Starting console ---\n\n");
	if (!opt.gfxhacks)
		report ("Graphics driver hacks disabled (if any)\n");

	_D ("Detect game");

	if (agi_detect_game (filename) == err_OK ||
		agi_detect_game (get_current_directory ()) == err_OK)
	{
		game.state = STATE_LOADED;
	} else {
		game.ver = -1;	/* Don't display the conf file warning */
	}

	_D ("Init sound");
	init_sound ();

	report (" \nSarien " VERSION " is ready.\n");
	if (game.state < STATE_LOADED) {
       		console_prompt ();
		do { main_cycle (); } while (game.state < STATE_RUNNING);
		game.ver = 0;	/* Display the conf file warning */
	}

	ec = run_game ();

	deinit_sound ();
	deinit_video ();
bail_out4:
	deinit_sprites ();
bail_out3:
#ifdef USE_HIRES
	free (game.hires);
#endif
bail_out2:
	free (game.sbuf);
bail_out:
	deinit_machine ();

	return ec;
}

int init_machine (int argc, char **argv)
{
#ifdef USE_DIRECTX
	gfx = &gfx_d3d;
#else
	gfx = &gfx_win32;
#endif

	return err_OK;
}

int deinit_machine ()
{
	return err_OK;
}


