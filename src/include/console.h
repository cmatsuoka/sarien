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

#ifdef	__cplusplus
extern "C" {
#endif

#ifdef USE_CONSOLE

#define CONSOLE_LINES_BUFFER    80
#define CONSOLE_LINE_SIZE	(GFX_WIDTH / 8)
#define	CONSOLE_ACTIVATE_KEY	'`'
#define CONSOLE_SWITCH_KEY	'~'

struct sarien_console {
	int active;
	int input_active;
	int index;
	int y;
	int max_y;
	int first_line;
	int count;
	char *line[CONSOLE_LINES_BUFFER];
};

struct sarien_debug {
	int enabled;
	int opcodes;
	int logic0;
	int steps;
	int priority;
	int statusline;
};

extern struct sarien_console console;

#endif /* USE_CONSOLE */

int	console_keyhandler	(int);
int	console_init	(void);
void	console_cycle	(void);
void	console_lock	(void);
void	console_prompt	(void);
void	report		(char *, ...);

#ifdef __cplusplus
}
#endif

#endif /* __AGI_CONSOLE_H */
