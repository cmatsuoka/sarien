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

#ifndef PALMOS
#include <ctype.h>
#endif

#include "sarien.h"
#include "agi.h"
#include "graphics.h"
#include "keyboard.h"
#include "menu.h"
#include "text.h"	/* remove later */

UINT8  last_sentence[40];

#ifdef USE_CONSOLE
extern struct sarien_console console;
#endif

/* FIXME */
extern int open_dialogue;

struct string_data {
	int x;
	int y;
	int len;
	int str;
};

struct string_data stringdata;

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


void get_string (int x, int y, int len, int str)
{
	new_input_mode (INPUT_GETSTRING);
	stringdata.x = x;
	stringdata.y = y;
	stringdata.len = len;
	stringdata.str = str;
}


/**
 * Raw key grabber.
 * poll_keyboard() is the raw key grabber (above the gfx driver, that is).
 * It handles console keys and insulates AGI from the console. In the main
 * loop, handle_keys() handles keyboard input and ego movement.
 */
int do_poll_keyboard ()
{
	int key = 0;

	/* If a key is ready, rip it */
	if (keypress ()) {
		key = get_key ();
		_D ("key %02x pressed", key);
	}

	return key;
}


int handle_controller (int key)
{
	struct vt_entry *v = &game.view_table[0];
	int i;

	/* Click-to-walk mouse interface */
	if (game.player_control && v->flags & ADJ_EGO_XY) {
		v->direction = get_direction (v->x_pos, v->y_pos,
			v->parm1, v->parm2, v->step_size);

		if (v->direction == 0)
			in_destination (v);
	}

	if (key == 0)
		return FALSE;

	_D (_D_WARN "key = %04x", key);

	for (i = 0; i < MAX_DIRS; i++) {
		if (game.ev_scan[i].data == KEY_SCAN(key) && KEY_ASCII(key) == 0) {
			_D ("event %d: scan code", i);
			game.ev_scan[i].occured = TRUE;
			report("event SC:%i occured\n", i);
			return TRUE;
		}
		if (game.ev_keyp[i].data == KEY_ASCII(key) && KEY_SCAN(key) == 0) {
			_D ("event %d: key press", i);
			game.ev_keyp[i].occured = TRUE;
			report ("event AC:%i occured\n", i);
			return TRUE;
		}
	}

	if (key == BUTTON_LEFT) {
		if (getflag (F_menus_work) && mouse.y <= CHAR_LINES) {
			new_input_mode (INPUT_MENU);
			return TRUE;
		}
	}

	if (game.player_control) {
		int d = 0;

		if (!KEY_ASCII (key)) {
			switch (key) {
			case KEY_UP:         d = 1; break;
			case KEY_DOWN:       d = 5; break;
			case KEY_LEFT:       d = 7; break;
			case KEY_RIGHT:      d = 3; break;
			case KEY_UP_RIGHT:   d = 2; break;
			case KEY_DOWN_RIGHT: d = 4; break;
			case KEY_UP_LEFT:    d = 8; break;
			case KEY_DOWN_LEFT:  d = 6; break;
			}
		}

		if (!opt.agimouse) {
			/* Handle mouse button events */
			if (key == BUTTON_LEFT) {
				v->flags |= ADJ_EGO_XY;
				v->parm1 = WIN_TO_PIC_X(mouse.x);
				v->parm2 = WIN_TO_PIC_Y(mouse.y);
				return TRUE;
			} 
		}

		v->flags &= ~ADJ_EGO_XY;

		if (d || key == KEY_STATIONARY) {
			v->direction = v->direction == d ? 0 : d;
			return TRUE;
		}
	}

	return FALSE;
}


void handle_getstring (int key)
{
	static int pos = 0;		/* Cursor position */
	static char buf[40];

	if (KEY_ASCII(key) == 0)
		return;

	_D ("handling key: %02x", key);

	switch (key) {
	case KEY_ENTER:
		_D ("KEY_ENTER");
		game.has_prompt = 0;
		buf[pos] = 0;
		strcpy (game.strings[stringdata.str], buf);
		_D (_D_WARN "buffer=[%s]", buf);
		buf[pos = 0] = 0;
		new_input_mode (INPUT_NORMAL);
		break;
	case KEY_ESCAPE:
		_D ("KEY_ESCAPE");
		new_input_mode (INPUT_MENU);
		break;
	case KEY_BACKSPACE: /*0x08:*/
		if (!pos)
			break;

		/* Print cursor */
		/*
		print_character (stringdata.x + (pos + 1), stringdata.y,
			game.cursor_char, game.color_bg, game.color_bg );
		*/

		print_character (stringdata.x + (pos + 1), stringdata.y, ' ', game.color_fg, game.color_bg);

		pos--;
		buf[pos] = 0;
		break;
	default:
		if (key < 0x20 || key > 0x7f)
			break;

		if (pos >= stringdata.len)
			break;

		buf[pos++] = key;
		buf[pos] = 0;

		/* Echo */
		print_character (stringdata.x + pos, stringdata.y,
			buf[pos - 1], game.color_fg, game.color_bg);

		break;
	}

	/* SGEO */
	print_character (stringdata.x + pos+1, stringdata.y,
		(char)game.cursor_char, game.color_fg, game.color_bg);
}


void handle_keys (int key)
{
	UINT8 *p=NULL;
	int c = 0;
	static UINT8 formated_entry[256];
	int l = game.line_user_input;
	int fg = game.color_fg, bg = game.color_bg;

	setvar (V_word_not_found, 0);

	_D ("handling key: %02x", key);

	switch (key) {
	case KEY_ENTER:
		_D (("KEY_ENTER"));
		game.keypress = 0;

		/* Remove all leading spaces */
		for (p = game.input_buffer; *p && *p == 0x20; p++);

		/* Copy to internal buffer */
		for (; *p; p++) {
			/* Squash spaces */
			if (*p == 0x20 && *(p + 1) == 0x20) {
				p++;
				continue;
			}
			formated_entry[c++] = tolower (*p);
		}
		formated_entry[c++] = 0;

		/* Handle string only if it's not empty */
		if (formated_entry[0]) {
			strcpy (game.echo_buffer, game.input_buffer);
			strcpy (last_sentence, formated_entry);
			dictionary_words (last_sentence);
		}

		/* Clear to start a new line*/
		game.has_prompt = 0;
		game.input_buffer[game.cursor_pos = 0] = 0;
		_D (_D_WARN "clear lines");
	   	clear_lines (l, l + 1, bg);
		flush_lines (l, l + 1);

		break;
	case KEY_ESCAPE:
		_D (("KEY_ESCAPE"));
		new_input_mode (INPUT_MENU);
		break;
	case KEY_BACKSPACE:
		/* Ignore backspace at start of line */
		if (game.cursor_pos == 0) break;

		/* erase cursor */
		print_character (game.cursor_pos + 1, l, ' ', fg, bg);
		game.input_buffer[--game.cursor_pos] = 0;
		/* Print cursor */
		print_character (game.cursor_pos + 1, l, game.cursor_char, fg, bg );
		break;
	default:
		/* Ignore invalid keystrokes */
		if (key < 0x20 || key > 0x7f)
			break;

		/* Maximum input size reached */
		if (game.cursor_pos >= getvar (V_max_input_chars))
			break;

		game.input_buffer[game.cursor_pos++] = key;
		game.input_buffer[game.cursor_pos] = 0;

		/* echo */
		print_character (game.cursor_pos, l,
			game.input_buffer[game.cursor_pos - 1], fg, bg);

		/* Print cursor */
		print_character (game.cursor_pos + 1, l, game.cursor_char, fg, bg );

		break;
	}
}


int wait_key ()
{
	int key;

	/* clear key queue */
	while (keypress ()) { get_key (); }

	_D (_D_WARN "waiting...");
	while (42) {
		poll_timer ();		/* msdos driver -> does nothing */
		key = do_poll_keyboard ();
		if (!console_keyhandler (key)) {
			if (key == KEY_ENTER || key == KEY_ESCAPE ||
				key == BUTTON_LEFT)
				break;
		}
		console_cycle ();
	}
	return key;
}


int wait_any_key ()
{
	int key;

	/* clear key queue */
	while (keypress ()) { get_key (); }

	_D (_D_WARN "waiting...");
	while (42) {
		poll_timer ();		/* msdos driver -> does nothing */
		key = do_poll_keyboard ();
		if (!console_keyhandler (key) && key)
				break;
		console_cycle ();
	}
	return key;
}



