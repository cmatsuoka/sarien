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


struct agi_menu {
	struct agi_menu	*next;		/**< next along */
	struct agi_menu	*down;		/**< next menu under this */
	int enabled;			/**< option is enabled or disabled */
	int event;			/**< menu event */
	char *text;			/**< text of menu item */
};

static struct agi_menu *master_menu = NULL;
static struct agi_menu *menu = NULL;


static void draw_horizontal_menu_bar (int cur_menu, int max_menu)
{
	struct agi_menu *men = NULL;
	int col, z;

	clear_lines (0, 0, MENU_BG);

	/* draw menu titles */
	men = master_menu->next;

	col = 1;
	for (z = 0; men; z++, men = men->next) {
		_D ("searching: %s", men->text);
		if (men->text) {
			if(z == cur_menu) {
				print_text (men->text, 0, col, 0, 40,
					MENU_BG, MENU_FG);
			} else {
				print_text (men->text, 0, col, 0, 40,
					MENU_FG, MENU_BG);
			}
			col += strlen (men->text) + 1;
		}
	}

	flush_lines (0, 0);
}


static void draw_vertical_menu (int h_menu, int cur_men, int max_men)
{
	/* draw box and pulldowns. */
	int col, lin, x, y, z, l, len;
	struct agi_menu *men = master_menu->next;
	struct agi_menu *down = NULL;
	char menu[64];

	_D (_D_WARN "menu %d, cur %d, max %d", h_menu, cur_men, max_men);
	/* find which vertical menu it is */
	for (col = x = 0; x < h_menu; x++) {
		if (men->text && *men->text)
			col += 1 + strlen (men->text);
		men = men->next;
	}

	down = men->down;

	len = 0;
	lin = 1;
	men = down;

	/* scan size of this vertical menu */
	while (men) {
		if (men->text) {
			x = strlen (men->text);
			if (len < x)
				len = x;
		}
		men = men->down;
	}

	if (len > 40)
		len = 38;
	if (col + len > 40 && len < 40)
		col = 38 - len;

	draw_box (col * CHAR_COLS, lin * CHAR_LINES,
		(col + len + 2) * CHAR_COLS,
		(max_men + 2) * CHAR_LINES,
		MENU_BG, MENU_LINE, LINES);

	men = down;
	x = col + 1;
	y = lin + 1;

	for (z = 0; men; z++, y++, men = men->down) {
		l = strlen (men->text);
		memmove (menu, men->text, l);
		memset (menu + l, ' ', len - l);
		menu[len] = 0;

		if (z == cur_men)
			print_text (menu, 0, x, y, len + 2, MENU_BG, MENU_FG);
		else
			print_text (menu, 0, x, y, len + 2, MENU_FG, MENU_BG);
	}

	flush_block (col * CHAR_COLS, lin * CHAR_LINES,
		(col + len + 2) * CHAR_COLS,
		(max_men + 2) * CHAR_LINES);
}


void init_menus ()
{
	menu = calloc (1, sizeof (struct agi_menu));
	assert (menu != NULL);
	master_menu = NULL;
}


void deinit_menus ()
{
	/* struct agi_menu *m0, *m1, *m2; */

	/* free all down's then all next's */

	/* FIXME: FR: bad memory deallocation */
#if 0
	while ((m0 = menu->next)) {
		while ((m1 = m0->down)) {
			m2 = m1->down;
			if (m1->text)
				free (m1->text);
			free (m1);
			m1 = m2;
		}
		m1 = m0->next;
		if (m0->text)
			free (m0->text);
		free (m0);
		m0 = m1;
	}
#endif
	free (menu);
}


void add_menu (char *message)
{
	struct agi_menu *m1 = menu;

	_D (_D_WARN "add menu: %s", message);
	while (m1->next != NULL)
		m1 = m1->next;

	m1->next = calloc (1, sizeof(struct agi_menu));
	m1 = m1->next;

	m1->enabled = TRUE;
	m1->event = 0xFF;
	m1->text = strdup (message);
}


void add_menu_item (char *message, int code)
{
	struct agi_menu *m1 = NULL;

	_D (_D_WARN "Adding menu item: %s", message);
	for (m1 = menu; m1->next; m1 = m1->next) {}
	for (; m1->down; m1 = m1->down) {}

	m1->down = calloc (1, sizeof(struct agi_menu));
	m1 = m1->down;

	m1->enabled = TRUE;
	m1->event = code;
	m1->text = strdup (message);
}


void submit_menu ()
{
	_D (_D_WARN "Submitting menu");
	master_menu = menu;
}


int menu_keyhandler (int key)
{
	static int clock_val;
	static int h_cur_menu = 0, h_max_menu = 0;
	static int v_cur_menu = 0, v_max_menu = 0;
	static struct agi_menu *men = NULL;
	static int menu_active = FALSE;
	int i;

	if (!getflag (F_menus_work)) 
		return FALSE;

	if (!menu_active) {
		clock_val = game.clock_enabled;
		game.clock_enabled = FALSE;

		/* calc size of horizontal menu */
		h_max_menu = 0;
		for (men = master_menu->next; men; h_max_menu++, men=men->next) {}
	
 		/* calc size of vertical menus */
		v_max_menu = 0;
   		for (i = 0, men = master_menu->next; i < h_cur_menu; i++)
   			men = men->next;
   		for (v_max_menu = 0; men; v_max_menu++, men = men->down) {}
	
   		draw_horizontal_menu_bar (h_cur_menu, h_max_menu);
   		draw_vertical_menu (h_cur_menu, v_cur_menu, v_max_menu);
		menu_active = TRUE;
	}
	
    	switch (key) {
	case KEY_ESCAPE:
		_D (_D_WARN "KEY_ESCAPE");
		goto exit_menu;
    	case KEY_ENTER:
		_D (_D_WARN "KEY_ENTER");
    		men = master_menu->next;
    		for (i = 0; i < h_cur_menu; i++, men = men->next) {}
    		men = men->down;
    		for (i = 0; i < v_cur_menu; i++, men = men->down) {}
    		if (men->enabled) {
			_D ("event %d registered", men->event);
    			game.ev_scan[men->event].occured = TRUE;
    			game.ev_scan[men->event].data = men->event;
			goto exit_menu;
    		}
    		break;
    	case KEY_DOWN:
    		if (1 + v_cur_menu >= v_max_menu - 1)
			break;
    	    	v_cur_menu++;
		draw_vertical_menu (h_cur_menu, v_cur_menu, v_max_menu);
    		break;
    	case KEY_UP:
    		if(v_cur_menu <= 0)
			break;
    		v_cur_menu--;
    		draw_vertical_menu (h_cur_menu, v_cur_menu, v_max_menu);
    		break;
    	case KEY_RIGHT:
		_D ("cur=%d, max=%d", h_cur_menu, h_max_menu);
    		if (1 + h_cur_menu >= h_max_menu)
			h_cur_menu = 0;
		else
    			h_cur_menu++;

		show_pic ();

		/* calc size of vertical menus */
		for(i = 0, men = master_menu->next; i < h_cur_menu; i++)
			men=men->next;
		for (v_max_menu = 0; men; v_max_menu++, men = men->down) {}
		v_cur_menu = 0;
		draw_horizontal_menu_bar (h_cur_menu, h_max_menu);
    		draw_vertical_menu (h_cur_menu, v_cur_menu, v_max_menu);
    		break;
    	case KEY_LEFT:
    		if (h_cur_menu <= 0)
			h_cur_menu = h_max_menu - 1;
		else
    			h_cur_menu--;

		show_pic ();

		/* calc size of vertical menus */
		for (i = 0, men = master_menu->next; i < h_cur_menu; i++)
			men=men->next;
		for (v_max_menu = 0; men; v_max_menu++, men=men->down) {}
		v_cur_menu = 0;
		draw_horizontal_menu_bar (h_cur_menu, h_max_menu);
    		draw_vertical_menu (h_cur_menu, v_cur_menu, v_max_menu);
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
	struct agi_menu *m0	= NULL;
	struct agi_menu *m1	= NULL;

	/* scan all menus for event number # */

	for (m0 = menu->next; m0 != NULL; ) {
		m1 = m0->down;
		while (m1 != NULL) {
			if (m1->event != event) {
				m1 = m1->down;
			} else {
				m1->enabled = state;
				return;
			}
		}
		if (m1 == NULL)
			m0 = m0->next;
	}
}

