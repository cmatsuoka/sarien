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

#include "sarien.h"
#include "agi.h"
#include "gfx_agi.h"
#include "gfx_base.h"
#include "keyboard.h"
#include "menu.h"
#include "text.h"


struct agi_menu {
	struct agi_menu	*next;		/* next along */
	struct agi_menu	*down;		/* next menu under this */
	int enabled;			/* enabled or disabled */
	int event;			/* menu event */
	char *text;			/* text of menu item */
};

static struct agi_menu *master_menu;
static struct agi_menu *menu;

extern struct sarien_console console;
extern struct agi_game game;
extern struct gfx_driver *gfx;


static void draw_horizontal_menu_bar (int cur_menu, int max_menu)
{
	struct agi_menu *men;
	int cx, cy, z;

	/* draw our empty title bar */
	for (cy = 0; cy < 8; cy++)
		for (cx = 0; cx < GFX_WIDTH; cx++)
			put_pixel (cx, cy, MENU_BG);

	/* draw menu titles */
	men = master_menu->next;

	cx = 8;
	for (z = 0; men; z++, men = men->next) {
		if (men->text) {
			if(z == cur_menu) {
				print_text (men->text, 0, cx, 0, 40,
					MENU_BG, MENU_FG);
			} else {
				print_text (men->text, 0, cx, 0, 40,
					MENU_FG, MENU_BG);
			}
			cx += (1 + strlen ((char*)men->text)) * 8;
		}
	}

	gfx->put_block (0, 0, 320, 8);
}


static void draw_vertical_menu (int h_menu, int cur_men, int max_men)
{
	/* draw box and verticle pulldowns. */
	int cx, cy, x, y, z, l, len;
	struct agi_menu *men = master_menu->next, *down;
	char menu[64];

	/* find which vertical menu it is */
	for (cx = x = 0; x < h_menu; x++) {
		if (men->text && *men->text)
			cx += 1 + strlen (men->text);
		men = men->next;
	}

	down = men->down;

	len = 0;
	men = down;
	/* scan size of this vertical menu */
	while (men) {
		if (men->text) {
			x = strlen (men->text);
			if (len < x)
				len = x;
		}
		men=men->down;
	}

	if (len > 40)
		len = 38;
	if (cx + len > 40 && len < 40)
		cx = 38 - len;

	cx *= 8;
	cy = 8;
	draw_box (cx, cy, cx + ((2 + len)<<3), ((2 + max_men)<<3),
		MENU_BG, MENU_LINE, LINES, 0);

	men = down;
	x = cx + 8;
	y = cy + 8;

	for (z = 0; men; z++, y += 8, men = men->down) {
		l = strlen (men->text);
		memmove (menu, men->text, l);
		memset (menu + l, ' ', len - l);
		menu[len] = 0;

		if (z == cur_men)
			print_text (menu, 0, x, y, len + 2, MENU_BG, MENU_FG);
		else
			print_text (menu, 0, x, y, len + 2, MENU_FG, MENU_BG);
	}

	gfx->put_block (cx, cy, cx + ((2 + len)<<3), (2 + max_men) << 3);
}


void init_menus ()
{
	menu = calloc (1, sizeof (struct agi_menu));
	master_menu = NULL;
}


void deinit_menus ()
{
	/* struct agi_menu *m0, *m1, *m2; */

	/* free all down's then all next's */

	/* FR:
         * FIXME: Fatal error while freeing the memory
	 */
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
	struct agi_menu *m1;

	for (m1 = menu; m1->next; m1 = m1->next);
	for (; m1->down; m1 = m1->down);

	m1->down = calloc (1, sizeof(struct agi_menu));
	m1 = m1->down;

	m1->enabled = TRUE;
	m1->event = code;
	m1->text = strdup (message);
}


void submit_menu ()
{
	master_menu = menu;
}


void do_menus ()
{
	int h_cur_menu = 0, h_max_menu = 0;
	int v_cur_menu = 0, v_max_menu = 0;
	int x;
	struct agi_menu *men;
	int clock_val;

	clock_val = game.clock_enabled;
	game.clock_enabled = FALSE;

	release_sprites ();
	save_screen ();
	redraw_sprites ();
	//gfx->put_block (0, 0, 319, console.y);
	put_screen ();

	/* calc size of horizontal menu */
	for (men = master_menu->next; men; h_max_menu++, men=men->next);

 	/* calc size of vertical menus */
   	for (x = 0, men = master_menu->next; x < h_cur_menu; x++)
   		men = men->next;
   	for (v_max_menu = 0; men; v_max_menu++, men = men->down);

   	draw_horizontal_menu_bar (h_cur_menu, h_max_menu);
   	draw_vertical_menu (h_cur_menu, v_cur_menu, v_max_menu);

	while (42) {
		main_cycle (FALSE);

    		switch (key) {
    		case KEY_ESCAPE:
			goto exit_menu;
    		case KEY_ENTER:
    			men = master_menu->next;
    			for (x = 0; x < h_cur_menu; x++, men = men->next);
    			men = men->down;
    			for (x = 0; x < v_cur_menu; x++, men=men->down);
    			if (men->enabled) {
    				game.events[men->event].occured = TRUE;
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
    			if (1 + h_cur_menu >= h_max_menu)
				break;
    			h_cur_menu++;
			release_sprites ();
			restore_screen_area ();
			redraw_sprites ();
			//gfx->put_block (0, 0, 319, console.y);
			put_screen ();

			/* calc size of vertical menus */
			for(x = 0, men = master_menu->next; x<h_cur_menu; x++)
				men=men->next;
			for (v_max_menu = 0; men; v_max_menu++, men=men->down);
			v_cur_menu = 0;
			draw_horizontal_menu_bar (h_cur_menu, h_max_menu);
    			draw_vertical_menu (h_cur_menu, v_cur_menu, v_max_menu);
    			break;
    		case KEY_LEFT:
    			if (h_cur_menu <= 0)
				break;
    			h_cur_menu--;
			release_sprites ();
			restore_screen_area ();
			redraw_sprites ();
			//gfx->put_block (0, 0, 319, console.y);
			put_screen ();

			/* calc size of vertical menus */
			for (x = 0, men = master_menu->next; x<h_cur_menu; x++)
				men=men->next;
			for (v_max_menu = 0; men; v_max_menu++, men=men->down);
			v_cur_menu = 0;
			draw_horizontal_menu_bar (h_cur_menu, h_max_menu);
    			draw_vertical_menu (h_cur_menu, v_cur_menu, v_max_menu);
    			break;
    		}
    	}
exit_menu:

	release_sprites ();
	restore_screen ();
	redraw_sprites ();

	setvar (V_key, 0);
	game.clock_enabled = clock_val;
}


void menu_set_item (int event, int state)
{
	struct agi_menu *m0, *m1;

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

