/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#include <windows.h>
#include <shlobj.h>
#include <stdio.h>
#include "sarien.h"
#include "agi.h"
#include "text.h"
#include "graphics.h"


volatile UINT32 clock_ticks;
volatile UINT32 clock_count;

struct sarien_options opt;
struct game_id_list game_info;
struct agi_game game;

#ifndef _TRACE
void _D (char *s, ...) { }
#endif


BOOL CheckForGame(char *szDir)
{
	WIN32_FIND_DATA ffd;
	HANDLE hFind;
	CHAR szDirLocal[MAX_PATH]	= {0};
	CHAR szFullPath[MAX_PATH]	= {0};
	BOOL bFound = FALSE;
	DWORD dwFileInfo;

	sprintf(szDirLocal, "%s\\words.tok", szDir);
	
	ZeroMemory(&ffd, sizeof(WIN32_FIND_DATA));

	hFind = FindFirstFile(szDirLocal, &ffd);
	
	do
	{
		sprintf(szFullPath, "%s\\%s", szDir, ffd.cFileName);
		dwFileInfo = GetFileAttributes(szFullPath);
		
		if(dwFileInfo != 0xFFFFFFFF && dwFileInfo & ~FILE_ATTRIBUTE_DIRECTORY)
			bFound = TRUE;
	}
	while(0 != FindNextFile(hFind, &ffd) && bFound != TRUE);

	FindClose(hFind);

	return bFound;
}

int CALLBACK BrowseCallbackProc(HWND hwnd,UINT uMsg,LPARAM lp, LPARAM pData)
{
	CHAR szDir[MAX_PATH]	= {0};
	DWORD ret				= 0;
	HKEY hKey				= NULL;
	DWORD dwDisposition		= 0;
	DWORD cbData			= MAX_PATH;


	switch(uMsg) 
	{
		case BFFM_INITIALIZED: 
		{
			if (ERROR_SUCCESS == RegCreateKeyEx(
						HKEY_CURRENT_USER, "SOFTWARE\\FreeAGI",
						0, NULL, REG_OPTION_NON_VOLATILE, KEY_QUERY_VALUE, 
						NULL, &hKey, &dwDisposition))
			{
				/* if the key exists, read the value from it */
				if (REG_OPENED_EXISTING_KEY == dwDisposition)
				{
					if (ERROR_SUCCESS != RegQueryValueEx(
						hKey, "LastFolder", NULL, NULL, 
						(unsigned char *)szDir, &cbData))
					{
						OutputDebugString("winmain.c: BrowseCallbackProc(): RegQueryValueEx != ERROR_SUCESS");
					}
				}
			}

			SendMessage(hwnd,BFFM_SETSELECTION,TRUE,(LPARAM)szDir);

		if (ERROR_SUCCESS != RegCloseKey(hKey))
		{
			OutputDebugString("winmain.c: open_file(): RegCloseKey != ERROR_SUCESS");
		}


			break;

		}
		case BFFM_SELCHANGED: 
		if (SHGetPathFromIDList((LPITEMIDLIST) lp ,szDir)) 

		{
			if(CheckForGame(szDir))
			{
				SendMessage(hwnd, BFFM_ENABLEOK, 0, TRUE);
				SendMessage(hwnd,BFFM_SETSTATUSTEXT,0,(LPARAM)szDir);
			}
			else
				SendMessage(hwnd, BFFM_ENABLEOK, 0, FALSE);
	   }
	   break;
	default:
	   break;
	}
	return 0;
}

static void open_file (HINSTANCE hThisInst, char *s)
{
	BROWSEINFO bi			= {0};
	CHAR szDir[MAX_PATH]	= {0};
	LPITEMIDLIST pidl		= {0};
	LPMALLOC pMalloc		= NULL;
	HKEY hKey				= NULL;
	DWORD dwDisposition		= 0;


	if (SUCCEEDED(SHGetMalloc(&pMalloc)))
	{
		bi.hwndOwner = NULL;
		bi.lpszTitle = "Select AGI game folder:"; 
		bi.pszDisplayName = 0;
		bi.pidlRoot = 0;
		/* USENEWUI flag has weird problem where "My Computer" is seen as a 
		 * valid folder by CheckForGame. TODO: figure out why and fix. */
		bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT /*| BIF_USENEWUI */;
		bi.lpfn = BrowseCallbackProc;
		pidl = SHBrowseForFolder(&bi);
		if (pidl) 
			if (SHGetPathFromIDList(pidl,szDir)) 
				strncpy(s, szDir, MAX_PATH);

		if (ERROR_SUCCESS != RegCreateKeyEx(
					HKEY_CURRENT_USER, "SOFTWARE\\FreeAGI",
					0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, 
					NULL, &hKey, &dwDisposition))
		{
			OutputDebugString("winmain.c: open_file(): RegCreateKeyEx != ERROR_SUCESS");
		}

		if (ERROR_SUCCESS != RegSetValueEx(hKey, "LastFolder", 0, REG_SZ, (const unsigned char *)szDir, MAX_PATH))
		{
			OutputDebugString("winmain.c: open_file(): RegSetValueEx != ERROR_SUCESS");
		}

		if (ERROR_SUCCESS != RegCloseKey(hKey))
		{
			OutputDebugString("winmain.c: open_file(): RegCloseKey != ERROR_SUCESS");
		}

		/* not c++ so vtbl */
		pMalloc->lpVtbl->Free(pMalloc,pidl);
		pMalloc->lpVtbl->Release(pMalloc);
	   
	} else 
		{
			OutputDebugString("open_file(): SHGetMalloc failed");
			exit(1);
		}
}


int WINAPI WinMain(HINSTANCE hThisInst, HINSTANCE hPrevInst, LPSTR lpszArgs, int nWinMode)
{
	int ec				= err_OK;
	char *c				= NULL;
	char filename[MAX_PATH]	= {0};
	
	game.clock_enabled = FALSE;
	game.state = STATE_INIT;

	for (c = GetCommandLine(); *c && *c != 0x20; c++);

	if (*c)
		strcpy (filename, ++c);
	else
		open_file (hThisInst, filename);

	init_machine (1, 0);

	game.color_fg = 15;
	game.color_bg = 0;

	if (init_video () != err_OK) {
		ec = err_Unk;
		goto bail_out;
	}
	console_init ();
	report ("--- Starting console ---\n\n");

	if (!opt.gfxhacks)
		report ("Graphics driver hacks disabled (if any)\n");

	_D ("Detect game");

	if (	agi_detect_game (filename) == err_OK ||
		agi_detect_game (get_current_directory ()) == err_OK)
	{
		game.state = STATE_LOADED;
	}

	_D ("Init sound");

	init_sound ();

	report (" \nSarien " VERSION " is ready.\n");
	if (game.state < STATE_LOADED) {
       		console_prompt ();
		do { main_cycle (); } while (game.state < STATE_RUNNING);
	}

	/* Execute the game */
    	do 

		{
			_D(_D_WARN "game loop"); 

			ec = agi_init ();
			game.state = STATE_RUNNING;

			if (ec == err_OK) 

			{
   			/* setup machine specific AGI flags, etc */
    			setvar (V_computer, 0);	/* IBM PC */
    			setvar (V_soundgen, 1);	/* IBM PC SOUND */
    			setvar (V_max_input_chars, 38);
    			setvar (V_monitor, 0x3); /* EGA monitor */
	   			game.horizon = HORIZON;
				game.player_control = FALSE;

				ec = run_game();
    		}

    		/* deinit our resources */

			game.state = STATE_LOADED;
    		agi_deinit ();
    	} while (ec == err_RestartGame);

	deinit_sound ();
	deinit_video ();

bail_out:
	if (ec == err_OK || ec == err_DoNothing) {
		deinit_machine ();
		exit (ec);
	}

#if 0
	printf ("Error %04i: ", ec);

	switch (ec) {
	case err_BadCLISwitch:
		printf("Bad CLI switch.\n");
		break;
	case err_InvalidAGIFile:
		printf("Invalid or inexistent AGI file.\n");
		break;
	case err_BadFileOpen:
		printf("Unable to open file.\n");
		break;
	case err_NotEnoughMemory:
		printf("Not enough memory.\n");
		break;
	case err_BadResource:
		printf("Error in resource.\n");
		break;
	case err_UnknownAGIVersion:
		printf("Unknown AGI version.\n");
		break;
	case err_NoGameList:
		printf("No game ID List was found!\n");
		break;
	}
	printf("\nUse parameter -h to list the command line options\n");
#endif

	deinit_machine ();

	return ec;
}

