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
#include "menu.h"

#define TICK_SECONDS 20
 
extern struct agi_logic logics[];
extern struct agi_view views[];
extern struct agi_view_table view_table[];


static void update_objects ()
{
	int i;

	for(i = 0; i < MAX_VIEWTABLE; i++) {
		struct agi_view_table *vt_obj = &view_table[i];

		if (!(vt_obj->flags & UPDATE))
			continue;

		if ((~vt_obj->flags & ANIMATED) || (~vt_obj->flags & DRAWN))
			continue;

		if (vt_obj->flags & CYCLING) {
			int cel, num_cels;

			vt_obj->cycle_time_count++;

			if (vt_obj->cycle_time_count <= vt_obj->cycle_time) {
				/* --draw_obj(i); */
				continue;
			}

			vt_obj->cycle_time_count = 1;
			cel = vt_obj->current_cel;
			num_cels = VT_LOOP(view_table[i]).num_cels;

			switch (vt_obj->cycle) {
			case CYCLE_NORMAL:
				if(++cel >= num_cels) 
					cel = 0;
				break;
 			case CYCLE_END_OF_LOOP:		
 				if(++cel >= num_cels) {
					cel--;
					vt_obj->flags &= ~CYCLING;
					setflag (vt_obj->parm1, TRUE);
				}
 				break;
			case CYCLE_REV:			/* reverse cycle */
				if(--cel < 0)
					cel = num_cels - 1;
				break;
 			case CYCLE_REV_LOOP:
 				if (--cel < 0) {
					cel = 0;
					vt_obj->flags &= ~CYCLING;
					setflag (vt_obj->parm1, TRUE);
 				} 
 				break;
			}
			set_cel (i, cel);
		}
	} 
}


void adj_direction (struct agi_view_table *v, int h, int w)
{
	int dir;

	if (h == 0 && w == 0)
		return;

	dir = v->direction;

	if (abs(w) > abs(h)) {
		if (w > 0)
			dir = h < 0 ? 2 : h > 0 ? 4 : 3;
		else if (w < 0)
			dir = h < 0 ? 8 : h > 0 ? 6 : 7;
		else
			dir = h <= 0 ? 1 : 5;
	} else {
		if (h > 0)
			dir = w < 0 ? 6 : w > 0 ? 4 : 5;
		else if (h < 0)
			dir = w < 0 ? 8 : w > 0 ? 2 : 1;
		else
			dir = w <= 0 ? 7 : 3;
	}

	/* Always call calc_direction to avoid moonwalks */
	calc_direction (v->entry);
}


static int check_position (struct agi_view_table *v)
{
	if (v->x_pos + VT_WIDTH((*v)) > _WIDTH)
		return -1;

	if (v->y_pos < VT_HEIGHT((*v)))
		return -1;

	if (v->y_pos > _HEIGHT)
		return -1;

	if (~v->flags & IGNORE_HORIZON && v->y_pos <= game.horizon)
		return -1;

	return 0;
}


static int check_clutter (struct agi_view_table *v)
{
	struct agi_view_table *u;

	if (v->flags & IGNORE_OBJECTS)
		return 0;

	for (u = &view_table[0]; u <= &view_table[MAX_VIEWTABLE]; u++) {
		if ((u->flags & (ANIMATED|DRAWN)) == (ANIMATED|DRAWN))
			continue;

		if (u->flags & IGNORE_OBJECTS)
			continue;

		if (v->entry == u->entry)
			continue;

		if (v->x_pos + VT_WIDTH((*v)) < u->x_pos)
			continue;

		if (v->x_pos > u->x_pos + VT_WIDTH((*u)))
			continue;

#if 0
		if (v->y_pos != u->y_pos) {
			if (v->y_pos > u->y_pos) {
				if (v->old_y_pos < u->old_y_pos) {
					
				}
			}
		}
#endif

		if (v->y_pos >= u->y_pos)
			continue;

#if 0
		if (v->old_y_pos <= u->old_y_pos)
			continue;	
#endif

		return 0;
	}
	
	return -1;
}


static int check_priority (struct agi_view_table *v)
{
	int i, cel_width = VT_WIDTH((*v));
	int trigger, water, pass;

	if (~v->flags & FIXED_PRIORITY) {
		v->priority = v->y_pos < 48 ? 4 :
			v->y_pos / 12 + 1;
	}

	trigger = 0;
	water = 1;
	pass = 1;

	for (i = v->x_pos + cel_width - 1; i >= v->x_pos; i--) {
		UINT8 x = xdata_data[v->y_pos * _WIDTH + i];

		if (x == 0) {		/* unconditional black. no go at all! */
			pass = 0;
			break;
		}

		if (x == 3)		/* water surface */
			continue;

		water = 0;

		if (x == 1) {		/* conditional blue */
			if (v->flags & IGNORE_BLOCKS)
				continue;

			_D (_D_WARN "Blocks observed!");
			pass = 0;
			break;
		}

		if (x == 2) { 		/* trigger */
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

	return pass ? 0 : -1;
}


static void normal_motion (struct agi_view_table *v)
{
	int dir, var, w = VT_WIDTH((*v));
	int x, y, mt[9][2] = {
		{  0,  0 }, {  0, -1 }, {  1, -1 }, {  1,  0 },
		{  1,  1 }, {  0,  1 }, { -1,  1 }, { -1,  0 },
		{ -1, -1 }
	};

	if ((dir = v->direction) == 0)		/* stationary */
		return;

	x = mt[dir][0];
	y = mt[dir][1];

	if (VT_VIEW((*v)).loop == NULL) {
		_D(_D_CRIT "Attempt to access NULL view_table[%d].loop",
			v->entry);
		return;
	}

	if (v->current_cel >= VT_LOOP((*v)).num_cels) {
		_D(_D_CRIT "Attempt to access cel(%d) >= num_cels(%d) in vt%d",
			v->current_cel, VT_LOOP((*v)).num_cels, v->entry);
		return;
	}

	var = v->entry == 0 ? V_border_touch_ego : V_border_touch_obj;

	if (x + v->x_pos < 0 && (dir == 8 || dir == 7 || dir == 6)) {
		if (v->entry) setvar (V_border_code, v->entry);
		setvar (var, 4);
		return;
	}

	if (x + v->x_pos > _WIDTH - w && (dir == 2 || dir == 3 || dir == 4)) {
		if (v->entry) setvar (V_border_code, v->entry);
		setvar (var, 2);
		return;
	}

	if (y + v->y_pos > _HEIGHT - 1 && (dir == 4 || dir == 5 || dir == 6)) {
		if (v->entry) setvar (V_border_code, v->entry);
		setvar (var, 3);
		return;
	}

	if (y + v->y_pos < game.horizon && (dir == 1 || dir == 2 || dir == 8)) {
		if (v->entry) setvar (V_border_code, v->entry);
		setvar (var, 1);
		return;
	}

	v->x_pos += x;
	v->y_pos += y;

	if (check_position (v) || check_priority (v)) {
		v->x_pos -= x;
		v->y_pos -= y;
		return;
	}

	/* New object direction */
	adj_direction (v, y, x);
}


/* Budin-Sonneveld offset */
void reposition (int em)
{
	struct agi_view_table *v = &view_table[em];
	int count, dir, tries;

	/* test horizon */

	dir = 0;
	count = tries = 1;

	while (check_position (v) || check_priority (v)) {
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

		count = tries;
	}
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
	adj_direction (vt_obj, y2 - y1, x2 - x1);

	//check_position (em, x1, y2);
	//check_priority (em, x1, y2);

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

		if (~vt_obj->flags & UPDATE)
			continue;

#if 0
		if (~vt_obj->flags & MOTION) {
			check_surface (em, vt_obj->x_pos, vt_obj->y_pos);
			continue;
		}
#endif

		vt_obj->step_time_count += vt_obj->step_size;

		if (vt_obj->step_time_count <= vt_obj->step_time)
			continue;

		vt_obj->step_time_count = 1;

		switch(vt_obj->motion) {
		case MOTION_NORMAL:		/* normal */
			normal_motion (vt_obj);
			break;
		case MOTION_WANDER:		/* wander */
			ox = vt_obj->x_pos;
			oy = vt_obj->y_pos;
			normal_motion (vt_obj);

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

		/* CM: commented out -- setvar (V_key, 0x0); */
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
			_D (_D_WARN "ego_in_new_room");
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


void update_timer ()
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


static int old_mode = -1;
void new_input_mode (int i)
{
	old_mode = game.input_mode;
	game.input_mode = i;
}

void old_input_mode ()
{
	game.input_mode = old_mode;
}

/* If main_cycle returns FALSE, don't process more events! */
int main_cycle ()
{
	int key, kascii;

	poll_timer ();		/* msdos driver -> does nothing */
	update_timer ();

	key = poll_keyboard ();
	kascii = KEY_ASCII (key);

	if (!console_keyhandler (key)) {
		if (kascii) setvar (V_key, kascii);
		switch (game.input_mode) {
		case INPUT_NORMAL:
			handle_controller (key);
			handle_keys (key);
			if (key) game.keypress = key;
			break;
		case INPUT_GETSTRING:
			handle_controller (key);
			handle_getstring (key);
			break;
		case INPUT_MENU:
			menu_keyhandler (key);
			console_cycle ();
			return FALSE;
		case INPUT_NONE:
			handle_controller (key);
			if (key) game.keypress = key;
			break;
		}
	} else {
		if (game.input_mode == INPUT_MENU) {
			console_cycle ();
			return FALSE;
		}
	}

	console_cycle ();

	if (getvar (V_window_reset) > 0) {
		game.msg_box_ticks = getvar (V_window_reset) * 10;
		setvar (V_window_reset, 0);
	}

	if (game.msg_box_ticks > 0)
		game.msg_box_ticks--;

	return TRUE;
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

	game.new_room_num = 0;
	game.quit_prog_now = FALSE;
	game.clock_enabled = TRUE;
	game.ego_in_new_room = FALSE;
	game.exit_all_logics = FALSE;
	game.input_mode = INPUT_NONE;

	report (" \nSarien " VERSION " is ready.\n");
	report ("Running AGI script.\n");

#ifdef USE_CONSOLE
	console.count = 20;
	console_prompt ();
#endif

	/* clean_keyboard (); */

	do {
		if (!main_cycle ())
			continue;

		x = 1 + clock_count;		/* x = 1..TICK_SECONDS */
		y = getvar (V_time_delay);	/* 1/20th of second delay */

		if (y == 0 || (x % y == 0 && z != x)) {
			z = x;
			interpret_cycle ();
			/* clean_keyboard (); */
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

