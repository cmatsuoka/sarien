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

#define SOUND_PLAYING   0x01
#define WAVEFORM_SIZE   64
#define ENV_ATTACK	10000		/**< envelope attack rate */
#define ENV_DECAY       1000		/**< envelope decay rate */
#define ENV_SUSTAIN     100		/**< envelope sustain level */
#define ENV_RELEASE	7500		/**< envelope release rate */
#define NUM_CHANNELS    7		/**< number of sound channels */

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

/**
 * AGI sound note structure.
 */
struct agi_note {
	UINT8 dur_lo;			/**< LSB of note duration */
	UINT8 dur_hi;			/**< MSB of note duration */
	UINT8 frq_0;			/**< LSB of note frequency */
	UINT8 frq_1;			/**< MSB of note frequency */
	UINT8 vol;			/**< note volume */
};

/**
 * Sarien sound channel structure.
 */
struct channel_info {
#define AGI_SOUND_SAMPLE	0x0001
#define AGI_SOUND_MIDI		0x0002
#define AGI_SOUND_4CHN		0x0008
	UINT32 type;
	struct agi_note *ptr;
#ifdef USE_PCM_SOUND
	SINT16 *ins;
	SINT32 size;
	UINT32 phase;
#endif
#define AGI_SOUND_LOOP		0x0001
#define AGI_SOUND_ENVELOPE	0x0002
	UINT32 flags;
#define AGI_SOUND_ENV_ATTACK	3
#define AGI_SOUND_ENV_DECAY	2
#define AGI_SOUND_ENV_SUSTAIN	1
#define AGI_SOUND_ENV_RELEASE	0
	UINT32 adsr;
	SINT32 timer;
	UINT32 end;
	UINT32 freq;
	UINT32 vol;
	UINT32 env;
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

