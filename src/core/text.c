/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999,2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include "sarien.h"
#include "agi.h"
#include "gfx.h"
#include "text.h"

extern struct gfx_driver *gfx;

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
		minx  = 320;
		ofoff = foff;
		for (m = msg, x1 = y1 = 0; *m; m++) {
			if (*m >= 0x20 || *m == 1 || *m == 2 || *m == 3) {
				/* FIXME */

				if((x1!=(len-1) || x1==39) && ((y1*8)+yoff <= 192)) {
					put_text_character (l, (x1 * 8) + xoff + foff,
						(y1 * 8) + yoff, *m, fg, bg);
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

	maxx <<= 3;
	minx <<= 3;

	if (update)
		gfx->put_block (foff+xoff+minx, yoff, ofoff+xoff+maxx+7, yoff+y1*8+9);
}


void print_text (char *msg, int foff, int xoff, int yoff, int len, int fg, int bg)
{
	print_text2 (0, msg, foff, xoff, yoff, len, fg, bg);
}


void print_text_layer (char *msg, int foff, int xoff, int yoff, int len, int fg, int bg)
{
	print_text2 (1, msg, foff, xoff, yoff, len, fg, bg);
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


/* len is in characters, not pixels!!
 */
void textbox (char *message, int x, int y, int len)
{
	/* if x | y = -1, then centre the box */
	int xoff, yoff, lin;
	char *msg, *m;

	_D (("(\"%s\", %d, %d, %d)", message, x, y, len));

	if (len <= 0 || len >= 40)
		len=30;

	xoff = x;
	yoff = y;
	len--;

	m = msg = word_wrap_string (message, &len);

	for (lin = 1; *m; m++)
		if (*m == '\n')
			lin++;

	_D ((": lin=%d", lin));

	if (lin * 8 > GFX_HEIGHT)
		lin = (GFX_HEIGHT / 8);

	if (xoff == -1)
		xoff = (GFX_WIDTH - ((len + 2) * 8)) / 2;

	if (yoff == -1)
		yoff = (GFX_HEIGHT - 16 - ((lin + 2) * 8)) / 2;

	draw_box (xoff, yoff, xoff + ((len + 2) * 8), yoff + ((lin + 2) * 8),
		MSG_BOX_COLOUR, MSG_BOX_LINE, LINES);

	print_text2 (2, msg, 0, 8 + xoff, 8 + yoff, len + 1,
		MSG_BOX_TEXT, MSG_BOX_COLOUR);

	gfx->put_block (xoff, yoff, xoff + ((len + 2) * 8),
		yoff + ((lin + 2) * 8));

	free (msg);
}

void message_box (char *message, ...)
{
	char x[512];
	va_list	args;

	_D (("(message, ...)"));
	va_start (args, message);

#ifdef HAVE_VSNPRINTF
	vsnprintf (x, 510, message, args);
#else
	vsprintf (x, message, args);
#endif

	va_end (args);

	save_screen ();
	redraw_sprites ();

	/* FR:
	 * Messy...
	 */
	allow_kyb_input = FALSE;

	textbox (x, -1, -1, -1);
	message_box_key = wait_key();

	allow_kyb_input = TRUE;

	release_sprites ();
	restore_screen ();
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

        print_text (x, 0, line_status, 0, 41, STATUS_FG, STATUS_BG);
	gfx->put_block (0, 0, 319, 8);
}
