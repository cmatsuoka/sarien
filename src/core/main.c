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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "sarien.h"
#include "agi.h"
#include "gfx_base.h"
#include "objects.h"
#include "picture.h"
#include "sound.h"
#include "console.h"
#include "text.h"


int	run_game2(void);
//UINT16	crc_game(void);

/* For the interactive picture viewer */
UINT8	show_screen_mode = 'x';

volatile UINT32 clock_ticks;
volatile UINT32 clock_count;

extern int optind;

extern struct sarien_console console;
extern UINT8 *font, font_english[];

extern struct agi_loader agi_v2;
extern struct agi_loader agi_v3;
extern struct agi_loader *loader;

struct sarien_options opt;
struct game_id_list game_info;

#ifndef _TRACE
INLINE void _D (char *s, ...) { }
#endif

static int detect_game (char *gn)
{
	int ec = err_OK;

	_D ("(gn = %s)", gn);
	if (gn == NULL)		/* assume current directory */
		gn = get_current_directory ();

	loader = &agi_v2;
	ec = loader->detect_game (gn);

	if (ec != err_OK) {
		loader = &agi_v3;
		ec = loader->detect_game (gn);
	}

	return ec;
}


static int view_pictures ()
{
	int ec = err_OK;
	UINT32 resnum = 0;
	char x[64];
	int i, pic, dir = 1;

	console.active = 0;
	console.y = 0;
	show_screen_mode = 'v';

	for (i = 0; ec == err_OK; i = 1) {
		for (pic = resnum; ; ) {
			/* scan for resource */
			if (game.dir_pic[pic].offset != _EMPTY)
				break;

			pic += dir;
			if (pic < 0)
				pic = MAX_DIRS - 1;

			if (pic > MAX_DIRS - 1) {
				pic = 0;
				if (i == 0) {		/* no pics? */
					ec = 1;
					fprintf (stderr, "No pictures found\n");
					goto end_view;
				}
			}
		}
		resnum = pic;

		if ((ec = loader->load_resource (rPICTURE, resnum)) != err_OK)
			continue;

		sprintf ((char*)x, "Picture:%3li     [drawing]     Show: %3s",
			resnum, opt.showscreendraw ? " on" : "off");
		print_text (x, 0, 4, 190, strlen ((char*)x) + 1, 15, 0);

		/* decodes the raw data to useable form */
		decode_picture (resnum);

		show_buffer (show_screen_mode);

update_statusline:
		sprintf (x, "V:Vis C:Con P:Pri X:P+C   +:Next -:Prev");
		print_text (x, 0, 4, 170, strlen (x) + 1, 15, 0);
		sprintf (x, "R:Redraw      D:Show toggle      Q:Quit");
		print_text (x, 0, 4, 180, strlen (x) + 1, 15, 0);
		sprintf (x, "Picture:%3li                   Show: %3s",
			resnum, opt.showscreendraw ? " on" : "off");
		print_text (x, 0, 4, 190, strlen (x) + 1, 15, 0);

		put_screen ();

		while (42) {
    			switch (tolower (get_key() & 0xFF)) {
    			case 'q':
				goto end_view;
    			case 'v':
				show_screen_mode = 'v';
    				dump_screenX ();
    				break;
    			case 'p':
    			case 'z':
				show_screen_mode = 'p';
    				dump_pri (resnum);
 				break;
    			case 'c':
				show_screen_mode = 'c';
    				dump_con (resnum);
				break;
			case 'd':
				opt.showscreendraw = !opt.showscreendraw;
				goto update_statusline;
    			case 'x':
				show_screen_mode = 'x';
    				dump_x (resnum);
  				break;
			case 'r':
				goto next_pic;
    			case '+':
    				pic = resnum;
 				if (pic < MAX_DIRS - 1)
    					pic++;
    				else
    					pic = 0;
    				dir = 1;
				goto next_pic;
    			case '-':
    				pic = resnum;
    				if (pic > 0)
    					pic--;
    				else
    					pic = MAX_DIRS - 1;
    				i = 0;
    				dir = -1;
				goto next_pic;
    			}
    		}
next_pic:
    		loader->unload_resource (rPICTURE, resnum);
    		resnum = pic;
	}

end_view:
	return ec;
}


static int run_game ()
{
	int ec = err_OK;

	_D ("F5 = %d", getflag (5));
	switch (opt.gamerun) {
	case gLIST_GAMES:
	case gCRC:
		break;
	case gRUN_GAME:
		ec = run_game2 ();
		break;
	case gVIEW_PICTURES:
		ec = view_pictures ();
		break;
	case gSHOW_WORDS:
		ec = show_words ();
		break;
	case gSHOW_OBJECTS:
		ec = show_objects ();
		break;
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
"Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka\n"
"Portions Copyright (C) 1998 Lance Ewing, (C) 1999 Felipe Rosinha\n"
#if !defined(HAVE_GETOPT_LONG) || !defined(HAVE_GLOB_H)
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
	screen_mode = GFX_MODE;
	//clock_count = 0;
	//clock_ticks = 0;

	loader = NULL;
	font = font_english;

	if (opt.gamerun == gLIST_GAMES) {
		list_games ();
		goto bail_out;
	}

	ec = detect_game (argc > 1 ? argv[optind] : get_current_directory ());
	if (ec != err_OK) {
		ec = err_InvalidAGIFile;
		goto bail_out;
	}

	if (opt.gamerun == gCRC) {
		/* FIXME: broken! */
		printf("              Game : %s\n", game_info.gName);
		printf("               CRC : 0x%06lX\n", game_info.crc);
		printf("Pre-built Switches : %s\n",
			game_info.switches[0] == 0 ? "(none)" :
			game_info.switches);
		printf("AGI Interpret Vers : %s%03X\n",
			game_info.version >= 0x3000 ? "3.002." :
			"2.", (int)game_info.version & 0xFFF);
		goto bail_out;
	}

	printf("AGI v%i game detected.\n", loader->version);

	if (opt.gamerun == gRUN_GAME || opt.gamerun == gVIEW_PICTURES) {
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
    				game.control_mode = CONTROL_PROGRAM;
    				//o_status = 5;	/* FIXME */

    				ec = run_game();
    			}

    			/* deinit our resources */
    			agi_deinit ();
    		} while (ec == err_RestartGame);
    	}

	if (opt.gamerun == gRUN_GAME) {
		deinit_sound ();
		deinit_video ();
        } else if (opt.gamerun == gVIEW_PICTURES) {
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

