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
#include "graphics.h"
#include "keyboard.h"
#include "menu.h"
#include "savegame.h"

#define TICK_SECONDS	20

#ifdef USE_MOUSE
struct mouse mouse;
#endif
 
#ifndef MACOSX_SDL
volatile UINT32 clock_ticks;
volatile UINT32 clock_count;
#endif


/**
 * Set up new room.
 * This function is called when ego enters a new room.
 * @param n room number
 */
void new_room (int n)
{
	struct vt_entry *v;
	int i;

	_D (_D_WARN "*** room %d ***", n);
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
	
	agi_unload_resources ();

	game.player_control = TRUE;
	game.block.active = FALSE;
	game.horizon = 36;
	game.vars[V_prev_room] = game.vars[V_cur_room];
	game.vars[V_cur_room] = n;
	game.vars[V_border_touch_obj] = 0;
	game.vars[V_border_code] = 0;
	game.vars[V_ego_view_resource] = game.view_table[0].current_view;

	agi_load_resource (rLOGIC, n);

	/* Reposition ego in the new room */
	switch (game.vars[V_border_touch_ego]) {
	case 1:
		game.view_table[0].y_pos = _HEIGHT - 1;
		break;
	case 2:
		game.view_table[0].x_pos = 0;
		break;
	case 3:
		game.view_table[0].y_pos = HORIZON + 1;
		break;
	case 4:
		game.view_table[0].x_pos = _WIDTH - game.view_table[0].x_size;
		break;
	}

	game.vars[V_border_touch_ego] = 0;
	setflag (F_new_room_exec, TRUE);

	game.exit_all_logics = TRUE;

	write_status ();
	write_prompt ();
}

static void reset_controllers ()
{
	int i;

	for (i = 0; i < MAX_DIRS; i++) {
		game.ev_scan[i].occured = FALSE;
		game.ev_keyp[i].occured = FALSE;
	}
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
	while (run_logic (0) == 0 && !game.quit_prog_now) {
		game.vars[V_word_not_found] = 0;
		game.vars[V_border_touch_obj] = 0;
		game.vars[V_border_code] = 0;
		old_score = game.vars[V_score];
		setflag (F_entered_cli, FALSE);
		game.exit_all_logics = FALSE;
		reset_controllers ();
	}
	reset_controllers ();

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

/**
 * Update AGI interpreter timer.
 */
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
	unsigned int key, kascii;
	struct vt_entry *v = &game.view_table[0];

	poll_timer ();		/* msdos driver -> does nothing */
	update_timer ();

	if (game.ver == 0) {
		message_box ("Warning: game CRC not listed, assuming "
			"AGI version 2.917.");
		game.ver = -1;
	}

	key = do_poll_keyboard ();

#ifdef USE_MOUSE
	/* In AGI Mouse emulation mode we must update the mouse-related
	 * vars in every interpreter cycle.
	 */
	if (opt.agimouse) {
		game.vars[28] = mouse.x / 2;
		game.vars[29] = mouse.y;
	}
#endif

#ifdef USE_CONSOLE
	if (key == KEY_PRIORITY) {
		erase_both ();
		debug.priority = !debug.priority;
		show_pic ();
		blit_both ();
		commit_both ();
		key = 0;
	}

	if (key == KEY_STATUSLN) {
		debug.statusline = !debug.statusline;
		write_status ();
		key = 0;
	}
#endif

	/* Click-to-walk mouse interface */
	if (game.player_control && v->flags & ADJ_EGO_XY) {
		v->direction = get_direction (v->x_pos, v->y_pos,
			v->parm1, v->parm2, v->step_size);

		if (v->direction == 0)
			in_destination (v);
	}

	kascii = KEY_ASCII (key);

	if (!console_keyhandler (key)) {
		if (kascii) setvar (V_key, kascii);
process_key:
		switch (game.input_mode) {
		case INPUT_NORMAL:
			if (!handle_controller (key)) {
				if (key == 0 || !game.input_enabled)
					break;
				handle_keys (key);

				/* if ESC pressed, activate menu before
				 * accept.input from the interpreter cycle
				 * sets the input mode to normal again
				 * (closes: #540856)
				 */
				if (key == KEY_ESCAPE) {
					key = 0;
					goto process_key;
				}

				/* commented out to close bug #438872
				 * if (key) game.keypress = key;
				 */
			}
			break;
		case INPUT_GETSTRING:
			handle_controller (key);
			handle_getstring (key);
			setvar (V_key, 0);	/* clear ENTER key */
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

static int play_game ()
{
	int ec = err_OK;

	_D (_D_WARN "initializing...");
	_D (_D_WARN "game.ver = 0x%x", game.ver);
	stop_sound ();
	clear_screen (0);

	game.horizon = HORIZON;
	game.player_control = FALSE;

	setflag (F_logic_zero_firsttime, TRUE);	/* not in 2.917 */
	setflag (F_new_room_exec, TRUE);	/* needed for MUMG and SQ2! */
	setflag (F_sound_on, TRUE);		/* enable sound */
	setvar (V_time_delay, 2);		/* "normal" speed */

	game.gfx_mode = TRUE;
	game.quit_prog_now = FALSE;
	game.clock_enabled = TRUE;
	game.line_user_input = 22;

#ifdef USE_MOUSE
	if (opt.agimouse)
		report ("Using AGI Mouse 1.0 protocol\n");
#endif

	report ("Running AGI script.\n");

#ifdef USE_CONSOLE
	console.count = 5;
	console_prompt ();
#endif

	setflag (F_entered_cli, FALSE);
	setflag (F_said_accepted_input, FALSE);

	_D (_D_WARN "Entering main loop");
	do {
		if (!main_cycle ())
			continue;
	
		if (getvar (V_time_delay) == 0 ||
		    (1 + clock_count) % getvar (V_time_delay) == 0)
		{
			if (!game.has_prompt && game.input_mode == INPUT_NORMAL) {
				write_prompt ();
				game.has_prompt = 1;
			} else
			if (game.has_prompt && game.input_mode == INPUT_NONE) {
				write_prompt ();
				game.has_prompt = 0;
			}

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


int run_game ()
{
	int ec = err_OK;

#ifdef USE_HIRES
	if (opt.cgaemu)
		opt.hires = 0;
#endif

	/* Execute the game */
    	do {
		_D(_D_WARN "game loop");
		_D(_D_WARN "game.ver = 0x%x", game.ver);

		if (agi_init () != err_OK)
			break;
		if (ec == err_RestartGame)
			setflag (F_restart_game, TRUE);

		setvar (V_computer, 0);		/* IBM PC (4 = Atari ST) */
		setvar (V_soundgen, 1);		/* IBM PC SOUND */
		setvar (V_monitor, 0x3);	/* EGA monitor */
		setvar (V_max_input_chars, 38);
		game.input_mode = INPUT_NONE;
		game.input_enabled = 0;
		game.has_prompt = 0;

		game.state = STATE_RUNNING;
		ec = play_game();
		game.state = STATE_LOADED;
		agi_deinit ();
    	} while (ec == err_RestartGame);

	menu_deinit();

	release_image_stack();

	return ec;
}

