/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999,2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

/* Solaris sound driver by Claudio Matsuoka <claudio@helllabs.org>
 * Note: the sound player needs 22 kHz signed 16 bit mono output so only
 * CS4231 is currently supported.
 *
 * Fixes by Keith Hargrove <Keith.Hargrove@Eng.Sun.COM> from the
 * xmp drivers
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/audioio.h>
#include <sys/stropts.h>

#include "sarien.h"
#include "sound.h"
#include "console.h"

static int solaris_init_sound (SINT16 *);
static void solaris_close_sound (void);
static void dump_buffer (void);
static SINT16 *buffer;

static struct sound_driver sound_solaris = {
	"Solaris /dev/audio sound output",
	solaris_init_sound,
	solaris_close_sound,
};

static int audio_fd;
static int audioctl_fd;


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
	snd = &sound_solaris;
}


static int solaris_init_sound (SINT16 *b)
{
	audio_info_t ainfo;
	int gain, port, bsize;

	buffer = b;
	gain = 50;
	port = AUDIO_SPEAKER;
	bsize = 8 << 10;

	AUDIO_INITINFO (&ainfo);

	if ((audio_fd = open ("/dev/audio", O_WRONLY)) < 0)
		return err_Unk;

	/* empty buffers before change config */
	ioctl (audio_fd, AUDIO_DRAIN, 0);	/* drain everything out */
	ioctl (audio_fd, I_FLUSH, FLUSHRW);	/* flush everything */

	/* try to open audioctl device */
	if ((audioctl_fd = open ("/dev/audioctl",O_RDWR)) > 0) {
		ioctl (audioctl_fd, I_FLUSH, FLUSHRW);  /* flush everything */

		/* get audio parameters */
		if (ioctl (audioctl_fd, AUDIO_GETINFO, &ainfo) == 0) {
			gain = ainfo.play.gain;
			port = ainfo.play.port;
		}

		close(audioctl_fd);
	}

	AUDIO_INITINFO (&ainfo);

	/* Set sound device to 16 bit, 22 kHz mono */

	ainfo.play.sample_rate = 22050;
	ainfo.play.channels = 1;
	ainfo.play.precision = 16;
	ainfo.play.encoding = AUDIO_ENCODING_LINEAR;
	ainfo.play.gain = gain;
	ainfo.play.port = port;
	ainfo.play.buffer_size = bsize;

	if (ioctl (audio_fd, AUDIO_SETINFO, &ainfo) < 0)
		return err_Unk;

	report ("sound_solaris: Solaris sound support by "
		"claudio@helllabs.org\n");

	pthread_create (&thread, NULL, sound_thread, NULL);
	pthread_detach (thread);

	return err_OK;
}


static void solaris_close_sound ()
{
	close (audio_fd);
}


static void dump_buffer ()
{
	int i = BUFFER_SIZE << 1, j;
	SINT16 *b = buffer;

	do {
		if ((j = write (audio_fd, b, i)) > 0) {
			i -= j;
			b += j;
		}
	} while (i);
}

