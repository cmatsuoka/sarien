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
 * FreeBSD sound driver by Joep Grooten <joep@di.nl>
 * Based on the BSD sound driver by Claudio Matsuoka <claudio@helllabs.org>
 */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <machine/soundcard.h>

#include "sarien.h"
#include "sound.h"

static int freebsd_init_sound (SINT16 *);
static void freebsd_close_sound (void);
static void dump_buffer (void);
static SINT16 *buffer;

static struct sound_driver sound_freebsd = {
	"FreeBSD /dev/audio sound output",
	freebsd_init_sound,
	freebsd_close_sound,
};

static int audio_fd;

#include <pthread.h>

static pthread_t thread;

static void *sound_thread (void *arg)
{
	while (1)
	{
		play_sound ();
		mix_sound ();
		dump_buffer ();
	}
}


void __init_sound ()
{
	snd = &sound_freebsd;
}


static int freebsd_init_sound (SINT16 *b)
{
	int	arg;

	buffer = b;

	if ((audio_fd = open ("/dev/dsp", O_WRONLY)) < 0)
		return err_Unk;

	arg = 22050;
	if (ioctl (audio_fd, SNDCTL_DSP_SPEED, &arg) == -1)
		return err_Unk;

	arg = 0;
	if (ioctl (audio_fd, SNDCTL_DSP_STEREO, &arg) == -1)
		return err_Unk;
	
	arg = 16;
	if (ioctl (audio_fd, SNDCTL_DSP_SAMPLESIZE, &arg) == -1)
		return err_Unk;

	arg = 820;
	if (ioctl (audio_fd, SNDCTL_DSP_SETBLKSIZE, &arg) == -1)
		return err_Unk;

	fprintf (stderr,"sound_freebsd: FreeBSD sound support by joep@di.nl\n");

	pthread_create (&thread, NULL, sound_thread, NULL);
	pthread_detach (thread);

	return err_OK;
}


static void freebsd_close_sound ()
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
