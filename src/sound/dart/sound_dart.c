/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

/*
 * OS/2 DART driver based on Kevin Langman's DART driver for xmp
 */

#define INCL_DOS
#include <os2.h>
#include <mcios2.h>
#include <meerror.h>
#include <os2medef.h>
#include "sarien.h"
#include "console.h"
#include "sound.h"

static int dart_init_sound (SINT16 *buffer);
static void dart_close_sound (void);

static struct sound_driver sound_dart = {
	"Direct Audio Realtime",
	dart_init_sound,
	dart_close_sound,
};


#define BUFFERCOUNT 4
#define BUF_MIN 8
#define BUF_MAX 32

static	MCI_MIX_BUFFER      MixBuffers[BUFFERCOUNT];
static	MCI_MIXSETUP_PARMS  MixSetupParms;
static	MCI_BUFFER_PARMS    BufferParms;
static	MCI_GENERIC_PARMS   GenericParms;

static ULONG DeviceID = 0;
static int bsize = 16;
static short next = 2;
static short ready = 1;

static HMTX mutex;


void __init_sound ()
{
	snd = &sound_dart;
}


static int dart_init_sound (SINT16 *buffer)
{
	int i, flags, device = 0;

	MCI_AMP_OPEN_PARMS AmpOpenParms;

	if ((bsize < BUF_MIN || bsize > BUF_MAX) && bsize != 0) {
		bsize = 16 * 1024;
	} else {
		bsize *= 1024;
	}

	MixBuffers[0].pBuffer = NULL;			/* marker */
	memset (&GenericParms,0,sizeof (MCI_GENERIC_PARMS));

	/* open AMP device */
	memset(&AmpOpenParms,0,sizeof(MCI_AMP_OPEN_PARMS));
	AmpOpenParms.usDeviceID=0;

	AmpOpenParms.pszDeviceType =
		(PSZ) MAKEULONG (MCI_DEVTYPE_AUDIO_AMPMIX, (USHORT)device); 

	flags = MCI_WAIT | MCI_OPEN_TYPE_ID | MCI_OPEN_SHAREABLE;

	if (mciSendCommand (0,MCI_OPEN, flags,
		(PVOID)&AmpOpenParms,0) != MCIERR_SUCCESS)
	{
		return -1;
	}

	DeviceID=AmpOpenParms.usDeviceID;

	/* setup playback parameters */
	memset(&MixSetupParms,0,sizeof(MCI_MIXSETUP_PARMS));

	MixSetupParms.ulBitsPerSample  = 16;
	MixSetupParms.ulFormatTag      = MCI_WAVE_FORMAT_PCM;
	MixSetupParms.ulSamplesPerSec  = 22050;
	MixSetupParms.ulChannels       = 1;
	MixSetupParms.ulFormatMode     = MCI_PLAY;
	MixSetupParms.ulDeviceType     = MCI_DEVTYPE_WAVEFORM_AUDIO;
	MixSetupParms.pmixEvent        = OS2_Dart_UpdateBuffers;

	if (mciSendCommand(DeviceID,MCI_MIXSETUP, MCI_WAIT | MCI_MIXSETUP_INIT,
		(PVOID)&MixSetupParms,0)!=MCIERR_SUCCESS)
	{
		mciSendCommand (DeviceID, MCI_CLOSE, MCI_WAIT,
			(PVOID)&GenericParms,0);
		return -1;
	}
        
	/* take in account the DART suggested buffer size... */
	if( bsize == 0 ){
    		bsize=MixSetupParms.ulBufferSize;
	}

	BufferParms.ulNumBuffers = BUFFERCOUNT;
	BufferParms.ulBufferSize = bsize;
	BufferParms.pBufList = MixBuffers;

	if (mciSendCommand (DeviceID, MCI_BUFFER,
		MCI_WAIT | MCI_ALLOCATE_MEMORY,
		(PVOID)&BufferParms,0) != MCIERR_SUCCESS)
	{
		mciSendCommand (DeviceID, MCI_CLOSE, MCI_WAIT,
			(PVOID)&GenericParms,0);
		return -1;
	}

	for (i = 0 ; i < BUFFERCOUNT ; i++ ){
		MixBuffers[i].ulBufferLength = bsize;		
	}

	/* Start Playback */
	memset (MixBuffers[0].pBuffer, /*32767*/0, bsize);
	memset (MixBuffers[1].pBuffer, /*32767*/0, bsize);
	MixSetupParms.pmixWrite (MixSetupParms.ulMixHandle,MixBuffers,2);

	return 0;
}

static void dart_close_sound (void)
{
	if (MixBuffers[0].pBuffer) {
		mciSendCommand (DeviceID, MCI_BUFFER,
			MCI_WAIT | MCI_DEALLOCATE_MEMORY, &BufferParms,0);
		MixBuffers[0].pBuffer = NULL;
	}

	if (DeviceID) {
		mciSendCommand (DeviceID, MCI_CLOSE, MCI_WAIT,
			(PVOID)&GenericParms, 0);
		DeviceID = 0;
	}
}


/* Buffer update thread (created and called by DART) */
static LONG APIENTRY
OS2_Dart_UpdateBuffers (ULONG ulStatus,PMCI_MIX_BUFFER pBuffer,ULONG ulFlags)
{

	if ((ulFlags == MIX_WRITE_COMPLETE) ||
		((ulFlags == (MIX_WRITE_COMPLETE|MIX_STREAM_ERROR)) &&
			(ulStatus == ERROR_DEVICE_UNDERRUN)))
	{
		DosRequestMutexSem (mutex,SEM_INDEFINITE_WAIT);
		ready++;
		DosReleaseMutexSem (mutex);
	}

	return TRUE;
}



static void dump_buffer (int i)
{
	static int index = 0;
	void *b;

	b = buffer;

	if (index + i > bsize) { 

		do {
	        	DosRequestMutexSem (mutex, SEM_INDEFINITE_WAIT);
			if (ready != 0) {
				DosReleaseMutexSem(mutex);
				break;
			}
	
			DosReleaseMutexSem (mutex);
			DosSleep (20);
		} while (TRUE);
	
		MixBuffers[next].ulBufferLength = index;
		MixSetupParms.pmixWrite (MixSetupParms.ulMixHandle,
			&(MixBuffers[next]),1);
		ready--;
		next++;
		index = 0;
	
		if (next == BUFFERCOUNT) {
			next=0;
		}
	}

	memcpy (&((char*)MixBuffers[next].pBuffer)[index], b, i);
	index += i;
}


