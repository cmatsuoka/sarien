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
#include "rand.h"
#include "sprite.h"
#include "graphics.h"
#include "keyboard.h"
#include "opcodes.h"
#include "menu.h"
#include "savegame.h"
#include "text.h"

#define p0	(p[0])
#define p1	(p[1])
#define p2	(p[2])
#define p3	(p[3])
#define p4	(p[4])
#define p5	(p[5])
#define p6	(p[6])

#define ip	cur_logic->cIP
#define vt	game.view_table[p0]

static struct agi_logic *cur_logic;

#define _v game.vars
#define cmd(x) static void cmd_##x (UINT8 *p)

cmd(increment)		{ if (_v[p0] != 0xff) ++_v[p0]; }
cmd(decrement)		{ if (_v[p0] != 0) --_v [p0]; }
cmd(assignn)		{ _v[p0] = p1; }
cmd(addn)		{ _v[p0] += p1; }
cmd(subn)		{ _v[p0] -= p1; }
cmd(assignv)		{ _v[p0] = _v[p1]; }
cmd(addv)		{ _v[p0] += _v[p1]; }
cmd(subv)		{ _v[p0] -= _v[p1]; }
cmd(mul_n)		{ _v[p0] *= p1; }
cmd(mul_v)		{ _v[p0] *= _v[p1]; }
cmd(div_n)		{ _v[p0] /= p1; }
cmd(div_v)		{ _v[p0] /= _v[p1]; }
cmd(random)		{ _v[p2] = rnd (1 + (p1 - p0)) + p0; }
cmd(lindirectn)		{ _v[_v[p0]] = p1; }
cmd(lindirectv)		{ _v[_v[p0]] = _v[p1]; }
cmd(rindirect)		{ _v[p0] = _v[_v[p1]]; }
cmd(set)		{ setflag (*p, TRUE); }
cmd(reset)		{ setflag (*p, FALSE); }
cmd(toggle)		{ setflag (*p, !getflag (*p)); }
cmd(set_v)		{ setflag (_v[p0], TRUE); }
cmd(reset_v)		{ setflag (_v[p0], FALSE); }
cmd(toggle_v)		{ setflag (_v[p0], !getflag (*p)); }
cmd(new_room)		{ new_room (p0); }
cmd(new_room_f)		{ new_room (_v[p0]); }
cmd(load_view)		{ agi_load_resource (rVIEW, p0); }
cmd(load_logic)		{ agi_load_resource (rLOGIC, p0); }
cmd(load_sound)		{ agi_load_resource (rSOUND, p0); }
cmd(load_view_f)	{ agi_load_resource (rVIEW, _v[p0]); }
cmd(load_logic_f)	{ agi_load_resource (rLOGIC, _v[p0] ); }
cmd(discard_view)	{ agi_unload_resource (rVIEW, p0); }
cmd(object_on_anything)	{ vt.flags &= ~(ON_WATER | ON_LAND); }
cmd(object_on_land)	{ _D ("p0 = %d", p0); vt.flags |= ON_LAND; }
cmd(object_on_water)	{ _D ("p0 = %d", p0); vt.flags |= ON_WATER; }
cmd(observe_horizon)	{ _D ("p0 = %d", p0); vt.flags &= ~IGNORE_HORIZON; }
cmd(ignore_horizon)	{ _D ("p0 = %d", p0); vt.flags |= IGNORE_HORIZON; }
cmd(observe_objs)	{ _D ("p0 = %d", p0); vt.flags &= ~IGNORE_OBJECTS; }
cmd(ignore_objs)	{ _D ("p0 = %d", p0); vt.flags |= IGNORE_OBJECTS; }
cmd(observe_blocks)	{ _D ("p0 = %d", p0); vt.flags &= ~IGNORE_BLOCKS; }
cmd(ignore_blocks)	{ _D ("p0 = %d", p0); vt.flags |= IGNORE_BLOCKS; }
cmd(set_horizon)	{ _D ("p0 = %d", p0); game.horizon = p0; }
cmd(get_priority)	{ _v[p1] = vt.priority; }
cmd(set_priority)	{ vt.flags |= FIXED_PRIORITY; vt.priority = p1; }
cmd(set_priority_f)	{ vt.flags |= FIXED_PRIORITY; vt.priority = _v[p1]; }
cmd(release_priority)	{ vt.flags &= ~FIXED_PRIORITY; }
cmd(set_upper_left)	{ /* do nothing (AGI 2.917) */ }
cmd(start_update)	{ start_update (&vt); }
cmd(stop_update)	{ stop_update (&vt); }
cmd(current_view)	{ _v[p1] = vt.current_view; }
cmd(current_cel)	{ _v[p1] = vt.current_cel; _D ("v%d=%d", p1, _v[p1]); }
cmd(current_loop)	{ _v[p1] = vt.current_loop; }
cmd(last_cel)		{ _v[p1] = vt.loop_data->num_cels - 1; }
cmd(set_cel)		{ set_cel (&vt, p1); vt.flags &= ~DONTUPDATE; }
cmd(set_cel_f)		{ set_cel (&vt, _v[p1]); vt.flags &= ~DONTUPDATE; }
cmd(set_view)		{ set_view (&vt, p1); }
cmd(set_view_f)		{ set_view (&vt, _v[p1]); }
cmd(set_loop)		{ set_loop (&vt, p1); }
cmd(set_loop_f)		{ set_loop (&vt, _v[p1]); }
cmd(number_of_loops)	{ _v[p1] = vt.num_loops; }
cmd(fix_loop)		{ vt.flags |= FIX_LOOP; }
cmd(release_loop)	{ vt.flags &= ~FIX_LOOP; }
cmd(step_size)		{ vt.step_size = _v[p1]; }
cmd(step_time)		{ vt.step_time = vt.step_time_count = _v[p1]; }
cmd(cycle_time)		{ vt.cycle_time = vt.cycle_time_count = _v[p1]; }
cmd(stop_cycling)	{ vt.flags &= ~CYCLING; }
cmd(start_cycling)	{ vt.flags |= CYCLING; }
cmd(normal_cycle)	{ vt.cycle = CYCLE_NORMAL; vt.flags |= CYCLING; }
cmd(reverse_cycle)	{ vt.cycle = CYCLE_REVERSE; vt.flags |= CYCLING; }
cmd(set_dir)		{ vt.direction = _v[p1]; }
cmd(get_dir)		{ _v[p1] = vt.direction; }
cmd(get_room_f)		{ _v[p1] = object_get_location (_v[p0]); }
cmd(put)		{ object_set_location (p0, _v[p1]); }
cmd(put_f)		{ object_set_location (_v[p0], _v[p1]); }
cmd(drop)		{ object_set_location (p0, 0); }
cmd(get)		{ object_set_location (p0, EGO_OWNED); }
cmd(get_f)		{ object_set_location (_v[p0], EGO_OWNED); }
cmd(parse)		{ dictionary_words (agi_sprintf(game.strings[p0])); }
cmd(set_text_attribute)	{ game.color_fg = p0; game.color_bg = p1; }
cmd(shake_screen)	{ shake_screen (p0); }
cmd(word_to_string)	{ strcpy (game.strings[p0], game.ego_words[p1].word); }
cmd(status_line_on)	{ game.status_line = TRUE; write_status (); }
cmd(status_line_off)	{ game.status_line = FALSE; write_status (); }
cmd(open_dialogue)	{ _D ("p0 = %d", p0); game.has_window = TRUE; }
cmd(close_dialogue)	{ _D ("p0 = %d", p0); game.has_window = FALSE; }
cmd(close_window)	{ close_window (); }
cmd(print)		{ print (cur_logic->texts[p0 - 1], 0, 0, 0); }
cmd(print_f)		{ print (cur_logic->texts[_v[p0] - 1], 0, 0, 0); }
cmd(print_at)		{ print (cur_logic->texts[p0 - 1], p1, p2, p3); }
cmd(print_at_v)		{ print (cur_logic->texts[_v[p0] - 1], p1, p2, p3); }
cmd(show_obj)		{ show_obj (p0); }
cmd(show_obj_v)		{ show_obj (_v[p0]); }
cmd(sound)		{ start_sound (p0, p1); }
cmd(stop_sound)		{ stop_sound (); }
cmd(accept_input)	{ new_input_mode (INPUT_NORMAL); }
cmd(prevent_input)	{ new_input_mode (INPUT_NONE); }
cmd(menu_input)		{ new_input_mode (INPUT_MENU); }
cmd(enable_item)	{ menu_set_item (p0, TRUE); }
cmd(disable_item)	{ menu_set_item (p0, FALSE); }
cmd(submit_menu)	{ submit_menu (); }
cmd(set_scan_start)	{ cur_logic->sIP = cur_logic->cIP; }
cmd(reset_scan_start)	{ cur_logic->sIP = 2; }
cmd(save_game)		{ savegame_dialog (); }
cmd(load_game)		{ loadgame_dialog (); }
cmd(init_disk)		{ /* do nothing */ }
cmd(log)		{ /* do nothing */ }
cmd(trace_on)		{ /* do nothing */ }
cmd(trace_info)		{ /* do nothing */ }
cmd(show_mem)		{ message_box ("Enough memory"); }
cmd(toggle_monitor)	{ report ("Not implemented: toggle.monitor\n"); }
cmd(init_joy)		{ report ("Not implemented: init.joystick\n"); }
cmd(script_size)	{ report ("Not implemented: script.size(%d)\n", p0); }
cmd(echo_line)		{ report ("Not implemented: echo.line\n"); }
cmd(cancel_line)	{ report ("Not implemented: cancel.line\n"); }
cmd(obj_status_f)	{ report ("Not implemented: obj.status.v\n"); }

/* unknown commands:
 * unk_170: Force savegame name -- j5
 * unk_171: script save -- j5
 * unk_172: script restore -- j5
 * unk_173: Activate keypressed control (ego only moves while key is pressed)
 * unk_174: Change priority table (used in KQ4) -- j5
 * unk_177: Disable menus completely -- j5
 * unk_181: Deactivate keypressed control (default control of ego)
 */
cmd(set_simple)		{ report ("Not implemented: set.simple\n"); }
cmd(push_script)	{ report ("Not implemented: push.script\n"); }
cmd(pop_script)		{ report ("Not implemented: pop.script\n"); }
cmd(hold_key)		{ report ("Not implemented: hold.key\n"); }
cmd(set_pri_base)	{ report ("Not implemented: set.pri.base\n"); }
cmd(discard_sound)	{ report ("Not implemented: discard.sound\n"); }
cmd(hide_mouse)		{ report ("Not implemented: hide.mouse\n"); }
cmd(allow_menu)		{ report ("Not implemented: allow.menu\n"); }
cmd(show_mouse)		{ report ("Not implemented: show.mouse\n"); }
cmd(fence_mouse)	{ report ("Not implemented: fence.mouse\n"); }
cmd(mouse_posn)		{ report ("Not implemented: mouse.posn\n"); }
cmd(release_key)	{ report ("Not implemented: release.key\n"); }
cmd(adj_ego_move_to_xy)	{ report ("Not implemented: adj.ego.move.to.xy\n"); }



cmd(call) {
	int old_cIP;
	int old_lognum;

	/* CM: we don't save sIP because set.scan.start can be
	 *     used in a called script (fixes xmas demo)
	 */
	old_cIP = cur_logic->cIP;
	old_lognum = game.lognum;

	run_logic (p0);

	game.lognum = old_lognum;
	cur_logic = &game.logics[game.lognum];
	cur_logic->cIP = old_cIP;
}

cmd(call_f) {
	cmd_call (&_v[p0]);
}

cmd(draw_pic) {
	_D (_D_WARN "=== draw pic %d ===", _v[p0]);
	erase_both ();
	decode_picture (_v[p0], TRUE);
	blit_both ();
	game.picture_shown = 0;
	_D (_D_WARN "--- end of draw pic %d ---", _v[p0]);
}

cmd(show_pic) {
	_D (_D_WARN "=== show pic ===");
	setflag (F_output_mode, FALSE);
	cmd_close_window (NULL);
	show_pic ();
	game.picture_shown = 1;
	_D (_D_WARN "--- end of show pic ---");
}

cmd(load_pic) {
	erase_both ();
	agi_load_resource (rPICTURE, _v[p0]);
	blit_both ();
}

cmd(discard_pic) {
	_D (_D_WARN "--- discard pic ---");
	/* do nothing */
}

cmd(overlay_pic) {
	_D (_D_WARN "--- overlay pic ---");
	erase_both ();
	decode_picture (_v[p0], FALSE);
	blit_both ();
	game.picture_shown = 0;
	commit_both ();
}

cmd(show_pri_screen) {
#ifdef USE_CONSOLE
	debug.priority = 1;
	erase_both();
	show_pic ();
	blit_both ();
	wait_key ();
	debug.priority = 0;
	erase_both();
	show_pic ();
	blit_both ();
#endif
}

cmd(animate_obj) {
	if (~vt.flags & ANIMATED) {
		_D (_D_WARN "animate vt entry #%d", p0);
		vt.flags = ANIMATED | UPDATE | CYCLING;
		vt.motion = MOTION_NORMAL;
		vt.cycle = CYCLE_NORMAL;
		vt.direction = 0;
	}
}

cmd(unanimate_all) {
	int i;
	for (i = 0; i < MAX_VIEWTABLE; i++)
		game.view_table[i].flags &= ~(ANIMATED | DRAWN);
}

cmd(draw) {
	if (~vt.flags & DRAWN) {
		vt.flags |= UPDATE;
		fix_position (p0);
		vt.x_pos2 = vt.x_pos;
		vt.y_pos2 = vt.y_pos;
		vt.cel_data_2 = vt.cel_data;
		erase_upd_sprites ();
		vt.flags |= DRAWN;
		blit_upd_sprites ();
		vt.flags &= ~DONTUPDATE;

		commit_block (vt.x_pos, vt.y_pos - vt.y_size + 1,
			vt.x_pos + vt.x_size - 1, vt.y_pos);

		_D ("vt entry #%d flags = %02x", p0, vt.flags);
	}
}

cmd(erase) {
	if (vt.flags & DRAWN) {
		erase_upd_sprites ();
		if (vt.flags & UPDATE) {
			vt.flags &= ~DRAWN;
		} else {
			erase_nonupd_sprites ();
			vt.flags &= ~DRAWN;
			blit_nonupd_sprites ();
		}
		blit_upd_sprites ();

		commit_block (vt.x_pos, vt.y_pos - vt.y_size + 1,
			vt.x_pos + vt.x_size - 1, vt.y_pos);
	}
}

cmd(position) {
	vt.x_pos = vt.x_pos2 = p1;
	vt.y_pos = vt.y_pos2 = p2;
}

cmd(position_f) {
	vt.x_pos = vt.x_pos2 = _v[p1];
	vt.y_pos = vt.y_pos2 = _v[p2];
}

cmd(get_posn) {
	game.vars[p1] = vt.x_pos;
	game.vars[p2] = vt.y_pos;
}

cmd(reposition) {
	int dx = (SINT8)_v[p1], dy = (SINT8)_v[p2];

	_D ("dx=%d, dy=%d", dx, dy);
	vt.flags |= UPDATE_POS;

	if (dx < 0 && vt.x_pos < dx)
		vt.x_pos = 0;
	else
		vt.x_pos += dx;

	if (dy < 0 && vt.y_pos < dy)
		vt.y_pos = 0;
	else
		vt.y_pos += dy;

	fix_position (p0);
}

cmd(reposition_to) {
	vt.x_pos = p1;
	vt.y_pos = p2;
	vt.flags |= UPDATE_POS;
	fix_position (p0);
}

cmd(reposition_to_f) {
	vt.x_pos = _v[p1];
	vt.y_pos = _v[p2];
	vt.flags |= UPDATE_POS;
	fix_position (p0);
}

cmd(add_to_pic) {
	add_to_pic (p0, p1, p2, p3, p4, p5, p6);
}

cmd(add_to_pic_f) {
	add_to_pic (_v[p0], _v[p1], _v[p2], _v[p3], _v[p4], _v[p5], _v[p6]);
}

cmd(force_update) {
	erase_both ();
	blit_both ();
	commit_both ();
}

cmd(reverse_loop) {
	vt.cycle = CYCLE_REV_LOOP;
	vt.flags |= (DONTUPDATE|UPDATE|CYCLING);
	vt.parm1 = p1;
	setflag (p1, FALSE);
}

cmd(end_of_loop) {
	_D (_D_WARN "p0 = %d", p0);
	vt.flags |= (DONTUPDATE|UPDATE|CYCLING);
	vt.cycle = CYCLE_END_OF_LOOP; 
	vt.parm1 = p1;
}

cmd(block) {
	_D (_D_WARN "x1=%d, y1=%d, x2=%d, y2=%d", p0, p1, p2, p3);
	game.block.active = TRUE;
	game.block.x1 = p0;
	game.block.y1 = p1;
	game.block.x2 = p2;
	game.block.y2 = p4;
}

cmd(unblock) {
	game.block.active = FALSE;
}

cmd(normal_motion) {
	vt.motion = MOTION_NORMAL;
}

cmd(stop_motion) {
	vt.direction = 0;
	vt.motion = MOTION_NORMAL;
	if (p0 == 0) {		/* ego only */
		_v[V_ego_dir] = 0;
		game.player_control = FALSE;
	}
}

cmd(start_motion) {
	vt.motion = MOTION_NORMAL;
	if (p0 == 0) {		/* ego only */
		_v[V_ego_dir] = 0;
		game.player_control = TRUE;
	}
}

cmd(player_control) {
	game.player_control = TRUE;
	game.view_table[0].motion = MOTION_NORMAL;
}

cmd(program_control) {
	game.player_control = FALSE;
}

cmd(follow_ego) {
	vt.motion = MOTION_FOLLOW_EGO;
	vt.parm1 = p1 > vt.step_size ? p1 : vt.step_size;
	vt.parm2 = p2;
	vt.parm3 = 0xff;
	setflag (p2, FALSE);
	vt.flags |= UPDATE;
}

cmd(move_obj) {
	_D (_D_WARN "o=%d, x=%d, y=%d, s=%d, f=%d", p0, p1, p2, p3, p4);

	vt.motion = MOTION_MOVE_OBJ;
	vt.parm1 = p1;
	vt.parm2 = p2;
	vt.parm3 = vt.step_size;
	vt.parm4 = p4;

	if (p3 != 0)
		vt.step_size = p3;
	
	setflag (p4, FALSE);
	vt.flags |= UPDATE;

	if (p0 == 0)
		game.player_control = FALSE;

	move_obj (&vt);
}

cmd(move_obj_f) {
	vt.motion = MOTION_MOVE_OBJ;
	vt.parm1 = _v[p1];
	vt.parm2 = _v[p2];
	vt.parm3 = vt.step_size;
	vt.parm4 = p4;

	if (_v[p3] != 0)
		vt.step_size = _v[p3];
	
	setflag (p4, FALSE);
	vt.flags |= UPDATE;

	if (p0 == 0)
		game.player_control = FALSE;

	move_obj (&vt);
}
	
cmd(wander) {
	if (p0 == 0)
		game.player_control = FALSE;
	vt.motion = MOTION_WANDER;
	vt.flags |= UPDATE;
}


cmd(set_game_id) {
	if (cur_logic->texts && (p0 - 1) <= cur_logic->num_texts)
		strncpy (game.id, cur_logic->texts[p0 - 1], 8);
	else
		game.id[0] = 0;

	report ("Game ID: \"%s\"\n", game.id);
}

cmd(pause) {
	int tmp = game.clock_enabled;
	game.clock_enabled = FALSE;
	message_box ("    Game is Paused.\nPress ENTER to continue.");
	game.clock_enabled = tmp;
}

cmd(set_menu) {
	_D ("text %02x of %02x", p0, cur_logic->num_texts);
	if (cur_logic->texts != NULL && p0 < cur_logic->num_texts)
		add_menu (cur_logic->texts[p0 - 1]);
}

cmd(set_menu_item) {
	_D ("text %02x of %02x", p0, cur_logic->num_texts);
	if (cur_logic->texts != NULL && p0 < cur_logic->num_texts)
		add_menu_item (cur_logic->texts[p0 - 1], p1);
}

cmd(version) {
	char ver_msg[] = TITLE " v" VERSION;
	char ver2_msg[] =
		"\n"
		"                             \n\n"
        	"Emulating Sierra AGI v%x.%03x\n";
	char ver3_msg[]=
    		"\n"
       		"                             \n\n"
       		"  Emulating AGI v%x.002.%03x\n";
		/* no Sierra as it wraps textbox */
	char *r, *q;
	int ver, maj, min;
	char msg[256];

	ver = agi_get_release ();
	maj = (ver >> 12) & 0xf;
	min = ver & 0xfff;

	q = maj == 2 ? ver2_msg : ver3_msg;
	r = strchr (q+1, '\n');

	strncpy (q+1 + ((r-q > 0 ? r - q : 1) / 4), ver_msg, strlen (ver_msg));
	sprintf (msg, q, maj, min);
	message_box (msg);
}

cmd(configure_screen) {
	game.line_min_print = p0;
	game.line_user_input = p1;
	game.line_status = p2;
}

cmd(text_screen) {
	game.gfx_mode = FALSE;
	/*
	 * Simulates the "bright background bit" of the PC video
	 * controller.
	 */
	if (game.color_bg)
		game.color_bg |= 0x08;
	clear_screen (game.color_bg);
}

cmd(graphics) {
	if (!game.gfx_mode) {
		game.gfx_mode = TRUE;
		clear_screen (0);
		show_pic ();
 		write_status ();
	}
}

cmd(status) {
	inventory();
}

cmd(quit) {
	if (p0) {
		game.quit_prog_now = TRUE;
	} else {
		switch (message_box ("   Press ENTER to quit.\n"
			"Press ESC to keep playing.")) {
		case 'Y':
		case 'y':
		case 0x0d:
		case 0x0a:
			game.quit_prog_now = TRUE;
			break;
		}
	}
}

cmd(restart_game) {
	switch (message_box ("Press ENTER to restart the game.\n"
		"Press ESC to continue this game.")) {
	case 0x0a:
	case 0x0d:
		game.quit_prog_now = 0xff;
		setflag (F_restart_game, TRUE);
		break;
	default:
		break;
	}
}

cmd(distance) {
	SINT16 x1, y1, x2, y2, d;
	struct vt_entry *v0 = &game.view_table[p0];
	struct vt_entry *v1 = &game.view_table[p1];

	if (v0->flags & DRAWN && v1->flags & DRAWN) {
		x1 = v0->x_pos + v0->x_size / 2;
		y1 = v0->y_pos;
		x2 = v1->x_pos + v1->x_size / 2;
		y2 = v1->y_pos;
		d = abs (x1 - x2) + abs (y1 - y2);
		if (d > 0xfe) d = 0xfe;
	} else {
		d = 0xff;
	}
	_v[p2] = d;
}

cmd(get_string) {
	_D ("%d %d %d %d %d", p0, p1, p2, p3, p4);
	new_input_mode (INPUT_GETSTRING);

	if (cur_logic->texts != NULL && cur_logic->num_texts >= (p1 - 1)) {
		int len = strlen (cur_logic->texts[p1 - 1]);
		print_text (cur_logic->texts[p1 - 1], 0, p3,
			p2, len, game.color_fg, game.color_bg);
		get_string (p3 + len - 1, p2, p4, p0);

		/* SGEO : display input char */
		print_character ((p3 + len ), p2, game.cursor_char, game.color_fg, game.color_bg);
	}

	do {
		main_cycle ();
	} while (game.input_mode == INPUT_GETSTRING);
}


/* FIXME: remove lowlevel print_text() call from here */
cmd(get_num) {
#if 0
	game.input_mode = INPUT_GETSTRING;

	if (cur_logic->texts != NULL && (p0 - 1) <= cur_logic->num_texts) {
		char *prompt = agi_printf (cur_logic->texts[p0 - 1]);
		print_text (prompt, 0, 0, 23 * CHAR_LINES, strlen (prompt) + 1,
			game.color_fg, game.color_bg);
		p = get_string (strlen (prompt) - 1) * CHAR_COLS,
			23 * CHAR_LINES, 4);
		_v[p1] = atoi (prompt);
	}
#endif
}

cmd(set_cursor_char)
{
	if (cur_logic->texts != NULL && (p0 - 1) <= cur_logic->num_texts) {
		game.cursor_char = *cur_logic->texts[p0 - 1];
	} else {
		/* default */
		game.cursor_char = '_';
	}
}

cmd(set_key) {
	_D ("%d %d %d", p0, p1, p2);
	if (p0 && p1) {
		_D (_D_WARN "FIXME: not registered!");
		return;
	}
	if (p0) {		/* keypress */
		game.ev_keyp[p2].data = p0;
		game.ev_keyp[p2].occured = FALSE;
	} else {		/* scancode */
		game.ev_scan[p2].data = p1;
		game.ev_scan[p2].occured = FALSE;
	}
}

cmd(set_string) {
	/* CM: to avoid crash in Groza (str = 150) */
	if (p0 > MAX_WORDS1) return;
	strcpy (game.strings[p0], cur_logic->texts[p1 - 1]);
}

cmd(display) {
	print_text (cur_logic->texts[p2 - 1], p1, 0, p0, 40,
		game.color_fg, game.color_bg);
}

cmd(display_f) {
	_D ("p0 = %d", p0);
	print_text (cur_logic->texts[_v[p2] - 1], _v[p1], 0, _v[p0], 40,
		game.color_fg, game.color_bg);
}

cmd(clear_text_rect) {
	int c, x1, y1, x2, y2;

	if ((c = p4) != 0) c = 15;
	x1 = p1 * CHAR_COLS;
	y1 = p0 * CHAR_LINES;
	x2 = (p3 + 1) * CHAR_COLS - 1;
	y2 = (p2 + 1) * CHAR_LINES - 1;

	/* Added to prevent crash with x2 = 40 in the iigs demo */
	if (x1 > GFX_WIDTH) x1 = GFX_WIDTH - 1; 
	if (x2 > GFX_WIDTH) x2 = GFX_WIDTH - 1; 
	if (y1 > GFX_HEIGHT) y1 = GFX_HEIGHT - 1;
	if (y2 > GFX_HEIGHT) y2 = GFX_HEIGHT - 1;

	draw_rectangle (x1, y1, x2, y2, c);
	flush_block (x1, y1, x2, y2);
}

/* FIXME: should call function in gfx_agi.h */
cmd(clear_lines) {
	clear_lines (p0, p1, p2);
	flush_lines (p0, p1);
}


static void (*agi_command[183])(UINT8 *) = {
	NULL,				/* 0x00 */
	cmd_increment,
	cmd_decrement,
	cmd_assignn,
	cmd_assignv,
	cmd_addn,
	cmd_addv,
	cmd_subn,
	cmd_subv,			/* 0x08 */
	cmd_lindirectv,
	cmd_rindirect,
	cmd_lindirectn,
	cmd_set,
	cmd_reset,
	cmd_toggle,
	cmd_set_v,
	cmd_reset_v,			/* 0x10 */
	cmd_toggle_v,
	cmd_new_room,
	cmd_new_room_f,
	cmd_load_logic,
	cmd_load_logic_f,
	cmd_call,
	cmd_call_f,
	cmd_load_pic,			/* 0x18 */
	cmd_draw_pic,
	cmd_show_pic,
	cmd_discard_pic,
	cmd_overlay_pic,
	cmd_show_pri_screen,
	cmd_load_view,
	cmd_load_view_f,
	cmd_discard_view,		/* 0x20 */
	cmd_animate_obj,
	cmd_unanimate_all,
	cmd_draw,
	cmd_erase,
	cmd_position,
	cmd_position_f,
	cmd_get_posn,
	cmd_reposition,			/* 0x28 */
	cmd_set_view,
	cmd_set_view_f,
	cmd_set_loop,
	cmd_set_loop_f,
	cmd_fix_loop,
	cmd_release_loop,
	cmd_set_cel,
	cmd_set_cel_f,			/* 0x30 */
	cmd_last_cel,
	cmd_current_cel,
	cmd_current_loop,
	cmd_current_view,
	cmd_number_of_loops,
	cmd_set_priority,
	cmd_set_priority_f,
	cmd_release_priority,		/* 0x38 */
	cmd_get_priority,
	cmd_stop_update,
	cmd_start_update,
	cmd_force_update,
	cmd_ignore_horizon,
	cmd_observe_horizon,
	cmd_set_horizon,
	cmd_object_on_water,		/* 0x40 */
	cmd_object_on_land,
	cmd_object_on_anything,
	cmd_ignore_objs,
	cmd_observe_objs,
	cmd_distance,
	cmd_stop_cycling,
	cmd_start_cycling,
	cmd_normal_cycle,		/* 0x48 */
	cmd_end_of_loop,
	cmd_reverse_cycle,
	cmd_reverse_loop,
	cmd_cycle_time,
	cmd_stop_motion,
	cmd_start_motion,
	cmd_step_size,
	cmd_step_time,			/* 0x50 */
	cmd_move_obj,
	cmd_move_obj_f,
	cmd_follow_ego,
	cmd_wander,
	cmd_normal_motion,
	cmd_set_dir,
	cmd_get_dir,
	cmd_ignore_blocks,		/* 0x58 */
	cmd_observe_blocks,
	cmd_block,
	cmd_unblock,
	cmd_get,
	cmd_get_f,
	cmd_drop,
	cmd_put,
	cmd_put_f,			/* 0x60 */
	cmd_get_room_f,
	cmd_load_sound,
	cmd_sound,
	cmd_stop_sound,
	cmd_print,
	cmd_print_f,
	cmd_display,
	cmd_display_f,			/* 0x68 */
	cmd_clear_lines,
	cmd_text_screen,
	cmd_graphics,
	cmd_set_cursor_char,
	cmd_set_text_attribute,
	cmd_shake_screen,
	cmd_configure_screen,
	cmd_status_line_on,		/* 0x70 */
	cmd_status_line_off,
	cmd_set_string,
	cmd_get_string,
	cmd_word_to_string,
	cmd_parse,
	cmd_get_num,
	cmd_prevent_input,
	cmd_accept_input,		/* 0x78 */
	cmd_set_key,
	cmd_add_to_pic,
	cmd_add_to_pic_f,
	cmd_status,
	cmd_save_game,
	cmd_load_game,
	cmd_init_disk,
	cmd_restart_game,		/* 0x80 */
	cmd_show_obj,
	cmd_random,
	cmd_program_control,
	cmd_player_control,
	cmd_obj_status_f,
	cmd_quit,
	cmd_show_mem,
	cmd_pause,			/* 0x88 */
	cmd_echo_line,
	cmd_cancel_line,
	cmd_init_joy,
	cmd_toggle_monitor,
	cmd_version,
	cmd_script_size,
	cmd_set_game_id,
	cmd_log,			/* 0x90 */
	cmd_set_scan_start,
	cmd_reset_scan_start,
	cmd_reposition_to,
	cmd_reposition_to_f,
	cmd_trace_on,
	cmd_trace_info,
	cmd_print_at,
	cmd_print_at_v,			/* 0x98 */
	cmd_discard_view,
	cmd_clear_text_rect,
	cmd_set_upper_left,
	cmd_set_menu,
	cmd_set_menu_item,
	cmd_submit_menu,
	cmd_enable_item,
	cmd_disable_item,		/* 0xa0 */
	cmd_menu_input,	
	cmd_show_obj_v,
	cmd_open_dialogue,
	cmd_close_dialogue,
	cmd_mul_n,
	cmd_mul_v,
	cmd_div_n,
	cmd_div_v,			/* 0xa8 */
	cmd_close_window,
	cmd_set_simple,
	cmd_push_script,
	cmd_pop_script,
	cmd_hold_key,
	cmd_set_pri_base,
	cmd_discard_sound,
	cmd_hide_mouse,			/* 0xb0 */
	cmd_allow_menu,	
	cmd_show_mouse,
	cmd_fence_mouse,
	cmd_mouse_posn,
	cmd_release_key,
	cmd_adj_ego_move_to_xy
};


#define CMD_BSIZE 12

int run_logic (int n)
{
	UINT8 op, p[CMD_BSIZE];
	UINT8 *code = NULL;
	int num;

	/* If logic not loaded, load it */
	if (~game.dir_logic[n].flags & RES_LOADED) {
		_D (_D_WARN "logic %d not loaded!", n);
		agi_load_resource (rLOGIC, n);
	}

	game.lognum = n;
	cur_logic = &game.logics[game.lognum];

	code = cur_logic->data;
	cur_logic->cIP = cur_logic->sIP;

	while (ip < game.logics[n].size && !game.quit_prog_now) {
#ifdef USE_CONSOLE
		if (debug.enabled) {
			if (debug.steps > 0) {
				if (debug.logic0 || n) {
					debug_console (n,
						lCOMMAND_MODE, NULL);
					debug.steps--;
				}
			} else {
				blit_both ();
				console_prompt ();
				do {
					main_cycle ();
				} while (!debug.steps && debug.enabled);
				console_lock ();
				erase_both ();
			}
		}
#endif

		switch (op = *(code + ip++)) {
		case 0xff:			/* if (open/close) */
			test_if_code (n);
			break;
		case 0xfe:			/* goto */
			ip += 2 + ((SINT16)lohi_getword (code + ip));
			/* +2 covers goto size */
			break;
		case 0x00:			/* return */
			return 1;
		default:
			num = logic_names_cmd[op].num_args;
			memmove (p, code + ip, num);
			memset (p + num, 0, CMD_BSIZE - num);
			agi_command[op](p);
			ip += num;
		}

		if (game.exit_all_logics)
			break;
	}

	return 0;	/* after executing new.room() */
}


void execute_agi_command (UINT8 op, UINT8 *p)
{
	agi_command[op](p);
}


