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
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "sarien.h"
#include "agi.h"
#include "gfx_agi.h"
#include "keyboard.h"
#include "rand.h"
#include "picture.h"
#include "view.h"
#include "logic.h"
#include "menu.h"
#include "console.h"


static struct agi_loader *loader;		/* loader */

struct agi_game game;

volatile UINT32	msg_box_secs2;		/* message box timeout in sec/2 */

extern struct agi_loader agi_v2;
extern struct agi_loader agi_v3;

extern struct sarien_options opt;
extern struct agi_picture pictures[];
extern struct agi_logic logics[];
extern struct agi_view views[];
extern UINT8 *font, font_english[], font_russian[];



int agi_init ()
{
	int ec, i;

	_D("()");

	/* set the font */
	font= opt.agds ? font_russian : font_english;

	/* reset all flags to false and all variables to 0 */
	for (i = 0; i < MAX_FLAGS; i++)
		setflag (i, 0);
	for (i = 0; i < MAX_VARS; i++)
		setvar(i, 0);

	/* clear all logics, pictures, views and events */
	memset (&logics, 0, MAX_DIRS * sizeof (struct agi_logic));
	memset (&pictures, 0, MAX_DIRS * sizeof (struct agi_picture));
	memset (&views, 0x0, MAX_DIRS * sizeof (struct agi_view));
	memset (&game.events, 0x0, MAX_DIRS * sizeof (struct agi_event));

	init_words ();
	init_view_table ();
	set_rnd_seed ();
	init_menus ();

	/* clear string buffer */
	for (i = 0; i < MAX_WORDS1; i++)
		game.strings[i][0] = 0;

	/* setup emulation */

	report ("Emulating Sierra AGI v");
	switch (loader->int_version>>12) {
	case 2:
		report ("%x.%03x\n",
			(int)(loader->int_version >> 12) & 0xF,
			(int)(loader->int_version) & 0xFFF);
		break;
	case 3:
		report ("%x.002.%03x\n",
			(int)(loader->int_version >> 12) & 0xF,
			(int)(loader->int_version) & 0xFFF);
		break;
	}

	game.game_flags |= opt.amiga ? ID_AMIGA : 0;
	game.game_flags |= opt.agds ? ID_AGDS : 0;

	if (game.game_flags & ID_AMIGA)
		printf ("Amiga padded game detected\n");

	if (game.game_flags & ID_AGDS)
		printf ("AGDS mode enabled\n");

	//screen_mode=GFX_MODE;
	/*allow_kyb_input=__FALSE;*/
	//txt_fg=0xF;
	//txt_bg=0x0;

	ec = loader->init ();		/* load vol files, etc */

	if (ec == err_OK)
		ec = load_objects (OBJECTS);

	/* CM: ec= commented out, demogs has no words.tok */
	if(ec == err_OK)
		/*ec =*/ load_words(WORDS);

	/* FIXME: load IIgs instruments and samples */
	/* load_instruments("kq.sys16"); */

	/* Load logic 0 into memory, set cache flag for logic 0 */
	if(ec == err_OK) {
		ec = loader->load_resource (rLOGIC, 0);
		game.dir_logic[0].flags |= RES_CACHED;	/* keep this one cached */
	}

	/* if cached, enable caching options */
	if (opt.cache) {
		for (i = 0; i < MAX_DIRS; i++) {
			game.dir_logic[i].flags |= RES_CACHED;
			game.dir_pic[i].flags |= RES_CACHED;
			game.dir_view[i].flags |= RES_CACHED;
			game.dir_sound[i].flags |= RES_CACHED;
		}
	}

	/* if forced, load all cacheable objects */
	if (opt.forceload && ec == err_OK) {
		for(i = 0; i < MAX_DIRS; i++) {
			printf ("Force loading cached resource: Logic %4i\r", i);
			fflush (stdout);
			loader->load_resource (rLOGIC, i);
		}
		printf ("\n");

		for(i = 0; i < MAX_DIRS; i++) {
			printf ("Force loading cached resource: Picture %4i\r", i);
			fflush (stdout);
			loader->load_resource (rPICTURE, i);
		}
		printf ("\n");

		for(i = 0; i < MAX_DIRS; i++) {
			printf ("Force loading cached resource: View %4i\r", i);
			fflush(stdout);
			loader->load_resource (rVIEW, i);
		}
		printf("\n");
	}

	return ec;
}


/* unload all resources */
static void unload_resources ()
{
	int i;

	for(i = 0; i < MAX_DIRS; i++) {
		game.dir_view[i].flags &= ~RES_CACHED;	/* clear cache flag */
		loader->unload_resource (rVIEW, i);	/* free view */

		game.dir_pic[i].flags &= ~RES_CACHED;	/* clear cache flag */
		loader->unload_resource (rPICTURE, i);	/* free resource */

		game.dir_logic[i].flags &= ~RES_CACHED;	/* clear cache flag */
		loader->unload_resource (rLOGIC, i);	/* free resource */

		game.dir_sound[i].flags &= ~RES_CACHED;	/* clear cache flag */
		loader->unload_resource (rSOUND, i);	/* free resource */
	}
}


int agi_deinit ()
{
	int ec;

	reset_graphics ();		/* clean out video memory */
	clean_input ();			/* remove all words from memory */

	deinit_menus ();		/* unload the menus */
	unload_resources ();		/* unload resources in memory */
	ec = loader->deinit ();
	unload_objects();
	unload_words();

	game.ego_in_new_room = 0;
	game.control_mode = 0;
	game.quit_prog_now = 0;
	game.status_line = FALSE;
	game.line_status = 0;
	game.line_user_input = 0;
	game.line_min_print = 0;
	game.allow_kyb_input = 0;
	game.clock_enabled = 0;
	game.message_box_key = 0;

	return ec;
}


int agi_detect_game (char *gn)
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


int agi_version ()
{
	return loader->version;
}


int agi_get_release ()
{
	return loader->int_version;
}


void agi_set_release (int n)
{
	loader->int_version = n;
}


int agi_load_resource (int r, int n)
{
	return loader->load_resource (r, n);
}


int agi_unload_resource (int r, int n)
{
	return loader->unload_resource (r, n);
};

