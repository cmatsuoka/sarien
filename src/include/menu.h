/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#ifndef __AGI_MENU_H
#define __AGI_MENU_H

#ifdef __cplusplus
extern "C"{
#endif

struct agi_menu {
	struct agi_menu	*next;		/* next along */
	struct agi_menu	*down;		/* next menu under this */
	int enabled;			/* enabled or disabled */
	int event;			/* menu event */
	char *text;			/* text of menu item */
};

void	init_menus	(void);
void	deinit_menus	(void);
void	add_menu	(char *);
void	add_menu_item	(char *, int);
void	submit_menu	(void);
void	do_menus	(void);

#ifdef __cplusplus
};
#endif

#endif
