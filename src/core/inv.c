/*
 *  Sarien AGI :: Copyright (C) 1999 Dark Fiber 
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sarien.h"
#include "agi.h"
#include "gfx.h"
#include "objects.h"
#include "keyboard.h"
#include "text.h"

extern struct agi_object *objects;
extern int num_objects;

extern struct gfx_driver *gfx;

void inventory ()
{
	int x, y;
	int cx, cy;

	save_screen();

	/* screen is white with black text */
	for (x = 0; x < 320; x++) {
		for (y = 0; y < 200; y++)
			put_pixel (x, y, STATUS_BG);
	}

	print_text ("You Are Carrying:", 0, (11<<3), 0, 40,
		STATUS_FG, STATUS_BG);

	/* FIXME: doesnt check if objects overflow off screen... */

	for(x = y = cx = 0, cy = 2; x < num_objects; x++) {
		if ((objects + x)->location == EGO_OWNED) {
			print_text ((objects + x)->name, 0,
				(cy % 2 ? 40 - strlen ((objects + x)->name) :
				0) << 3, ((cy / 2) + 1) << 3, 40,
				STATUS_FG, STATUS_BG);

			cy++;
			y++;
		}
	}

	if (y == 0) {
		print_text ("Nothing", 0, 15 << 3, cy << 3, 40,
			STATUS_FG, STATUS_BG);
	}

	print_text ("Press a Key to return to the game.", 0, 4 << 3, 24 << 3,
		40, STATUS_FG, STATUS_BG);

	gfx->put_block (0, 0, 320, 200);

	/* use wait_key instead of gfx->get_key() to be console aware */
	wait_key ();

	restore_screen();
}

