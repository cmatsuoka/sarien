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

/* OSS sound driver by Claudio Matsuoka <claudio@helllabs.org> */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>

#include "sarien.h"
#include "sound.h"
#include "console.h"

#define FRAGNUM		(32 - 1)
#define FRAGSIZE	(9)

static int oss_init_sound (SINT16 *);
static void oss_close_sound (void);
static void dump_buffer (void);
static SINT16 *buffer;

static SOUND_DRIVER sound_oss = {
	"OSS /dev/dsp sound output",
	oss_init_sound,
	oss_close_sound,
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
	snd = &sound_oss;
}


static int oss_init_sound (SINT16 *b)
{
	int i;
	audio_buf_info info;

	buffer = b;

	if ((audio_fd = open ("/dev/dsp", O_WRONLY)) == -1)
		return -1;

	report ("sound_oss: OSS support by claudio@helllabs.org\n");

	/* Set sound device to 16 bit, 22 kHz mono */

	i = (FRAGNUM << 16 | FRAGSIZE);
	ioctl (audio_fd, SNDCTL_DSP_SETFRAGMENT, &i);

	ioctl (audio_fd, SNDCTL_DSP_GETOSPACE, &info);
	report ("sound_oss: %d fragments of %d bytes\n",
		info.fragstotal, info.fragsize);

	i = AFMT_S16_LE;
	ioctl (audio_fd, SNDCTL_DSP_SETFMT, &i);
	i = 0;
	ioctl (audio_fd, SNDCTL_DSP_STEREO, &i);
	i = 22000;
	ioctl (audio_fd, SNDCTL_DSP_SPEED, &i);

	pthread_create (&thread, NULL, sound_thread, NULL);
	pthread_detach (thread);

	return 0;
}


static void oss_close_sound ()
{
	ioctl (audio_fd, SNDCTL_DSP_SYNC);
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

