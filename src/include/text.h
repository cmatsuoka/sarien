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

void	message_box	(char *, ...);
void	print_status	(char *, ...);
void	textbox		(char *, int, int, int);
void	print_text	(char *, int, int, int, int, int, int);
void	print_text_layer(char *, int, int, int, int, int, int);
char	*word_wrap_string (char *, int *);
char	*agi_printf	(char *, int);

#ifdef __cplusplus
};
#endif

#endif

