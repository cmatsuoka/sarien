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
#include "text.h"
#include "sprite.h"
#include "keyboard.h"
#include "menu.h"

#define TICK_SECONDS 20
 
/**
 * Set up new room.
 * This function is called when ego enters a new room.
 * @param n room number
 */
void new_room (int n)
{
	struct vt_entry *v;
	int i;

	_D (_D_WARN "room %d", n);
	stop_sound ();

	i = 0;
	for_each_vt_entry (v) {
		v->entry = i++;
		v->flags &= ~(ANIMATED|DRAWN);
		v->flags |= UPDATE;
		v->step_time = 1;
		v->step_time_count = 1;
		v->cycle_time = 1;
		v->cycle_time_count = 1;
		v->step_size = 1;
	}
	
	/* function 0x10d0 */

	game.player_control = TRUE;
	game.block.active = FALSE;
	game.horizon = 36;
	game.vars[V_prev_room] = game.vars[V_cur_room];
	game.vars[V_cur_room] = n;
	game.vars[V_border_touch_obj] = 0;
	game.vars[V_border_code] = 0;
	game.vars[V_ego_view_resource] = game.view_table[0].current_view;

	agi_load_resource (rLOGIC, n);		/*load_logic (n);*/

	/* Reposition ego in the new room */
	switch (game.vars[V_border_touch_ego]) {
	case 1:
		game.view_table[0].y_pos = _HEIGHT;
		break;
	case 2:
		game.view_table[0].x_pos = 0;
		break;
	case 3:
		game.view_table[0].y_pos = HORIZON + 1;
		break;
	case 4:
		game.view_table[0].x_pos = _WIDTH - v->x_size;
		break;
	}

	game.vars[V_border_touch_ego] = 0;
	setflag (F_new_room_exec, TRUE);

	game.exit_all_logics = TRUE;

	/* clear kb buffer */
	/* write status */
	/* function 0x38d5 */
}


static void interpret_cycle ()
{
	int old_sound, old_score;

	if (game.player_control)
		game.vars[V_ego_dir] = game.view_table[0].direction;
	else
		game.view_table[0].direction = game.vars[V_ego_dir];

	check_all_motions ();
	old_score = game.vars[V_score];
	old_sound = getflag (F_sound_on);
	
	game.exit_all_logics = FALSE;
	while (run_logic (0) == 0) {
		game.vars[V_word_not_found] = 0;
		game.vars[V_border_touch_obj] = 0;
		game.vars[V_border_code] = 0;
		old_score = game.vars[V_score];
		setflag (F_entered_cli, FALSE);
		game.exit_all_logics = FALSE;
	}

	game.view_table[0].direction = game.vars[V_ego_dir];

	if (game.vars[V_score] != old_score || getflag(F_sound_on) != old_sound)
		write_status ();	

	game.vars[V_border_touch_obj] = 0;
	game.vars[V_border_code] = 0;
	setflag (F_new_room_exec, FALSE);
	setflag (F_restart_game, FALSE);
	setflag (F_restore_just_ran, FALSE);

	if (game.gfx_mode) {
		update_viewtable ();
		do_update ();
	}
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

	if (key == KEY_PRIORITY) {
		erase_both ();
		debug.priority = !debug.priority;
		show_pic ();
		blit_both ();
		key = 0;
	}

	kascii = KEY_ASCII (key);

	if (!console_keyhandler (key)) {
		if (kascii) setvar (V_key, kascii);
		switch (game.input_mode) {
		case INPUT_NORMAL:
			if (!handle_controller (key)) {
				handle_keys (key);
				if (key) game.keypress = key;
			}
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

	if (game.msg_box_ticks > 0)
		game.msg_box_ticks--;

	return TRUE;
}


int run_game2 ()
{
	int ec = err_OK;
	int x, y;

	_D (_D_WARN "initializing...");
	stop_sound ();

	setflag (F_logic_zero_firsttime, TRUE);	/* not in 2.917 */
	setflag (F_new_room_exec, TRUE);	/* needed for MUMG and SQ2! */
	setflag (F_restart_game, FALSE);
	setflag (F_sound_on, TRUE);		/* enable sound */
	setvar (V_time_delay, 2);		/* "normal" speed */

	game.quit_prog_now = FALSE;
	game.clock_enabled = TRUE;
	game.input_mode = INPUT_NONE;
	game.line_user_input = 22;

	report (" \nSarien " VERSION " is ready.\n");
	report ("Running AGI script.\n");

#ifdef USE_CONSOLE
	console.count = 20;
	console_prompt ();
#endif

	setflag (F_entered_cli, FALSE);
	setflag (F_said_accepted_input, FALSE);

	_D (_D_WARN "Entering main loop");
	do {
		if (!main_cycle ())
			continue;
	
		x = 1 + clock_count;		/* x = 1..TICK_SECONDS */
		y = getvar (V_time_delay);	/* 1/20th of second delay */
	
		if (y == 0 || (x % y == 0)) {
			interpret_cycle ();
			setflag (F_entered_cli, FALSE);
			setflag (F_said_accepted_input, FALSE);
		}

		if (game.quit_prog_now == 0xff)
			ec = err_RestartGame;

	} while (game.quit_prog_now == 0);

	stop_sound ();

	return ec;
}

