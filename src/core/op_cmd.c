/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

/*
 * Dark Minister provided some info:
 * variable v24 == max chars you can enter on the command line
 *
 * opcode 173 + 181
 * unknown173 ()
 * --- Activate keypressed control (ego only moves when a key is pressed)
 * unknown181 ()
 * --- Desactivate keypressed control (default control of ego)
 *
 * commands to FINISH o_O;;
 *
 * add to pic (view.cc)
 * draw
 *
 * log
 * script size
 * echo line
 * cancel line
 * menu input
 * save
 * load
 * init disk
 * trace on
 * trace info
 * close dialogue
 * open dialogue
 * obj status v
 * set upper left
 * block
 * unblock
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#include "sarien.h"
#include "agi.h"
#include "rand.h"
#include "gfx_agi.h"
#include "gfx_base.h"		/* FIXME: hmm.. */
#include "keyboard.h"
#include "opcodes.h"
#include "picture.h"
#include "view.h"
#include "logic.h"
#include "sound.h"
#include "menu.h"
#include "savegame.h"
#include "console.h"
#include "text.h"	/* remove later */

/* shw msgs boxes on commands 171+ */
/*#define DISPLAY_DUDCODE*/


#define ip	logics[lognum].cIP
#define code	logics[lognum].data
#define vt	view_table[entry]

int open_dialogue = 0;		/* fix this properly too */
static int window_nonblocking = 0;	/* Yuck! Remove it later! */

extern struct agi_logic logics[];
extern struct agi_view views[];
extern struct agi_view_table view_table[];

extern UINT32 msg_box_ticks;

void cmd_position (UINT8 entry, UINT8 x, UINT8 y)
{
	vt.x_pos = x;
	vt.y_pos = y;
}


void cmd_set_loop (UINT8 entry, UINT8 loop)
{
	set_loop (entry, loop);
}


void cmd_set_view (UINT8 entry, UINT8 view)
{
	add_view_table (entry, view);
}


void cmd_call (UINT8 log)
{
	UINT16 oip;
	UINT16 osp;

#ifndef NO_DEBUG
	if (opt.debug == 4)
		opt.debug = TRUE;
#endif
	oip = logics[log].cIP;
	osp = logics[log].sIP;
	run_logic (log);
	logics[log].cIP = oip;
	logics[log].sIP = osp;
}


void cmd_load_view (UINT8 entry)
{
	_D("(view = %d)", entry);
	agi_load_resource (rVIEW, entry);

	if(entry == 0 && game.control_mode == CONTROL_PLAYER) {
		vt.direction = 0;
		setvar (V_ego_dir, 0);
	}
}


void cmd_load_logic (UINT8 log)
{
	_D("(logic = %d", log);
	agi_load_resource (rLOGIC, log);
#ifdef DISABLE_COPYPROTECTION
	break_copy_protection (log);
#endif
}


void cmd_new_room (UINT8 room)
{
	_D (_D_WARN "(room = %d)", room);
	game.new_room_num = room;
	game.ego_in_new_room = TRUE;
	clear_buffer ();
	window_nonblocking = 0;	/***************************/
	game.exit_all_logics = TRUE;
}


void cmd_assign (UINT8 var, UINT8 val)
{
	setvar (var, val);
}


void cmd_add_to_pic (UINT8 view, UINT8 loop, UINT8 cel, UINT8 x, UINT8 y, UINT8 pri, UINT8 mar)
{
	add_to_pic (view, loop, cel, x, y, pri, mar);
}


void cmd_add (UINT8 var, UINT8 val)
{
	setvar (var, getvar (var)+val);
}


void cmd_sub (UINT8 var, UINT8 val)
{
	setvar (var, getvar (var) - val);
}


void cmd_lindirect (UINT8 var, UINT8 val)
{
	setvar (getvar (var), val);
}


void cmd_rindirect (UINT8 var, UINT8 val)
{
	setvar (var, getvar (val));
}


void cmd_set (UINT8 flag)
{
	setflag (flag, TRUE);
}


void cmd_reset (UINT8 flag)
{
	setflag (flag, FALSE);
}


void cmd_toggle (UINT8 flag)
{
	setflag (flag, !getflag (flag));
}


void cmd_inc (UINT8 var)
{
	if (getvar (var) != 0xFF)
		setvar (var, getvar (var) + 1);
}


void cmd_dec (UINT8 var)
{
	if (getvar (var) != 0)
		setvar (var, getvar (var) - 1);
}


void cmd_force_update (UINT8 entry)
{
	/* update obj (entry); */
	report ("Hack: force.update (%d)\n", entry);

	redraw_sprites ();
	erase_sprites ();
	set_cel (entry, vt.current_cel);
	draw_sprites ();
	release_sprites ();

}


void cmd_draw (UINT8 entry)
{
	_D ("(%d)", entry);
	
	redraw_sprites ();	/* KQ4 draws behind non-updating views */
	erase_sprites ();
	vt.flags |= DRAWN | UPDATE;
	set_cel (entry, vt.current_cel);
	draw_sprites ();
	release_sprites ();
}


void cmd_erase (UINT8 entry)
{
	_D ("(%d)", entry);
	redraw_sprites ();
	erase_sprites ();
	vt.flags &= ~(DRAWN | UPDATE);
	draw_sprites ();
	release_sprites ();

   	if (entry == EGO_VIEW_TABLE) {
   		vt.direction = 0;
   		setvar (V_ego_dir, 0);
	}
}


void cmd_reposition (UINT8 entry, UINT8 x, UINT8 y)
{
	vt.x_pos += (SINT8)getvar (x);
	vt.y_pos += (SINT8)getvar (y);
}


void cmd_fix_loop (UINT8 entry)
{
	vt.flags |= FIX_LOOP;
}


void cmd_object_on_any (UINT8 entry)
{
	vt.flags &= ~ (ON_WATER | ON_LAND);
}


void cmd_object_on_land (UINT8 entry)
{
	vt.flags |= ON_LAND;
}


void cmd_object_on_water (UINT8 entry)
{
	vt.flags |= ON_WATER;
}


void cmd_set_horizon (UINT8 h)
{
	game.horizon = h;
}


void cmd_ignore_horizon (UINT8 entry)
{
	vt.flags |= IGNORE_HORIZON;
}


void cmd_observe_horizon (UINT8 entry)
{
	vt.flags &= ~IGNORE_HORIZON;
}


void cmd_start_update (UINT8 entry)
{
	if (vt.flags & UPDATE)
		return;

	if (~vt.flags & DRAWN) {
		vt.flags |= UPDATE;
		return;
	}

	redraw_sprites ();
	vt.flags |= UPDATE;
	release_sprites ();
}


void cmd_stop_update (UINT8 entry)
{
	if (~vt.flags & UPDATE)
		return;

	if (~vt.flags & DRAWN) {
		vt.flags &= ~UPDATE;
		return;
	}

	redraw_sprites ();
	vt.flags &= ~UPDATE;
	release_sprites ();
}


void cmd_get_priority (UINT8 entry, UINT8 v)
{
	setvar (v, vt.priority);
}


void cmd_release_priority (UINT8 entry)
{
	vt.flags &= ~FIXED_PRIORITY;
}


void cmd_set_priority (UINT8 entry, UINT8 p)
{
	_D ("(%d, %d)", entry, p);

	vt.flags |= FIXED_PRIORITY;

	/* CM: can't set priority smaller than 4 */
	if (p < 4)
		p = 4;
	vt.priority = p;
}


void cmd_set_num_loops (UINT8 entry, UINT8 l)
{
	/*vt.num_loops=l;*/
	/* setvar (l, vt.num_loops); ******/
	setvar (l, VT_VIEW(vt).num_loops);
}


void cmd_cur_view (UINT8 entry, UINT8 v)
{
	setvar (v, vt.current_view);
}


void cmd_cur_loop (UINT8 entry, UINT8 v)
{
	setvar (v, vt.current_loop);
}


void cmd_cur_cel (UINT8 entry, UINT8 v)
{
	setvar (v, vt.current_cel);
}


void cmd_last_cel (UINT8 entry, UINT8 v)
{
	setvar (v, VT_LOOP(vt).num_cels > 0 ? VT_LOOP(vt).num_cels - 1 : 0);
}


void cmd_set_cel (UINT8 entry, UINT8 c)
{
	set_cel (entry, c);
}


void cmd_release_loop (UINT8 entry)
{
	vt.flags &= ~FIX_LOOP;
}


void cmd_reverse_loop (UINT8 entry, UINT8 p1)
{
	_D ("(%d, %d)", entry, p1);
	vt.parm1        = p1;
	vt.cycle_status = CYCLE_REV_LOOP;
/*	
   vt.end_of_loop_flag = TRUE; 
*/
	vt.flags |= UPDATE | CYCLING;
}


void cmd_reverse_cycle (UINT8 entry)
{
	vt.cycle_status = CYCLE_REV;
	vt.flags |= UPDATE | CYCLING;
}


void cmd_end_of_loop (UINT8 entry, UINT8 p1)
{
	_D ("(%d, %d)", entry, p1);
	vt.parm1        = p1;
	vt.cycle_status = CYCLE_END_OF_LOOP; 
/*
	vt.end_of_loop_flag = TRUE;
*/
	vt.flags |= UPDATE | CYCLING;
}


void cmd_distance (UINT8 ob1, UINT8 ob2, UINT8 v)
{
	UINT8	x1, y1, x2, y2;

	/* if ob1 & ob2 on screen, else v=255 */
	if ((view_table[ob1].flags&DRAWN)==DRAWN && (view_table[ob2].flags&DRAWN)==DRAWN)
	{
		x1=view_table[ob1].x_pos;
		y1=view_table[ob1].y_pos;
		x2=view_table[ob2].x_pos;
		y2=view_table[ob2].y_pos;
		setvar (v,  abs (x1-x2)+abs (y1-y2));
	}
	else
		setvar (v, 0xFF);
}


void cmd_observe_objs (UINT8 entry)
{
	vt.flags &= ~IGNORE_OBJECTS;
}


void cmd_ignore_objs (UINT8 entry)
{
	vt.flags |= IGNORE_OBJECTS;
}


void cmd_word_to_string (UINT8 sn, UINT8 wn)
{
	strcpy (game.strings[sn], game.ego_words[wn].word);
}


/* FIXME: remove lowlevel print_text() call from here */
void cmd_get_num (UINT8 logic, UINT8 msg, UINT8 num)
{
	UINT8	*p=NULL;

	if (logics[logic].texts!=NULL && (msg-1)<=logics[logic].num_texts) {
		p = agi_printf (logics[logic].texts[msg-1], logic);
		print_text (p, 0, 0, 23*8, strlen ((char*)p) + 1, txt_fg, txt_bg);
		p = get_string (8* (strlen ((char*)p)-1), 23*8, 4);
		while (*p!=0x0 && isspace (*p)!=0)
			p++;
		setvar (num, (UINT8)atoi ((char*)p));
	}
}


void cmd_parse (UINT8 logic, UINT8 str)
{
	UINT8	*p;

	_D ("(%d, %d)\n", logic, str);
	p = agi_printf (game.strings[str], logic);
	dictionary_words (p);
}


void cmd_cycle_time (UINT8 entry, UINT8 v)
{
	vt.cycle_time = getvar (v);
}


void cmd_block (UINT8 x1, UINT8 y1, UINT8 x2, UINT8 y2)
{
	report ("Not implemented: block (%d,%d,%d,%d)\n", x1, y1, x2, y2);
}


void cmd_unblock ()
{
	report ("Not implemented: unblock ()\n");
}


void cmd_follow_ego (UINT8 entry, UINT8 sv, UINT8 f)
{
	_D ("(%d, %d, %d)", entry, sv, f);
	vt.step_size = sv;
	vt.parm1 = vt.step_size;
	vt.parm2 = f;
	vt.motion = MOTION_FOLLOW_EGO;
	vt.flags |= MOTION;
}


void cmd_wander ( UINT8 entry)
{
	vt.motion = MOTION_WANDER;
	vt.flags |= MOTION;
}


void cmd_norm_motion (UINT8 entry)
{
	vt.motion = MOTION_NORMAL;
	vt.flags |= MOTION;
}


void cmd_set_dir (UINT8 entry, UINT8 d)
{
	vt.direction = getvar (d);
	calc_direction (entry);

	/*set_cel (entry, 0);*/
}


void cmd_get_dir (UINT8 entry, UINT8 v)
{
	setvar (v, vt.direction);
}


void cmd_ignore_blocks (UINT8 entry)
{
	vt.flags |= IGNORE_BLOCKS;
}


void cmd_observe_blocks (UINT8 entry)
{
	vt.flags &= ~IGNORE_BLOCKS;
}


void cmd_move_obj (UINT8 entry, UINT8 x, UINT8 y, UINT8 step, UINT8 flag)
{
	_D ("(entry=%d, x=%d, y=%d, step=%d, flag=%d)", entry, x, y, step, flag);

	/* CM: I don't know if this is the correct behaviour, but
	 *     KQ2 demo needs this in the lion sequence. This test
	 *     won't permit move.obj() if you're already at the
	 *     destination point!
	 */
	if (vt.parm1 == x && vt.parm2 == y &&
		vt.x_pos == x && vt.y_pos == y)
	{
		_D (_D_WARN "Already at destination point!");
		setflag (flag, TRUE);
		return;
	}

	vt.parm1 = x;
	vt.parm2 = y;
	vt.parm3 = vt.step_size;
	if (step > 0)
		vt.step_size = step;
	vt.parm4 = flag;
	vt.motion = MOTION_MOVE_OBJ;

	setflag (flag, FALSE);		/* Needed for KQ2 demo!! */

	vt.flags |= MOTION;
	vt.cycle_status = CYCLE_NORMAL;

	/* FR: Guess the direction of the movement (should call adj_direction)
	 * CM: Ok, calling adj_direction
	 */
	adj_direction (entry, y - vt.y_pos, x - vt.x_pos);

	/* CM: added according to AGDS docs */
	if (!entry)
		cmd_prog_control ();
}


void cmd_get_roomv (UINT8 o, UINT8 v)
{
	setvar (v, object_get_location (o));
}


void cmd_put (UINT8 o, UINT8 v)
{
	object_set_location (o, v);
}


void cmd_drop (UINT8 o)
{
	object_set_location (o, 0);
}


void cmd_get (UINT8 o)
{
	object_set_location (o, EGO_OWNED);
}


void cmd_set_cur_char (UINT8 logic, UINT8 msg)
{
	UINT8	*p=NULL;

	if (logics[logic].texts != NULL && (msg-1) <= logics[logic].num_texts)
	{
		p=agi_printf (logics[logic].texts[msg-1], logic);
		txt_char=*p;
	}
	else
		txt_char='_';
}


void cmd_set_text_attr (UINT8 fg, UINT8 bg)
{
	_D ("(%d, %d)", fg, bg);

	txt_fg = fg;
	txt_bg = bg;
}


void cmd_shake_screen (UINT8 n)
{
	redraw_sprites ();
	shake_screen (n);
	release_sprites ();
}


void cmd_accept_input ()
{
	game.allow_kyb_input = TRUE;
}


void cmd_stop_input ()
{
	game.allow_kyb_input = FALSE;

	/* Clear the area of the user input (the +1 is for the last line)*/
	/* CM: produces white strip at bottom of larry demo! */
#if 0
	cmd_clear_lines ( line_user_input, line_user_input + 1, txt_bg );
#endif
	/* CM: Maybe this workaround could work... */
	cmd_clear_lines (game.line_user_input, game.line_user_input + 1, 0 );
}


void cmd_set_key (UINT8 ac, UINT8 sc, UINT8 ec)
{
	if (ac == 0) {
		game.events[ec].event = eSCAN_CODE;
		game.events[ec].data = sc;
		game.events[ec].occured = FALSE;
	} else {
		game.events[ec].event = eKEY_PRESS;
		game.events[ec].data = ac;
		game.events[ec].occured = FALSE;
	}
}


void cmd_get_posn (UINT8 entry, UINT8 x1, UINT8 y1)
{
	setvar (x1, vt.x_pos);
	setvar (y1, vt.y_pos);
}


void cmd_get_string (UINT8 logic, UINT8 str, UINT8 msg, UINT8 y, UINT8 x, UINT8 len)
{
	char *p;

	/* FR:
	 * Changed here
	 */
	if (logics[logic].texts!=NULL && (msg-1)<=logics[logic].num_texts) {

		/* FIXME : hack to stop input line on a getstring command */
    	open_dialogue=1;

		p=agi_printf (logics[logic].texts[msg-1], logic);
		print_text (p, 0, x * 8, y * 8, strlen (p), txt_fg, txt_bg);
		p = get_string ((x * 8) + (8 * (strlen (p) - 1)), y*8, len);
		strcpy (game.strings[str], p );

		open_dialogue=0;
	}
}


void cmd_config_screen (UINT8 mpl, UINT8 upl, UINT8 sl)
{
	game.line_status = sl;
	game.line_user_input = upl;
	game.line_min_print = mpl;
}


void cmd_clear_lines (UINT8 sl, UINT8 el, UINT8 c)
{
	UINT16	x, y, z;

	/* do we need to adjust for +8 on topline?
	 * inc for endline so it mateches the correct num
	 * ie, from 22 to 24 is 3 lines, not 2 lines.
	 */

	if (c != 0)
		c = 15;

	z = (1+el)*8;

	for (y = sl * 8; y < z; y++)
		for (x = 0; x < GFX_WIDTH; x++)
			put_pixel (x, y, c);

	put_block (0, sl * 8, GFX_WIDTH - 1, sl * 8 + z - 1);
}


void cmd_txt ()
{
	if (screen_mode != TXT_MODE)
		save_screen ();

	screen_mode = TXT_MODE;

	if (txt_bg == 7)
		txt_bg = 15;

	clear_buffer ();
	put_screen ();
}


void cmd_gfx ()
{
	if (screen_mode == TXT_MODE)
		restore_screen ();

	screen_mode = GFX_MODE;
	put_screen ();
 	update_status_line (TRUE);
}


void cmd_status ()
{
	inventory();
	setvar (25, 0xFF);
}


void cmd_status_line_on ()
{
	game.status_line = TRUE;

	/* MK: I re-inserted (un-commented) the call to update_status_line
	 * here (and in cmd_status_line_off()), because any AGI script call to
	 * get.string() (cf. the computer scene in Police Quest) would mean
	 * that the status line wouldn't be updated before the next interpreter
	 * cycle. This was also necessary as I previously removed the call to
	 * update_status_line in cmd_show_pic() :-).
	 */
	update_status_line (TRUE);
}


void cmd_status_line_off ()
{
	game.status_line = FALSE;
	update_status_line (TRUE);
}


void cmd_load_sound (UINT8 s)
{
	_D ("(sound = %d)", s);
	agi_load_resource (rSOUND, s);
}


void cmd_play_sound (UINT8 s, UINT8 f)
{
	_D ("(%d, %d)", s, f);
	start_sound (s, f);
}


void cmd_stop_sound ()
{
	_D ("()");
	stop_sound ();
}


void cmd_print (UINT8 logic, UINT8 msg)
{
	_D ("(%d, %d)", logic, msg);
	cmd_print_at (logic, msg, -1, -1, -1);
}


void cmd_print_at (UINT8 logic, UINT8 msg, SINT8 y, SINT8 x, SINT8 len)
{
	UINT8	*p;

	_D ("(%d, %d, %d, %d, %d)", logic, msg, y, x, len);
	if (logics[logic].texts==NULL || logics[logic].num_texts< (msg-1))
		return;

	if (window_nonblocking) {
		_D (_D_WARN "window_nonblocking=1 => remove window");
		restore_screen_area ();	/* Yuck! */
		window_nonblocking = 0;
	}

	save_screen ();
	redraw_sprites ();

	p = agi_printf (logics[logic].texts[msg-1], logic);
	if (len != -1)
		textbox (p, (x-1)*8, y*8, len);
	else
		textbox (p, -1, -1, -1);

	/* From the specs:
	 *
	 * f15 determines the output mode of `print' and `print_at' commands: 
	 *    1 - message window is left on the screen 
	 *    0 - message window is closed when ENTER or ESC key are pressed.
	 *        If v21 is not 0, the window is closed automatically after
	 *	  1/2 * Var (21) seconds.
	 * 
	 * So 1 is nonblocking, and 0 is blocking!
	 */
 
	if (getflag (15) && !getvar (V_window_reset)) {
		_D (_D_WARN "f15==1, v21==0 => nonblocking");
		window_nonblocking = 1;
		release_sprites ();
	} else {
		if (getvar (V_window_reset) > 0) {
			_D (_D_WARN "f15==0, v21==%d => timed", getvar (21));
			setvar (V_key, 0);
			msg_box_secs2 = getvar (V_window_reset);
			_D (_D_WARN "msg_box_secs2 = %ld", msg_box_secs2);

			//while (getvar (V_window_reset) > 0) {
			do {
				/* FR: The call to main cycle fills the
				 * keyboard internal buffer!
				 */
				/* CM: not anymore */
				main_cycle (FALSE);

				if (key) {
					_D (_D_WARN "key");
					setvar (V_key, key = 0);
					setvar (V_window_reset, 0);
					break;
				}
			//}
			} while (msg_box_ticks > 0);
		} else {
			_D (_D_WARN "f15==0, v21==0 ==> waitkey");
			setvar (V_key, key = 0);
			wait_key ();
		}
		release_sprites ();
		restore_screen_area ();
	}
}


void cmd_stop_motion (UINT8 entry)
{
	vt.flags&=~MOTION;
	vt.direction=0;
	vt.motion=MOTION_NORMAL;

	/* CM: added to fix LSL1 from room11 <-> room12
	 */
	if (!entry)
		cmd_prog_control ();
}


void cmd_start_motion (UINT8 entry)
{
	vt.flags|=MOTION;
	vt.motion=MOTION_NORMAL;

	/* CM: added these to fix LSL1 from room11 <-> room12
	 */
	if (!entry)
	{
		move_ego (0);
		cmd_ego_control ();
	}
}


void cmd_close_window ()
{
	report ("Hack: close.window ()\n");
	if (!window_nonblocking)		/* CM: Fixes 'flashback' bug */
		return;
	restore_screen_area ();
	window_nonblocking = 0;
}


void cmd_close_dialogue ()
{
	/* FIXME see open_dialogue */
	report ("Not implemented: close.dialogue ()\n");
}


void cmd_open_dialogue ()
{
	report ("Not implemented: open.dialogue ()\n");
}


/* FIXME: ugliest function in the world */
void cmd_show_obj (UINT8 n)
{
	struct view_cel *c;
	UINT8 *bg;
	int x, y, w, h, x_, y_, w_, i, j;

	report ("TEST: cmd_show_obj(%i)\n", n);

	agi_load_resource (rVIEW, n);
	if (! (c = &views[n].loop[0].cel[0]))
		return;
	
	w_ = c->width;
	h = c->height;
	w = w_ * 2;
	x = _WIDTH - w_;
	x_ = x / 2;
	y_ = 120;
	y = game.line_min_print ? y_ + 8 : y_;
	bg = malloc (w * h);


	/* FIXME: flush_block () coordinates */

	for (i = w - 1; i >= 0; i--)
		for (j = h - 1; j >= 0; j--)
			bg[i + w * j] = layer1_data[x + i + 320 * (y + j)];

	for (i = w - 1; i >= 0; i--)
		for (j = h - 1; j >= 0; j--)
			layer1_data[x + i + 320 * (y + j)] = c->data[i / 2 + w_ * j];
	flush_block (x, y, x + w, y + h);

	/* FIXME: should call agi_printf */
	report("TEST: cmd_show_obj : descr [%s]\n", views[n].descr);
	message_box (views[n].descr);

	for (i = w - 1; i >= 0; i--)
		for (j = h - 1; j >= 0; j--)
			layer1_data[x + i + 320 * (y + j)] = bg[i + w * j];
	flush_block (x, y, x + w, y + h);

	free (bg);
}


void cmd_obj_statusv ()
{
	report ("Not implemented: obj.status.v ()\n");
}


void cmd_set_upper_left ()
{
	/* change basepoint from bottom right to upper left */
	/* x, y ? */
	report ("Not implemented: set.upper.left ()\n");
}


void cmd_clear_text_rect (UINT8 x1, UINT8 y1, UINT8 x2, UINT8 y2, UINT8 c)
{
	_D ("(%d, %d, %d, %d, %d)", x1, y1, x2, y2, c);
/*
	if (screen_mode==GFX_MODE)
		c=0xF;
	else
		c=txt_bg;
*/
	if (c!=0)
		c=15;

	/*y1++;*/
	x2++;
	y2++;
	draw_box (y1 * 8, x1 * 8, y2 * 8, x2 * 8, c, c,
		BX_SAVE | NO_LINES, game.line_min_print * 8);
	put_block (y1 * 8, x1 * 8, y2 * 8, x2 * 8);
}


void cmd_unanimate_all ()
{
	UINT16 num;

	for (num=0; num<MAX_VIEWTABLE; num++)
		view_table[num].flags &= ~(UPDATE | MOTION | ANIMATED | DRAWN);
}


void cmd_stop_cycling (UINT8 entry)
{
	vt.flags &= ~CYCLING;
}


void cmd_start_cycling (UINT8 entry)
{
	vt.flags |= CYCLING;
}


void cmd_normal_cycling (UINT8 entry)
{
	vt.cycle_status = CYCLE_NORMAL;
}


void cmd_step_time (UINT8 entry, UINT8 v)
{
	vt.step_time = getvar (v);
}


void cmd_step_size (UINT8 entry, UINT8 v)
{
	vt.step_size = getvar (v);
}


void cmd_show_mem ()
{
	message_box ((UINT8*)"Free memory is irrelevant, "
		"we have plenty of it to go around.");
}


void cmd_quit (UINT8 f)
{
	if (f) {
		game.quit_prog_now = TRUE;
	} else {
		message_box ((UINT8*)"   Press ENTER to quit.\n"
			"Press ESC to keep playing.");

		switch (game.message_box_key & 0xFF) {
		case 'Y':
		case 'y':
		case 0x0d:
		case 0x0a:
			game.quit_prog_now = TRUE;
			break;
		}
	}
}


void cmd_display (UINT8 logic, UINT8 y, UINT8 x, UINT8 msg)
{
	UINT8	*p;

	/*_D (("(%d, %d, %d, %d)", logic, y, x, msg));*/
	if (logics[logic].texts!=NULL && (msg-1)<=logics[logic].num_texts) {
		p = agi_printf (logics[logic].texts[msg-1], logic);
		print_text (p, x*8, 0, y*8, 40, txt_fg, txt_bg);
	}
}


/* CM: why are these reversed ?? */
void cmd_ego_control ()
{
	view_table[0].flags |= MOTION;
	game.control_mode = CONTROL_PROGRAM;
}


void cmd_prog_control ()
{
	game.control_mode = CONTROL_PLAYER;
}


void cmd_repos_to (UINT8 entry, UINT8 x, UINT8 y)
{
	vt.x_pos = x;
	vt.y_pos = y;
}


void cmd_trace_on ()
{
}


void cmd_trace_info ()
{
}


void cmd_animate_obj (UINT8 entry)
{
	/* Object is included in the list of object controlled by the
	 * interpreter. OBJECTS NOT INCLUDED IN THAT LIST ARE CONSIDERED
	 * INEXISTENT!
	 */

	_D ("(%d)", entry);
	vt.flags = ANIMATED | UPDATE | CYCLING | MOTION;

	/* from meka, unknown */
	vt.motion = MOTION_NORMAL;
	vt.cycle_status = CYCLE_NORMAL;

	if (entry !=EGO_VIEW_TABLE)
		vt.direction = 0;

	/* This can't be right! Ego will walk backwards if this sequence of commands 
    * Is issued (go right room, return to left room */
	/* ?? vt.direction = 0;  */ 
}


void cmd_menu_input ()
{
	do_menus ();
}


void cmd_enable_item (UINT8 event)
{
	menu_set_item (event, TRUE);
}


void cmd_disable_item (UINT8 event)
{
	menu_set_item (event, FALSE);
}


void cmd_unk_170 ()
{
#ifdef DISPLAY_DUDCODE
	message_box (logic_names_cmd[170].name);
#endif
}


void cmd_unk_171 ()
{
#ifdef DISPLAY_DUDCODE
	message_box (logic_names_cmd[171].name);
#endif
}


void cmd_unk_172 ()
{
#ifdef DISPLAY_DUDCODE
	message_box (logic_names_cmd[172].name);
#endif
}


void cmd_unk_173 ()
{
#ifdef DISPLAY_DUDCODE
	message_box (logic_names_cmd[173].name);
#endif
}


void cmd_unk_174 ()
{
#ifdef DISPLAY_DUDCODE
	message_box (logic_names_cmd[174].name);
#endif
}


void cmd_unk_175 ()
{
#ifdef DISPLAY_DUDCODE
	message_box (logic_names_cmd[175].name);
#endif
}


void cmd_unk_176 ()
{
#ifdef DISPLAY_DUDCODE
	message_box (logic_names_cmd[176].name);
#endif
}


void cmd_unk_177 ()
{
#ifdef DISPLAY_DUDCODE
	message_box (logic_names_cmd[177].name);
#endif
}


void cmd_unk_178 ()
{
#ifdef DISPLAY_DUDCODE
	message_box (logic_names_cmd[178].name);
#endif
}


void cmd_unk_179 ()
{
#ifdef DISPLAY_DUDCODE
	message_box (logic_names_cmd[179].name);
#endif
}


void cmd_unk_180 ()
{
#ifdef DISPLAY_DUDCODE
	message_box (logic_names_cmd[180].name);
#endif
}


void cmd_unk_181 ()
{
#ifdef DISPLAY_DUDCODE
	message_box (logic_names_cmd[181].name);
#endif
}


void cmd_set_string (UINT8 logic, UINT8 str, UINT8 txt)
{
	_D ("(%d, %d, %d)", logic, str, txt);
	txt--;

	/* CM: to avoid crash in Groza (str = 150) */
	if (str > MAX_WORDS1)
		return;

	strcpy (game.strings[str], logics[logic].texts[txt]);
}


void cmd_save_game ()
{
	report ("Debug: save.game ()\n");
	save_game ("savetest.iff", "Save game test");
	message_box ("Game saved.");
}


void cmd_load_game ()
{
	/* CM: we'll implemente the browser later */
	if (load_game ("savetest.iff") == err_OK)
		message_box ((UINT8*)"Gamed loaded.");
	else
		message_box ((UINT8*)"Error loading game.");
	redraw_sprites ();
}


void cmd_init_disk ()
{
}


void cmd_restart_game ()
{
	/* implement restart game */
	save_screen ();
	textbox ((UINT8*)"Press ENTER to restart the game.\n"
		"Press ESC to continue this game.", -1, -1, 24);
	switch (wait_key ()) {
	case 0x0A:
	case 0x0D:
		game.quit_prog_now = 0xFF;
		setflag (F_restart_game, TRUE);
		break;
	default:
		break;
	}
	restore_screen ();
}


void cmd_show_pri_screen ()
{
	save_screen ();
	dump_x_screen ();
	wait_key ();
	restore_screen ();
	/* MK: Doesn't seem to be necessary.
	 * update_status_line (TRUE);
	 */
}

void cmd_discard_view (UINT8 vw)
{
	agi_unload_resource (rVIEW, vw);
}


void cmd_set_scan_start (UINT8 logic, UINT16 xip)
{
	logics[logic].sIP=xip;
}


void cmd_reset_scan_start (UINT8 logic)
{
	logics[logic].sIP=2;
}


void cmd_log ()
{
}


void cmd_set_game_id (UINT8 logic, UINT8 msg)
{
#if 0
	if (gid != NULL)
		free (gid);
#endif

	if (logics[logic].texts && (msg - 1) <= logics[logic].num_texts)
		strncpy (game.id, logics[logic].texts[msg - 1], 8);
	else
		game.id[0] = 0;

	report ("Game ID: \"%s\"\n", game.id);
}


void cmd_toggle_monitor ()
{
	report ("Nani?! You want to toggle your monitor?\n");
}


void cmd_init_joystick ()
{
	report ("Nan desu ka... Joystick not implemented yet.\n");
}


void cmd_script_size (UINT8 n)
{
	report ("Not implemented: script.size (%d)\n", n);
}


void cmd_echo_line ()
{
	report ("Not implemented: echo.line ()\n");
}


void cmd_cancel_line ()
{
	report ("Not implemented: cancel.line ()\n");
}


void cmd_pause ()
{
	int clock;

	clock = game.clock_enabled;
	game.clock_enabled = FALSE;
	message_box ("    Game is Paused.\nPress ENTER to continue.");
	game.clock_enabled = clock;
}


void cmd_submit_menu ()
{
	_D ("()");
	submit_menu ();
}


void cmd_set_menu (UINT8 logic, UINT8 msg)
{
	_D ("(%d, %d)", logic, msg);
	if (logics[logic].texts != NULL && (msg - 1) <= logics[logic].num_texts)
		add_menu (logics[logic].texts[msg - 1]);
}


void cmd_set_menu_item (UINT8 logic, UINT8 msg, UINT8 cont)
{
	_D ("(%d, %d, %d)", logic, msg, cont);
	if (logics[logic].texts != NULL && (msg - 1) <= logics[logic].num_texts)
		add_menu_item (logics[logic].texts[msg - 1], cont);
}


void cmd_mul (UINT8 var, UINT8 val)
{
	setvar (var, getvar (var) * val);
}


void cmd_div (UINT8 var, UINT8 val)
{
	setvar (var, val ? getvar (var) / val : 0);
}


void cmd_rand_num (UINT8 n0, UINT8 n1, UINT8 var)
{
	setvar (var, rnd (1 + (n1 - n0)) + n0);
}


void cmd_load_pic (UINT8 pic)
{
	_D("(pic = %d)", pic);
	agi_load_resource (rPICTURE, getvar (pic));
}


extern int greatest_kludge_of_all_time;

void cmd_draw_pic (UINT8 pic)
{
	_D (_D_WARN "(pic = %d)", getvar(pic));
	pic_clear_flag = TRUE;
	decode_picture (getvar (pic));
	/* CM: needed for nonblocking window removal
	 */
	window_nonblocking = 0;
	greatest_kludge_of_all_time = 1;
}


void cmd_overlay_pic (UINT8 pic)
{
	pic_clear_flag = FALSE;
	decode_picture (getvar (pic));
	pic_clear_flag = TRUE;
}


void cmd_show_pic ()
{
	dump_screenX ();
	greatest_kludge_of_all_time = 0;
	put_screen ();
	update_status_line (TRUE);
}


void cmd_discard_pic ()
{
}


void cmd_version ()
{
	/* FIXME */
	char ver_msg[] = TITLE " v" VERSION;
	char ver2_msg[] =
			"\n"
			"                             \n\n"
        	"Emulating Sierra AGI v%x.%03x\n";
	char ver3_msg[]=
    		"\n"
       		"                             \n\n"
       		"  Emulating AGI v%x.002.%03x\n"; // no Sierra as it wraps textbox
	char *p, *q;
	int ver, maj, min;

	ver = agi_get_release ();
	maj = (ver >> 12) & 0xf;
	min = ver & 0xfff;

	q = maj == 2 ? ver2_msg : ver3_msg;
	p = strchr (q+1, '\n');

	strncpy (q+1 + ((p-q>0 ? p-q : 1)/4), ver_msg, strlen (ver_msg));
	message_box (q, maj, min);
}


/* FIXME: change to array of function pointers? */
void execute_agi_command (UINT8 op, UINT16 lognum, UINT8 *p)
{
	switch (op) {
	case 0x01:				/* inc */
		cmd_inc (p[0]);
		break;
	case 0x02:				/* dec */
		cmd_dec (p[0]);
		break;
	case 0x03:				/* assign.n */
		cmd_assign (p[0], p[1]);
		break;
	case 0x04:				/* assign.v */
		cmd_assign (p[0], getvar (p[1]));
		break;
	case 0x05:				/* add.n */
		cmd_add (p[0], p[1]);
		break;
	case 0x06:				/* add.v */
		cmd_add (p[0], getvar (p[1]));
		break;
	case 0x07:				/* sub.n */
		cmd_sub (p[0], p[1]);
		break;
	case 0x08:				/* sub.v */
		cmd_sub (p[0], getvar (p[1]));
		break;
	case 0x09:				/* lindirect.v */
		cmd_lindirect (p[0], getvar (p[1]));
		break;
	case 0x0A:				/* ridirect */
		/* FR
		 * According to the specs, should be : v30 = *v31
		 */
		cmd_rindirect (p[0], getvar (p[1]));
		break;
	case 0x0B:				/* lindirect.n */
		cmd_lindirect (p[0], p[1]);
		break;
	case 0x0C:				/* set */
		cmd_set (p[0]);
		break;
	case 0x0D:				/* reset */
		cmd_reset (p[0]);
		break;
	case 0x0E:				/* toggle */
		cmd_toggle (p[0]);
		break;
	case 0x0F:				/* set.v */
		cmd_set (getvar (p[0]));
		break;
	case 0x10:				/* reset.v */
		cmd_reset (getvar (p[0]));
		break;
	case 0x11:				/* toggle.v */
		cmd_toggle (getvar (p[0]));
		break;
	case 0x12:				/* new.room */
		cmd_new_room (p[0]);
		break;
	case 0x13:				/* new.room.v */
		cmd_new_room (getvar (p[0]));
		break;
	case 0x14:				/* load.logic */
		cmd_load_logic (p[0]);
		break;
	case 0x15:				/* load.logic.v */
		cmd_load_logic (getvar (p[0]));
		break;
	case 0x16:				/* call */
		cmd_call (p[0]);
		break;
	case 0x17:				/* call.v */
		cmd_call (getvar (p[0]));
		break;
	case 0x18:				/* load pic */
		cmd_load_pic (p[0]);
		break;
	case 0x19:				/* show pic */
		_D (_D_WARN "-----------------------");
		cmd_draw_pic (p[0]);
		break;
	case 0x1A:				/* show pic */
		cmd_show_pic ();
		break;
	case 0x1B:				/* discard pic */
		cmd_discard_pic ();
		break;
	case 0x1C:				/* overlay pic */
		cmd_overlay_pic (p[0]);
		break;
	case 0x1D:				/* show priority */
		cmd_show_pri_screen ();
		break;
	case 0x1E:				/* load.view */
   		cmd_load_view (p[0]);
		break;
	case 0x1F:				/* load.view.v */
   		cmd_load_view (getvar (p[0]));
		break;
	case 0x20:				/* discard view */
		cmd_discard_view (p[0]);
		break;
	case 0x21:				/* animate obj */
		cmd_animate_obj (p[0]);
		break;
	case 0x22:				/* stop all anim */
		cmd_unanimate_all ();
		break;
	case 0x23:				/* draw */
		cmd_draw (p[0]);
		break;
	case 0x24:				/* erase */
		cmd_erase (p[0]);
		break;
	case 0x25:				/* position */
		cmd_position (p[0], p[1], p[2]);
		break;
	case 0x26:				/* position.v */
		cmd_position (p[0], getvar (p[1]), getvar (p[2]));
		break;
	case 0x27:				/* get position */
		cmd_get_posn (p[0], p[1], p[2]);
		break;
	case 0x28:				/* set position */
		cmd_reposition (p[0], p[1], p[2]);
		break;
	case 0x29:				/* set.view */
		cmd_set_view (p[0], p[1]);
		break;
	case 0x2A:				/* set.view.v */
		cmd_set_view (p[0], getvar (p[1]));
		break;
	case 0x2B:				/* set.loop */
		cmd_set_loop (p[0], p[1]);
		break;
	case 0x2C:				/* set.loop.v */
		cmd_set_loop (p[0], getvar (p[1]));
		break;
	case 0x2D:				/* fix loop */
		cmd_fix_loop (p[0]);
		break;
	case 0x2E:				/* release loop */
		cmd_release_loop (p[0]);
		break;
	case 0x2F:				/* set cel */
		cmd_set_cel (p[0], p[1]);
		break;
	case 0x30:				/* set cel v */
		cmd_set_cel (p[0], getvar (p[1]));
		break;
	case 0x31:				/* set last cel */
		cmd_last_cel (p[0], p[1]);
		break;
	case 0x32:				/* set cur cel */
		cmd_cur_cel (p[0], p[1]);
		break;
	case 0x33:				/* set cur loop */
		cmd_cur_loop (p[0], p[1]);
		break;
	case 0x34:				/* set cur view */
		cmd_cur_view (p[0], p[1]);
		break;
	case 0x35:				/* set num loops */
		cmd_set_num_loops (p[0], p[1]);
		break;
	case 0x36:				/* set pri */
		cmd_set_priority (p[0], p[1]);
		break;
	case 0x37:				/* set priv*/
		cmd_set_priority (p[0], getvar (p[1]));
		break;
	case 0x38:				/* release priority */
		cmd_release_priority (p[0]);
		break;
	case 0x39:				/* get priority */
		cmd_get_priority (p[0], p[1]);
		break;
	case 0x3A:				/* stop update */
		cmd_stop_update (p[0]);
		break;
	case 0x3B:				/* start update */
		cmd_start_update (p[0]);
		break;
	case 0x3C:				/* force update */
		cmd_force_update (p[0]);
		break;
	case 0x3D:				/* ignore horizon */
		cmd_ignore_horizon (p[0]);
		break;
	case 0x3E:				/* obsv horizon */
		cmd_observe_horizon (p[0]);
		break;
	case 0x3F:				/* set horizon */
		cmd_set_horizon (p[0]);
		break;
	case 0x40:				/* obj on water */
		cmd_object_on_water (p[0]);
		break;
	case 0x41:				/* obj on land */
		cmd_object_on_land (p[0]);
		break;
	case 0x42:				/* obj on any */
		cmd_object_on_any (p[0]);
		break;
	case 0x43:				/* ignore objs */
		cmd_ignore_objs (p[0]);
		break;
	case 0x44:				/* objserve objs */
		cmd_observe_objs (p[0]);
		break;
	case 0x45:				/* distance */
		cmd_distance (p[0], p[1], p[2]);
		break;
	case 0x46:				/* stop cycle */
		cmd_stop_cycling (p[0]);
		break;
	case 0x47:				/* start cycle */
		cmd_start_cycling (p[0]);
		break;
	case 0x48:				/* normal cycle */
		cmd_normal_cycling (p[0]);
		break;
	case 0x49:				/* end of loop */
		cmd_end_of_loop (p[0], p[1]);
		break;
	case 0x4A:				/* reverse cycle */
		cmd_reverse_cycle (p[0]);
		break;
	case 0x4B:				/* reverse loop */
		cmd_reverse_loop (p[0], p[1]);
		break;
	case 0x4C:				/* cycle time */
		cmd_cycle_time (p[0], p[1]);
		break;
	case 0x4D:				/* stop motion */
		cmd_stop_motion (p[0]);
		break;
	case 0x4E:				/* start motion */
		cmd_start_motion (p[0]);
		break;
	case 0x4F:				/* step size */
		cmd_step_size (p[0], p[1]);
		break;
	case 0x50:				/* step time */
		cmd_step_time (p[0], p[1]);
		break;
	case 0x51:				/* move obj */
		cmd_move_obj (p[0], p[1], p[2], p[3], p[4]);
		break;
	case 0x52:				/* move objv */
		cmd_move_obj (p[0], getvar (p[1]),
			getvar (p[2]), getvar (p[3]), p[4]);
		break;
	case 0x53:				/* follow ego */
		cmd_follow_ego (p[0], p[1], p[2]);
		break;
	case 0x54:				/* wander */
		cmd_wander (p[0]);
		break;
	case 0x55:				/* normal motion */
		cmd_norm_motion (p[0]);
		break;
	case 0x56:				/* set dir */
		cmd_set_dir (p[0], p[1]);
		break;
	case 0x57:				/* get dir */
		cmd_get_dir (p[0], p[1]);
		break;
	case 0x58:				/* ignore blocks */
		cmd_ignore_blocks (p[0]);
		break;
	case 0x59:				/* obsrve blocks */
		cmd_observe_blocks (p[0]);
		break;
	case 0x5A:				/* block */
		cmd_block (p[0], p[1], p[2], p[3]);
		break;
	case 0x5B:				/* unblock */
		cmd_unblock ();
		break;
	case 0x5C:				/* get */
		cmd_get (p[0]);
		break;
	case 0x5D:				/* getv */
		cmd_get (getvar (p[0]));
		break;
	case 0x5E:				/* drop */
		cmd_drop (p[0]);
		break;
	case 0x5F:				/* put */
		cmd_put (p[0], p[1]);
		break;
	case 0x60:				/* putv */
		cmd_put (p[0], getvar (p[1]));
		break;
	case 0x61:				/* get roomv */
		cmd_get_roomv (p[0], p[1]);
		break;
	case 0x62:				/* load sound */
		cmd_load_sound (p[0]);
		break;
	case 0x63:				/* play sound */
		cmd_play_sound (p[0], p[1]);
		break;
	case 0x64:				/* stop sound */
		cmd_stop_sound ();
		break;
	case 0x65:				/* print */
		cmd_print (lognum, p[0]);
		break;
	case 0x66:				/* printv */
		cmd_print (lognum, getvar (p[0]));
		break;
	case 0x67:				/* display */
		cmd_display (lognum, p[0], p[1], p[2]);
		break;
	case 0x68:				/* displayv */
		cmd_display (lognum, getvar (p[0]),
			getvar (p[1]), getvar (p[2]));
		break;
	case 0x69:				/* clear lines */
		cmd_clear_lines (p[0], p[1], p[2]);
		break;
	case 0x6A:				/* text mode */
		cmd_txt ();
		break;
	case 0x6B:				/* graphics */
		cmd_gfx ();
		break;
	case 0x6C:				/* cursor char */
		cmd_set_cur_char (lognum, p[0]);
		break;
	case 0x6D:				/* text attribute */
		cmd_set_text_attr (p[0], p[1]);
		break;
	case 0x6E:				/* shake screen */
		cmd_shake_screen (p[0]);
		break;
	case 0x6F:				/* config screen */
		cmd_config_screen (p[0], p[1], p[2]);
		break;
	case 0x70:				/* status on */
		cmd_status_line_on ();
		break;
	case 0x71:				/* status off */
		cmd_status_line_off ();
		break;
	case 0x72:				/* set string */
		cmd_set_string (lognum, p[0], p[1]);
		break;
	case 0x73:				/* get string */
		cmd_get_string (lognum, p[0], p[1], p[2], p[3], p[4]);
		break;
	case 0x74:				/* word 2 string */
		cmd_word_to_string (p[0], p[1]);
		break;
	case 0x75:				/* parse */
		cmd_parse (lognum, p[0]);
		break;
	case 0x76:				/* get number */
		cmd_get_num (lognum, p[0], p[1]);
		break;
	case 0x77:				/* stop input */
		cmd_stop_input ();
		break;
	case 0x78:				/* accept input */
		cmd_accept_input ();
		break;
	case 0x79:				/* set key */
		cmd_set_key (p[0], p[1], p[2]);
		break;
	case 0x7A:				/* add.to.pic */
		cmd_add_to_pic (p[0], p[1], p[2], p[3], p[4], p[5], p[6]);
		break;
	case 0x7B:				/* add.to.pic.v */
		cmd_add_to_pic (getvar (p[0]), getvar (p[1]),
			getvar (p[2]), getvar (p[3]), getvar (p[4]),
			getvar (p[5]), getvar (p[6]));
		break;
	case 0x7C:				/* status */
		cmd_status ();
		break;
	case 0x7D:				/* save game */
		cmd_save_game ();
		break;
	case 0x7E:				/* load game */
		cmd_load_game ();
		break;
	case 0x7F:				/* init disk */
		cmd_init_disk ();
		break;
	case 0x80:				/* restart */
		cmd_restart_game ();
		break;
	case 0x81:				/* show object */
		cmd_show_obj (p[0]);
		break;
	case 0x82:				/* random */
		cmd_rand_num (p[0], p[1], p[2]);
		break;
	case 0x83:				/* prog control */
		cmd_prog_control ();
		break;
	case 0x84:				/* ego control */
		cmd_ego_control ();
		break;
	case 0x85:				/* obj status v */
		cmd_obj_statusv ();
		break;
	case 0x86:				/* quit */
		if (logic_names_cmd[0x86].num_args == 0)
			cmd_quit (1);
		else
			cmd_quit (p[0]);
		break;
	case 0x87:				/* show mem */
		cmd_show_mem ();
		break;
	case 0x88:				/* pause */
		cmd_pause ();
		break;
	case 0x89:				/* echo line */
		cmd_echo_line ();
		break;
	case 0x8A:				/* cancle line */
		cmd_cancel_line ();
		break;
	case 0x8B:				/* joystick */
		cmd_init_joystick ();
		break;
	case 0x8C:				/* toggle monitor */
		cmd_toggle_monitor ();
		break;
	case 0x8D:				/* version */
		cmd_version ();
		break;
	case 0x8E:				/* script size */
		cmd_script_size (p[0]);
		break;
	case 0x8F:				/* game id */
		cmd_set_game_id (lognum, p[0]);
		break;
	case 0x90:				/* log */
		cmd_log ();
		break;
	case 0x91:				/* set start */
		cmd_set_scan_start (lognum, ip +
			logic_names_cmd[op].num_args);
		break;
	case 0x92:				/* reset start */
		cmd_reset_scan_start (lognum);
		break;
	case 0x93:				/* repos */
		cmd_repos_to (p[0], p[1], p[2]);
		break;
	case 0x94:				/* repos v */
		cmd_repos_to (p[0], getvar (p[1]), getvar (p[2]));
		break;
	case 0x95:				/* trace on */
		cmd_trace_on ();
		break;
	case 0x96:				/* trace info */
		cmd_trace_info ();
		break;
	case 0x97:				/* print at */
		if (logic_names_cmd[0x97].num_args==3)
			cmd_print_at (lognum, p[0], p[1], p[2], 0);
		else
			cmd_print_at (lognum, p[0], p[1], p[2], p[3]);
		break;
	case 0x98:				/* print at v */
		if (logic_names_cmd[0x98].num_args == 3)
			cmd_print_at (lognum, getvar (p[0]),
				p[1], p[2], 0);
		else
			cmd_print_at (lognum, getvar (p[0]),
				p[1], p[2], p[3]);
		break;
	case 0x99:				/* discard view v */
		cmd_discard_view (getvar (p[0]));
		break;
	case 0x9A:				/* clr text rect */
		cmd_clear_text_rect (p[0], p[1], p[2], p[3], p[4]);
		break;
	case 0x9B:				/* upper left */
		cmd_set_upper_left ();
		break;
	case 0x9C:				/* set menu */
		cmd_set_menu (lognum, p[0]);
		break;
	case 0x9D:				/* set menu item */
		cmd_set_menu_item (lognum, p[0], p[1]);
		break;
	case 0x9E:				/* submit menu */
		cmd_submit_menu ();
		break;
	case 0x9F:				/* enable menu */
		cmd_enable_item (p[0]);
		break;
	case 0xA0:				/* disable menu */
		cmd_disable_item (p[0]);
		break;
	case 0xA1:				/* menu input */
		cmd_menu_input ();
		break;
	case 0xA2:				/* show.object.v */
		cmd_show_obj (getvar (p[0]));
		break;
	case 0xA3:				/* open dialogue */
		cmd_open_dialogue ();
		break;
	case 0xA4:				/* close dialogue */
		cmd_close_dialogue ();
		break;
	case 0xA5:				/* muln */
		cmd_mul (p[0], p[1]);
		break;
	case 0xA6:				/* mulv */
		cmd_mul (p[0], getvar (p[1]));
		break;
	case 0xA7:				/* divn */
		cmd_div (p[0], p[1]);
		break;
	case 0xA8:				/* divv */
		cmd_div (p[0], getvar (p[1]));
		break;
	case 0xA9:				/* close window */
		cmd_close_window ();
		break;
	case 0xAA:
		cmd_unk_170 ();
		break;
	case 0xAB:
		cmd_unk_171 ();
		break;
	case 0xAC:
		cmd_unk_172 ();
		break;
	case 0xAD:
		cmd_unk_173 ();
		break;
	case 0xAE:
		cmd_unk_174 ();
		break;
	case 0xAF:
		cmd_unk_175 ();
		break;
	case 0xB0:
		cmd_unk_176 ();
		break;
	case 0xB1:
		cmd_unk_177 ();
		break;
	case 0xB2:
		cmd_unk_178 ();
		break;
	case 0xB3:
		cmd_unk_179 ();
		break;
	case 0xB4:
		cmd_unk_180 ();
		break;
	case 0xB5:
		cmd_unk_181 ();
		break;
	}
}


void run_logic (int lognum)
{
	UINT16	saved_ip;
	UINT16	last_ip;
	UINT8	op;
	UINT8	p[16];

	/* If logic not loaded, load it */
	if (~game.dir_logic[lognum].flags & RES_LOADED) {
		agi_load_resource (rLOGIC, lognum);
#ifdef DISABLE_COPYPROTECTION
		break_copy_protection (lognum);
#endif
	}

	if (getflag (F_new_room_exec)) {
		if (view_table[EGO_VIEW_TABLE].y_pos <= game.horizon)
			view_table[EGO_VIEW_TABLE].y_pos = game.horizon + 1;
	}

	saved_ip = logics[lognum].cIP;
	logics[lognum].cIP = logics[lognum].sIP;
	last_ip = ip;

	while (ip < logics[lognum].size && !game.quit_prog_now) {
		if (debug.enabled) {
			if (debug.steps > 0) {
				if (debug.logic0 || lognum) {
					debug_console (lognum,
						lCOMMAND_MODE, NULL);
					debug.steps--;
				}
			} else {
				redraw_sprites ();
				console_prompt ();
				do {
					main_cycle (TRUE);
				} while (!debug.steps && debug.enabled);
				console_lock ();
				release_sprites ();
			}
		}

		last_ip = ip;
		op = *(code + ip++);

		memmove (&p, (code + ip), 16);

		switch (op) {
		case 0xFF:				/* IF (open/close) */
			test_if_code (lognum);
			break;
		case 0xFE:				/* GOTO */
			ip += 2+((SINT16)lohi_getword (code+ip));	/* +2 covers goto size*/
			break;
		case 0x00:				/* return */
			return;
		default:
			execute_agi_command (op, lognum, p);
			ip += logic_names_cmd[op].num_args;
		}

		if (game.exit_all_logics)
			break;
	}
}
