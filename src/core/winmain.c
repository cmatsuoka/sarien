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
#include <stdio.h>
#include "sarien.h"
#include "agi.h"
#include "text.h"
#include "graphics.h"


volatile UINT32 clock_ticks;
volatile UINT32 clock_count;

extern UINT8 *font, font_english[];

struct sarien_options opt;
struct game_id_list game_info;
struct agi_game game;

#ifndef _TRACE
INLINE void _D (char *s, ...) { }
#endif


static void open_file (HINSTANCE hThisInst, char *s)
{
	OPENFILENAME OpenFile;	/* Structure for Open common dialog box */
	
	ZeroMemory (&OpenFile, sizeof(OPENFILENAME));
	OpenFile.lStructSize = sizeof (OPENFILENAME);
	OpenFile.hwndOwner = HWND_DESKTOP;
	OpenFile.hInstance = hThisInst;
	OpenFile.lpstrFile = s;
	OpenFile.nMaxFile = MAX_PATH - 1;
	OpenFile.lpstrTitle = TITLE " " VERSION " - FIXME: OpenFile hack";
	OpenFile.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	
	GetOpenFileName(&OpenFile);
}


int WINAPI WinMain(HINSTANCE hThisInst, HINSTANCE hPrevInst, LPSTR lpszArgs, int nWinMode)
{
	int ec = err_OK;
	char *c, filename[MAX_PATH];
	
	game.clock_enabled = FALSE;
	game.state = STATE_INIT;

	for (c = GetCommandLine(); *c && *c != 0x20; c++);

	if (*c)
		strcpy (filename, ++c);
	else
		open_file (hThisInst, filename);

	init_machine (1, 0);
	game.gfx_mode = TRUE;
	game.color_fg = 15;
	game.color_bg = 0;

	font = font_english;

	if (init_video () != err_OK) {
		ec = err_Unk;
		goto bail_out;
	}
	console_init ();
	report ("--- Starting console ---\n\n");
	if (!opt.gfxhacks)
		report ("Graphics driver hacks disabled (if any)\n");

	if (	agi_detect_game (filename) == err_OK ||
		agi_detect_game (get_current_directory ()) == err_OK)
	{
		game.state = STATE_LOADED;
	}

	init_sound ();

	report (" \nSarien " VERSION " is ready.\n");
	if (game.state < STATE_LOADED) {
       		console_prompt ();
		do { main_cycle (); } while (game.state < STATE_RUNNING);
	}

	/* Execute the game */
    	do {
		if (game.state < STATE_RUNNING) {
    			ec = agi_init ();
			game.state = STATE_RUNNING;
		}

		if (ec == err_OK) {
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
    		agi_deinit ();
    	} while (ec == err_RestartGame || game.state == STATE_RUNNING);

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

