/*
 *  Sarien AGI :: Copyright (C) 1999 Dark Fiber 
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
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

static SOUND_DRIVER sound_alsa = {
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
	snd_pcm_format_t format;
	snd_pcm_playback_status_t ps;
	snd_pcm_playback_params_t pp;

	buffer = b;

	if (snd_pcm_open (&pcm_handle, 0, 0, SND_PCM_OPEN_PLAYBACK) < 0)
		return -1;

	report ("sound_alsa: ALSA sound support written by "
		"claudio@helllabs.org\n");

	/* Set sound device to 16 bit, 22 kHz mono */

	format.rate = 22050;
	format.channels = 1;
	format.format = SND_PCM_SFMT_S16_LE;

	if (snd_pcm_playback_format (pcm_handle, &format))
		return -1;

	memset (&pp, 0, sizeof(pp));
	pp.fragment_size = FRAGSIZE;
	pp.fragments_max = FRAGNUM;
	pp.fragments_room = 1;
	snd_pcm_playback_params (pcm_handle, &pp);
	snd_pcm_playback_status (pcm_handle, &ps);

	report ("sound_alsa: %d fragments of %d bytes\n",
		ps.fragments, ps.fragment_size);

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
	snd_pcm_write (pcm_handle, buffer, BUFFER_SIZE << 1);
}

