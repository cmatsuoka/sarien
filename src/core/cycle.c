/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#include <stdlib.h>

#include "sarien.h"
#include "agi.h"
#include "rand.h"
#include "gfx_agi.h"
#include "gfx_base.h"		/* FIXME: check this */
#include "keyboard.h"
#include "picture.h"
#include "view.h"
#include "logic.h"
#include "sound.h"
#include "opcodes.h"
#include "console.h"

#define TICK_SECONDS 20
 
#ifdef USE_CONSOLE
extern struct sarien_console console;
#endif

extern struct agi_logic logics[];
extern struct agi_view views[];
extern struct agi_view_table view_table[];

static void update_objects ()
{
	int i, ccel;

	/* copy the bitmaps onto the screen */
	for(i = 0; i < MAX_VIEWTABLE; i++) {
#define VT view_table[i]
		/* FR
		 * Changed here 
		 */
		if (!(VT.flags & UPDATE))
			continue;

		if (!(VT.flags & ANIMATED) || !(VT.flags & DRAWN))
			continue;

		if (VT.flags & CYCLING) {
			VT.cycle_time_count++;

			if (VT.cycle_time_count <= VT.cycle_time) {
				/* --draw_obj(i); */
				continue;
			}

			VT.cycle_time_count = 1;
			ccel = VT.current_cel;

			switch (VT.cycle_status) {
			case CYCLE_NORMAL:
				if(++ccel >= VT_LOOP(VT).num_cels) 
					ccel = 0;
				set_cel (i, ccel);
				break;
 			case CYCLE_END_OF_LOOP:		
 				if(++ccel >= VT_LOOP(VT).num_cels) {
					VT.flags &= ~CYCLING;
					setflag (VT.parm1, TRUE);
  				} else {
 					set_cel (i, ccel);
				}
 				break;
			case CYCLE_REV:			/* reverse cycle */
				if(--ccel < 0)
					ccel = VT_LOOP(VT).num_cels - 1;

				set_cel (i, ccel);
				break;
 			case CYCLE_REV_LOOP:
 				if (--ccel < 0) {
					VT.flags &= ~CYCLING;
					setflag (VT.parm1, TRUE);
 				} 
 				set_cel (i, ccel);
 				break;
			}
		} else {
			if ((ccel = VT.current_cel) >= VT_LOOP(VT).num_cels)
				ccel = 0;

			set_cel (i, ccel);
		}
	} 
#undef VT
}


void adj_direction (int entry, int h, int w)
{
	struct agi_view_table *vt_obj;

	if (h == 0 && w == 0)
		return;

	vt_obj = &view_table[entry];

	if (abs(w) > abs(h)) {
		if (w > 0)
			vt_obj->direction = h < 0 ? 2 : h > 0 ? 4 : 3;
		else if (w < 0)
			vt_obj->direction = h < 0 ? 8 : h > 0 ? 6 : 7;
		else
			vt_obj->direction = h <= 0 ? 1 : 5;
	} else {
		if (h > 0)
			vt_obj->direction = w < 0 ? 6 : w > 0 ? 4 : 5;
		else if (h < 0)
			vt_obj->direction = w < 0 ? 8 : w > 0 ? 2 : 1;
		else
			vt_obj->direction = w <= 0 ? 7 : 3;
	}

	/* Always call calc_direction to avoid moonwalks */
	calc_direction (entry);
}


static void normal_motion (int em, int x, int y)
{
	int dir, v, i, e, w;
	struct agi_view_table *vt_obj;
	int cel_width;

	if (VT_VIEW(view_table[em]).loop == NULL) {
		_D(_D_CRIT "Attempt to access NULL view_table[%d].loop", em);
		return;
	}

	vt_obj = &view_table[em];
	cel_width = VT_WIDTH(view_table[em]);

	x += vt_obj->x_pos;
	y += vt_obj->y_pos;
	dir = vt_obj->direction;
	e = (em == EGO_VIEW_TABLE);

	v = e ? V_border_touch_ego : V_border_touch_obj;

	if (x < 0 && (dir == 8 || dir == 7 || dir == 6)) {
		_D (_D_WARN "left border: vt %d, x %d, dir %d", em, x, dir); 
		if (!e)
			setvar (V_border_code, em);
		setvar (v, 4);
		return;
	}

	if (x > _WIDTH - cel_width && (dir == 2 || dir == 3 || dir == 4)) {
		_D (_D_WARN "right border: vt %d, x %d, dir %d", em, x, dir); 
		if (!e)
			setvar (V_border_code, em);
		setvar (v, 2);
		return;
	}

	if (y > _HEIGHT - 1 && (dir == 4 || dir == 5 || dir == 6)) {
		_D (_D_WARN "bottom border: vt %d, x %d, dir %d", em, x, dir); 
		if (!e)
			setvar (V_border_code, em);
		setvar (v, 3);
		return;
	}

	if (y < game.horizon && (dir == 1 || dir == 2 || dir == 8)) {
		_D (_D_WARN "top border: vt %d, x %d, dir %d", em, x, dir); 
		if (!e)
			setvar (V_border_code, em);
		setvar (v, 1);
		return;
	}

	if (e) {
		setflag (F_ego_water, FALSE);
		setflag (F_ego_touched_p2, FALSE);
	}

	/* do control lines n shit in here */

	w = 0;
	for (i = x + cel_width - 1; i >= x; i--) {
		switch (control_data[y * _WIDTH + i]) {
		case 0:	/* unconditional black. no go at all! */
			return;
		case 1:			/* conditional blue */
			if (~vt_obj->flags & IGNORE_BLOCKS)
				return;
			break;
		case 2:			/* trigger */
			if (!e)
				break;
			setflag (3, TRUE);
			vt_obj->x_pos = x;
			vt_obj->y_pos = y;
			return;
		case 3:			/* water */
			if (!e)
				break;
			w++;
			break;
		}
	}

	if (e) {
		/* Check if ego is completely on water */
		if (w >= cel_width) {
			_D (_D_WARN "Ego is completely on water");
			vt_obj->x_pos = x;
			vt_obj->y_pos = y;
			setflag (F_ego_water, TRUE);
			return;
		}
	}

	for (i = x + cel_width - 1; i >= x; i--) {
		int z;

		if (y < game.horizon || y >= _HEIGHT || i < 0 || i >= _WIDTH)
			return;

		z = control_data[y * _WIDTH + i];

		if ((vt_obj->flags & ON_WATER) && z != 3)
			return;

		if ((vt_obj->flags & ON_LAND) && z != 4)
			return;
	}

	/* New object direction */
	adj_direction (em, y - vt_obj->y_pos, x - vt_obj->x_pos);

	vt_obj->x_pos = x;
	vt_obj->y_pos = y;
}


static void set_motion (int em)
{
	int i, mt[9][2] = {
		{  0,  0 }, {  0, -1 }, {  1, -1 }, {  1,  0 },
		{  1,  1 }, {  0,  1 }, { -1,  1 }, { -1,  0 },
		{ -1, -1 }
	};

	i = view_table[em].direction;

	normal_motion (em, mt[i][0], mt[i][1]);
}


static void adj_pos (int em, int x2, int y2)
{
	int x1, y1, z;
	static int frac = 0;
	struct agi_view_table *vt_obj;

	vt_obj = &view_table[em];

	x1 = vt_obj->x_pos;
	y1 = vt_obj->y_pos;

	/* step size is given * 4, must compute fractionary step increments */
	if (vt_obj->motion == MOTION_FOLLOW_EGO) {
		z = vt_obj->step_size >> 2;
		frac += vt_obj->step_size & 0x03;
		if (frac >= 4) {
			frac -= 4;
			z++;
		}
	} else {
		z = vt_obj->step_size;
	}

	/* adjust the direction */
	adj_direction (em, y2 - y1, x2 - x1);

#define CLAMP_MAX(a,b,c) { if(((a)+=(c))>(b)) { (a)=(b); } }
#define CLAMP_MIN(a,b,c) { if(((a)-=(c))<(b)) { (a)=(b); } }
#define CLAMP(a,b,c) { if((a)<(b)) CLAMP_MAX(a,b,c) else CLAMP_MIN(a,b,c) }

	CLAMP (x1, x2, z);
	CLAMP (y1, y2, z);

	vt_obj->x_pos = x1;
	vt_obj->y_pos = y1;
}


static void calc_obj_motion ()
{
	int original_direction;
	int em, ox, oy;
	struct agi_view_table *vt_obj, *vt_ego;

	vt_ego = &view_table[EGO_VIEW_TABLE];

	for (em = 0; em < MAX_VIEWTABLE; em++) {
		vt_obj = &view_table[em];

		original_direction = vt_obj->direction;

		if ((~vt_obj->flags & MOTION) || (~vt_obj->flags & UPDATE))
			continue;

		vt_obj->step_time_count += vt_obj->step_size;

		if(vt_obj->step_time_count <= vt_obj->step_time)
			continue;

		vt_obj->step_time_count=1;

		switch(vt_obj->motion) {
		case MOTION_NORMAL:		/* normal */
			set_motion (em);
			break;
		case MOTION_WANDER:		/* wander */
			ox = vt_obj->x_pos;
			oy = vt_obj->y_pos;
			set_motion (em);

			/* CM: FIXME: when object walks offscreen (x < 0)
			 *     it returns x_pos == ox, y_pos == oy and
			 *     the last frame blitted shows object facing
			 *     another direction.
			 */
			if (vt_obj->x_pos == ox && vt_obj->y_pos == oy)
				vt_obj->direction = rnd (9);
			break;
		case MOTION_FOLLOW_EGO:		/* follow ego */
			adj_pos (em, vt_ego->x_pos, vt_ego->y_pos);

			if (vt_obj->x_pos == vt_ego->x_pos &&
				vt_obj->y_pos == vt_ego->y_pos)
			{
				vt_obj->motion = MOTION_NORMAL;
				vt_obj->flags &= ~MOTION;
				setflag (vt_obj->parm2, TRUE);
				vt_obj->parm2 = 1;
				vt_obj->step_size = vt_obj->parm1;
			}
			break;
		case MOTION_MOVE_OBJ:		/* move obj */
			adj_pos (em, vt_obj->parm1, vt_obj->parm2);

			if (vt_obj->x_pos == vt_obj->parm1 &&
				vt_obj->y_pos == vt_obj->parm2)
			{
				_D (_D_WARN "obj %d at (%d, %d), set %d!", em,
					vt_obj->x_pos, vt_obj->y_pos,
					vt_obj->parm4);
				if (em == EGO_VIEW_TABLE)
					setvar (V_ego_dir, 0);

				vt_obj->direction = 0;
				setflag (vt_obj->parm4, TRUE);
				vt_obj->step_size = vt_obj->parm3;
				vt_obj->flags &= ~MOTION;
				vt_obj->motion = MOTION_NORMAL;
			}
			break;
		}

		if (vt_obj->direction != original_direction)
   			calc_direction (em);
	}
}


static void interpret_cycle ()
{
	int line_prompt = FALSE;

	if (game.control_mode == CONTROL_PROGRAM)
		view_table[EGO_VIEW_TABLE].direction = getvar (V_ego_dir);
	else
		setvar (V_ego_dir, view_table[EGO_VIEW_TABLE].direction);

	update_status_line (FALSE);

	release_sprites ();

	do {
		/* set current start IP */
		logics[0].cIP = logics[0].sIP;

		run_logic (0);

		/* make sure logic 0 is not set to 'firsttime' */
		setflag (F_logic_zeron_firsttime, FALSE);

		setvar (V_key, 0x0);
		setvar (V_word_not_found, 0);
		setvar (V_border_code, 0);			/* 5 */
		setvar (V_border_touch_obj, 0);			/* 4 */
		setflag (F_entered_cli, FALSE);
		setflag (F_said_accepted_input, FALSE);
		setflag (F_new_room_exec, FALSE);		/* 5 */
		setflag (F_output_mode, FALSE);	/*************************/
		setflag (F_restart_game, FALSE);		/* 6 */
		setflag (F_status_selects_items, FALSE);	/* 12? */

		if (game.control_mode == CONTROL_PROGRAM) {
			view_table[EGO_VIEW_TABLE].direction = getvar (V_ego_dir);
		} else {
			setvar (V_ego_dir, view_table[EGO_VIEW_TABLE].direction);
		}

		clean_input (); 

#ifndef NO_DEBUG
		/* quit built in debugger command */
		if (opt.debug == 3 || opt.debug == 4)
			opt.debug = TRUE;
#endif

		if (game.quit_prog_now)
			break;

		update_status_line (FALSE);

		if (game.ego_in_new_room) {
			new_room (game.new_room_num);
			update_status_line (TRUE);
			game.ego_in_new_room = FALSE;
			line_prompt = TRUE;
		} else {
			if (screen_mode == GFX_MODE) {
				calc_obj_motion ();
				update_objects ();
			}
			break;
		}

		/* CM: hmm... it looks strange :\ */
		game.exit_all_logics = FALSE;
	} while (!game.exit_all_logics);

	redraw_sprites ();

	if (line_prompt)
		print_line_prompt();
}


static void update_timer ()
{
	if (!game.clock_enabled)
		return;

	clock_count++;
	if (clock_count <= TICK_SECONDS)
		return;

	clock_count -= TICK_SECONDS;
	setvar (V_seconds, getvar (V_seconds) + 1);
	if (getvar (V_seconds) < 60)
		return;

	setvar (V_seconds, 0);
	setvar (V_minutes, getvar (V_minutes) + 1);
	if (getvar (V_minutes) < 60)
		return;

	setvar (V_minutes, 0);
	setvar (V_hours, getvar (V_hours) + 1);
	if (getvar (V_hours) < 24)
		return;

	setvar (V_hours, 0);
	setvar (V_days, getvar (V_days) + 1);
}


void main_cycle (int accept_key)
{
	poll_timer ();		/* msdos driver -> does nothing */
	update_timer ();

	poll_keyboard ();

#ifdef USE_CONSOLE
	if (console.active && console.input_active)
		handle_console_keys ();
	else
#endif
	if (accept_key)
		handle_keys ();

#ifdef USE_CONSOLE
	console_cycle ();
#endif

	if (getvar (V_window_reset) > 0) {
		game.msg_box_ticks = getvar (V_window_reset) * 10;
		setvar (V_window_reset, 0);
	}

	if (game.msg_box_ticks > 0)
		game.msg_box_ticks--;
}


int run_game2 ()
{
	int ec = err_OK;
	UINT32 x, y, z = 12345678;

	_D ("()");

	stop_sound ();
	clear_buffer ();

	/*setflag(F_logic_zeron_firsttime, TRUE);*/
	setflag (F_new_room_exec, TRUE);
	setflag (F_restart_game, FALSE);
	setflag (F_sound_on, TRUE);		/* enable sound */
	setvar (V_time_delay, 2);		/* "normal" speed */

	game.allow_kyb_input = FALSE;
	game.new_room_num = 0;
	game.quit_prog_now = FALSE;
	game.clock_enabled = TRUE;
	game.ego_in_new_room = FALSE;
	game.exit_all_logics = FALSE;

	report (" \nSarien " VERSION " is ready.\n");
	report ("Running AGI script.\n");

#ifdef USE_CONSOLE
	console.count = 20;
	console_prompt ();
#endif

	clean_keyboard ();

	do {
		main_cycle (TRUE);

		x = 1 + clock_count;		/* x = 1..TICK_SECONDS */
		y = getvar (V_time_delay);	/* 1/20th of second delay */

		if (y == 0 || (x % y == 0 && z != x)) {
			z = x;
			interpret_cycle ();
			clean_keyboard (); 
		}

		do_blit ();

		if (game.quit_prog_now == 0xff) {
			ec = err_RestartGame;
			break;
		}
	} while (!game.quit_prog_now);

	stop_sound ();
	clear_buffer ();
	put_screen ();

	return ec;
}

