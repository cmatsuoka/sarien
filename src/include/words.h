/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#ifndef __AGI_WORDS_H
#define __AGI_WORDS_H

#ifdef __cplusplus
extern "C"{
#endif

struct agi_word {
	int id;
	char *word;
};

int	show_words	(void);
int	load_words	(char *);
void	unload_words	(void);
int	find_word	(char *);
void	dictionary_words(char *);


#ifdef __cplusplus
};
#endif

#endif /* __AGI_WORDS_H */

