/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2003 Stuart George and Claudio Matsuoka
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
#include "sprite.h"

extern int optind;

struct sarien_options opt;
struct game_id_list game_info;
struct agi_game game;

#if (!defined(_TRACE) && !defined(__GNUC__))
INLINE void _D (char *s, ...) { s = s; }
#endif


#ifdef MACOSX
#  ifdef MACOSX_SDL
#    define main SDL_main     
#  else
     /* real main for OS X is in src/graphics/cocoa/cocoa.m */
#    define main gamemain
#  endif
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

#ifdef USE_COMMAND_LINE
	if ((ec = parse_cli (argc, argv)) != err_OK)
		goto bail_out;

	if (opt.gamerun == GAMERUN_CRC) {
		char name[80];
		agi_detect_game (argc > 1 ? argv[optind] :
			get_current_directory ());
		printf ("CRC: 0x%x (Ver 0x%x)\n", game.crc, game.ver);
		if (match_crc (game.crc, get_config_file(), name, 80))
			printf(" AGI game detected: %s\n", name);
		else
			printf(" Unknown game (config file: %s)\n",
				get_config_file());
		exit (0);
	}

	if (opt.gamerun == GAMERUN_GAMES) {
		list_games ();
		exit (0);
	}
#ifdef OPT_LIST_OBJECTS
	if (opt.gamerun == GAMERUN_OBJECTS) {
		agi_detect_game(argc > 1 ? argv[optind] : get_current_directory());
		agi_init();
		show_objects();

		/* errgh!! ugly goto */
		goto bail_out;
	}
#endif
#ifdef OPT_LIST_DICT
	if (opt.gamerun == GAMERUN_WORDS){
	    agi_detect_game(argc > 1 ? argv[optind] : get_current_directory());
	    agi_init();
	    show_words();

	    goto bail_out;
	}
#endif
#endif

	init_machine (argc, argv);

	game.color_fg = 15;
	game.color_bg = 0;

	if ((game.sbuf = calloc (_WIDTH, _HEIGHT)) == NULL) {
		ec = err_NotEnoughMemory;
		goto bail_out;
	}
#ifdef USE_HIRES
	if ((game.hires = calloc (_WIDTH * 2, _HEIGHT)) == NULL) {
		ec = err_NotEnoughMemory;
		goto bail_out_2;
	};
#endif

	if (init_sprites () != err_OK) {
		ec = err_NotEnoughMemory;
		goto bail_out_3;
	}

	if (init_video () != err_OK) {
		ec = err_Unk;
		goto bail_out_4;
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

#if !defined (__MSDOS__)
	/* printf() breaks GCC 3.0 build */
	fprintf (stdout,
TITLE " " VERSION " - A Sierra AGI resource interpreter engine.\n"
"Copyright (C) 1999-2002 Stuart George\n"
"Portions Copyright (C) 1998 Lance Ewing, (C) 1999 Felipe Rosinha,\n"
" (C) 1999-2002 Claudio Matsuoka, (C) 1999-2001 Igor Nesterov,\n"
" (C) 2001,2002 Vasyl Tsvirkunov, (C) 2001,2002 Thomas Akesson\n"
"Scale2x Copyright (C) 2001-2002 Andrea Mazzoleni\n"
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

	game.ver = -1;		/* Don't display the conf file warning */

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
	}

	_D ("Init sound");
	init_sound ();

	report (" \nSarien " VERSION " is ready.\n");
	if (game.state < STATE_LOADED) {
       		console_prompt ();
		do { main_cycle (); } while (game.state < STATE_RUNNING);
		if (game.ver < 0) game.ver = 0;	/* Enable conf file warning */
	}

	ec = run_game ();

	deinit_sound ();
	deinit_video ();

bail_out_4:
	deinit_sprites ();
bail_out_3:
#ifdef USE_HIRES
	free (game.hires);
#endif
bail_out_2:
	free (game.sbuf);
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

