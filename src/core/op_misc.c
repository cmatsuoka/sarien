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
#include <time.h>
#include <ctype.h>

#include "sarien.h"
#include "gfx.h"
#include "opcodes.h"
#include "view.h"
#include "logic.h"


#define ip (logics[lognum].cIP)
#define code (logics[lognum].data)


extern    UINT8   *gid;

extern struct agi_view_table view_table[];
extern struct agi_logic logics[];


void new_room (UINT16 r)
{
	_D (("(%d)", r));

	/* stop all animation */

	/* stop all sounds */
	cmd_stop_sound ();

	/* reset all VIEWS */
	/* unload all logic resources */
	/* turn control over to EGO */
	/* release all sprite bg info */
	/* reset views */

	erase_sprites ();
	new_room_resources ();

	switch (getvar (V_border_touch_ego))
	{
	case 1:
		view_table[EGO_VIEW_TABLE].y_pos = _HEIGHT - 1;
		break;
	case 2:
		view_table[EGO_VIEW_TABLE].x_pos = 0;
		break;
	case 3:
		view_table[EGO_VIEW_TABLE].y_pos = HORIZON + 1;
		break; /* horizon + 1*/
	case 4:
		view_table[EGO_VIEW_TABLE].x_pos = _WIDTH -
			view_table[EGO_VIEW_TABLE].x_size;
		break;
	}

 	cmd_set_horizon (HORIZON);

	setvar (V_prev_room, getvar(V_cur_room));
	setvar (V_cur_room, r);
	setvar (V_border_touch_obj, 0);
	setvar (V_border_code, 0);
	setvar (V_word_not_found, 0);
	setvar (V_ego_view_resource, 0);

	/* adjust ego position */
	setvar (V_border_touch_ego, 0);
	setflag (F_entered_cli, FALSE);
	setflag (F_new_room_exec, TRUE);
}

/*
 * Patches
 */

static UINT8 kq4data_find[]= {
	0x0C, 0x04, 0xFF, 0x07, 0x05, 0xFF, 0x15, 0x00,
	0x03, 0x0A, 0x00, 0x77, 0x83, 0x71, 0x0D, 0x97,
	0x03, 0x98, 0xCE, 0x18, 0x98, 0x19, 0x98, 0x1B,
	0x98, 0x0C, 0x5A, 0x1A, 0x00
};

static UINT8 kq4data_fix[]= {
	/* v19 = 0
	 * new.room(96)
	 * return
	 */
	0x03, 0x13, 0x0, 0x12, 0x60, 0x00
};

static UINT8 grdata_find[]= {
	0x0C, 0x04, 0xFF, 0x07, 0x05, 0xFF, 0x16, 0x00,
	0x0C, 0x96, 0x03, 0x0A, 0x00, 0x77, 0x83, 0x71,
	0x0D, 0xD9, 0x03, 0xDC, 0xBF, 0x18, 0xDC, 0x19,
	0xDC, 0x1B, 0xDC, 0x0C, 0x95, 0x1A
};

static UINT8 grdata_fix[]= {
	/* reset(227)
	 * v19 = 0
	 * v246 = 1
	 * set(15)
	 * new.room(73)
	 */
	0x0D, 0xE3, 0x03, 0x13, 0x00, 0x03, 0xF6, 0x01,
	0x0C, 0x0F, 0x12, 0x49
};

static UINT8 lsl1data_find[]= {
	0xFF, 0xFD, 0x07, 0x1E, 0xFC, 0x07, 0x6D, 0x01,
	0x5F, 0x03, 0xFC, 0xFF, 0x12, 0x00, 0x0C, 0x6D,
	0x78, 0x8A, 0x77, 0x69, 0x16, 0x18, 0x00, 0x0D,
	0x30, 0x0D, 0x55, 0x78, 0x65, 0x0A
};

static UINT8 lsl1data_fix[]= {
	/* set(109)
	 * reset(48)
	 * reset(85)
	 * accept.input()
	 * new.room(11)
	 */
	0x0C, 0x6D, 0x0D, 0x30, 0x0D, 0x55, 0x78, 0x12,
	0x0B
};

static UINT8 mh1data_find[]= {
	0xFF, 0x07, 0x05, 0xFF, 0xE6, 0x00,
	0x03, 0x0A, 0x02, 0x77, 0x83, 0x71,
	0x6F, 0x01, 0x17, 0x00, 0x03, 0x00,
	0x9F, 0x03, 0x37, 0x00, 0x03, 0x32,
	0x03, 0x03, 0x3B, 0x00, 0x6C, 0x03
};

static UINT8 mh1data_fix[]= {
	0x0C, 0x05, 0x16, 0x5A, 0x12, 0x99
};


void break_copy_protection (int lognum)
{

	switch(lognum) {
	case 6:
		/* lsl1 bypass questions */
		if (!strcmp (gid, "LLLLL")) {
			if (!memcmp (lsl1data_find, (code+ip), 30))
				memmove ((code+ip), lsl1data_fix, 9);
		}
		break;

	case 125:
		/* gold rush code break */
		if (!strcmp (gid, "GR")) {
			if (!memcmp (grdata_find, (code+ip), 30))
				memmove((code+ip), grdata_fix, 12);
		}
		break;

	case 140:
		/* kings quest 4 code break */
		if (!strcmp (gid, "KQ4")) {
			if(memcmp(kq4data_find, (code+ip),  29)==0)
				memmove((code+ip), kq4data_fix, 6);
		}
		break;
	case 159:
		/* manhunter 1 amiga */
		if(!memcmp(mh1data_find, (code+ip), 30)) {
			memmove ((code+ip), mh1data_fix, 6);
		}
		break;  
	}
}

