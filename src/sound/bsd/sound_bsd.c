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
 * BSD sound driver by Claudio Matsuoka <claudio@helllabs.org>
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/audioio.h>

#include "typedef.h"
#include "sound.h"

static int bsd_init_sound (SINT16 *);
static void bsd_close_sound (void);
static void dump_buffer (void);
static SINT16 *buffer;

static struct sound_driver sound_bsd = {
	"BSD /dev/audio sound output",
	bsd_init_sound,
	bsd_close_sound,
};

static int audio_fd;


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
	snd = &sound_bsd;
}


static int bsd_init_sound (SINT16 *b)
{
	audio_info_t ainfo;

	buffer = b;

	if ((audio_fd = open ("/dev/audio", O_WRONLY)) < 0)
		return err_Unk;

	AUDIO_INITINFO (&ainfo);
	ainfo.play.sample_rate = 22050;
	ainfo.play.channels = 1;
	ainfo.play.precision = 16;
	ainfo.play.encoding = AUDIO_ENCODING_LINEAR;
	ainfo.play.buffer_size = 16384;

	if (ioctl (audio_fd, AUDIO_SETINFO, &ainfo) == -1)
		return err_Unk;

	report ("BSD sound driver written by claudio@helllabs.org.\n");

	/* Set sound device to 16 bit, 22 kHz mono */

	pthread_create (&thread, NULL, sound_thread, NULL);
	pthread_detach (thread);

	return err_OK;
}


static void bsd_close_sound ()
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

