/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999,2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */


#include "sarien.h"
#include "sound.h"
#include "console.h"

static int hpux_init_sound (SINT16 *);
static void hpux_close_sound (void);
static void hpux_dump_buffer (void);

static struct sound_driver sound_hpux = {
	"HP-UX sound output",
	hpux_init_sound,
	hpux_close_sound,
	hpux_dump_buffer
};

static int fd;


void __init_sound ()
{
	snd = &sound_hpux;
}


static int hpux_init_sound (SINT16 *b)
{
	int flags;
	int gain = 128;
	int bsize = 32 * 1024;
	int port = AUDIO_OUT_SPEAKER;
	struct audio_gains agains;
	struct audio_describe adescribe;

	if ((flags = fcntl (fd, F_GETFL, 0)) < 0)
		return -1;

	flags |= O_NDELAY;
	if ((flags = fcntl (fd, F_SETFL, flags)) < 0)
		return -1;

	if (ioctl (fd, AUDIO_SET_DATA_FORMAT, AUDIO_FORMAT_LINEAR16BIT) == -1)
		return -1;

	if (ioctl (fd, AUDIO_SET_CHANNELS, 1) == -1)
		return -1;

	if (ioctl (fd, AUDIO_SET_OUTPUT, port) == -1)
		return -1;

	if (ioctl (fd, AUDIO_SET_SAMPLE_RATE, 22050) == -1)
		return -1;

	if (ioctl (fd, AUDIO_DESCRIBE, &adescribe) == -1)
		return -1;

	if (ioctl (fd, AUDIO_GET_GAINS, &agains) == -1)
		return -1;

	agains.transmit_gain = adescribe.min_transmit_gain +
		(adescribe.max_transmit_gain - adescribe.min_transmit_gain) *
		gain / 256;

	if (ioctl (fd, AUDIO_SET_GAINS, &agains) == -1)
		return -1;

	if (ioctl(fd, AUDIO_SET_TXBUFSIZE, bsize) == -1)
		return -1;

	return 0;
}


static void hpux_close_sound ()
{
	close (fd);
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

