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
#include <proto/ahi.h>
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
static struct MsgPort      *AHImp     = NULL;
static struct AHIRequest   *AHIio     = NULL;
static BYTE                 AHIDevice = -1;
static struct AHIAudioCtrl *actrl     = NULL;


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
}

#if 0


#define MAXSAMPLES 16

#define INT_FREQ   50

APTR samples[MAXSAMPLES] = { 0 };

LONG mixfreq = 0;

/* Prototypes */


/******************************************************************************
** PlayerFunc *****************************************************************
******************************************************************************/

__asm __interrupt __saveds static void PlayerFunc(
    register __a0 struct Hook *hook,
    register __a2 struct AHIAudioCtrl *actrl,
    register __a1 APTR ignored) {

  int i;
  
  for(i = 0; i < CHANNELS; i++) {

    if(channelstate[i].FadeVolume) {

      channelstate[i].Volume = (channelstate[i].Volume * 90) / 100; // Fade volume

      if(channelstate[i].Volume == 0) {
        channelstate[i].FadeVolume = FALSE;
      }

      AHI_SetVol(i, channelstate[i].Volume, channelstate[i].Position,
          actrl, AHISF_IMM);
    }
  }
  return;
}

struct Hook PlayerHook = {
  0,0,
  (ULONG (* )()) PlayerFunc,
  NULL,
  NULL,
};

/* Ask user for an audio mode and allocate it */

BOOL AllocAudio(void) {
  struct AHIAudioModeRequester *req;
  BOOL   rc = FALSE;

  req = AHI_AllocAudioRequest(
      AHIR_PubScreenName, NULL,
      AHIR_TitleText,     "Select a mode and rate",
      AHIR_DoMixFreq,     TRUE,
      TAG_DONE);

  if(req) {
    if(AHI_AudioRequest(req, TAG_DONE)) {
      actrl = AHI_AllocAudio(
          AHIA_AudioID,         req->ahiam_AudioID,
          AHIA_MixFreq,         req->ahiam_MixFreq,
          AHIA_Channels,        CHANNELS,
          AHIA_Sounds,          MAXSAMPLES,
          AHIA_PlayerFunc,      &PlayerHook,
          AHIA_PlayerFreq,      INT_FREQ<<16,
          AHIA_MinPlayerFreq,   INT_FREQ<<16,
          AHIA_MaxPlayerFreq,   INT_FREQ<<16,
          TAG_DONE);
      if(actrl) {
        // Get real mixing frequency
        AHI_ControlAudio(actrl, AHIC_MixFreq_Query, &mixfreq, TAG_DONE);
        rc = TRUE;
      }
    }
    AHI_FreeAudioRequest(req);
  }
  return rc;
}


/******************************************************************************
**** LoadSample ***************************************************************
******************************************************************************/

/* Load a (raw) 8 or 16 bit sample from disk. The sample ID is returned
   (or AHI_NOSOUND on error). */

UWORD LoadSample(char *filename, ULONG type) {
  struct AHISampleInfo sample;
  APTR *samplearray = samples;
  UWORD id = 0, rc = AHI_NOSOUND;
  BPTR file;

  // Find a free sample slot

  while(*samplearray) {
    id++;
    samplearray++;
    if(id >= MAXSAMPLES) {
      return AHI_NOSOUND;
    }
  }

  file = Open(filename, MODE_OLDFILE);
  
  if(file) {
    int length;

    Seek(file, 0, OFFSET_END);
    length = Seek(file, 0, OFFSET_BEGINNING);
    *samplearray = AllocVec(length, MEMF_PUBLIC);
    if(*samplearray) {
      Read(file, *samplearray, length);

      sample.ahisi_Type = type;
      sample.ahisi_Address = *samplearray;
#if USE_AHI_V4
      sample.ahisi_Length = length / AHI_SampleFrameSize(type);
#else
      sample.ahisi_Length = length / (type == AHIST_M16S ? 2 : 1);
#endif
      if(! AHI_LoadSound(id, AHIST_SAMPLE, &sample, actrl)) {
        rc = id;
      }
    }
    Close(file);
  }
  return rc;
}


/******************************************************************************
**** UnloadSample *************************************************************
******************************************************************************/

void UnloadSample(UWORD id) {

  AHI_UnloadSound(id, actrl);
  FreeVec(samples[id]);
  samples[id] = NULL;
}

/******************************************************************************
**** main *********************************************************************
******************************************************************************/

int main() {

        // Start feeding samples to sound hardware
        if(!(AHI_ControlAudio(actrl, AHIC_Play, TRUE, TAG_DONE))) {

          AHI_Play(actrl,
            // A forever-looping sample on channel 0
            AHIP_BeginChannel,  0,
            AHIP_Freq,          17640,
            AHIP_Vol,           0x10000,
            AHIP_Pan,           0xc000,
            AHIP_Sound,         louise,
            AHIP_EndChannel,    NULL,

            // A oneshot sample on channel 1
            AHIP_BeginChannel,  1,
            AHIP_Freq,          22254,
            AHIP_Vol,           0x10000,
            AHIP_Pan,           0x4000,
            AHIP_Sound,         ahem,
            AHIP_LoopSound,     AHI_NOSOUND,
            AHIP_EndChannel,    NULL,

            TAG_DONE);


          channelstate[0].Volume   = 0x10000;
          channelstate[0].Position = 0xc000;
          channelstate[1].Volume   = 0x10000;
          channelstate[1].Position = 0x4000;

          // Wait 5 seconds
          
          Delay(5 * TICKS_PER_SECOND);


        }
      }
    }
  }

  return 0;
}

#endif
