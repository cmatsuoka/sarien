/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include "sarien.h"
#include "agi.h"
#include "sprite.h"		/* for commit_both() */
#include "graphics.h"
#include "keyboard.h"
#include "text.h"


static void print_text2 (int l, char *msg, int foff, int xoff, int yoff, int len, int fg, int bg)
{
	int x1, y1;
	int maxx, minx, ofoff;
	int update;
	/* Note: Must be unsigned to use cyrillic characters! */
	unsigned char *m;

	/* kludge! */
	update = 1;
	if (l == 2) {
		update = l = 0;
	}

	/* FR: strings with len == 1 were not printed
	 */
	if (len == 1) {
		put_text_character (l, xoff + foff, yoff, *msg, fg, bg);
		maxx  = 1;
		minx  = 0;
		ofoff = foff;
		y1 = 0;		/* Check this */
	} else {
		maxx  = 0;
		minx  = GFX_WIDTH;
		ofoff = foff;
		for (m = (unsigned char*)msg, x1 = y1 = 0; *m; m++) {
			if (*m >= 0x20 || *m == 1 || *m == 2 || *m == 3) {
				/* FIXME */

				if((x1!=(len-1) || x1==39) && ((y1*CHAR_LINES)+yoff <= (GFX_HEIGHT - CHAR_LINES))) {
					put_text_character (l, (x1 * CHAR_COLS) + xoff + foff,
						(y1 * CHAR_LINES) + yoff, *m, fg, bg);
					if (x1>maxx)
						maxx=x1;
					if (x1<minx)
						minx=x1;
				}

				x1++;
				/* DF: changed the len-1 to len... */
				if(x1 == len && m[1] != '\n')
					y1++, x1 = foff = 0;
			} else {
				y1++;
				x1 = foff = 0;
			}
		}
	}

	if (l)
		return;

	if (maxx < minx)
		return;

	maxx *= CHAR_COLS;
	minx *= CHAR_COLS;

	if (update) {
		schedule_update (foff + xoff + minx, yoff,
			ofoff + xoff + maxx + CHAR_COLS - 1,
			yoff + y1 * CHAR_LINES + CHAR_LINES + 1);
		/* Making synchronous text updates reduces CPU load
		 * when updating status line and input area
		 */
		do_update ();
	}
}

/* len is in characters, not pixels!!
 */
static void blit_textbox (char *p, int y, int x, int len)
{
	/* if x | y = -1, then centre the box */
	int xoff, yoff, lin, h, w;
	char *msg, *m;

	_D ("x=%d, y=%d, len=%d", x, y, len);
	if (game.window.active)
		close_window ();

	if (x == 0 && y == 0 && len == 0)
		x = y = -1;

	if (len <= 0 || len >= 40)
		len = 32;

	xoff = x * CHAR_COLS;
	yoff = y * CHAR_LINES;
	len--;

	m = msg = word_wrap_string (agi_sprintf (p), &len);

	for (lin = 1; *m; m++) {
		if (*m == '\n')
			lin++;
	}

	if (lin * CHAR_LINES > GFX_HEIGHT)
		lin = (GFX_HEIGHT / CHAR_LINES);

	w = (len + 2) * CHAR_COLS;
	h = (lin + 2) * CHAR_LINES;

	if (xoff < 0)
		xoff = (GFX_WIDTH - w) / 2;
	else
		xoff -= CHAR_COLS;

	if (yoff < 0)
		yoff = (GFX_HEIGHT - 2 * CHAR_LINES - h) / 2;

	game.window.active = TRUE;
	game.window.x1 = xoff;
	game.window.y1 = yoff;
	game.window.x2 = xoff + w - 1;
	game.window.y2 = yoff + h - 1;
	game.window.buffer = malloc (w * h);
	
	_D (_D_WARN "x1=%d, y1=%d, x2=%d, y2=%d", game.window.x1,
		game.window.y1, game.window.x2, game.window.y2);

	save_block (game.window.x1, game.window.y1, game.window.x2,
		game.window.y2, game.window.buffer);

	draw_box (game.window.x1, game.window.y1, game.window.x2,
		game.window.y2, MSG_BOX_COLOUR, MSG_BOX_LINE, 2);

	print_text2 (2, msg, 0, CHAR_COLS + xoff, CHAR_LINES + yoff, len + 1,
		MSG_BOX_TEXT, MSG_BOX_COLOUR);

	free (msg);

	do_update ();
}

static void erase_textbox ()
{
	if (!game.window.active) {
		_D ("no window active");
		return;
	}

	_D (_D_WARN "x1=%d, y1=%d, x2=%d, y2=%d", game.window.x1,
		game.window.y1, game.window.x2, game.window.y2);

	restore_block (game.window.x1, game.window.y1,
		game.window.x2, game.window.y2, game.window.buffer);

	free (game.window.buffer);
	game.window.active = FALSE;

	do_update ();
}

/*
 * Public functions
 */

void draw_text (char *msg, int f, int x, int y, int len, int fg, int bg)
{
	print_text2 (0, agi_sprintf (msg), f, x, y, len, fg, bg);
}

/**
 * Print text in the Sarien screen.
 */
void print_text (char *msg, int f, int x, int y, int len, int fg, int bg)
{
	f *= CHAR_COLS;
	x *= CHAR_COLS;
	y *= CHAR_LINES;

	_D (_D_WARN "%s, %d, %d, %d, %d, %d, %d", msg, f, x, y, len, fg, bg);
	print_text2 (0, agi_sprintf (msg), f, x, y, len, fg, bg);
}

/**
 * Print text in the Sarien console.
 */
void print_text_console (char *msg, int x, int y, int len, int fg, int bg)
{
	x *= CHAR_COLS;
	y *= 10;

	print_text2 (1, msg, 0, x, y, len, fg, bg);
}

/**
 * Wrap text line to the specified width. 
 */
char* word_wrap_string (char *mesg, int *len)
{
	/* If the message has a long word (longer than 31 character) then
	 * loop in line 239 (for (; *v != ' '; v--, c--);) can wrap
	 * around 0 and write large number in c. This causes returned
	 * length to be negative (!) and eventually crashes in calling
	 * code. The fix is simple -- remove unsigned in maxc, c, l
	 * declaration.  --Vasyl
	 */
	char *msg, *v, *e;
	int maxc, c, l = *len;

	v = msg = strdup (mesg);
	e = msg + strlen (msg);
	maxc = 0;

	while (42) {
		/* _D ("[%s], %d", msg, maxc); */
		if (strchr (v, ' ') == NULL && strlen (v) > l) {
			_D (_D_CRIT "Word too long in message");
			l = strlen (v);
		}
		while ((c = strcspn (v, "\n")) <= l) {
			if (c > maxc)
				maxc = c;
			if ((v += c + 1) >= e)
				goto end;
		}
		c = l;
		if ((v += l) >= e)
			break;

		/* The same line that caused that bug I mentioned
		 * should also do another check:
		 * for (; *v != ' ' && *v != '\n'; v--, c--);
	 	 * While this does not matter in most cases, in the case of
		 * long words it caused extra \n inserted in the line
		 * preceding long word. This one is definitely non-critical;
		 * one might argue that the function is not supposed to deal
		 * with long words. BTW, that condition at the beginning of
		 * the while loop that checks word length does not make much
		 * sense -- it verifies the length of the first word but for
		 * the rest it does something odd. Overall, even with these
		 * changes the function is still not completely robust.
		 * --Vasyl
		 */
		if (*v != ' ')
			for (; *v != ' ' && *v != '\n'; v--, c--);
		if (c > maxc)
			maxc = c;
		*v++ = '\n';
	}
end:
	*len = maxc;
	return msg;
}

/**
 * Remove existing window, if any.
 */
void close_window ()
{
	_D (_D_WARN "close window");
	erase_both ();
	erase_textbox ();		/* remove window, if any */
	blit_both ();
	commit_both ();			/* redraw sprites */
	game.has_window = FALSE;
}

/**
 * Display a message box.
 * This function displays the specified message in a text box
 * centered in the screen and waits until a key is pressed.
 * @param p The text to be displayed
 */
int message_box (char *s)
{
	int k;

	erase_both ();
	blit_textbox (s, -1, -1, -1);
	blit_both ();
	k = wait_key ();
	_D (_D_WARN "wait_key returned %02x", k);
	close_window ();

	return k;
}

/**
 * Display a message box with buttons.
 * This function displays the specified message in a text box
 * centered in the screen and waits until a button is pressed.
 * @param p The text to be displayed
 * @param b NULL-terminated list of button labels
 */
int selection_box (char *m, char **b)
{
	int x, y, i, s;
	int key, active = 0;
	int rc = -1;

	erase_both ();
	blit_textbox (m, -1, -1, -1);

	x = game.window.x1 + 5 * CHAR_COLS / 2;
	y = game.window.y2 - 5 * CHAR_LINES / 2;
	s = game.window.x2 - game.window.x1 + 1 - 5 * CHAR_COLS;
	_D ("s = %d", s);

	/* Get space between buttons */
	for (i = 0; b[i]; i++) { s -= CHAR_COLS * strlen (b[i]); }
	if (i > 1) {
		_D ("s / %d = %d", i - 1, s / (i - 1));
		s /= (i - 1);
	} else {
		x += s / 2;
	}

	blit_both ();

	/* clear key queue */
	while (keypress ()) { get_key (); }

	_D (_D_WARN "waiting...");
	while (42) {
		int xx = x;
		for (i = 0; b[i]; i++) {
			draw_button (xx, y, b[i], i == active, 0);
			xx += CHAR_COLS * strlen (b[i]) + s;
		}

		poll_timer ();		/* msdos driver -> does nothing */
		key = do_poll_keyboard ();
		if (!console_keyhandler (key)) {
			switch (key) {
			case KEY_ENTER:
				rc = active;
				goto press;
			case KEY_ESCAPE:
				rc = -1;
				goto getout;
#ifdef USE_MOUSE
			case BUTTON_LEFT: {
				int xx = x;
				for (i = 0; b[i]; i++) {
					if (test_button (xx, y, b[i])) {
						rc = active = i;
						goto press;
					}
					xx += CHAR_COLS * strlen (b[i]) + s;
				}
				break; }
#endif
			case 0x09:		/* Tab */
				_D ("Focus change");
				active++;
				active %= i;
				break;
			}
		}
		console_cycle ();
	}

press:
	_D (_D_WARN "Button pressed: %d", rc);

getout:
	close_window ();

	return rc;
}

/**
 *
 */
int print (char *p, int lin, int col, int len)
{
	assert (p != NULL);

	blit_textbox (p, lin, col, len);

	if (getflag (F_output_mode)) {
		/* non-blocking window */
		setflag (F_output_mode, FALSE);
		return 1;
	}
	
	/* blocking */

	if (game.vars[V_window_reset] == 0) {
		int k;
		setvar (V_key, 0);
		k = wait_key();
		close_window ();
		return k;
	}

	/* timed window */

	_D (_D_WARN "f15==0, v21==%d => timed", getvar (21));
	game.msg_box_ticks = getvar (V_window_reset) * 10;
	setvar (V_key, 0);

	do {
		main_cycle ();
		if (game.keypress == KEY_ENTER) {
			_D (_D_WARN "KEY_ENTER");
			setvar (V_window_reset, 0);
			game.keypress = 0;
			break;
		}
	} while (game.msg_box_ticks > 0);

	setvar (V_window_reset, 0);

	close_window ();

	return 0;
}

/**
 *
 */
void print_status (char *message, ...)
{
	char x[42];
	va_list	args;

	va_start (args, message);

#ifdef HAVE_VSNPRINTF
	vsnprintf (x, 41, message, args);
#else
	vsprintf (x, message, args);
#endif

	va_end (args);

	_D (_D_WARN "fg=%d, bg=%d", STATUS_FG, STATUS_BG);
        print_text (x, 0, 0, game.line_status, 40, STATUS_FG, STATUS_BG);
}


static char *safe_strcat (char *s, const char *t)
{
	if (t != NULL)
		strcat (s, t);

	return s;
}


/**
 * Formats AGI string.
 * @param s  string containing the format specifier
 * @param n  logic number
 */
#define MAX_LEN 768
char *agi_sprintf (char *s)
{
	static char x[MAX_LEN], y[MAX_LEN];
	char z[16], *p;
	int xx, xy;

	_D ("logic %d, '%s'", game.lognum, s);
	/* turn a AGI string into a real string */
	p = x;

	for (*p = xx = xy = 0; *s; ) {
		switch (*s) {
		case '\\':
			s++;
			continue;
		case '%':
			s++;
			switch (*s++) {
			case 'v':
				xx = atoi(s);
				while (*s >= '0' && *s <= '9')
					s++;
				sprintf (z, "%03i", getvar(xx));

				xy = 99;
				if (*s=='|') {
					s++;
					xy = atoi (s);
					while (*s >= '0' && *s <= '9')
						s++;
				}
				xx = 0;
				if (xy == 99) {
					/* remove all leading 0' */
					/* dont remove the 3rd zero if 000 */
					while(z[xx]=='0' && xx<2) xx++;
				}
				else
					xx = 3 - xy;
				safe_strcat(p, z + xx);
				break;
			case '0':
				safe_strcat(p, object_name (atol(s)-1));
				break;
			case 'g':
				safe_strcat(p, game.logics[0].texts[atol(s)-1]);
				break;
			case 'w':
				safe_strcat(p, game.ego_words[atol(s)-1].word);
				break;
			case 's':
				safe_strcat(p, game.strings[atol(s)]);
				break;
			case 'm': {
				int n = game.lognum, m = atoi(s) - 1;
				if (game.logics[n].num_texts > m)
					safe_strcat(p, game.logics[n].texts[m]);
				break;
				}
			}

			while (*s >= '0' && *s <= '9') s++;
			while (*p) p++;
			break;

		default:
			assert (p < x + MAX_LEN);
			*p++ = *s++;
			*p = 0;
			break;
		}
	}

	p = x;
	if (strchr (x, '%') != NULL) {
		strcpy (y, x);
		p = agi_sprintf (y);
	}

	return p;
}

/**
 * Write the status line.
 */
void write_status ()
{
	char x[64];

#ifdef USE_CONSOLE
	if (debug.statusline) {
#ifdef USE_MOUSE
		print_status ("%3d(%03d) %3d,%3d(%3d,%3d)               ",
			getvar (0), getvar (1),
			game.view_table[0].x_pos,
			game.view_table[0].y_pos,
			WIN_TO_PIC_X(mouse.x), WIN_TO_PIC_Y(mouse.y));
#else
		print_status ("%3d(%03d) %3d,%3d                        ",
			getvar (0), getvar (1),
			game.view_table[0].x_pos,
			game.view_table[0].y_pos);
#endif
		return;
	}
#endif

	if (!game.status_line) {
		clear_lines (0, 0, 0);
		flush_lines (0, 0);
		return;
	}

	sprintf (x, " Score:%i of %-3i", game.vars[V_score],
		game.vars[V_max_score]);
	print_status ("%-17s             Sound:%s ", x,
		getflag (F_sound_on) ? "on " : "off");
}


/**
 * Clear text lines in the screen.
 * @param l1  start line
 * @param l2  end line
 * @param c   color
 */
void clear_lines (int l1, int l2, int c)
{
	/* do we need to adjust for +8 on topline?
	 * inc for endline so it matches the correct num
	 * ie, from 22 to 24 is 3 lines, not 2 lines.
	 */

	/* if (c) c = 15; */
	l1 *= CHAR_LINES;
	l2 *= CHAR_LINES;
	l2 += CHAR_LINES - 1;

	draw_rectangle (0, l1, GFX_WIDTH - 1, l2, c);
}

/**
 *
 */
void flush_lines (int l1, int l2)
{
	l1 *= CHAR_LINES;
	l2 *= CHAR_LINES;
	l2 += CHAR_LINES - 1;

	flush_block (0, l1, GFX_WIDTH - 1, l2);
}

/* end: text.c */

