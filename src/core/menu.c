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
#include "sarien.h"
#include "agi.h"
#include "sprite.h"
#include "graphics.h"
#include "keyboard.h"
#include "menu.h"
#include "text.h"
#include "list.h"


struct agi_menu {
	struct list_head list;		/**< list head for menubar list */
	struct list_head down;		/**< list head for menu options */
	int index;			/**< number of menu in menubar */
	int width;			/**< width of menu in characters */
	int height;			/**< height of menu in characters */
	int col;			/**< column of menubar entry */
	int wincol;			/**< column of menu window */
	char *text;			/**< menu name */
};

struct agi_menu_option {
	struct list_head list;		/**< list head for menu options */
	int enabled;			/**< option is enabled or disabled */
	int event;			/**< menu event */
	int index;			/**< number of option in this menu */
	char *text;			/**< text of menu option */
};

static LIST_HEAD(menubar);

static int h_cur_menu;
static int v_cur_menu;


static struct agi_menu *get_menu (int i)
{
	struct list_head *h;
	struct agi_menu *m;

	list_for_each (h, &menubar, next) {
		m = list_entry (h, struct agi_menu, list);
		if (m->index == i)
			return m;
	}

	return NULL;
}

static struct agi_menu_option *get_menu_option (int i, int j)
{
	struct list_head *h;
	struct agi_menu *m;
	struct agi_menu_option *d;

	m = get_menu (i);

	list_for_each (h, &m->down, next) {
		d = list_entry (h, struct agi_menu_option, list);
		if (d->index == j)
			return d;
	}

	return NULL;
}

static void draw_menu_bar ()
{
	struct list_head *h;
	struct agi_menu *m;

#ifdef FANCY_BOX
	draw_box (0, 0, GFX_WIDTH - 1, 12, MENU_BG, MENU_LINE, 0);
#else
	clear_lines (0, 0, MENU_BG);
	flush_lines (0, 0);
#endif

	list_for_each (h, &menubar, next) {
		m = list_entry (h, struct agi_menu, list);
#ifdef FANCY_BOX
		draw_text (m->text, 0, m->col * CHAR_COLS,
			3, 40, MENU_FG, MENU_BG);
#else
		print_text (m->text, 0, m->col, 0, 40, MENU_FG, MENU_BG);
#endif
	}

}


static void draw_menu_hilite (int cur_menu)
{
	struct agi_menu *m;

	m = get_menu (cur_menu);
	_D ("[%s]", m->text);
#ifdef FANCY_BOX
	draw_box (m->col * CHAR_COLS - 4, 1, (m->col + strlen (m->text)) *
		CHAR_COLS + 2, 11, MENU_BG, MENU_LINE, 0);
	draw_text (m->text, 0, m->col * CHAR_COLS, 3, 40, MENU_FG, MENU_BG);
#else
	print_text (m->text, 0, m->col, 0, 40, MENU_BG, MENU_FG);
#endif
	flush_lines (0, 0);
}

/* draw box and pulldowns. */
static void draw_menu_option (int h_menu)
{
	struct list_head *h;
	struct agi_menu *m = NULL;
	struct agi_menu_option *d = NULL;

	/* find which vertical menu it is */
	m = get_menu (h_menu);

#ifdef FANCY_BOX
	draw_box (m->wincol * CHAR_COLS + 2,
		1 * CHAR_LINES + 4,
		(m->wincol + m->width + 2) * CHAR_COLS - 3,
		(1 + m->height + 1) * CHAR_LINES + 1 + m->height * 2,
		MENU_BG, MENU_LINE, 0);

	list_for_each (h, &m->down, next) {
		d = list_entry (h, struct agi_menu_option, list);
		draw_text (d->text, 0, (m->wincol + 1) * CHAR_COLS,
			(d->index + 2) * CHAR_LINES + d->index * 2,
			m->width + 2, MENU_FG, MENU_BG);
#else
	draw_box (m->wincol * CHAR_COLS, 1 * CHAR_LINES,
		(m->wincol + m->width + 2) * CHAR_COLS,
		(1 + m->height + 2) * CHAR_LINES, MENU_BG, MENU_LINE, 0);

	list_for_each (h, &m->down, next) {
		d = list_entry (h, struct agi_menu_option, list);
		print_text (d->text, 0, m->wincol + 1, d->index + 2,
			m->width + 2, MENU_FG, MENU_BG);
#endif
	}
}

static void draw_menu_option_hilite (int h_menu, int v_menu)
{
	struct agi_menu *m;
	struct agi_menu_option *d;

	m = get_menu (h_menu);
	d = get_menu_option (h_menu, v_menu);

#ifdef FANCY_BOX
	draw_box (m->wincol * CHAR_COLS + 4,
		(v_menu + 2) * CHAR_LINES - 2 + v_menu * 2,
		(m->wincol + m->width + 1) * CHAR_COLS + 3,
		(v_menu + 2) * CHAR_LINES + 9 + v_menu * 2,
		MENU_BG, MENU_LINE, 0);

	draw_text (d->text, 0, (m->wincol + 1) * CHAR_COLS,
		(v_menu + 2) * CHAR_LINES + v_menu * 2, m->width + 2,
		MENU_FG, MENU_BG);
#else
	print_text (d->text, 0, m->wincol + 1, v_menu + 2, m->width + 2,
		MENU_BG, MENU_FG);
#endif
}


static void new_menu_selected (i)
{
	show_pic ();
	draw_menu_bar ();
   	draw_menu_hilite (i);
    	draw_menu_option (i);
}

#ifdef USE_MOUSE
static int mouse_over_text (unsigned int line, unsigned int col, char *s)
{
	if (mouse.x < col * CHAR_COLS)
		return FALSE;

	if (mouse.x > (col + strlen (s)) * CHAR_COLS)
		return FALSE;

#ifdef FANCY_BOX
	if ((mouse.y + 4) < line * (CHAR_LINES + 2))
		return FALSE;

	if ((mouse.y + 4) >= (line + 1) * (CHAR_LINES + 2))
		return FALSE;
#else
	if (mouse.y < line * CHAR_LINES)
		return FALSE;

	if (mouse.y >= (line + 1) * CHAR_LINES)
		return FALSE;
#endif

	return TRUE;
}
#endif

static int h_index;
static int v_index;
static int h_col;
static int h_max_menu;
static int v_max_menu[10];


#if 0
static void add_about_option ()
{
	struct agi_menu *m;
	struct agi_menu_option *d;
	char text[] = "About Sarien";

	d = malloc (sizeof (struct agi_menu_option));
	d->text = strdup (text);
	d->enabled = TRUE;
	d->event = 255;
	d->index = (v_max_menu[0] += 1);

	m = list_entry (menubar.next, struct agi_menu, list);
	list_add_tail (&d->list, &m->down);
	m->height++;
	if (m->width < strlen (text))
		m->width = strlen (text);
}
#endif

/*
 * Public functions
 */

void init_menus ()
{
	h_index = 0;
	h_col = 1;
	h_cur_menu = 0;
	v_cur_menu = 0;
}


void deinit_menus ()
{
	struct list_head *h, *h2, *v, *v2;
	struct agi_menu *m = NULL;
	struct agi_menu_option *d = NULL;

	for (h = (&menubar)->prev; h != (&menubar); h = h2) {
		m = list_entry (h, struct agi_menu, list);
		h2 = h->prev;
		_D ("deiniting hmenu %s", m->text);
		for (v = (&m->down)->prev; v != (&m->down); v = v2) {
			d = list_entry (v, struct agi_menu_option, list);
			v2 = v->prev;
			_D ("  deiniting vmenu %s", d->text);
			list_del (v);
			free (d->text);
			free (d);
		}
		list_del (h);
		free (m->text);
		free (m);
	}
}


void add_menu (char *s)
{
	struct agi_menu *m;

	m = malloc (sizeof (struct agi_menu));
	m->text = strdup (s);
	while (m->text[strlen(m->text) - 1] == ' ')
		m->text[strlen(m->text) - 1] = 0;
	m->down.next = &m->down;
	m->down.prev = &m->down;
	m->width = 0;
	m->height = 0;
	m->index = h_index++;
	m->col = h_col;
	m->wincol = h_col - 1;
	v_index = 0;
	v_max_menu[m->index] = 0;
	h_col += strlen (m->text) + 1;
	h_max_menu = m->index;

	_D (_D_WARN "add menu: '%s' %02x", s, m->text[strlen(m->text)]);
	list_add_tail (&m->list, &menubar);
}


void add_menu_item (char *s, int code)
{
	struct agi_menu *m;
	struct agi_menu_option *d;
	int l;

	d = malloc (sizeof (struct agi_menu_option));
	d->text = strdup (s);
	d->enabled = TRUE;
	d->event = code;
	d->index = v_index++;

	m = list_entry (menubar.prev, struct agi_menu, list);
	m->height++;

	v_max_menu[m->index] = d->index;

	l = strlen (d->text);
	if (l > 40)
		l = 38;
	if (m->wincol + l > 38)
		m->wincol = 38 - l;
	if (l > m->width)
		m->width = l;

	_D (_D_WARN "Adding menu item: %s (size = %d)", s, m->height);
	list_add_tail (&d->list, &m->down);
}

void submit_menu ()
{
	struct list_head *h, *h2;
	struct agi_menu *m = NULL;

	_D (_D_WARN "Submitting menu");

	/* add_about_option (); */

	/* If a menu has no options, delete it */
	for (h = (&menubar)->prev; h != (&menubar); h = h2) {
		m = list_entry (h, struct agi_menu, list);
		h2 = h->prev;
		if ((&m->down)->prev == (&m->down)) {
			list_del (h);
			free (m->text);
			free (m);
			h_max_menu--;
		}
	}
}

int menu_keyhandler (int key)
{
	static int clock_val;
	static int menu_active = FALSE;
	struct agi_menu_option *d;
	struct list_head *h;
	struct agi_menu *m;
	static int button_used = 0;

	if (!getflag (F_menus_work)) 
		return FALSE;

	if (!menu_active) {
		clock_val = game.clock_enabled;
		game.clock_enabled = FALSE;
   		draw_menu_bar ();
	}

#ifdef USE_MOUSE
	/*
	 * Mouse handling
	 */
	if (mouse.button) {
		int hmenu, vmenu;

		button_used = 1;	/* Button has been used at least once */
		if (mouse.y <= CHAR_LINES) {
			/* on the menubar */
			hmenu = 0;

			list_for_each (h, &menubar, next) {
				m = list_entry (h, struct agi_menu, list);
				if (mouse_over_text (0, m->col, m->text)) {
					break;
				} else {
					hmenu++;
				}
			}
	
			if (hmenu <= h_max_menu) {
				if (h_cur_menu != hmenu) {
					v_cur_menu = -1;
					new_menu_selected (hmenu);
				}
				h_cur_menu = hmenu;
			}
		} else {
			/* not in menubar */
			struct agi_menu_option *d;

			vmenu = 0;

			m = get_menu (h_cur_menu);
			list_for_each (h, &m->down, next) {	
				d = list_entry (h, struct agi_menu_option, list);
				if (mouse_over_text (2 + d->index,
					m->wincol + 1, d->text))
				{
					break;
				} else {
					vmenu++;
				}
			}

			if (vmenu <= v_max_menu[h_cur_menu]) {
				if (v_cur_menu != vmenu) {
					draw_menu_option (h_cur_menu);
					draw_menu_option_hilite (h_cur_menu,
						vmenu);
				}
				v_cur_menu = vmenu;
			}
		}
	} else if (button_used) {
		/* Button released */
		button_used = 0;

		_D (_D_WARN "button released!");

		if (v_cur_menu < 0)
			v_cur_menu = 0;

		draw_menu_option_hilite (h_cur_menu, v_cur_menu);

		if (mouse.y <= CHAR_LINES) {
			/* on the menubar */
		} else {
			/* see which option we selected */
			m = get_menu (h_cur_menu);
			list_for_each (h, &m->down, next) {	
				d = list_entry (h, struct agi_menu_option, list);
				if (mouse_over_text (2 + d->index, m->wincol + 1, d->text)) {
					/* activate that option */
					if (d->enabled) {
						_D ("event %d registered", d->event);
	    					game.ev_scan[d->event].occured = TRUE;
	    					game.ev_scan[d->event].data = d->event;
						goto exit_menu;
					}
				}
			}
			goto exit_menu;
		}
	}
#endif /* USE_MOUSE */

	if (!menu_active) {
   		if (h_cur_menu >= 0) {
			draw_menu_hilite (h_cur_menu);
			draw_menu_option (h_cur_menu);
   			if (!button_used && v_cur_menu >= 0)
   				draw_menu_option_hilite(h_cur_menu, v_cur_menu);
		}
		menu_active = TRUE;
	}

    	switch (key) {
	case KEY_ESCAPE:
		_D (_D_WARN "KEY_ESCAPE");
		goto exit_menu;
    	case KEY_ENTER:
		_D (_D_WARN "KEY_ENTER");
		d = get_menu_option (h_cur_menu, v_cur_menu);
		if (d->enabled) {
			_D ("event %d registered", d->event);
    			game.ev_scan[d->event].occured = TRUE;
    			game.ev_scan[d->event].data = d->event;
			goto exit_menu;
    		}
    		break;
    	case KEY_DOWN:
    	case KEY_UP:
    	    	v_cur_menu += key == KEY_DOWN ? 1 : -1;

		if (v_cur_menu < 0)
			v_cur_menu = 0;
		if (v_cur_menu > v_max_menu[h_cur_menu])
			v_cur_menu = v_max_menu[h_cur_menu];

		draw_menu_option (h_cur_menu);
		draw_menu_option_hilite (h_cur_menu, v_cur_menu);
    		break;
    	case KEY_RIGHT:
    	case KEY_LEFT:
		h_cur_menu += key == KEY_RIGHT ? 1 : -1;
		
		if (h_cur_menu < 0)
			h_cur_menu = h_max_menu;
		if (h_cur_menu > h_max_menu)
			h_cur_menu = 0;

		v_cur_menu = 0;
		new_menu_selected (h_cur_menu);
    		draw_menu_option_hilite (h_cur_menu, v_cur_menu);
    		break;
    	}

	return TRUE;

exit_menu:
	button_used = 0;
	show_pic ();
	write_status ();

	setvar (V_key, 0);
	game.keypress = 0;
	game.clock_enabled = clock_val;
	old_input_mode ();
	_D (_D_WARN "exit_menu: input mode reset to %d", game.input_mode);
	menu_active = FALSE;

	return TRUE;
}


void menu_set_item (int event, int state)
{
	struct list_head *h, *v;
	struct agi_menu *m = NULL;
	struct agi_menu_option *d = NULL;

	/* scan all menus for event number # */

	list_for_each (h, &menubar, next) {
		m = list_entry (h, struct agi_menu, list);
		list_for_each (v, &m->down, next) {	
			d = list_entry (v, struct agi_menu_option, list);
			if (d->event == event) {
				d->enabled = state;
				return;
			}
		}
	}
}

/* end of file: menu.c */

