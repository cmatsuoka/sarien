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

/* BSD sound driver by Claudio Matsuoka <claudio@helllabs.org>
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

static SOUND_DRIVER sound_bsd = {
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

	fprintf (stderr, "sound_bsd: BSD sound support by "
		"claudio@helllabs.org\n");

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

