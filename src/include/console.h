/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#ifndef __AGI_CONSOLE_H
#define __AGI_CONSOLE_H

#ifdef USE_CONSOLE

#define CONSOLE_LINES_ONSCREEN	20
#define CONSOLE_LINES_BUFFER	128
#define CONSOLE_PROMPT		"$"
#define CONSOLE_CURSOR_HOLLOW	1
#define CONSOLE_CURSOR_SOLID	2
#define CONSOLE_CURSOR_EMPTY	3
#define CONSOLE_COLOR		14

#define	CONSOLE_ACTIVATE_KEY	'`'
#define CONSOLE_SWITCH_KEY	'~'
#define CONSOLE_SCROLLUP_KEY	KEY_PGUP
#define CONSOLE_SCROLLDN_KEY	KEY_PGDN
#define CONSOLE_START_KEY	KEY_HOME
#define CONSOLE_END_KEY		KEY_END

#define CONSOLE_INPUT_SIZE	39

struct sarien_console {
	int active;
	int input_active;
	int index;
	int y;
	int first_line;
	int input_key;
	int count;
	char *line[CONSOLE_LINES_BUFFER];
};

#ifdef	__cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

#endif /* USE_CONSOLE */

int	console_init	(void);
void	console_cycle	(void);
void	console_lock	(void);
void	console_prompt	(void);
void	report		(char *, ...);

#endif /* __AGI_CONSOLE_H */
