/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999,2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

/*
 * Amiga sound driver by Paul Hill <paul@lagernet.clara.co.uk>
 * Based on the BSD sound driver by Claudio Matsuoka <claudio@helllabs.org>
 * and the Basilisk II Amiga AHI audio driver by Christian Bauer
 */

#include <stdio.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <devices/ahi.h>
#include <proto/exec.h>
#include <proto/ahi.h>
#include <proto/dos.h>
#include <proto/alib.h>

#include "sarien.h"
#include "sound.h"

static int ahi_init_sound (SINT16 *b);
void ahi_close_sound();

static struct sound_driver sound_ahi = {
	"amiga /dev/audio sound output",
	ahi_init_sound,
	ahi_close_sound,
};

struct Library      *AHIBase = NULL;
struct MsgPort      *AHImp     = NULL;
struct AHIRequest   *AHIio     = NULL;
BYTE                 AHIDevice = -1;

// Global variables
static ULONG ahi_id = AHI_DEFAULT_ID;			// AHI audio ID
static struct AHIAudioCtrl *ahi_ctrl = NULL;
static struct AHISampleInfo sample[2];			// Two sample infos for double-buffering
static struct Hook sf_hook;
static int play_buf = 0;						// Number of currently played buffer

// Prototypes
static __saveds __attribute__((regparm(3))) ULONG audio_callback(struct Hook *hook /*a0*/, struct AHISoundMessage *msg /*a1*/, struct AHIAudioCtrl *ahi_ctrl /*a2*/);

static SINT16 *buffer;

/*
 *  Initialization
 */

int AudioInit(void)
{
//	int audio_frames_per_block;
	printf("** AudioInit()\n");
	sample[0].ahisi_Address = sample[1].ahisi_Address = NULL;

	// Sound disabled in prefs? Then do nothing
	//	if (PrefsFindBool("nosound")) return 0;

	// AHI available?
	if (AHIBase == NULL)
	{
		printf("No AHI!\n");
		return 0;
	}

	// Initialize callback hook
	sf_hook.h_Entry = (HOOKFUNC)audio_callback;

	// Read "sound" preferences
	//	const char *str = PrefsFindString("sound");
	//	if (str)
	//		sscanf(str, "ahi/%08lx", &ahi_id);

	// Open audio control structure
	if ((ahi_ctrl = AHI_AllocAudio(
		AHIA_AudioID, ahi_id,
		AHIA_Channels, 1,
		AHIA_Sounds, 2,
		AHIA_SoundFunc, (ULONG)&sf_hook,
		TAG_END)) == NULL) {
		printf("No AHI control!\n");
		return 0;
	}

	// Prepare SampleInfos and load sounds (two sounds for double buffering)
	sample[0].ahisi_Type = AHIST_M16S;
	sample[0].ahisi_Length = BUFFER_SIZE;
	sample[0].ahisi_Address = AllocVec(BUFFER_SIZE * 2, MEMF_PUBLIC | MEMF_CLEAR);
	sample[1].ahisi_Type = AHIST_M16S;
	sample[1].ahisi_Length = BUFFER_SIZE;
	sample[1].ahisi_Address = AllocVec(BUFFER_SIZE * 2, MEMF_PUBLIC | MEMF_CLEAR);
	if (sample[0].ahisi_Address == NULL || sample[1].ahisi_Address == NULL) return 0;
	AHI_LoadSound(0, AHIST_DYNAMICSAMPLE, &sample[0], ahi_ctrl);
	AHI_LoadSound(1, AHIST_DYNAMICSAMPLE, &sample[1], ahi_ctrl);

	// Set parameters
	play_buf = 0;
	AHI_SetVol(0, 0x10000, 0x8000, ahi_ctrl, AHISF_IMM);
	AHI_SetFreq(0, 22050, ahi_ctrl, AHISF_IMM);
	AHI_SetSound(0, play_buf, 0, 0, ahi_ctrl, AHISF_IMM);

	// Everything OK
	return 1;
}


/*
 *  Deinitialization
 */

void AudioExit(void)
{
	// Free everything
	if (ahi_ctrl)
	{
		AHI_ControlAudio(ahi_ctrl, AHIC_Play, FALSE, TAG_END);
		AHI_FreeAudio(ahi_ctrl);
		ahi_ctrl = NULL;
	}

	if (sample[0].ahisi_Address)
	{
		FreeVec(sample[0].ahisi_Address);
		sample[0].ahisi_Address = NULL;
	}
	if (sample[1].ahisi_Address)
	{
		FreeVec(sample[1].ahisi_Address);
		sample[1].ahisi_Address = NULL;
	}
}


/*
 *  AHI sound callback, request next buffer
 */

static __saveds __attribute__((regparm(3))) ULONG audio_callback(struct Hook *hook /*a0*/, struct AHISoundMessage *msg /*a1*/, struct AHIAudioCtrl *ahi_ctrl /*a2*/)
{
	play_buf ^= 1;

	memcpy(sample[play_buf].ahisi_Address, buffer, BUFFER_SIZE * 2);

	// Play next buffer
	AHI_SetSound(0, play_buf, 0, 0, ahi_ctrl, 0);

	// Mix the next buffer...
	play_sound ();
	mix_sound ();

	return 0;
}


BOOL OpenAHI(void)
{
	if ((AHImp = CreateMsgPort()))
	{
		if ((AHIio = (struct AHIRequest *)CreateIORequest(AHImp,sizeof(struct AHIRequest))))
		{
			AHIio->ahir_Version = 4;
			if(!(AHIDevice = OpenDevice(AHINAME, AHI_NO_UNIT, (struct IORequest *) AHIio,NULL))) {
				AHIBase = (struct Library *) AHIio->ahir_Std.io_Device;
				return TRUE;
			}
		}
	 }
	return FALSE;
}


void CloseAHI(void)
{
	if(!AHIDevice) CloseDevice((struct IORequest *)AHIio);
	AHIDevice = -1;

	if (AHIio) DeleteIORequest((struct IORequest *)AHIio);
	AHIio = NULL;

	if (AHImp) DeleteMsgPort(AHImp);
	AHImp = NULL;
}

void __init_sound ()
{
	snd = &sound_ahi;
}


static int ahi_init_sound (SINT16 *b)
{
	buffer = b;
	printf("** ahi_init_sound()\n");

	report ("Amiga sound driver written by Paul Hill\n");

	if (OpenAHI())
	{
		printf("** AHI open\n");
		if (AudioInit())
		{
			printf("** starting audio\n");
			AHI_ControlAudio(ahi_ctrl, AHIC_Play, TRUE, TAG_END);

			return err_OK;
		}
		report ("sound_ahi: error initialising audio\n");
		return err_Unk;
	}
	report ("sound_ahi: error initialising AHI\n");
	return err_Unk;
}


void ahi_close_sound()
{
	printf("** ahi_close_sound()\n");

	Delay(10);
	AudioExit();
	Delay(10);
	CloseAHI();
}

