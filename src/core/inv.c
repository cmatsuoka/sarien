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

#include "sarien.h"
#include "agi.h"
#include "sprite.h"
#include "graphics.h"
#include "keyboard.h"
#include "text.h"
#include "keyboard.h"

#define NOTHING_X	15
#define NOTHING_Y	3
#define NOTHING_MSG	"Nothing"

#define ANY_KEY_X	4
#define ANY_KEY_Y	24
#define ANY_KEY_MSG	"Press a Key to return to the game."

#define YOUHAVE_X	11
#define YOUHAVE_Y	0
#define YOUHAVE_MSG	"You Are Carrying:"

#define SELECT_X	2
#define SELECT_Y	24
#define SELECT_MSG	"Press ENTER to select, ESC to cancel."


void inventory ()
{
	int x, y, cx, cy;
	UINT8 *intobj = NULL;
	int objcount;
	int joffs = 0, jlen = 0;
	int old_fg, old_bg;

	/* screen is white with black text */
	old_fg = game.color_fg;
	old_bg = game.color_bg;
	game.color_fg = 0;
	game.color_bg = 15;
	clear_screen (game.color_bg);

	print_text (YOUHAVE_MSG, 0, YOUHAVE_X, YOUHAVE_Y, 40,
		STATUS_FG, STATUS_BG);

	/* FIXME: doesnt check if objects overflow off screen... */

	intobj = malloc (4 + game.num_objects);

	for (x = y = cx = 0, cy = 2, objcount = 0; x < game.num_objects; x++) {
		if (object_get_location (x) == EGO_OWNED) {
			/* add object to our list! */
			intobj[objcount++] = x;

			print_text (object_name (x), 0,
				cy % 2 ? 40 - strlen (object_name (x)) : 0,
				(cy / 2) + 1, 40, STATUS_FG, STATUS_BG);

			cy++;
			y++;
		}
	}

	if (y == 0) {
		print_text (NOTHING_MSG, 0, NOTHING_X, NOTHING_Y, 40,
			STATUS_FG, STATUS_BG);
	}

	if (getflag (F_status_selects_items))
		print_text (SELECT_MSG, 0, SELECT_X, SELECT_Y, 40,
			STATUS_FG, STATUS_BG);
	else
		print_text(ANY_KEY_MSG, 0, ANY_KEY_X, ANY_KEY_Y, 40,
			STATUS_FG, STATUS_BG);
 
	/* dump to screen. */
	flush_screen ();

	/*
	 * test flag(13)
	 * (13)=1 == We want to highlight + select an item.
	 * opon selection, put objnum in var(25)
	 * then on esc put in var(25)=0xFF;
	 */
 
	if (getflag (F_status_selects_items)) {
		int flag = TRUE;
		int lcol, rcol;
		int fsel = 0, show = 1;
		int lx1 = 0, ly1 = 0, lx2 = 0, ls = 0;

		cy = 2;
		rcol = objcount / 2;
		lcol = rcol + (objcount % 2);

		while (flag) {
			cy = 2 + fsel;
			if (show) {
				show = 0;

				/* nothing is a special case! */
				if (!objcount) {
					print_text (NOTHING_MSG, 0,
						NOTHING_X, NOTHING_Y, 40,
						STATUS_BG, STATUS_FG);
					schedule_update (NOTHING_X, NOTHING_Y,
						NOTHING_X + strlen (NOTHING_MSG)
						* 8, NOTHING_Y+8);
				} else {
					if (ls) {
						print_text (object_name (intobj[ls-1]),
						0, lx1, (ly1/2)+1,
						40, STATUS_FG, STATUS_BG);
						schedule_update (lx1, ly1, lx2, cy+8);
					}

					jlen = strlen (object_name(intobj[fsel]));
					joffs = cy % 2 == 0 ? 0 : 40-jlen;

					lx1 = joffs;
					ly1 = cy;
					lx2 = joffs+(jlen*8);
					ls = fsel+1;		/* ls ALWAYS >0 */

					print_text (object_name (intobj[fsel]),
						0, lx1, (cy/2)+1, 40,
						STATUS_BG, STATUS_FG);
					schedule_update (lx1, ly1, lx2, cy+8);
				}
			}

			/* DF : FIXME : get_key() is not console aware */
			/* switch(wait_key()) */

			switch(get_key()) {
			case KEY_ENTER:
				setvar(V_sel_item, intobj[fsel]);
				report("show_obj() -> %i\n", intobj[fsel]);
				setvar(V_sel_item, intobj[fsel]);
				flag = FALSE;
				break;
			case KEY_ESCAPE:
				setvar(V_sel_item, 0xff);
				report("show_obj() -> %i\n", intobj[fsel]);
				flag = FALSE;
				break;
			case KEY_UP:
				if (fsel >= 2) {
					fsel -= 2;
					show = 1;
				}
				break;
			case KEY_DOWN:
				if(fsel%2==0) {
					if((2+fsel)<=lcol) {
						fsel+=2;
						show=1;
					}
				} else {
					if((2+fsel)<=rcol+1) {
						fsel+=2;
						show=1;
					}
				}
				break;
			case KEY_LEFT:
				if(fsel%2==1) {
					fsel--;
					show=1;
				}
				break;
			case KEY_RIGHT:
				if(fsel%2==0) {
					fsel++;
					show=1;
				}
				break;
			default:
				break;
			}

			do_update ();
		}
	}

	free (intobj);

	/* use wait_key instead of get_key() to be console aware */
	if (!getflag (F_status_selects_items))
		wait_key();

	clear_screen (0);
	write_status ();
	show_pic ();
	game.color_fg = old_fg;
	game.color_bg = old_bg;
	print_line_prompt ();
	flush_lines (game.line_user_input, 24);
}

