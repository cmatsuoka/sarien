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
 * Win32 sound driver by Felipe Rosinha
 */

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <mmsystem.h>

#include "win32.h"
#include "sarien.h"
#include "sound.h"
#include "console.h"

#define BUFFER_SOUND 8200
#define BUFFER       3

static SINT16   *buffer;

struct _win32_sound_buffer {
	WAVEHDR *waveBuffer;
	SINT16  *sound;
} _sound_buffer[BUFFER];

static HWAVEOUT hSoundDevice;

static int   win32_init_sound   (SINT16 *buffer);
static void  win32_close_sound  (void);
static void  win32_swap_buffers (PWAVEHDR);

static struct sound_driver sound_win32 = {
	"Win32 native sound output",
	win32_init_sound,
	win32_close_sound,
};


void __init_sound ()
{
	snd = &sound_win32;
}
 
static int win32_init_sound (SINT16 *b)
{
	int          nCount;
	int          nSoundDevice = -1;
	WAVEFORMATEX waveFormat;
	WAVEOUTCAPS  waveCaps;

	report ("sound_win32: searching for a capable sound device.\n");

	buffer      = b;
	flush_sound = win32_swap_buffers;

	memset(&waveFormat, 0, sizeof(waveFormat));

	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.nChannels = 1;
	waveFormat.nSamplesPerSec = 22050;
	waveFormat.wBitsPerSample = 16;
	waveFormat.nBlockAlign = waveFormat.nChannels *
		(waveFormat.wBitsPerSample / 8);
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec *
		waveFormat.nBlockAlign;

	nSoundDevice = waveOutOpen (
		&hSoundDevice,
		WAVE_MAPPER,
		&waveFormat,
		(DWORD) hwndMain,
		0,
		CALLBACK_WINDOW);

	if (nSoundDevice != MMSYSERR_NOERROR) {
		report ("sound_win32: waveOutOpen() error.\n");
		return -1;
	}

	if (waveOutGetDevCaps((UINT) hSoundDevice, &waveCaps, sizeof(waveCaps))
		!= MMSYSERR_NOERROR)
	{
		report("sound_win32: waveOutGetDevCaps() error.\n");
		return -1;
	}
					
	report("sound_win32: Audio device %s\n", waveCaps.szPname);

	/* Wave Buffers 0 */
	for ( nCount = 0; nCount < BUFFER; nCount ++) {
		_sound_buffer[nCount].sound = (SINT16 *)malloc (BUFFER_SOUND);
		memset( _sound_buffer[nCount].sound, 0, BUFFER_SOUND );

		_sound_buffer[nCount].waveBuffer = malloc (sizeof(WAVEHDR));
		_sound_buffer[nCount].waveBuffer->lpData =
			(BYTE *) _sound_buffer[nCount].sound;
		_sound_buffer[nCount].waveBuffer->dwBufferLength = BUFFER_SOUND;
		_sound_buffer[nCount].waveBuffer->dwFlags = 0;

		if (waveOutPrepareHeader (hSoundDevice,
			_sound_buffer[nCount].waveBuffer, sizeof(WAVEHDR)) !=
			MMSYSERR_NOERROR)
		{
			report("Error: waveOutPrepareHeader()\n");
		}

		if (waveOutWrite(hSoundDevice, _sound_buffer[nCount].waveBuffer,
			sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
		{
			report("Error: waveOutWrite()\n");
		}
	}

	return 0;
}

static void win32_close_sound (void)
{
	int nCount;
	
	/* Free the sound buffer */
	for (nCount = 0; nCount < BUFFER; nCount ++) {
		waveOutUnprepareHeader (hSoundDevice,
			_sound_buffer[nCount].waveBuffer, sizeof(WAVEHDR));

		free (_sound_buffer[nCount].waveBuffer);
		free (_sound_buffer[nCount].sound);
	}

	if ( hSoundDevice )
		waveOutClose( hSoundDevice );  
}

static void win32_swap_buffers (PWAVEHDR pWave) 
{
	int    acc, i;
	static int pos_buffer = 0, ex = 0;

	if ((acc = ex) != 0) {
		memcpy ((SINT16 *)pWave->lpData,
			(SINT16*)(buffer + pos_buffer), ex);
		ex = 0;
	}

	do {
		play_sound();
		i = mix_sound() << 1;
		
		if ((acc + i) > BUFFER_SOUND) {
			i = ex = BUFFER_SOUND - acc;
			pos_buffer  = (i + acc) - BUFFER_SOUND;
		}

		memcpy ((SINT16 *)(pWave->lpData + acc), (SINT16 *)buffer, i);
		acc += i;
	} while (acc < BUFFER_SOUND);

	waveOutWrite( hSoundDevice, pWave, sizeof(WAVEHDR) );
}

