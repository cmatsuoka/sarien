/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#ifndef PALMOS		/* We'll use a different main file for PalmOS */

#include <stdio.h>
#include <string.h>
#include "sarien.h"
#include "agi.h"
#include "text.h"
#include "graphics.h"


int	run_game2(void);
/* UINT16	crc_game(void); */

/* For the interactive picture viewer */
UINT8	show_screen_mode = 'x';

volatile UINT32 clock_ticks;
volatile UINT32 clock_count;

extern int optind;

extern UINT8 *font, font_english[];

struct sarien_options opt;
struct game_id_list game_info;

#ifndef _TRACE
INLINE void _D (char *s, ...) { }
#endif


static int run_game ()
{
	int ec = err_OK;

	_D ("let's go");
	switch (opt.gamerun) {
	case gLIST_GAMES:
	case gCRC:
		break;
	case gRUN_GAME:
		ec = run_game2 ();
		break;
#ifdef OPT_LIST_DICT
	case gSHOW_WORDS:
		ec = show_words ();
		break;
#endif
#ifdef OPT_LIST_OBJECTS
	case gSHOW_OBJECTS:
		ec = show_objects ();
		break;
#endif
	}

	return ec;
}


int main(int argc, char *argv[])
{
	int ec;

	/* we must do this before _ANYTHING_ else if using allegro!! */
#ifdef HAVE_ALLEGRO
	allegro_init ();
#endif

	printf(
TITLE " " VERSION " - A Sierra AGI resource interpreter engine.\n"
"Copyright (C) 1999-2001 Stuart George\n"
"Portions Copyright (C) 1998 Lance Ewing, (C) 1999 Felipe Rosinha,\n"
"(C) 1999-2001 Claudio Matsuoka, (C) 1999-2001 Igor Nesterov\n"
#ifndef HAVE_GETOPT_LONG
"Portions Copyright (C) 1989-1997 Free Software Foundation, Inc.\n"
#endif
"\n"
"This program is free software; you can redistribute it and/or modify it\n"
"under the terms of the GNU General Public License, version 2 or later,\n"
"as published by the the Free Software Foundation.\n"
"\n");

	game.clock_enabled = FALSE;

	if ((ec = parse_cli (argc, argv)) != err_OK)
		goto bail_out;

	init_machine (argc, argv);
	game.gfx_mode = TRUE;
	game.color_fg = 15;
	game.color_bg = 0;

	font = font_english;

	if (opt.gamerun == gLIST_GAMES) {
		list_games ();
		goto bail_out;
	}

	ec = agi_detect_game (argc > 1 ? argv[optind] : get_current_directory ());
	if (ec != err_OK) {
		ec = err_InvalidAGIFile;
		goto bail_out;
	}

	if (opt.gamerun == gCRC) {
		/* FIXME: broken! */
		printf("              Game : %s\n", game_info.gName);
		printf("               CRC : 0x%06x\n", game_info.crc);
		printf("Pre-built Switches : %s\n",
			game_info.switches[0] == 0 ? "(none)" :
			game_info.switches);
		printf("AGI Interpret Vers : %s%03X\n",
			game_info.version >= 0x3000 ? "3.002." :
			"2.", (int)game_info.version & 0xfff);
		goto bail_out;
	}

	printf("AGI v%i game detected.\n", agi_version ());

	if (opt.gamerun == gRUN_GAME) {
		if (init_video () != err_OK) {
			ec = err_Unk;
			goto bail_out;
		}
	}

	if (opt.gamerun == gRUN_GAME) {
		report ("Enabling interpreter console\n");
		console_init ();
		report ("--- Starting console ---\n\n");
		init_sound ();
	}

	/* Execute the game */
	if (opt.gamerun != gCRC && opt.gamerun != gLIST_GAMES) {
    		do {
    			ec = agi_init ();

    			if (ec == err_OK) {
    				/* setup machine specific AGI flags, etc */
    				setvar (V_computer, 0);	/* IBM PC */
    				setvar (V_soundgen, 1);	/* IBM PC SOUND */
    				setvar (V_max_input_chars, 38);
    				setvar (V_monitor, 0x3); /* EGA monitor */

    				game.horizon = HORIZON;
    				game.player_control = FALSE;
    				/* o_status = 5; */	/* FIXME */

    				ec = run_game();
    			}

    			/* deinit our resources */
    			agi_deinit ();
    		} while (ec == err_RestartGame);
    	}

	if (opt.gamerun == gRUN_GAME) {
		deinit_sound ();
		deinit_video ();
	}

bail_out:
	if (ec == err_OK || ec == err_DoNothing) {
		deinit_machine ();
		exit (ec);
	}

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

	deinit_machine ();

	return ec;
}

#endif /* PALMOS */
