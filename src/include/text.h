/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#ifndef __AGI_TEXT_H
#define __AGI_TEXT_H

#ifdef __cplusplus
extern "C"{
#endif

int	message_box	(char *);
int	selection_box	(char *, char **);
void	close_window	(void);
void	draw_window	(int, int, int, int);
void	print_text	(char *, int, int, int, int, int, int);
void	print_text_console
			(char *, int, int, int, int, int);
int	print		(char *, int, int, int);
char	*word_wrap_string (char *, int *);
char	*agi_sprintf	(char *);
void	write_status	(void);
void	write_prompt	(void);
void	clear_lines	(int, int, int);
void	flush_lines	(int, int);

#ifdef __cplusplus
};
#endif

#endif /* __AGI_TEXT_H */
