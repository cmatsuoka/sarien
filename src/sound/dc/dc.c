/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  Dreamcast files Copyright (C) 2002 Brian Peek/Ganksoft Entertainment
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <mp3/sndstream.h>

#include "sarien.h"
#include "sound.h"
#include "console.h"

#define BUFFER_SOUND (32768*2)

static SINT16 *buffer;
static SINT16 ret_buff[BUFFER_SOUND];

static int  dc_init_sound   (SINT16 *buffer);
static void dc_close_sound  (void);
void* dc_play_sound(int count);

static struct sound_driver sound_dc = {
	"Dreamcast native sound output",
	dc_init_sound,
	dc_close_sound,
};


void __init_sound ()
{
	snd = &sound_dc;
}
 
static int dc_init_sound (SINT16 *b)
{
	memset(ret_buff, 0, BUFFER_SOUND);
	buffer = b;
	snd_stream_init(dc_play_sound);
	snd_stream_start(22050, 0);
	return 0;
}

static void dc_close_sound (void)
{
	snd_stream_stop();
}

void* dc_play_sound(int count)
{
	SINT16 acc, i, x;
	static SINT16 pos_buffer = 0, ex = 0;
	SINT16 *pp;

	count /= 2;

	if ((acc = ex) != 0)
	{
		pp = ret_buff;
		for(x = 0; x < ex; x++)
			*pp++ = *(buffer + pos_buffer + x);
		//memcpy ((SINT16 *)ret_buff, (SINT16*)(buffer + pos_buffer), ex);
		ex = 0;
	}

	do
	{
		play_sound();

		i = mix_sound();// << 1;

		if ((acc + i) > BUFFER_SOUND)
		{
			i = ex = BUFFER_SOUND - acc;
			pos_buffer  = (i + acc) - BUFFER_SOUND;
		}
		pp = (ret_buff + acc);
		for(x = 0; x < i; x++)
			*pp++ = *(buffer + x);
		//memcpy ((SINT16 *)(ret_buff + acc), (SINT16 *)buffer, i);
		acc += i;
	} while (acc < count);

	return ret_buff;
}
