/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999,2001 Stuart George and Claudio Matsuoka
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

#define SOUND_PLAYING	0x01
#define BUFFER_SIZE	410
#define WAVEFORM_SIZE	64
#define ENV_DECAY	800
#define ENV_SUSTAIN	160
#define NUM_CHANNELS	16
#define USE_INTERPOLATION

#define SOUND_EMU_NONE	0
#define SOUND_EMU_PC	1
#define SOUND_EMU_TANDY	2
#define SOUND_EMU_MAC	3
#define SOUND_EMU_AMIGA	4

struct agi_note {
	UINT8 dur_lo;
	UINT8 dur_hi;
	UINT8 frq_0;
	UINT8 frq_1;
	UINT8 vol;
};

struct channel_info {
#define AGI_SOUND_SAMPLE	0x0001
#define AGI_SOUND_MIDI		0x0002
#define AGI_SOUND_4CHN		0x0008
	UINT32 type;
	struct agi_note *ptr;
	SINT16 *ins;
	SINT32 size;
#define AGI_SOUND_LOOP		0x0001
#define AGI_SOUND_ENVELOPE	0x0002
	UINT32 flags;
	SINT32 timer;
	UINT32 end;
	UINT32 freq;
	UINT32 phase;
	UINT32 vol;
	UINT32 env;
};

struct sound_driver {
	char *description;
	int (*init)(SINT16 *buffer);
	void (*deinit)(void);
};

struct agi_sound {
	UINT32	flen;			/* size of raw data */
	UINT8	*rdata;			/* raw sound data */
	UINT8	flags;			/* sound flags */
	UINT16	type;			/* sound resource type */
};

extern	struct sound_driver	*snd;
//extern  struct sound_driver	sound_dummy;
//extern	struct agi_sound	sounds[];

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

#ifdef __cplusplus
};
#endif

#endif

