/*
 *  Sarien AGI :: Silicon Graphics Audio Library (AL) sound driver
 *
 *  Copyright (C) 1999 Ari Heikkinen
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

#include <stdio.h>
#include <string.h>
#include <dmedia/audio.h>
#include <errno.h>

#include "sarien.h"
#include "sound.h"
#include "console.h"

static int sgi_init_sound (SINT16 *sbuf);
static void sgi_close_sound (void);
static void sgi_dump_buffer (void);

static SINT16 *sound_buffer = 0;
static ALport al_port = 0;
static double old_sample_rate;

static pthread_t sound_thread;

SOUND_DRIVER sound_sgi = {
	"SGI Sound Driver",
	sgi_init_sound,
	sgi_close_sound
};

void __init_sound ()
{
	snd = &sound_sgi;
}

static void *sgi_sound_handler(void *arg)
{
	for (;;) {
		play_sound();
		mix_sound();
		sgi_dump_buffer();
	}
}

static double sgi_get_sample_rate(void)
{
	ALpv alpv;

	alpv.param = AL_RATE;
	if (alGetParams(AL_DEFAULT_OUTPUT, &alpv, 1) != 1)
		return 0;
	return alFixedToDouble(alpv.value.ll);
}

static int sgi_set_sample_rate(double rate)
{
	ALpv alpv[2];

	if (rate < 0)
		return -1;
	alpv[0].param = AL_MASTER_CLOCK;
	alpv[0].value.i = AL_CRYSTAL_MCLK_TYPE;
	alpv[1].param = AL_RATE;
	alpv[1].value.ll = alDoubleToFixed(rate);
	if (alSetParams(AL_DEFAULT_OUTPUT, alpv, 2) != 2)
		return -1;
	return 0;
}

static int sgi_init_sound (SINT16 *sbuf)
{
	ALconfig conf;

	sgi_close_sound();
	old_sample_rate = sgi_get_sample_rate();
	conf = alNewConfig();
	if (!conf) {
		fprintf(stderr, "[sound_sgi] alNewConfig: %s\n",
			strerror(oserror()));
		return -1;
	}
	alSetQueueSize(conf, 1102);
	alSetChannels(conf, AL_MONO);
	al_port = alOpenPort("Sarien Sound", "w", conf);
	alFreeConfig(conf);
	if (!al_port) {
		fprintf(stderr, "[sound_sgi] alOpenPort: %s\n",
			strerror(oserror()));
		return -1;
	}
	sgi_set_sample_rate((double)22050);
	sound_buffer = sbuf;
	if (pthread_create(&sound_thread, 0, sgi_sound_handler, 0)) {
		fprintf(stderr, "[sound_sgi] pthread_create: %s\n",
			strerror(errno));
	}
	pthread_detach(sound_thread);
	fprintf(stderr, "[sound_sgi] Sound init successful.\n");

	return 0;
}

static void sgi_close_sound (void)
{
	sound_buffer = 0;
	if (al_port) {
		alClosePort(al_port);
		sgi_set_sample_rate(old_sample_rate);
	}
	al_port = 0;
}

static void sgi_dump_buffer (void)
{
	if (al_port && sound_buffer)
		alWriteFrames(al_port, sound_buffer, BUFFER_SIZE);
}
