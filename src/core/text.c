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
#include "graphics.h"
#include "keyboard.h"
#include "text.h"


static void print_text2 (int l, char *msg, int foff, int xoff, int yoff,
	int len, int fg, int bg)
{
	char *m;
	int x1, y1;
	int maxx, minx, ofoff;
	int update;

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
		for (m = msg, x1 = y1 = 0; *m; m++) {
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


/*
 * Public functions
 */

/**
 * Print text in the Sarien screen.
 */
void print_text (char *msg, int f, int x, int y, int len, int fg, int bg)
{
	f *= CHAR_COLS;
	x *= CHAR_COLS;
	y *= CHAR_LINES;

	print_text2 (0, agi_sprintf (msg, 0), f, x, y, len, fg, bg);
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


/* CM: Ok, this is my attempt to make a good line wrapping algorithm.
 *     Sierra like, that is.
 */
char* word_wrap_string (char *mesg, int *len)
{
	char *msg, *v, *e;
	int maxc, c, l = *len;

	v = msg = strdup ((char*)mesg);
	e = msg + strlen ((char*)msg);
	maxc = 0;

	while (42) {
		while ((c = strcspn (v, "\n")) <= l) {
			if (c > maxc)
				maxc = c;
			if ((v += c + 1) >= e)
				goto end;
		}
		c = l;
		if ((v += l) >= e)
			break;
		if (*v != ' ')
			for (; *v != ' '; v--, c--);
		if (c > maxc)
			maxc = c;
		*v++ = '\n';
	}
end:
	*len = maxc;
	return msg;
}


/**
 *
 */
void close_window ()
{
	if (game.window.active)
		restore_window_area ();		/* remove window */
	game.has_window = FALSE;
	game.window.active = FALSE;
}


/* len is in characters, not pixels!!
 */
static void textbox (char *p, int y, int x, int len)
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
		len=30;

	xoff = x * CHAR_COLS;
	yoff = y * CHAR_LINES;
	len--;
	
	m = msg = word_wrap_string (agi_sprintf (p, 0), &len);

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

	if (yoff < 0)
		yoff = (GFX_HEIGHT - 2 * CHAR_LINES - h) / 2;

	game.window.active = 1;
	game.window.x1 = xoff / 2;
	game.window.y1 = yoff - 8;
	game.window.x2 = (xoff + w - 1) / 2;
	game.window.y2 = (yoff + h - 1) - 8;

	draw_box (xoff, yoff, xoff + w - 1, yoff + h - 1,
		MSG_BOX_COLOUR, MSG_BOX_LINE, LINES);

	print_text2 (2, msg, 0, CHAR_COLS + xoff, CHAR_LINES + yoff, len + 1,
		MSG_BOX_TEXT, MSG_BOX_COLOUR);

	free (msg);

	schedule_update (xoff, yoff, xoff + w, yoff + h);
	do_update ();
}

int message_box (char *p, int y, int x, int len)
{
	int k;

	textbox (p, y, x, len);
	k = wait_key ();
	close_window ();

	return k;
}

/**
 *
 */
int print (char *p, int lin, int col, int len)
{
	assert (p != NULL);

	textbox (p, lin, col, len);

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

        print_text (x, 0, game.line_status, 0, 40,
		STATUS_FG, STATUS_BG);
	flush_lines (game.line_status, game.line_status);
}


/**
 * Formats AGI string.
 * @param s  string containing the format specifier
 * @param n  logic number
 */
char *agi_sprintf (char *s, int n)
{
	static char x[256], y[256];
	char z[16], *p;
	int xx, xy;

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

				xy=99;
				if(*s=='|') {
					s++;
					xy = atoi (s);
					while (*s >= '0' && *s <= '9')
						s++;
				}
				xx=0;
				if(xy==99) {
					/* remove all leading 0' */
					/* dont remove the 3rd zero if 000 */
					while(z[xx]=='0' && xx<2) xx++;
				}
				else
					xx=3-xy;
				strcat(p, z + xx);
				break;
			case '0':
				strcat(p, object_name (atol(s)-1));
				break;
			case 'g':
				strcat(p, game.logics[0].texts[atol(s)-1]);
				break;
			case 'w':
				strcat(p, game.ego_words[atol(s)-1].word);
				break;
			case 's':
				strcat(p, game.strings[atol(s)]);
				break;
			case 'm':
				strcat(p, game.logics[n].texts[atol(s)-1]);
				break;
			default:
				break;
			}

			while (*s >= '0' && *s <= '9') s++;
			while (*p) p++;
			break;

		default:
			*p++ = *s++;
			*p = 0;
			break;
		}
	}

	p = x;
	if (strchr (x, '%') != NULL) {
		strcpy (y, x);
		p = agi_sprintf (y, n);
	}

	return p;
}

/**
 *
 */
void write_status ()
{
	char x[64];

	if (/*game.line_min_print == 0 ||*/ !game.status_line) {
		clear_lines (0, 0, 0);
		return;
	}

	sprintf (x, " Score:%i of %03i", game.vars[V_score],
		game.vars[V_max_score]);
	print_status ("%-17s             Sound:%s ", x,
		getflag(F_sound_on) ? "On " : "Off");
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

	if (c) c = 15;
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

