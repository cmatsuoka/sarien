/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999,2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#include "sarien.h"

#ifdef USE_CONSOLE

/*
 * The console has been added to sarien in version 0.4.9. The main
 * interpreter engine has not been designed to have a console, and a few
 * kludges were needed to make the console work. First of all, the main
 * interpreter loop works at cycle level, not instruction level. To keep
 * the illusion that the console is threaded, a console updating function
 * is called from loops inside instructions such as get.string().
 */ 
 
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "agi.h"
#include "graphics.h"
#include "sprite.h"
#include "text.h"
#include "keyboard.h"
#include "opcodes.h"
#include "console.h"

#define CONSOLE_LINES_ONSCREEN	20
#define CONSOLE_PROMPT		"$"
#define CONSOLE_CURSOR_HOLLOW	1
#define CONSOLE_CURSOR_SOLID	2
#define CONSOLE_CURSOR_EMPTY	3
#define CONSOLE_COLOR		14
#define CONSOLE_SCROLLUP_KEY	KEY_PGUP
#define CONSOLE_SCROLLDN_KEY	KEY_PGDN
#define CONSOLE_START_KEY	KEY_HOME
#define CONSOLE_END_KEY		KEY_END
#define CONSOLE_INPUT_SIZE	39


struct console_command {
	struct console_command *next;
	char *cmd;
	char *dsc;
	void (*handler)(void);
};

struct sarien_console console;
struct sarien_debug debug;

static struct console_command *ccmd_head = NULL;
static UINT8 has_console;
static UINT8 console_input = 0;

static char *_p0, *_p1, *_p2, *_p3, *_p4, *_p5;	/* FIXME: array */
static char _p[80];
static int _pn;


/*
 * Console command parsing
 */

static UINT8 console_parse (char *b)
{
	struct console_command *d;
	UINT8 i;

	for (; *b && *b == ' '; b++) {}	/* eat spaces */
	for (; *b && b[strlen(b) - 1] == ' '; b[strlen(b) - 1] = 0) {}
	if (!*b)
		return 0;

	/* get or set flag/var values */
	if ((b[0] == 'f' || b[0] == 'v') && isdigit (b[1])) {
		char *e;
		int f = (int)strtoul (&b[1], &e, 10);
		if (*e == 0) {
			if (f >= 0 && f <= 255) {
				switch (b[0]) { 
				case 'f':
					report (getflag (f) ?
						"TRUE\n" : "FALSE\n");
					break;
				case 'v':
					report ("%3d\n", getvar (f));
					break;
				}
				return 0;
			}
			return -1;
		} else if (*e == '=') {
			int n = (int)strtoul (e + 1, NULL, 0);
			switch (b[0]) {
			case 'f':
				setflag (f, !!n);
				break;
			case 'v':
				setvar (f, n);
				break;
			}
			return 0;
		}
	}

	/* tokenize the input line */
	if (strchr (b, ' '))
		strcpy (_p, strchr (b, ' ') + 1);
	_p0 = strtok (b, " ");
	_p1 = strtok (NULL, " ");
	_p2 = strtok (NULL, " ");
	_p3 = strtok (NULL, " ");
	_p4 = strtok (NULL, " ");
	_p5 = strtok (NULL, " ");

	/* set number of parameters. ugh, must rewrite this later */
	_pn = 5;
	if (!_p5) _pn = 4;
	if (!_p4) _pn = 3;
	if (!_p3) _pn = 2;
	if (!_p2) _pn = 1;
	if (!_p1) _pn = 0;

	for (d = ccmd_head; d; d = d->next) {
		if (!strcmp (_p0, d->cmd) && d->handler) {
			d->handler ();
			return 0;
		}
	}

	for (i = 0; logic_names_cmd[i].name; i++) {
		if (!strcmp (_p0, logic_names_cmd[i].name)) {
			UINT8 p[16];
			if (_pn != logic_names_cmd[i].num_args) {
				report ("AGI command wants %d arguments\n",
					logic_names_cmd[i].num_args);
				return 0;
			}
			p[0] = _p1 ? (char)strtoul (_p1, NULL, 0) : 0;
			p[1] = _p2 ? (char)strtoul (_p2, NULL, 0) : 0; 
			p[2] = _p3 ? (char)strtoul (_p3, NULL, 0) : 0;
			p[3] = _p4 ? (char)strtoul (_p4, NULL, 0) : 0;
			p[4] = _p5 ? (char)strtoul (_p5, NULL, 0) : 0;

			execute_agi_command (i, p); 

			return 0;
		}
	}

	return -1;
}


/*
 * Console commands
 */

static void ccmd_help ()
{
	struct console_command *d;

	if (!_p1) {
		report ("Command Description\n");
		report ("------- --------------------------------\n");
		for (d = ccmd_head; d; d = d->next)
			report ("%-8s%s\n", d->cmd, d->dsc);
		return;
	}

	for (d = ccmd_head; d; d = d->next) {
		if (!strcmp (d->cmd, _p1) && d->handler) { 
			report ("%s\n", d->dsc);
			return;
		}
	}

	report ("Unknown command or no help available\n");

	return;
}

static void ccmd_ver ()
{
	report (VERSION "\n");
	return;
}

static void ccmd_crc ()
{
	report ("0x%05x\n", game.crc);
	return;
}

static void ccmd_load ()
{
	stop_sound ();

	if (game.state >= STATE_LOADED) {
		report ("AGI game already loaded.\n");
		return;
	}

	report ("Opening \"%s\".\n", _p1);
	if (agi_detect_game (_p1) == err_OK) {
		game.state = STATE_RUNNING;
		report ("AGI game successfully loaded.\n");
		agi_init ();
		console_prompt ();
		return;
	}
	
	report ("AGI game load failed.\n");
	return;
}

static void ccmd_quit ()
{
	deinit_video ();
	deinit_machine ();
	exit (0);
}

#if 0
static void ccmd_exec ()
{
	if (game.state < STATE_LOADED) {
		report ("No game loaded.\n");
		return;
	}

	if (game.state >= STATE_RUNNING) {
		report ("Game already running.\n");
		return;
	}

	report ("Executing AGI game.\n");
	game.state = STATE_RUNNING;
}
#endif

#ifdef USE_HIRES

static void ccmd_hires ()
{
	if (_pn != 1 || (strcmp (_p1, "on") && strcmp (_p1, "off"))) {
		report ("Usage: hires on|off\n");
		return;
	}

	opt.hires = !strcmp (_p1, "on");
	erase_both ();	
	show_pic ();
	blit_both ();
	return;
}

#endif

static void ccmd_agiver ()
{
	int ver, maj, min;

	ver = agi_get_release ();
	maj = (ver >> 12) & 0xf;
	min = ver & 0xfff;

	report (maj == 2 ? "%x.%03x\n" : "%x.002.%03x\n", maj, min);
	return;
}

static void ccmd_flags ()
{
	int i, j;

	report ("    ");
	for (j = 0; j < 10; j++)
		report ("%d ", j);
	report ("\n");

	for (i = 0; i < 255;) {
		report ("%3d ", i);
		for (j = 0; j < 10; j++, i++) {
			report ("%c ", getflag (i) ? 'T' : 'F');
		}
		report ("\n");
	}
	return;
}

static void ccmd_vars ()
{
	int i, j;

	for (i = 0; i < 255;) {
		for (j = 0; j < 5; j++, i++) {
			report ("%03d:%3d ", i, getvar (i));
		}
		report ("\n");
	}
	return;
}

#if 0

static void ccmd_say ()
{
	setflag (F_entered_cli, TRUE);
	dictionary_words (_p);
}

static void ccmd_inv ()
{
	unsigned int i, j;

	for (j = i = 0; i < game.num_objects; i++) {
		if (object_get_location (i) == EGO_OWNED) {
			report ("%3d]%-16.16s", i, object_name (i));
			if (j % 2)
				report ("\n");
			j++;
		}
	}
	if (j == 0) {
		report ("none\n");
		return;
	}
	if (j % 2)
		report ("\n");
}

#endif

static void ccmd_objs ()
{
	unsigned int i;

	for (i = 0; i < game.num_objects; i++) {
		report ("%3d]%-24s(%3d)\n", i, object_name (i),
			object_get_location (i));
	}
	return;
}

static void ccmd_opcode ()
{
	if (_pn != 1 || (strcmp (_p1, "on") && strcmp (_p1, "off"))) {
		report ("Usage: opcode on|off\n");
		return;
	}

	debug.opcodes = !strcmp (_p1, "on");
	return;
}

static void ccmd_logic0 ()
{
	if (_pn != 1 || (strcmp (_p1, "on") && strcmp (_p1, "off"))) {
		report ("Usage: logic0 on|off\n");
		return;
	}

	debug.logic0 = !strcmp (_p1, "on");
	return;
}

static void ccmd_step ()
{
	debug.enabled = 1;

	if (_pn == 0) {
		debug.steps = 1;
		return;
	}

	debug.steps = strtoul (_p1, NULL, 0);
	return;
}

static void ccmd_debug ()
{
	debug.enabled = 1;
	debug.steps = 0;
	return;
}

static void ccmd_cont ()
{
	debug.enabled = 0;
	debug.steps = 0;
	return;
}


/*
 * Register console commands
 */
static void console_cmd (char *cmd, char *dsc, void (*handler)(void))
{
	struct console_command *c, *d;

	c = malloc (sizeof (struct console_command));
	c->cmd = strdup (cmd);
	c->dsc = strdup (dsc);
	c->handler = handler;
	c->next = NULL;

	if (ccmd_head == NULL) {
		ccmd_head = c;
		return;
	}

	for (d = ccmd_head; d->next; d = d->next) {}
	d->next = c;
}


/* Console reporting */

/* A slightly modified strtok() for report() */
static char *get_token (char *s, char d)
{
	static char *x;
	char *n, *m;

	if (s)
		x = s;

	m = x;
	n = strchr (x, d);

	if (n) {
		*n = 0;
		x = n + 1;
	} else {
		x = strchr (x, 0);
	}

	return m;
}


static void build_console_lines (int n)
{
	int i, j, y1;

	clear_console_screen (GFX_HEIGHT - n * 10);

	for (j = CONSOLE_LINES_ONSCREEN - n; j < CONSOLE_LINES_ONSCREEN; j++) {
		i = console.first_line + j;
		print_text_console (console.line[i], 0, j,
			strlen (console.line[i]) + 1, CONSOLE_COLOR, 0);
	}

	y1 = console.y - n * 10;
	if (y1 < 0)
		y1 = 0;
	
	/* CM:
	 * This will cause blinking when using menus+console, but this
	 * function is called by report() which can be called from any
	 * point with or without sprites placed. If we protect the
	 * flush_block() call with redraw/release sprites cloning will
	 * happen when activating the console. This could be fixed by
	 * keeping a control flag in redraw/release to not do the
	 * actions twice, but I don't want to do that -- not yet.
	 */
	flush_block (0, y1, GFX_WIDTH - 1, console.y);
}


/*
 * Public functions
 */

int console_init ()
{
	console_cmd ("agiver", "Show emulated Sierra AGI version", ccmd_agiver);
	console_cmd ("cont",   "Resume interpreter execution", ccmd_cont);
	console_cmd ("debug",  "Stop interpreter execution", ccmd_debug); 
	console_cmd ("crc",    "Show AGI game CRC", ccmd_crc); 
#if 0
	console_cmd ("exec",   "Execute loaded AGI game", ccmd_exec);
#endif
	console_cmd ("flags",  "Dump all AGI flags", ccmd_flags);
	console_cmd ("help",   "List available commands", ccmd_help);
#ifdef USE_HIRES
	console_cmd ("hires",  "Turn hi-res mode on/off", ccmd_hires);
#endif
	console_cmd ("logic0", "Turn logic 0 debugging on/off", ccmd_logic0);
	console_cmd ("load",   "Load AGI game", ccmd_load);
	console_cmd ("objs",   "List all objects and locations", ccmd_objs);
	console_cmd ("opcode", "Turn opcodes on/off in debug", ccmd_opcode);
	console_cmd ("quit",   "Quit the interpreter", ccmd_quit);
#if 0
	console_cmd ("inv",    "List current inventory", ccmd_inv);
	console_cmd ("say",    "Pass argument to the AGI parser", ccmd_say);
#endif
	console_cmd ("step",   "Execute the next AGI instruction", ccmd_step);
	console_cmd ("vars",   "Dump all AGI variables", ccmd_vars);
	console_cmd ("ver",    "Show interpreter version", ccmd_ver);

	console.active = 1;
	console.input_active = 1;
	console.index = 0;
	console.max_y = 150;
	console.y = console.max_y;
	console.first_line = CONSOLE_LINES_BUFFER - CONSOLE_LINES_ONSCREEN;

	debug.enabled = 0;
	debug.opcodes = 0;
	debug.logic0 = 1;
	debug.priority = 0;

	has_console = 1;

	return err_OK;
}

static void build_console_layer ()
{
	build_console_lines (console.max_y / 10);
}


void report (char *message, ...)
{
	char x[512], y[512], z[64], *m, *n;
	va_list	args;
	int i, s, len;

	va_start (args, message);

#ifdef HAVE_VSNPRINTF
	vsnprintf (y, 510, (char*)message, args);
#else
	vsprintf (y, (char*)message, args);
#endif

	va_end (args);

	if (!has_console) {
#ifndef NATIVE_MACOSX
		fprintf (stderr, "%s", y);
#endif
		return;
	}

	if (console_input) {
		strcpy (z, console.line[CONSOLE_LINES_BUFFER - 1]);
		strcpy (x, ">");
	} else {
		strcpy (x, console.line[CONSOLE_LINES_BUFFER - 1]);
	}

	strcat (x, y);

	len = 40;
	m = n = word_wrap_string (x, &len);

	for (s = 1; *n; n++)
		if (*n == '\n')
			s++;

	/* Scroll console */
	for (i = 0; i < (s - 1); i++)
		free (console.line[i]);
	for (i = 0; i < (CONSOLE_LINES_BUFFER - (s - 1)); i++)
		console.line[i] = console.line[i + (s - 1)];
	console.line[CONSOLE_LINES_BUFFER - s] = strdup (get_token (m, '\n'));
	for (i = 1; i < s; i++) {
		n = get_token (NULL, '\n');
		console.line[CONSOLE_LINES_BUFFER - s + i] = strdup (n);
	}

	console.first_line = CONSOLE_LINES_BUFFER - CONSOLE_LINES_ONSCREEN;

	if (console_input) {
		free (console.line[CONSOLE_LINES_BUFFER - 1]);
		console.line[CONSOLE_LINES_BUFFER - 1] = strdup (z);
	}

	/* Build layer */
	if (console.y) {
		if (s > 1)
			build_console_layer ();
		else
			build_console_lines (1);
	}

	do_update ();
}



void console_cycle ()
{
	static int old_y = 0;
	static int blink = 0;
	static char cursor[] = "  ";

	/* If no game has been loaded, keep the console visible! */
	if (game.state < STATE_RUNNING) {
		console.active = 1;
		console.count = 10;
	}

	/* Initial console auto-hide timer */
	if (console.count > 0)
		console.count--;
	if (console.count == 0) {
		console.active = 0;
		console.count = -1;
	}

	if (console.active) {
		if (console.y < console.max_y)
			console.y += 15;
		else
			console.y = console.max_y;
	} else {
		console.count = -1;

		if (console.y > 0)
			console.y -= 15;
		else
			console.y = 0;
	}

	/* console shading animation */
	if (old_y != console.y) {
		int y = console.y;
		if (old_y > console.y) {
			/* going up */
			y = old_y;
		} 
		flush_block (0, 0, GFX_WIDTH - 1, y);
		old_y = console.y;
	}

	blink++;
	blink %= 16;
	if (console.input_active) {
		*cursor = blink < 8 ?
			CONSOLE_CURSOR_SOLID : CONSOLE_CURSOR_EMPTY;
	} else {
		*cursor = CONSOLE_CURSOR_HOLLOW;
	}
	if (console.y > 0 && console.first_line == CONSOLE_LINES_BUFFER -
		CONSOLE_LINES_ONSCREEN)
	{
		SINT16 y1 = console.y - 10, y2 = console.y - 1;
		if (y1 < 0) y1 = 0;
		if (y2 < 0) y2 = 0;
		print_text_console (cursor,
			(1 + console.index), 19, 2, CONSOLE_COLOR, 0);
		flush_block ((1 + console.index) * 8, y1,
			(1 + console.index) * 8 + 7, y2 - 1);
	}

	do_update ();
}


/* Return TRUE if key was handled */
int console_keyhandler (int k)
{
	static char buffer[CONSOLE_INPUT_SIZE];
	SINT16 y1,y2;
	char m[2];

	/* Right button switches console on/off*/
	if (k == BUTTON_RIGHT)
		k = CONSOLE_ACTIVATE_KEY;

	if (!console.active) {
		if (k == CONSOLE_ACTIVATE_KEY) {
			console.active = 1;
			return TRUE;
		}
		return FALSE;
	}

	if (!console.input_active) {
		if (k == CONSOLE_SWITCH_KEY) {
			console.input_active = 1;
			return TRUE;
		}
		if (k == CONSOLE_ACTIVATE_KEY) {
			console.active = 0;
			return TRUE;
		}
		return FALSE;
	}

	y1 = console.y - 10;
	y2 = console.y - 1;

	if (y1 < 0) y1 = 0;
	if (y2 < 0) y2 = 0;

	/* Ignore left button in console */
	if (k == BUTTON_LEFT)
		return TRUE;

	if (k) {
		if (k != KEY_ENTER && console.first_line != CONSOLE_LINES_BUFFER -
			CONSOLE_LINES_ONSCREEN) {
			console.first_line = CONSOLE_LINES_BUFFER -
				CONSOLE_LINES_ONSCREEN;
			build_console_layer ();
		}
		console.count = -1;
	}

	switch (k) {
	case KEY_ENTER:
		console_lock ();
		console.index = 0;
		report ("\n");

		if (console_parse (buffer) != 0)
			report ("What? Where?\n");

		buffer[0] = 0;
		console_prompt ();
		break;
	case KEY_BACKSPACE:
       		if (!console.index)
			break;
		console.line[CONSOLE_LINES_BUFFER-1][console.index]=0;
		*m = CONSOLE_CURSOR_EMPTY;
		print_text_console (m, (console.index+1), 19, 2,
			CONSOLE_COLOR, 0);
		flush_block ((console.index+1)*8, y1, (console.index+1)*8+7, y2);
       		console.index--;
       		buffer[console.index] = 0;
		break;
	case CONSOLE_ACTIVATE_KEY:
		console.active = !console.active;
		if (console.active)
			build_console_layer ();
		break;
	case CONSOLE_SWITCH_KEY:
		console.count = -1;
		if (console.y)
			console.input_active = !console.input_active;
		break;
	case CONSOLE_SCROLLUP_KEY:
		console.count = -1;
		if (!console.y)
			break;
		if (console.first_line > (CONSOLE_LINES_ONSCREEN / 2))
			console.first_line -= CONSOLE_LINES_ONSCREEN / 2;
		else
			console.first_line = 0;
		build_console_layer ();
		break;
	case CONSOLE_SCROLLDN_KEY:
		console.count = -1;
		if (!console.y)
			break;
		if (console.first_line < (CONSOLE_LINES_BUFFER -
			CONSOLE_LINES_ONSCREEN - CONSOLE_LINES_ONSCREEN / 2))
			console.first_line += CONSOLE_LINES_ONSCREEN / 2;
		else
			console.first_line = CONSOLE_LINES_BUFFER -
				CONSOLE_LINES_ONSCREEN;
		build_console_layer ();
		break;
	case CONSOLE_START_KEY:
		console.count = -1;
		if (console.y)
			console.first_line = 0;
		break;
	case CONSOLE_END_KEY:
		console.count = -1;
		if (console.y)
			console.first_line = CONSOLE_LINES_BUFFER -
				CONSOLE_LINES_ONSCREEN;
		break;
	default:
        	if (k >= 0x20 && k <= 0x7f && (console.index < CONSOLE_INPUT_SIZE - 2))
		{
			char l[42];
			buffer[console.index] = k;
			*m=k;m[1]=0;
			console.index++;

			sprintf (l, "%s%c",
				console.line[CONSOLE_LINES_BUFFER-1],k);
			free (console.line[CONSOLE_LINES_BUFFER-1]);
			console.line[CONSOLE_LINES_BUFFER-1] = strdup(l);

			buffer[console.index] = 0;
			print_text_console (m, console.index, 19, 2,
				CONSOLE_COLOR, 0);
			flush_block (console.index*8, y1, console.index*8+7, y2);
		}
               	break;
	}

	do_update ();

	return TRUE;
}

void console_prompt ()
{
	report (CONSOLE_PROMPT);
	console_input = 1;
}


void console_lock ()
{
	console_input = 0;
}


#else

void *debug;

void report (char *message, ...)
{
	/* dummy */
}

int console_init ()
{
	return 0;
}

void console_cycle ()
{
	/* dummy */
}

void console_lock ()
{
	/* dummy */
}

void console_prompt ()
{
	/* dummy */
}

int console_keyhandler (int i)
{
	return FALSE;
}

#endif /* USE_CONSOLE */
