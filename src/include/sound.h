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

typedef struct SOUND_DRIVER
{
	char *description;
	int (*init)(SINT16 *buffer);
	void (*deinit)(void);
} SOUND_DRIVER;

typedef struct AGI_SOUND
{
	UINT32	flen;			/* size of raw data */
	UINT8	*rdata;			/* raw sound data */
	UINT8	flags;			/* sound flags */
	UINT16	type;			/* sound resource type */
} AGI_SOUND;

typedef struct AGI_ENVELOPE
{
	UINT8 bp;
	UINT8 inc_hi;
	UINT8 inc_lo;
} ENVELOPE;

typedef struct AGI_WAVELIST
{
	UINT8 top;
	UINT8 addr;
	UINT8 size;
	UINT8 mode;
	UINT8 rel_hi;
	UINT8 rel_lo;
} WAVELIST;

typedef struct AGI_INSTRUMENT
{
	ENVELOPE env[8];
	UINT8 relseg;
	UINT8 priority;
	UINT8 bendrange;
	UINT8 vibdepth;
	UINT8 vibspeed;
	UINT8 spare;
	UINT8 wac;
	UINT8 wbc;
	WAVELIST wal[8];
	WAVELIST wbl[8];
} AGI_INSTRUMENT;

typedef struct AGI_IIGS_SAMPLE
{
	UINT8 type_lo;
	UINT8 type_hi;
	UINT8 srate_lo;
	UINT8 srate_hi;
	UINT16 unknown[2];
	UINT8 size_lo;
	UINT8 size_hi;
	UINT16 unknown2[13];
} AGI_IIGS_SAMPLE;


extern	SOUND_DRIVER	*snd;
extern	AGI_SOUND	sounds[];
extern  SOUND_DRIVER	sound_dummy;

extern	void	decode_sound(UINT16 resnum);
extern	void	unload_sound(UINT16 resnum);
extern	void	play_sound (void);
extern	int	init_sound (void);
extern	void	deinit_sound (void);
extern	void	start_sound (UINT16, UINT16);
extern	void	stop_sound (void);
extern	UINT32	mix_sound (void);
extern	void	__init_sound (void);
extern	UINT16	load_instruments(UINT8 *fname);

#ifdef __cplusplus
};
#endif

#endif

