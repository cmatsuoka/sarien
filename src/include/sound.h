/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#ifndef __AGI_SOUND_H
#define __AGI_SOUND_H

#ifdef __cplusplus
extern "C"{
#endif

#define BUFFER_SIZE	410

#define SOUND_EMU_NONE	0
#define SOUND_EMU_PC	1
#define SOUND_EMU_TANDY	2
#define SOUND_EMU_MAC	3
#define SOUND_EMU_AMIGA	4

/**
 * Sarien sound driver structure.
 */
struct sound_driver {
	char *description;
	int (*init)(SINT16 *buffer);
	void (*deinit)(void);
};

/**
 * AGI sound resource structure.
 */
struct agi_sound {
	UINT32	flen;			/**< size of raw data */
	UINT8	*rdata;			/**< raw sound data */
	UINT8	flags;			/**< sound flags */
	UINT16	type;			/**< sound resource type */
};

void	decode_sound		(int);
void	unload_sound		(int);
void	play_sound		(void);
int	init_sound		(void);
void	deinit_sound		(void);
void	start_sound		(int, int);
void	stop_sound		(void);
UINT32	mix_sound		(void);
void	__init_sound		(void);
int	load_instruments	(char *fname);

extern	struct sound_driver	*snd;

#ifdef __cplusplus
};
#endif

#endif /* __AGI_SOUND_H */

