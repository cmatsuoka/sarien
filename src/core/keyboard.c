/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999,2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "sarien.h"
#include "agi.h"
#include "gfx_base.h"
#include "keyboard.h"
#include "view.h"
#include "menu.h"
#include "opcodes.h"	/* remove later */
#include "console.h"
#include "text.h"	/* remove later */

UINT8  buffer[40];
UINT8  last_sentence[40];

static int xInput, yInput;
static int IsGetString=FALSE;

#ifdef USE_CONSOLE
extern struct sarien_console console;
#endif

extern struct agi_view_table view_table[];

/* FIXME */
extern int open_dialogue;


/*
 * IBM-PC keyboard scancodes
 */
UINT8 scancode_table[26] = {
	30,			/* A */
	48,			/* B */
	46,			/* C */
	32,			/* D */
	18,			/* E */
	33,			/* F */
	34,			/* G */
	35,			/* H */
	23,			/* I */
	36,			/* J */
	37,			/* K */
	38,			/* L */
	50,			/* M */
	49,			/* N */
	24,			/* O */
	25,			/* P */
	16,			/* Q */
	19,			/* R */
	31,			/* S */
	20,			/* T */
	22,			/* U */
	47,			/* V */
	17,			/* W */
	45,			/* X */
	21,			/* Y */
	44			/* Z */
};


void init_words ()
{
	clean_input ();
}


void clean_input ()
{
	int i;

	for (i = 0; i < MAX_WORDS; i++) {
		game.ego_words[i].word = "";
		game.ego_words[i].id = 0xffff;
	}

	game.num_ego_words = 0;
}


/* FR:
 * The capture of the keys is done inside main_cycle()
 * It's safe to return last_sentence because all the functions who call
 * get_string copy the result into another buffer
 */
char *get_string (int x, int y, int len)
{
	UINT8 old_max_chars = getvar (V_max_input_chars);
	int tmp;

	tmp = game.input_mode;
	game.input_mode = INPUT_GETSTRING;

	setvar (V_max_input_chars, len);

	xInput = x;
	yInput = y;

	while (IsGetString) {
		poll_timer ();		/* msdos driver -> does nothing */
		update_timer ();

		poll_keyboard ();

#ifdef USE_CONSOLE
		if (console.active && console.input_active)
			handle_console_keys ();
		else
			handle_keys ();
		console_cycle ();
#else
		handle_keys ();
#endif
	}

	game.input_mode = tmp;
	setvar (V_max_input_chars, old_max_chars);

	return last_sentence;
}


/* Called if ego enters a new room */
/* FIXME: remove lowlevel print_text call! */

void print_line_prompt ()
{
	int k;

	if (game.input_mode == INPUT_NORMAL) {
		/* Command prompt */

      		print_text (agi_printf (game.strings[0], 0), 0, 0,
    			game.line_user_input * CHAR_LINES, 40, txt_fg, txt_bg );


    		/* internal keyboard buffer */
    		for (k = 0; buffer[k]; k++) {
    			print_character ((k + 1) * CHAR_COLS,
				game.line_user_input * CHAR_LINES,
    				buffer[k], txt_fg, txt_bg );
    		}

    		/* cursor prompt */
    		if (IsGetString) {
    			print_character (xInput + (k + 1) * CHAR_COLS, yInput,
    				txt_char, txt_fg, txt_bg );
    		} else {
    			print_character ((k + 1) * CHAR_COLS,
				game.line_user_input * CHAR_LINES,
    				txt_char, txt_fg, txt_bg );
    		}
	}
}


static void move_ego (UINT8 direction)
{
	static UINT8 last_ego_dir = 0xFF;

	_D ("(%d)", direction);
	_D (_D_WARN "last_ego_dir = %d", last_ego_dir);

	if (view_table[EGO_VIEW_TABLE].flags & MOTION) {
		if (last_ego_dir != direction) {
			last_ego_dir = direction;
			view_table[EGO_VIEW_TABLE].direction = last_ego_dir;

			/* FR:
			 * Set the loop (in ego view table) corresponding to
			 * this direction
			 */
			calc_direction (EGO_VIEW_TABLE);
		} else {
			/* stop ego from moving in same direction */
			last_ego_dir = 0;
		}
		setvar (V_ego_dir, last_ego_dir);
	}
}


void clean_keyboard ()
{
	clean_input ();
	setvar (V_key, 0);
	setflag (F_entered_cli, FALSE);
	setflag (F_said_accepted_input, FALSE);
}


/*
 * poll_keyboard() is the raw key grabber (above the gfx driver, that is).
 * It handles console keys and insulates AGI from the console. In the main
 * loop, handle_keys() handles keyboard input and ego movement.
 */
void poll_keyboard ()
{
	int i, key = 0;

	setvar (V_word_not_found, 0);

#ifdef USE_CONSOLE
	if (!console.active || !console.input_active)
#endif
	{
		for (i = 0; i < MAX_DIRS; i++)
			game.events[i].occured = FALSE;
	}

	/* If a key is ready, rip it */
	while (keypress ()) {
		key = get_key ();
		_D ("key %02x pressed", key);

#ifdef USE_CONSOLE
		if (console_keyhandler (key))
			continue;

		if (console.active && console.input_active)
			continue;
#endif
		_D ("key = %d", key);

		/* For controller() */
		for (i = 0; game.input_mode != INPUT_MENU && i < MAX_DIRS; i++){
			switch (game.events[i].event) {
			case eSCAN_CODE:
				if (game.events[i].data == KEY_SCAN(key) &&
					KEY_ASCII(key) == 0)
				{
					_D ("event: scan code");
					game.events[i].occured = TRUE;
					report("event SC:%i occured\n", i);
				}
				break;
			case eKEY_PRESS:
				if (game.events[i].data == KEY_ASCII(key) &&
					KEY_SCAN(key) == 0)
				{
					_D ("event: key press");
					game.events[i].occured = TRUE;
					report ("event AC:%i occured\n", i);
				}
				break;
			}
		}
	}

#ifdef USE_CONSOLE
	if (key == CONSOLE_ACTIVATE_KEY key == CONSOLE_SWITCH_KEY)
		return;

	if (console.active && console.input_active) {
		if (key == CONSOLE_SCROLLUP_KEY || key == CONSOLE_SCROLLDN_KEY)
			return;
		console.input_key = KEY_ASCII (key);
		return;	
	}
#endif

	if (game.input_mode == INPUT_NORMAL && !KEY_ASCII (key)) {
		switch (key) {
		case KEY_UP:         move_ego (1); break;
		case KEY_DOWN:       move_ego (5); break;
		case KEY_LEFT:       move_ego (7); break;
		case KEY_RIGHT:      move_ego (3); break;
		case KEY_UP_RIGHT:   move_ego (2); break;
		case KEY_DOWN_RIGHT: move_ego (4); break;
		case KEY_UP_LEFT:    move_ego (8); break;
		case KEY_DOWN_LEFT:  move_ego (6); break;
		}
		return;
	}

	if (key)
		game.keypress = key;

	/* If you comment this out all keystrokes will be echoed */
	if (game.input_mode != INPUT_NONE || KEY_ASCII (key) == KEY_ENTER)
		setvar (V_key, KEY_ASCII (key));

}


void handle_keys ()
{
	UINT8 *p=NULL;
	int key, c = 0;
	static UINT8  old_input_mode = INPUT_NONE;
	static UINT8  new_line = 1;
	static UINT8  formated_entry[256];
	static UINT16 bufindex = 0;

	key = getvar (V_key);
	if (!key)
		return;

	_D ("handling key: %02x", key);
	setvar (V_key, 0);
 	
	switch (key) {
	case KEY_ENTER:
		_D (("KEY_ENTER"));

		/* remove all the crap from string */
		p = buffer;
		while (*p && *p == 0x20)	/* Remove all leading spaces */
			p++;

		if (IsGetString) {
			strcpy (last_sentence, p);
			IsGetString = FALSE;
		} else {
			while (*p) {	
 				/* Squash spaces */
				if (*p == 0x20 && *(p + 1) == 0x20) {	
					p++;
					continue;
				}
				formated_entry[c++] = tolower (*p);
				p++;
			}
			formated_entry[c++] = 0;

			/* Handle string only if it's not empty */
			if (formated_entry[0]) {
				strcpy (last_sentence, formated_entry);
				dictionary_words (last_sentence);
			}
		}

		/* Clear to start a new line*/
		new_line         = 1;
		bufindex         = 0;
		buffer[bufindex] = 0;

		break;
	case KEY_ESCAPE:
		_D (("KEY_ESCAPE"));
		if (getflag (F_menus_work))
			do_menus ();
		break;
	case 0x08:
		if (!bufindex)
			break;

		if (IsGetString) { 
			print_character (xInput + (bufindex + 1) * CHAR_COLS,
				yInput, txt_char, txt_bg, txt_bg );
		} else {
			print_character ((bufindex + 1) * CHAR_COLS,
				game.line_user_input * CHAR_LINES,
				txt_char, txt_bg, txt_bg);
		}

		bufindex--;
		buffer[bufindex]=0;
		break;
	default:
		if (key < 0x20 || key > 0x7f)
			break;

		if (bufindex >= getvar (V_max_input_chars))
			return;

		buffer[bufindex++] = key;
		buffer[bufindex] = 0;

		if (IsGetString) {
			print_character (xInput + (bufindex * CHAR_COLS),
				yInput, buffer[bufindex - 1], txt_fg, txt_bg );
		} else {
			print_character (bufindex * CHAR_COLS,
				game.line_user_input * CHAR_LINES,
				buffer[bufindex - 1], txt_fg, txt_bg);
		}
		break;
	}

	if (old_input_mode != game.input_mode) {
		old_input_mode = game.input_mode;

		if (game.input_mode == INPUT_NORMAL) {
			print_line_prompt();
		} else {
			cmd_clear_lines (game.line_user_input,
				game.line_user_input, txt_bg);
		}
	} else if (game.input_mode == INPUT_NORMAL) {
		if (new_line) {
			new_line = 0;

			/* TODO: Should handle IsGetString */

           		if (!IsGetString) {
	   			cmd_clear_lines (game.line_user_input,
					game.line_user_input, txt_bg);
	   			print_text (agi_printf (game.strings[0], 0), 0, 0,
					game.line_user_input * CHAR_LINES,
					40, txt_fg, txt_bg);
       			}
		}

		/* Print txt_char */
		if (IsGetString) {
			print_character (xInput + ((bufindex + 1) * CHAR_COLS),
				yInput, txt_char, txt_fg, txt_bg);
		} else {
			print_character ((bufindex + 1) * CHAR_COLS,
				game.line_user_input * CHAR_LINES, txt_char,
				txt_fg, txt_bg );
		}
	}
}


int wait_key ()
{
	_D (_D_WARN "waiting...");
	while(42) {
		int x;

		main_cycle (FALSE);
		if ((x = getvar (V_key))) {
			setvar (V_key, 0);
			return x;
		}
	}
}

