/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999,2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

/* ALSA sound driver by Claudio Matsuoka <claudio@helllabs.org> */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/asoundlib.h>

#include "sarien.h"
#include "sound.h"
#include "console.h"

#define FRAGNUM 16
#define FRAGSIZE 1024

static int alsa_init_sound (SINT16 *);
static void alsa_close_sound (void);
static void dump_buffer (void);
static SINT16 *buffer;

static struct sound_driver sound_alsa = {
	"ALSA PCM sound output",
	alsa_init_sound,
	alsa_close_sound,
};

static snd_pcm_t *pcm_handle;


#include <pthread.h>

static pthread_t thread;


static void *sound_thread (void *arg)
{
	while (42) {
		play_sound ();
		mix_sound ();
		dump_buffer ();
	}
}


void __init_sound ()
{
	snd = &sound_alsa;
}


static int alsa_init_sound (SINT16 *b)
{
	snd_pcm_channel_params_t params;
	snd_pcm_channel_setup_t setup;
	int err;

	buffer = b;

	if ((err = snd_pcm_open (&pcm_handle, 0, 0, SND_PCM_OPEN_PLAYBACK)) < 0){
		report ("sound_alsa: Error: %s\n", snd_strerror (err));
		return -1;
	}

	report ("ALSA driver written by claudio@helllabs.org.\n");

	memset (&params, 0, sizeof(snd_pcm_channel_params_t));
	params.mode = SND_PCM_MODE_BLOCK;
	params.buf.block.frag_size = FRAGSIZE;
	params.buf.block.frags_min = 1;
	params.buf.block.frags_max = FRAGNUM;

	params.channel = SND_PCM_CHANNEL_PLAYBACK;
	params.start_mode = SND_PCM_START_FULL;
	params.stop_mode = SND_PCM_STOP_ROLLOVER;

	/* Set sound device to 16 bit, 22 kHz mono */

	params.format.interleave = 1;
	params.format.format = SND_PCM_SFMT_S16_LE;
	params.format.rate = 22050;
	params.format.voices = 1;

	if ((err = snd_pcm_plugin_params (pcm_handle, &params)) < 0) {
		report ("sound_alsa: Error: %s\n", snd_strerror (err));
		return -1;
	}

	if (snd_pcm_plugin_prepare (pcm_handle, SND_PCM_CHANNEL_PLAYBACK) < 0) {
		report ("sound_alsa: can't prepare\n");
		return -1;
	}

	memset (&setup, 0, sizeof (setup));
	setup.mode = SND_PCM_MODE_STREAM;
	setup.channel = SND_PCM_CHANNEL_PLAYBACK;

	if (snd_pcm_channel_setup (pcm_handle, &setup) < 0) {
		report ("sound_alsa: can't setup\n");
		return -1;
	}

	pthread_create (&thread, NULL, sound_thread, NULL);
	pthread_detach (thread);

	return 0;
}


static void alsa_close_sound ()
{
	snd_pcm_close (pcm_handle);
}


static void dump_buffer ()
{
	snd_pcm_plugin_write (pcm_handle, buffer, BUFFER_SIZE << 1);
}

