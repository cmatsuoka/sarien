/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2003 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#include <stdio.h>
#include <string.h>
#include <allegro.h>
#include "sarien.h"
#include "sound.h"

static int alleg_init_sound (SINT16 *);
static void alleg_close_sound (void);
static AUDIOSTREAM *stream;
static SINT16 *audiobuffer;

static struct sound_driver sound_alleg = {
	"Allegro sound output",
	alleg_init_sound,
	alleg_close_sound,
};

#define BUFFER_LEN 16384


void fill_audio ()
{
	UINT32 p;
	static UINT32 n = 0, s = 0;
	UINT16 *buffer;
	int len = BUFFER_LEN;

	if((buffer = get_audio_stream_buffer(stream)) == NULL)
		return;

	memcpy (buffer, (UINT8 *)audiobuffer + s, p = n);
	for (n = 0, len -= p; n < len; p += n, len -= n) {
		play_sound ();
		n = mix_sound () << 1;
		memcpy (buffer + p, audiobuffer, n);
	}
	play_sound ();
	n = mix_sound () << 1;
	memcpy (buffer + p, audiobuffer, s = len);
	n -= s;

	free_audio_stream_buffer(stream);
}


void __init_sound ()
{
	snd = &sound_alleg;
}


static int alleg_init_sound (SINT16 *b)
{
	report ("Allegro sound driver written by claudio@helllabs.org.\n");

	audiobuffer = b;

	install_sound(DIGI_AUTODETECT, MIDI_AUTODETECT, NULL);
	stream = play_audio_stream(BUFFER_LEN, 16, 0, 22050, 200, 255);

	report ("Allegro sound initialized.\n");

	return 0;
}


static void alleg_close_sound ()
{
	stop_audio_stream(stream);
	remove_sound();
}

