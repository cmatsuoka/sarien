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
#include <stdlib.h>
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

UINT16 key; 
UINT8  buffer[40];
UINT8  last_sentence[40];
UINT8  IsGetString=FALSE;
UINT16  xInput, yInput;

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


/* Called if ego enters a new room */
/* FIXME: remove lowlevel print_text call! */

void print_line_prompt ()
{
	int k;

	if (game.allow_kyb_input) {
		/* Command prompt */

		/* FIXME : dont show line input if its a dialogue box */
		if (open_dialogue==0){

      		print_text (agi_printf (game.strings[0], 0), 0, 0,
    			game.line_user_input * 8, 40, txt_fg, txt_bg );


    		/* internal keyboard buffer */
    		for (k = 0; buffer[k]; k++) {
    			print_character ((k + 1) * 8, game.line_user_input * 8,
    				buffer[k], txt_fg, txt_bg );
    		}

    		/* cursor prompt */
    		if (IsGetString) {
    			print_character (xInput + ((k + 1) * 8), yInput,
    				txt_char, txt_fg, txt_bg );
    		} else {
    			print_character ((k + 1) * 8, game.line_user_input * 8,
    				txt_char, txt_fg, txt_bg );
    		}

		}

	}
}

/* FR:
 * The capture of the keys is done inside of main_cycle()
 * It's safe to return last_sentence because all the functions who call
 * get_string copy the result into another buffer
 */
UINT8 *get_string (int x, int y, int len)
{
	UINT8 old_kyb_input = game.allow_kyb_input;
	UINT8 old_max_chars = getvar (V_max_input_chars);

	game.allow_kyb_input = TRUE;
	setvar( V_max_input_chars, len );

	xInput = x;
	yInput = y;

	IsGetString = TRUE;

	while (IsGetString)
		main_cycle(TRUE);

	game.allow_kyb_input = old_kyb_input;
	setvar (V_max_input_chars, old_max_chars);

	return (UINT8 *) last_sentence;
}


/* move_ego() shouldn't be in keyboard.c */
void move_ego (UINT8 direction)
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
	setvar (V_key, key = 0);
	setflag (F_entered_cli, FALSE);
	setflag (F_said_accepted_input, FALSE);
	key = 0;
}


/*
 * poll_keyboard() is the raw key grabber (above the gfx driver, that is).
 * It handles console keys and insulates AGI from the console. In the main
 * loop, handle_keys() handles keyboard input and ego movement.
 */
void poll_keyboard ()
{
	UINT16 xkey, c1;

	if (getvar (V_key) != KEY_ENTER)
		setvar (V_key, key = 0);

	setvar (V_word_not_found, 0);

#ifdef USE_CONSOLE
	if (!console.active || !console.input_active)
#endif
	{
		for (c1 = 0; c1 < MAX_DIRS; c1++)
			game.events[c1].occured = FALSE;
	}

	/* If a key is ready, rip it */
	while (keypress ()) {
		xkey = get_key ();

#ifdef USE_CONSOLE
		if (console_keyhandler (xkey))
			continue;
#endif

		key = xkey;
		_D ("key = %d", key);

#ifdef USE_CONSOLE
		if (console.active && console.input_active)
			continue;
#endif

		/* For controller() */
		for (c1 = 0; c1 < MAX_DIRS; c1++) {

#if 0
			if (game.events[c1].data &&
				game.events[c1].data == KEY_SCAN (xkey))
			{
				report ("xevent SC:%i=%04X %i:%i\n", c1,
					KEY_SCAN(xkey), eSCAN_CODE,
					game.events[c1].event);
			}
			if(game.events[c1].data &&
				game.events[c1].data == KEY_ASCII (xkey))
			{
	                       	report ("xevent AC:%i=%04X %i:%i\n", c1,
					KEY_ASCII(xkey), eKEY_PRESS,
					game.events[c1].event);
			}
#endif

			switch (game.events[c1].event) {
			case eSCAN_CODE:
				if (game.events[c1].data == KEY_SCAN(key) &&
					KEY_ASCII(key) == 0)
				{
					_D ("event: scan code");
					game.events[c1].occured = TRUE;
					report("event SC:%i occured\n", c1);
				}
				break;
			case eKEY_PRESS:
				if (game.events[c1].data == KEY_ASCII(key) &&
					KEY_SCAN(key) == 0)
				{
					_D ("event: key press");
					game.events[c1].occured = TRUE;
					report ("event AC:%i occured\n", c1);
				}
				break;
			}
		}
	}

	if (!KEY_ASCII (key))
		return;

#ifdef USE_CONSOLE
	if (console.active && console.input_active) {
		console.input_key = KEY_ASCII (key);
		key = 0;
	} else
#endif
	{
		/* If you comment this out all keystrokes will be echoed */
		if (!game.allow_kyb_input)
			return;

		setvar (V_key, KEY_ASCII (key));
	}
}


void handle_keys ()
{
	UINT8 *p=NULL;
	UINT16 k, c = 0;
	static UINT8  old_keyboard_status = FALSE;
	static UINT8  new_line = 1;
	static UINT8  formated_entry[256];
	static UINT16 bufindex = 0;

	/*
	 */
	switch (k = getvar (V_key)) {
	case KEY_ENTER:
		_D (("KEY_ENTER"));

		/* remove all the crap from their strings */
		p = (UINT8*) buffer;
		while ((*p == 0x20) && (*p))	/* Remove all leading spaces */
			p++;

		if ( IsGetString ) {
			setvar (V_key, 0);
			strcpy((char *) last_sentence, (char *) p);

			IsGetString = FALSE;
		} else {
			while (*p) {	
 				/* Remove a sequence of spaces */
				if ((*p == 0x20) && (*(p + 1) == 0x20)) {	
					p++;
					continue;
				}
				formated_entry[c++] = tolower( *p );
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
			print_character (xInput + (bufindex + 1) * 8, yInput,
				txt_char, txt_bg, txt_bg );
		} else {
			print_character ((bufindex + 1) * 8,
				game.line_user_input * 8,
				txt_char, txt_bg, txt_bg);
		}

		bufindex--;
		buffer[bufindex]=0;
		break;
	default:
		if (k >= 0x20 && k <= 0x7f && (bufindex < getvar (V_max_input_chars) - 1))
		{
			buffer[bufindex] = k;

			bufindex++;
			buffer[bufindex]=0;

			if (IsGetString) {
				print_character (xInput + (bufindex * 8),
					yInput, buffer[bufindex - 1],
					txt_fg, txt_bg );
			} else {
				print_character (bufindex * 8,
					game.line_user_input * 8,
					buffer[bufindex - 1], txt_fg, txt_bg);
			}
		}
		break;
	}

	if (old_keyboard_status != game.allow_kyb_input) {
		old_keyboard_status = game.allow_kyb_input;

		if (game.allow_kyb_input) {
			print_line_prompt();
		} else {
			cmd_clear_lines (game.line_user_input,
				game.line_user_input, txt_bg);
		}
	} else if (game.allow_kyb_input) {
		if (new_line) {
			new_line = 0;

			/* TODO: Should handle IsGetString */

           		if (!IsGetString) {
	   			cmd_clear_lines (game.line_user_input,
					game.line_user_input, txt_bg);
	   			print_text (agi_printf (game.strings[0], 0), 0, 0,
					game.line_user_input * 8,
					40, txt_fg, txt_bg);
       			}
		}

		/* Print txt_char */
		if (IsGetString) {
			print_character (xInput + ((bufindex + 1) * 8),
				yInput, txt_char, txt_fg, txt_bg);
		} else {
			print_character ((bufindex + 1) * 8,
				game.line_user_input * 8, txt_char,
				txt_fg, txt_bg );
		}
	}


	if (key < 0x100)
		return;

	switch (key) {
	case KEY_UP:
		move_ego (1);
		return;
	case KEY_DOWN:
		move_ego (5);
		return;
	case KEY_LEFT:
		move_ego (7);
		return;
	case KEY_RIGHT:
		move_ego (3);
		return;
	case KEY_UP_RIGHT:
		move_ego (2);
		return;
	case KEY_DOWN_RIGHT:
		move_ego (4);
		return;
	case KEY_UP_LEFT:
		move_ego (8);
		return;
	case KEY_DOWN_LEFT:
		move_ego (6);
		return;
	}
}


int wait_key ()
{
	int x;

	_D (_D_WARN "waiting...");
	while(42) {
		main_cycle (FALSE);

		if ((x = getvar (V_key))) {
			setvar (V_key, key = 0);
			return x;
		}
	}
}

