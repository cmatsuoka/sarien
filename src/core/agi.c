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
#include <limits.h>

#include "sarien.h"
#include "agi.h"
#include "opcodes.h"
#include "keyboard.h"
#include "rand.h"
#include "menu.h"
#include "savegame.h"

static struct agi_loader *loader;		/* loader */


#ifndef PALMOS
extern struct agi_loader agi_v2;
extern struct agi_loader agi_v3;
#else
extern struct agi_loader agi_v4;
#endif


static void init_pri_table ()
{
	int i, p, y = 0;

#if 0
	if (*game.pri_table != 0xff)
		return;
#endif

	for (p = 1; p < 15; p++) {
		for (i = 0; i < 12; i++) {
			game.pri_table[y++] = p < 4 ? 4 : p;
		}
	}
}

int agi_init ()
{
	int ec, i;

	_D("initializing");

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
		memset (&game.ev_scan[i],  0, sizeof (struct agi_event));
		memset (&game.ev_keyp[i],  0, sizeof (struct agi_event));
	}

	/* clear view table */
	for (i = 0; i < MAX_VIEWTABLE; i++)
		memset (&game.view_table[i], 0, sizeof (struct vt_entry));

	init_words ();
	set_rnd_seed ();
	init_menus ();
	init_pri_table ();

	/* clear string buffer */
	for (i = 0; i < MAX_WORDS1; i++)
		game.strings[i][0] = 0;

	/* setup emulation */

	report ("Emulating Sierra AGI v");
	switch (loader->int_version >> 12) {
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
		report ("Amiga padded game detected.\n");

	if (game.game_flags & ID_AGDS)
		report ("AGDS mode enabled.\n");

	ec = loader->init ();		/* load vol files, etc */

	if (ec == err_OK)
		ec = loader->load_objects(OBJECTS);

	/* CM: ec= commented out, demogs has no words.tok */
	if(ec == err_OK)
		/*ec =*/ loader->load_words(WORDS);

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

	for (i = 0; i < MAX_DIRS; i++) {
		loader->unload_resource (rLOGIC, i);
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
	ec = loader->deinit ();
	unload_objects();
	unload_words();

	clear_image_stack();

	return ec;
}

int agi_detect_game (char *gn)
{
	int ec = err_OK;

#ifdef DREAMCAST
	strcpy(g_gamename, UNKNOWN_GAME);
#endif

	_D ("(gn = %s)", gn);
	if (gn == NULL)		/* assume current directory */
		gn = get_current_directory ();

#ifndef PALMOS
	loader = &agi_v2;
	ec = loader->detect_game (gn);

#ifndef FAKE_PALMOS
	if (ec != err_OK) {
		loader = &agi_v3;
		ec = loader->detect_game (gn);
	}
#endif

#else
	loader = &agi_v4;
	ec = loader->detect_game(gn);
#endif

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
#ifdef DISABLE_COPYPROTECTION
	if (r == rLOGIC)
		patch_logic (n);
#endif

	return i;
}


int agi_unload_resource (int r, int n)
{
	return loader->unload_resource (r, n);
}

