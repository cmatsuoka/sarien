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
#include "graphics.h"
#include "keyboard.h"
#include "menu.h"
#include "text.h"	/* remove later */

UINT8  buffer[40];
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
int poll_keyboard ()
{
	int key = 0;

	/* If a key is ready, rip it */
	while (keypress ()) {
		key = get_key ();
		_D ("key %02x pressed", key);
	}

	return key;
}


int handle_controller (int key)
{
	int i;

	if (key == 0)
		return FALSE;

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

	if (!KEY_ASCII (key)) {
		int d = 0;
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

		if (d) {
			game.view_table[0].direction = 
				game.view_table[0].direction == d ? 0 : d;
			return TRUE;
		}
	}

	return FALSE;
}


void handle_getstring (int key)
{
	static int pos = 0;		/* Cursor position */

	if (KEY_ASCII(key) == 0)
		return;

	_D ("handling key: %02x", key);
 	
	switch (key) {
	case KEY_ENTER:
		_D ("KEY_ENTER");
		buffer[pos] = 0;
		strcpy (game.strings[stringdata.str], buffer);
		buffer[pos = 0] = 0;
		new_input_mode (INPUT_NORMAL);
		break;
	case KEY_ESCAPE:
		_D ("KEY_ESCAPE");
		new_input_mode (INPUT_MENU);
		break;
	case 0x08:
		if (!pos)
			break;

		/* Print cursor */
		print_character (stringdata.x + (pos + 1), stringdata.y,
			game.cursor_char, game.color_bg, game.color_bg );

		pos--;
		buffer[pos] = 0;
		break;
	default:
		if (key < 0x20 || key > 0x7f)
			break;

		if (pos >= stringdata.len)
			break;

		buffer[pos++] = key;
		buffer[pos] = 0;

		/* Echo */
		print_character (stringdata.x + pos, stringdata.y,
			buffer[pos - 1], game.color_fg, game.color_bg);
		break;
	}
}


void handle_keys (int key)
{
	UINT8 *p=NULL;
	int c = 0;
	static UINT8  new_line = 1;
	static UINT8  formated_entry[256];
	static UINT16 pos = 0;
	static int has_prompt = 0;

	setvar (V_word_not_found, 0);

	if (!KEY_ASCII(key)) {
		if (!has_prompt)
			print_line_prompt ();	
		has_prompt = 1;
		return;
	}

	_D ("handling key: %02x", key);
 	
	switch (key) {
	case KEY_ENTER:
		_D (("KEY_ENTER"));

		/* Remove all leading spaces */
		for (p = buffer; *p && *p == 0x20; p++);

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
			strcpy (last_sentence, formated_entry);
			dictionary_words (last_sentence);
		}

		/* Clear to start a new line*/
		new_line = 1;
		has_prompt = 0;
		buffer[pos = 0] = 0;

		break;
	case KEY_ESCAPE:
		_D (("KEY_ESCAPE"));
		new_input_mode (INPUT_MENU);
		break;
	case KEY_BACKSPACE:
		/* Ignore backspace at start of line */
		if (pos == 0) break;

		/* Print cursor */
		print_character (pos + 1, game.line_user_input,
			game.cursor_char, game.color_bg, game.color_bg);

		buffer[--pos] = 0;
		break;
	default:
		/* Ignore invalid keystrokes */
		if (key < 0x20 || key > 0x7f)
			break;

		/* Maximum input size reached */
		if (pos >= getvar (V_max_input_chars))
			break;

		buffer[pos++] = key;
		buffer[pos] = 0;

		/* echo */
		print_character (pos, game.line_user_input,
			buffer[pos - 1], game.color_fg,
			game.color_bg);
		break;
	}

	if (new_line) {
		int l = game.line_user_input;

		new_line = 0;
		_D (_D_WARN "clear lines");
	   	clear_lines (l, l + 1, game.color_bg);
		flush_lines (l, l + 1);
		print_line_prompt();
	}

	/* Print cursor */
	print_character (pos + 1, game.line_user_input, game.cursor_char,
		game.color_fg, game.color_bg );
}


int wait_key ()
{
	int key;

	_D (_D_WARN "waiting...");
	while (42) {
		poll_timer ();		/* msdos driver -> does nothing */
		key = poll_keyboard ();
		if (!console_keyhandler (key)) {
			if (key == KEY_ENTER || key == KEY_ESCAPE)
				break;
		}
		console_cycle ();
	}
	return key;
}


/**
 * Print user input prompt.
 */
void print_line_prompt ()
{
	_D (_D_WARN "input mode = %d", game.input_mode);
	if (game.input_mode == INPUT_NORMAL) {
		_D (_D_WARN "prompt = '%s'", agi_sprintf (game.strings[0],0));
		print_text (game.strings[0], 0, 0,
			game.line_user_input, 1, game.color_fg, game.color_bg);
		do_update ();	/* synchronous */
	}
}

