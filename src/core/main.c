/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#include <stdio.h>
#include <string.h>

#ifdef HAVE_ALLEGRO
#include <allegro.h>
#endif

#include "sarien.h"
#include "agi.h"
#include "text.h"
#include "graphics.h"


volatile UINT32 clock_ticks;
volatile UINT32 clock_count;

extern int optind;

struct sarien_options opt;
struct game_id_list game_info;
struct agi_game game;

#if !defined(_TRACE) && !defined(__GNUC__)
INLINE void _D (char *s, ...) { s = s; }
#endif

#ifdef NATIVE_MACOSX
#define main gamemain
#endif

int main (int argc, char *argv[])
{
	int ec;

	/* we must do this before _ANYTHING_ else if using allegro!! */
#ifdef HAVE_ALLEGRO
	allegro_init ();
	install_keyboard ();
#endif

#ifdef __MSDOS__
	exec_name = strdup(argv[0]);
#endif

	game.clock_enabled = FALSE;
	game.state = STATE_INIT;

	opt.scale = 1;
#ifndef __MPW__
#ifdef USE_COMMAND_LINE
	if ((ec = parse_cli (argc, argv)) != err_OK)
		goto bail_out;
#endif

	if (opt.gamerun == GAMERUN_CRC) {
		agi_detect_game (argc > 1 ? argv[optind] :
			get_current_directory ());
		exit (0);
	}

	if (opt.gamerun == GAMERUN_GAMES) {
		list_games ();
		exit (0);
	}
#endif

	init_machine (argc, argv);

	game.color_fg = 15;
	game.color_bg = 0;

	if (init_video () != err_OK) {
		ec = err_Unk;
		goto bail_out;
	}

#ifdef OPT_PICTURE_VIEWER
	if (opt.gamerun == GAMERUN_PICVIEW) {
		console.y = 0;
		if (agi_detect_game (argc > 1 ? argv[optind] :
			get_current_directory ()) == err_OK)
		{
			agi_init ();
			view_pictures ();
		} 

		goto bail_out;
	}
#endif

#if !defined (__MSDOS__) && !defined (__MPW__)
	/* printf() breaks GCC 3.0 build */
	fprintf (stdout,
TITLE " " VERSION " - A Sierra AGI resource interpreter engine.\n"
"Copyright (C) 1999-2001 Stuart George\n"
"Portions Copyright (C) 1998 Lance Ewing, (C) 1999 Felipe Rosinha,\n"
"(C) 1999-2001 Claudio Matsuoka, (C) 1999-2001 Igor Nesterov,\n"
"(C) 2001 Vasyl Tsvirkunov, (C) 2001 Thomas Akesson\n"
#ifndef HAVE_GETOPT_LONG
"Portions Copyright (C) 1989-1997 Free Software Foundation, Inc.\n"
#endif
"\n"
"This program is free software; you can redistribute it and/or modify it\n"
"under the terms of the GNU General Public License, version 2 or later,\n"
"as published by the the Free Software Foundation.\n"
"\n");
#endif

	report ("Enabling interpreter console\n");
	console_init ();
	report ("--- Starting console ---\n\n");
	if (!opt.gfxhacks)
		report ("Graphics driver hacks disabled (if any)\n");

#ifndef __MPW__
	_D ("Detect game");
	if (agi_detect_game (argc > 1 ? argv[optind] :
		get_current_directory ()) == err_OK)
	{
		game.state = STATE_LOADED;
		_D (_D_WARN "game loaded");
	} else {
		if (argc > optind) {
			report ("Could not open AGI game \"%s\".\n\n",
				argv[optind]);
		}
		game.ver = -1;	/* Don't display the conf file warning */
	}
#endif

	_D ("Init sound");
	init_sound ();

	report (" \nSarien " VERSION " is ready.\n");
	if (game.state < STATE_LOADED) {
       		console_prompt ();
		do { main_cycle (); } while (game.state < STATE_RUNNING);
		game.ver = 0;	/* Enable conf file warning */
	}

	/* Execute the game */
    	do {
		_D(_D_WARN "game loop");

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

#ifdef HAVE_ALLEGRO
	remove_keyboard();
#endif
#ifdef __MSDOS__
	free(exec_name);
#endif

	return ec;
}


