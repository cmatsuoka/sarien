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
#include <assert.h>
#include "sarien.h"
#include "agi.h"
#include "sprite.h"
#include "graphics.h"
#include "keyboard.h"
#include "menu.h"
#include "text.h"
#include "list.h"


struct menu {
	struct list_head list;
	struct list_head down;
	int index;
	int width;
	int height;
	int col;
	int wincol;
	char *text;			/**< text of menu item */
};

struct menu_option {
	struct list_head list;
	int enabled;			/**< option is enabled or disabled */
	int event;			/**< menu event */
	int index;
	char *text;			/**< text of menu item */
};

static LIST_HEAD(menubar);


static struct menu *get_menu (int i)
{
	struct list_head *h;
	struct menu *m;

	list_for_each (h, &menubar, next) {
		m = list_entry (h, struct menu, list);
		if (m->index == i)
			return m;
	}

	return NULL;
}

static struct menu_option *get_menu_option (int i, int j)
{
	struct list_head *h;
	struct menu *m;
	struct menu_option *d;

	m = get_menu (i);

	list_for_each (h, &m->down, next) {
		d = list_entry (h, struct menu_option, list);
		if (d->index == j)
			return d;
	}

	return NULL;
}

static void draw_menu_bar ()
{
	struct list_head *h;
	struct menu *m;
	int c, i;

	clear_lines (0, 0, MENU_BG);

	i = 0; c = 1;
	list_for_each (h, &menubar, next) {
		m = list_entry (h, struct menu, list);
		print_text (m->text, 0, m->col, 0, 40, MENU_FG, MENU_BG);
	}

	flush_lines (0, 0);
}


static void draw_menu_hilite (int cur_menu)
{
	struct menu *m;

	m = get_menu (cur_menu);
	_D ("[%s]", m->text);
	print_text (m->text, 0, m->col, 0, 40, MENU_BG, MENU_FG);
	flush_lines (0, 0);
}

/* draw box and pulldowns. */
static void draw_menu_option (int h_menu)
{
	struct list_head *h;
	struct menu *m = NULL;
	struct menu_option *d = NULL;

	/* find which vertical menu it is */
	m = get_menu (h_menu);

	draw_box (m->wincol * CHAR_COLS, 1 * CHAR_LINES,
		(m->wincol + m->width + 2) * CHAR_COLS,
		(1 + m->height + 2) * CHAR_LINES,
		MENU_BG, MENU_LINE, LINES);

	list_for_each (h, &m->down, next) {
		d = list_entry (h, struct menu_option, list);
		print_text (d->text, 0, m->wincol + 1, d->index + 2,
			m->width + 2, MENU_FG, MENU_BG);
	}
}

static void draw_menu_option_hilite (int h_menu, int v_menu)
{
	struct menu *m;
	struct menu_option *d;

	m = get_menu (h_menu);
	d = get_menu_option (h_menu, v_menu);
	print_text (d->text, 0, m->wincol + 1, v_menu + 2, m->width + 2,
		MENU_BG, MENU_FG);
}


static void new_menu_selected (i)
{
	show_pic ();
	draw_menu_bar ();
   	draw_menu_hilite (i);
    	draw_menu_option (i);
}

static int h_index;
static int v_index;
static int h_col;
static int h_max_menu;
static int v_max_menu[10];


/*
 * Public functions
 */

void init_menus ()
{
	h_index = 0;
	h_col = 1;
}


void deinit_menus ()
{
}


void add_menu (char *s)
{
	struct menu *m;

	_D (_D_WARN "add menu: %s", s);
	
	m = malloc (sizeof (struct menu));
	m->text = strdup (s);
	m->down.next = &m->down;
	m->down.prev = &m->down;
	m->width = 0;
	m->height = 0;
	m->index = h_index++;
	m->col = h_col;
	m->wincol = h_col - 1;
	v_index = 0;
	v_max_menu[m->index] = 0;
	h_col += strlen (s) + 1;
	h_max_menu = m->index;

	while (m->text[strlen(m->text)] == ' ')
		m->text[strlen(m->text)] = 0;

	list_add_tail (&m->list, &menubar);
}


void add_menu_item (char *s, int code)
{
	struct menu *m;
	struct menu_option *d;
	int l;

	d = malloc (sizeof (struct menu_option));
	d->text = strdup (s);
	d->enabled = TRUE;
	d->event = code;
	d->index = v_index++;

	m = list_entry (menubar.prev, struct menu, list);
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
	_D (_D_WARN "Submitting menu");
}

int menu_keyhandler (int key)
{
	static int clock_val;
	static int h_cur_menu = 0;
	static int v_cur_menu = 0;
	static int menu_active = FALSE;
	struct menu_option *d;

	if (!getflag (F_menus_work)) 
		return FALSE;

	if (!menu_active) {
		clock_val = game.clock_enabled;
		game.clock_enabled = FALSE;
   		draw_menu_bar ();
	}

	/*
	 * Mouse handling
	 */
	if (mouse.button) {
		struct list_head *h;
		struct menu *m;
		int hmenu, vmenu;

		if (mouse.y <= CHAR_LINES) {
			hmenu = 0;

			list_for_each (h, &menubar, next) {
				m = list_entry (h, struct menu, list);
				if (mouse.x > m->col * CHAR_COLS &&
					mouse.x < (m->col + strlen(m->text)) *
					CHAR_COLS)
				{
					break;
				} else {
					hmenu++;
				}
			}
	
			if (hmenu <= h_max_menu) {
				if (h_cur_menu != hmenu) {
					v_cur_menu = 0;
					new_menu_selected (hmenu);
				}
				h_cur_menu = hmenu;
			}
		} else {
			struct menu_option *d;

			vmenu = 0;

			m = get_menu (h_cur_menu);
			list_for_each (h, &m->down, next) {	
				d = list_entry (h, struct menu_option, list);
				if (mouse.x > (m->wincol + 1) * CHAR_COLS &&
					mouse.x < (m->wincol + strlen (m->text))
						* CHAR_COLS &&
					mouse.y >=(2 + d->index) * CHAR_LINES &&
					mouse.y < (3 + d->index) * CHAR_LINES)
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
	}
#if 0
	else {
		goto exit_menu;
	}
#endif

	if (!menu_active) {
 		/* calc size of vertical menus */
   		if (h_cur_menu >= 0) {
			draw_menu_hilite (h_cur_menu);
   			draw_menu_option (h_cur_menu);
			if (v_cur_menu >= 0)
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
	struct menu *m = NULL;
	struct menu_option *d = NULL;

	/* scan all menus for event number # */

	list_for_each (h, &menubar, next) {
		m = list_entry (h, struct menu, list);
		list_for_each (v, &m->down, next) {	
			d = list_entry (v, struct menu_option, list);
			if (d->event == event) {
				d->enabled = state;
				return;
			}
		}
	}
}

/* end of file: menu.c */

