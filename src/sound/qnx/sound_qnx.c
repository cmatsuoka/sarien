/* Sarien - A Sierra AGI resource interpreter engine
 * Copyright (C) 1999,2001 Stuart George and Claudio Matsuoka
 *
 * $Id$
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; see docs/COPYING for further details.
 */

/* QNX sound driver hacked by Claudio Matsuoka <claudio@helllabs.org> */

#include <sys/audio.h>
#include <sys/ioctl.h>

#include "sarien.h"
#include "sound.h"
#include "console.h"


static int fd_audio;

static int qnx_init_sound (SINT16 *);
static void qnx_close_sound (void);
static void dump_buffer (void);
static SINT16 *buffer;


static struct sound_driver sound_oss = {
	"QNX sound driver",
	qnx_init_sound,
	qnx_close_sound
};

void __init_sound ()
{
        snd = &sound_qnx;
}

static int qnx_init_sound (SINT16 *b)
{
	int rc, rate, bits, stereo, bsize;
 
	rate = 22050;
	bits = 16;
	stereo = 0;
	bufsize = 32 * 1024;

	fd_audio = open (dev, O_WRONLY);
	if (fd_audio < 0) {
		report ("sound_qnx: can't open audio device\n");
		return -1;
	}

	report ("QNX sound support hacked by claudio@helllabs.org.\n");

	if (ioctl (fd_audio, SOUND_PCM_WRITE_BITS, &bits) < 0) {
		report ("sound_qnx: can't set resolution\n");
		goto error;
	}

	if (ioctl (fd, SNDCTL_DSP_STEREO, &stereo) < 0) {
		report ("sound_qnx: can't set channels\n");
		goto error;
    	}

	if (ioctl(fd, SNDCTL_DSP_SPEED, &rate) < 0) {
		report ("sound_qnx: can't set rate\n");
		goto error;
	}

	if (ioctl (fd, SNDCTL_DSP_GETBLKSIZE, &bufsize) < 0) {
		perror ("sound_qnx: can't set buffer\n");
		goto error;
	}

	return 0;

error:
	close (fd_audio);
	return -1;
}


static void qnx_close_sound ()
{
	ioctl (fd, SNDCTL_DSP_SYNC, NULL);
	close (fd_audio);
}


static void dump_buffer ()
{
	int j, i = BUFFER_SIZE << 1;
	SINT *b = buffer;

	do {
		if ((j = write (fd_audio, b, i)) > 0) {
			i -= j;
			b += j;
		} else
			break;
	} while (i);
}


