/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999,2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#ifndef __AGI_KEYBOARD
#define __AGI_KEYBOARD

#include "words.h"


#ifdef __cplusplus
extern "C"{
#endif

struct agi_event {
	UINT8	event;
	UINT8	occured;
	UINT16	data;
};


#define	KEY_ESCAPE	0x1B
#define KEY_ENTER	0x0D
#define KEY_UP		0x4800
#define	KEY_DOWN	0x5000
#define KEY_LEFT	0x4B00
#define KEY_RIGHT	0x4D00

#define KEY_DOWN_LEFT	0x4F00
#define KEY_DOWN_RIGHT	0x5100
#define KEY_UP_LEFT	0x4700
#define KEY_UP_RIGHT	0x4900

#define KEY_PGUP	0xff55	/* FIXME -- these are X11 codes,*/
#define KEY_PGDN	0xff56  /*          must change to PC keyb */
#define KEY_HOME	0xff57  /*          scancodes! */
#define KEY_END		0xff58

#define KEY_SCAN(k)	(k >> 8)
#define KEY_ASCII(k)	(k & 0xff)


extern	UINT8		scancode_table[];
extern	UINT16		key;

//extern	struct agi_events	events[];
extern	struct agi_word	ego_words[];
extern	int		num_ego_words;
extern	UINT8		strings[MAX_WORDS1][MAX_WORDS2];


void	init_words	(void);
void	clean_input	(void);
void	poll_keyboard	(void);
void	clean_keyboard	(void);
void	handle_keys	(void);
void	handle_console_keys	(void);
UINT8	*get_string	(int, int, int);
UINT16	agi_get_keypress(void);
void	move_ego	(UINT8);
int	wait_key	(void);

void print_line_prompt(void);

#ifdef __cplusplus
};
#endif
#endif
