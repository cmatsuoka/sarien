/* Sarien - A Sierra AGI resource interpreter engine
 * Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  
 * $Id$
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; see docs/COPYING for further details.
 */

#include <stdio.h>
#include <string.h>

#include "sarien.h"
#include "agi.h"
#include "sprite.h"
#include "graphics.h"
#include "keyboard.h"
#include "text.h"
#include "keyboard.h"


/*
 * Messages and coordinates
 */

#define NOTHING_X	16
#define NOTHING_Y	3
#define NOTHING_MSG	"nothing"

#define ANY_KEY_X	4
#define ANY_KEY_Y	24
#define ANY_KEY_MSG	"Press a key to return to the game"

#define YOUHAVE_X	11
#define YOUHAVE_Y	0
#define YOUHAVE_MSG	"You are carrying:"

#define SELECT_X	2
#define SELECT_Y	24
#define SELECT_MSG	"Press ENTER to select, ESC to cancel"


static UINT8 *intobj = NULL;



static void print_item (int n, int fg, int bg)
{
	print_text (object_name (intobj[n]), 0,
		n % 2 ? 39 - strlen (object_name (intobj[n])) : 1,
		(n / 2) + 2, 40, fg, bg);
}

#ifdef USE_MOUSE
static int find_item ()
{
	int r, c;

	r = mouse.y / CHAR_LINES;
	c = mouse.x / CHAR_COLS;

	_D (_D_WARN "r = %d, c = %d", r, c);

	if (r < 2)
		return -1;

	return (r - 2) * 2 + (c > 20);
}
#endif

static int show_items ()
{
	unsigned int x, i;

	for (x = i = 0; x < game.num_objects; x++) {
		if (object_get_location (x) == EGO_OWNED) {
			/* add object to our list! */
			intobj[i] = x;
			print_item (i, STATUS_FG, STATUS_BG);
			i++;
		}
	}

	if (i == 0) {
		print_text (NOTHING_MSG, 0, NOTHING_X, NOTHING_Y, 40,
			STATUS_FG, STATUS_BG);
	}

	return i;
}

static void select_items (int n)
{
	int fsel = 0;

	while (42) {
		if (n > 0)
			print_item (fsel, STATUS_BG, STATUS_FG);

		switch (wait_any_key ()) {
		case KEY_ENTER:
			setvar (V_sel_item, intobj[fsel]);
			goto exit_select;
		case KEY_ESCAPE:
			setvar (V_sel_item, 0xff);
			goto exit_select;
		case KEY_UP:
			if (fsel >= 2) fsel -= 2;
			break;
		case KEY_DOWN:
			if (fsel + 2 < n) fsel += 2;
			break;
		case KEY_LEFT:
			if (fsel % 2 == 1) fsel--;
			break;
		case KEY_RIGHT:
			if (fsel % 2 == 0 && fsel + 1 < n) fsel++;
			break;
#ifdef USE_MOUSE
		case BUTTON_LEFT: {
			int i = find_item ();
			if (i >= 0 && i < n) {
				setvar (V_sel_item, intobj[fsel = i]);
				_D (_D_WARN "item found: %d", fsel);
				show_items ();
				print_item (fsel, STATUS_BG, STATUS_FG);
				do_update ();
				goto exit_select;
			}
			break; }
#endif
		default:
			break;
		}

		show_items ();
		do_update ();
	}
exit_select:
	_D ("selected: %d", fsel);
}

/*
 * Public functions
 */

/**
 * Display inventory items.
 */
void inventory ()
{
	int old_fg, old_bg;
	int n;

	/* screen is white with black text */
	old_fg = game.color_fg;
	old_bg = game.color_bg;
	game.color_fg = 0;
	game.color_bg = 15;
	clear_screen (game.color_bg);

	print_text (YOUHAVE_MSG, 0, YOUHAVE_X, YOUHAVE_Y, 40,
		STATUS_FG, STATUS_BG);

	/* FIXME: doesn't check if objects overflow off screen... */

	intobj = malloc (4 + game.num_objects);
	memset(intobj, 0, (4 + game.num_objects));

	n = show_items ();

	if (getflag (F_status_selects_items)) {
		print_text (SELECT_MSG, 0, SELECT_X, SELECT_Y, 40,
			STATUS_FG, STATUS_BG);
	} else {
		print_text (ANY_KEY_MSG, 0, ANY_KEY_X, ANY_KEY_Y, 40,
			STATUS_FG, STATUS_BG);
	}
 
	flush_screen ();

	/* If flag 13 is set, we want to highlight & select an item.
	 * opon selection, put objnum in var 25. Then on esc put in
	 * var 25 = 0xff.
	 */
 
	if (getflag (F_status_selects_items))
		select_items (n);

	free (intobj);

	if (!getflag (F_status_selects_items))
		wait_any_key();

	clear_screen (0);
	write_status ();
	show_pic ();
	game.color_fg = old_fg;
	game.color_bg = old_bg;
	game.has_prompt = 0;
	flush_lines (game.line_user_input, 24);
}


