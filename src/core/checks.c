/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#include "sarien.h"
#include "agi.h"


static int dir_table_x[9] = {
	0,  0,  1,  1,  1,  0, -1, -1, -1
};

static int dir_table_y[9] = {
	0, -1, -1,  0,  1,  1,  1,  0, -1
};

static int check_position (struct vt_entry *v)
{
	if (	v->x_pos < 0 ||
		v->x_pos + v->x_size > _WIDTH ||
		v->y_pos + v->y_size + 1 < 0 ||
		v->y_pos < v->y_size ||
		v->y_pos >= _HEIGHT ||
		((~v->flags & IGNORE_HORIZON) && v->y_pos <= game.horizon))
	{
		/*_D (_D_WARN "check position failed: x=%d, y=%d, h=%d, w=%d",
			v->x_pos, v->y_pos, v->x_size, v->y_size);*/
		return 0;
	}

	return 1;
}

/**
 * Checks if there's another object on the way
 */
static int check_clutter (struct vt_entry *v)
{
	struct vt_entry *u;

	if (v->flags & IGNORE_OBJECTS)
		return 0;

	for_each_vt_entry (u) {
		if ((u->flags & (ANIMATED|DRAWN)) != (ANIMATED|DRAWN))
			continue;

		if (u->flags & IGNORE_OBJECTS)
			continue;

		if (v->entry == u->entry)
			continue;

		if (v->x_pos + v->x_size < u->x_pos)
			continue;

		if (v->x_pos > u->x_pos + u->x_size)
			continue;

		if (v->y_pos == u->y_pos)
			return 1;

		if (v->y_pos > u->y_pos) {
			if (v->y_pos2 < u->y_pos2)
				return 1;
		}

		if (v->y_pos >= u->y_pos)
			continue;

		if (v->y_pos <= u->y_pos)
			continue;

		return 1;
	}
	
	return 0;
}

static int check_priority (struct vt_entry *v)
{
	int i, trigger, water, pass, pri;
	UINT8 *p0;

	if (~v->flags & FIXED_PRIORITY) {
		v->priority = v->y_pos < 48 ? 4 :
			v->y_pos / 12 + 1;
	}

	trigger = 0;
	water = 1;
	pass = 1;

	p0 = &game.sbuf[v->x_pos + v->y_pos * _WIDTH];

	for (i = 0; i < v->x_size; i++, p0++) {
		pri = *p0 >> 4;

		if (pri == 0) {		/* unconditional black. no go at all! */
			pass = 0;
			break;
		}

		if (pri == 3)		/* water surface */
			continue;

		water = 0;

		/* This test fixes oprecon but breaks the Gold Rush! demo.
		 * I'll leave it here because AGI 2.917 also implements it.
		 */
		if (v->entry != 0)	/* test if ego */
			break;

		if (pri == 1) {		/* conditional blue */
			if (v->flags & IGNORE_BLOCKS)
				continue;

			_D (_D_WARN "Blocks observed!");
			pass = 0;
			break;
		}

		
		if (pri == 2) { 	/* trigger */
			_D (_D_WARN "stepped on trigger");
			trigger = 1;
		}
	}

	if (pass) {
		if (!water && v->flags & ON_WATER)
			pass = 0;
		if (water && v->flags & ON_LAND)
			pass = 0;
	}

	if (v->entry == 0) {
		setflag (F_ego_touched_p2, trigger ? TRUE : FALSE);
		setflag (F_ego_water, water ? TRUE : FALSE);
	}

	return pass ? 0 : 1;
}

/*
 * Public functions
 */

/**
 *
 */
void update_position ()
{
	struct vt_entry *v;
	int x, x2, y, y2, dir, step, border;

	game.vars[V_border_code] = 0;
	game.vars[V_border_touch_ego] = 0;
	game.vars[V_border_touch_obj] = 0;

	for_each_vt_entry (v) {
		if ((v->flags & (ANIMATED|UPDATE|DRAWN))
			!= (ANIMATED|UPDATE|DRAWN))
		{
			continue;
		}

		if (v->step_time_count != 0) {
			if (--v->step_time_count != 0)
				continue;
		}
			
		v->step_time_count = v->step_time;

		x = x2 = v->x_pos;
		y = y2 = v->y_pos;
		
		if (~v->flags & FLAG10) {
			dir = v->direction;
			step = v->step_size;
			
			x += step * dir_table_x[dir];
			y += step * dir_table_y[dir];
		}

		border = 0;

		if (x < 0) {
			x = 0;
			border = 4;
		} else if (x + v->x_size > _WIDTH) {
			x = 160 - v->x_size;
			border = 2;
		} else if (y - v->y_size + 1 < 0) {
			y = v->y_size - 1;
			border = 1;
		} else if (y > _HEIGHT - 1) {
			y = _HEIGHT - 1;
			border = 3;
		} else if ((~v->flags & IGNORE_HORIZON) && y <= game.horizon) {
			y++;
			border = 1;
		}

		v->x_pos = x;
		v->y_pos = y;
		
		if (check_clutter (v) || check_priority (v)) {
			v->x_pos = x2;
			v->y_pos = y2;
			border = 0;
			fix_position (v->entry);
		}

		if (border != 0) {
			if_is_ego_view (v) {
				game.vars[V_border_touch_ego] = border;
			} else {
				game.vars[V_border_code] = v->entry;
				game.vars[V_border_touch_obj] = border;
			}
			if (v->motion == MOTION_MOVE_OBJ) {
				in_destination (v);
			}
		}

		v->flags &= ~FLAG10;
	}
}

/**
 * The Budin-Sonneveld offset
 * @param n view table entry number
 */
void fix_position (int n)
{
	struct vt_entry *v = &game.view_table[n];
	int count, dir, tries;

	_D (_D_WARN "adjusting view table entry #%d", n);

	/* test horizon */

	dir = 0;
	count = tries = 1;

	while (!check_position (v) || check_clutter (v) || check_priority (v)) {
		//_D (_D_WARN "from %d, %d", v->x_pos, v->y_pos);
		switch (dir) {
		case 0:			/* west */
			v->x_pos--;
			if (--count) continue;
			dir = 1;
			break;
		case 1:			/* south */
			v->y_pos++;
			if (--count) continue;
			dir = 2;
			tries++;
			break;
		case 2:			/* east */
			v->x_pos++;
			if (--count) continue;
			dir = 3;
			break;
		case 3:			/* north */
			v->y_pos--;
			if (--count) continue;
			dir = 0;
			tries++;
			break;
		}
		//_D (_D_WARN "to %d, %d", v->x_pos, v->y_pos);

		count = tries;
	}
}

/* end: checks.c */
