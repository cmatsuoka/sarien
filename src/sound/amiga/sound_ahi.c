/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#include <devices/ahi.h>
#include <exec/exec.h>
#if 0
#include <proto/ahi.h>
#endif
#include <proto/dos.h>
#include <proto/exec.h>
#include "sarien.h"
#include "console.h"
#include "sound.h"

static int ahi_init_sound (SINT16 *buffer);
static void ahi_close_sound (void);

static struct sound_driver sound_ahi = {
	"AHI sound driver",
	ahi_init_sound,
	ahi_close_sound
};


static struct Library      *AHIBase;
static struct MsgPort      *AHImp    = NULL;
static struct AHIRequest   *AHIio    = NULL;
static BYTE                AHIDevice = -1;
static struct AHIAudioCtrl *actrl    = NULL;
static BOOL                DBflag    = FALSE;	/**< double buffer flag */
static ULONG               BUFFER_SIZE = NULL;



static struct AHISampleInfo Sample0 = {
	AHIST_M16S,
	NULL,
	NULL,
};
 
static struct AHISampleInfo Sample1 = {
	AHIST_M16S,
	NULL,
	NULL,
};
 
__asm __saveds ULONG SoundFunc (
	register __a0 struct Hook *hook,
	register __a2 struct AHIAudioCtrl *actrl,
	register __a1 struct AHISoundMessage *smsg)
{
	if (DBflag = !DBflag)		/* Flip and test */
		AHI_SetSound (0, 1, 0, 0, actrl, NULL);
	else
		AHI_SetSound (0, 0, 0, 0, actrl, NULL);

	play_sound ();
	mix_sound ();
	/* FIXME: copy sound to buffer */
	
	Signal (actrl->ahiac_UserData, (1L<<signal));
	return NULL;
}

static struct Hook SoundHook = {
  0,0,
  (ULONG (* )()) SoundFunc,
  NULL,
  NULL,
};

void __init_sound ()
{
	snd = &sound_ahi;
}


static int ahi_init_sound (SINT16 *buffer)
{
	report ("sound_ahi: initializing AHI\n");

	AHImp = CreateMsgPort();
	if (AHImp == NULL) {
		report ("sound_ahi: error: CreateMsgPort()\n");
		goto err1;
	}

	AHIio = (struct AHIRequest *)CreateIORequest (AHImp,
		sizeof (struct AHIRequest)));
	if (AHIio == NULL) {
		report ("sound_ahi: error: CreateIORequest()\n");
		goto err2;
	}
	
	AHIio->ahir_Version = 4;

	AHIDevice = OpenDevice (AHINAME, AHI_NO_UNIT,
		(struct IORequest *)AHIio, NULL);
	if (AHIDevice != 0) {
		report ("sound_ahi: error: OpenDevice()\n");
		goto err3;
	}

	AHIBase = (struct Library *)AHIio->ahir_Std.io_Device;

	actrl = AHI_AllocAudio(
		AHIA_AudioID,   AHI_DEFAULT_ID,
		AHIA_MixFreq,   22050,
		AHIA_Channels,  1,
		AHIA_Sounds,    2,
		AHIA_SoundFunc, &SoundHook,
		AHIA_UserData,	FindTask (NULL),
		TAG_DONE);

	Sample0.ahisi_Length = BUFFER_SIZE;
	Sample0.ahisi_Address = AllocVec (BUFFER_SIZE * 2,
		MEMF_PUBLIC | MEMF_CLEAR);

	Sample1.ahisi_Length = BUFFER_SIZE;
	Sample1.ahisi_Address = AllocVec (BUFFER_SIZE * 2,
		MEMF_PUBLIC | MEMF_CLEAR);

	return 0;

err3:
	DeleteIORequest ((struct IORequest *)AHIio);
err2:
	DeleteMsgPort (AHImp);
err1:
	AHI_FreeAudio (actrl);
	return -1;
}

static void ahi_close_sound ()
{
	Flush (Output ());
	AHI_ControlAudio(actrl, AHIC_Play, FALSE, TAG_DONE);
	AHI_FreeAudio (actrl);
	CloseDevice((struct IORequest *)AHIio);
	DeleteIORequest ((struct IORequest *)AHIio);
	DeleteMsgPort (AHImp);
	FreeVec (Sample1);
	FreeVec (Sample0);
}

