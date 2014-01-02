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
#include "sarien.h"
#include "agi.h"
#include "opcodes.h"
#include "keyboard.h"
#include "rand.h"
#include "menu.h"
#include "savegame.h"

static struct agi_loader *loader;		/* loader */


extern struct agi_loader agi_v2;
extern struct agi_loader agi_v3;


static void init_pri_table ()
{
	int i, p, y = 0;

	for (p = 1; p < 15; p++) {
		for (i = 0; i < 12; i++) {
			game.pri_table[y++] = p < 4 ? 4 : p;
		}
	}
}


int agi_init ()
{
	int ec, i;

	_D ("initializing");
	_D (_D_WARN "game.ver = 0x%x", game.ver);

	/* reset all flags to false and all variables to 0 */
	for (i = 0; i < MAX_FLAGS; i++)
		game.flags[i] = 0;
	for (i = 0; i < MAX_VARS; i++)
		game.vars[i] = 0;

	/* clear all resources and events */
	for (i = 0; i < MAX_DIRS; i++) {
		memset (&game.views[i],    0, sizeof (struct agi_view));
		memset (&game.pictures[i], 0, sizeof (struct agi_picture));
		memset (&game.logics[i],   0, sizeof (struct agi_logic));
		memset (&game.sounds[i],   0, sizeof (struct agi_sound));
	}

	/* clear view table */
	for (i = 0; i < MAX_VIEWTABLE; i++)
		memset (&game.view_table[i], 0, sizeof (struct vt_entry));

	init_words ();
	set_rnd_seed ();
	menu_init ();
	init_pri_table ();

	/* clear string buffer */
	for (i = 0; i < MAX_STRINGS; i++)
		game.strings[i][0] = 0;

	/* setup emulation */

	switch (loader->int_version >> 12) {
	case 2:
		report ("Emulating Sierra AGI v%x.%03x\n",
			(int)(loader->int_version >> 12) & 0xF,
			(int)(loader->int_version) & 0xFFF);
		break;
	case 3:
		report ("Emulating Sierra AGI v%x.002.%03x\n",
			(int)(loader->int_version >> 12) & 0xF,
			(int)(loader->int_version) & 0xFFF);
		break;
	}

	game.game_flags |= opt.amiga ? ID_AMIGA : 0;
	game.game_flags |= opt.agds ? ID_AGDS : 0;

	if (game.game_flags & ID_AMIGA)
		report ("Amiga padded game detected.\n");

	if (game.game_flags & ID_AGDS)
		report ("AGDS mode enabled.\n");

	ec = loader->init ();		/* load vol files, etc */

	if (ec == err_OK)
		ec = loader->load_objects(OBJECTS);

	/* note: demogs has no words.tok */
	if(ec == err_OK)
		ec = loader->load_words(WORDS);

	/* FIXME: load IIgs instruments and samples */
	/* load_instruments("kq.sys16"); */

	/* Load logic 0 into memory */
	if (ec == err_OK)
		ec = loader->load_resource (rLOGIC, 0);

	return ec;
}


/*
 * Public functions
 */

void agi_unload_resources ()
{
	int i;

	/* Make sure logic 0 is always loaded */
	for (i = 1; i < MAX_DIRS; i++) {
		loader->unload_resource (rLOGIC, i);
	}
	for (i = 0; i < MAX_DIRS; i++) {
		loader->unload_resource (rVIEW, i);
		loader->unload_resource (rPICTURE, i);
		loader->unload_resource (rSOUND, i);
	}
}

int agi_deinit ()
{
	int ec;

	clean_input ();			/* remove all words from memory */
	agi_unload_resources ();	/* unload resources in memory */
	loader->unload_resource (rLOGIC, 0);
	ec = loader->deinit ();
	unload_objects();
	unload_words();

	clear_image_stack();

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
	int i;

	i = loader->load_resource (r, n);
#ifdef PATCH_LOGIC
	if (r == rLOGIC)
		patch_logic (n);
#endif

	return i;
}


int agi_unload_resource (int r, int n)
{
	return loader->unload_resource (r, n);
}

