/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#ifndef __AGI_KEYBOARD_H
#define __AGI_KEYBOARD_H

#ifdef __cplusplus
extern "C"{
#endif

#define KEY_BACKSPACE	0x08
#define	KEY_ESCAPE	0x1B
#define KEY_ENTER	0x0D
#define KEY_UP		0x4800
#define	KEY_DOWN	0x5000
#define KEY_LEFT	0x4B00
#define KEY_STATIONARY	0x4C00
#define KEY_RIGHT	0x4D00

#define KEY_DOWN_LEFT	0x4F00
#define KEY_DOWN_RIGHT	0x5100
#define KEY_UP_LEFT	0x4700
#define KEY_UP_RIGHT	0x4900

#define KEY_PRIORITY	0xff61

#define KEY_PGUP	0x4A2D		/* keypad + */
#define KEY_PGDN	0x4E2B  	/* keypad - */
#define KEY_HOME	0x352F  	/* keypad / */
#define KEY_END		0x372A  	/* keypad * */

#define KEY_SCAN(k)	(k >> 8)
#define KEY_ASCII(k)	(k & 0xff)

extern	UINT8		scancode_table[];


void	init_words	(void);
void	clean_input	(void);
int		do_poll_keyboard	(void);
void	clean_keyboard	(void);
void	handle_keys	(int);
void	handle_getstring(int);
int	handle_controller(int);
void	get_string	(int, int, int, int);
UINT16	agi_get_keypress(void);
int	wait_key	(void);
int	wait_any_key	(void);

void print_line_prompt(void);

#ifdef __cplusplus
};
#endif

#endif /* __AGI_KEYBOARD_H */
