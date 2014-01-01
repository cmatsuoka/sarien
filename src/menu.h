/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2003 Stuart George and Claudio Matsuoka
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

#define MENU_BG		0x0f		/* White */
#define MENU_DISABLED	0x07		/* Grey */

#define MENU_FG		0x00		/* Black */
#define MENU_LINE	0x00		/* Black */

void	menu_init	(void);
void	menu_deinit	(void);
void	menu_add	(char *);
void	menu_add_item	(char *, int);
void	menu_submit	(void);
void	menu_set_item	(int, int);
int	menu_keyhandler	(int);
void	menu_enable_all	(void);

#ifdef __cplusplus
};
#endif

#endif
