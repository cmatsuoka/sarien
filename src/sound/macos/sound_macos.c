/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 *
 *  Carbon MacOS Sound Driver written by Jeremy Penner, jeremy@astra.mb.ca
 */

#include <Carbon/Carbon.h>

#include "sarien.h"
#include "console.h"
#include "sound.h"

static int macos_init_sound (SINT16 *buffer);
static void macos_close_sound (void);

static UINT16 *sarienbuf;
static UINT8 *buffer[2];
static CmpSoundHeader header;
static SndChannel *channel = NULL;

static struct sound_driver sound_macos = {
	"MacOS Carbon Sound Driver",
	macos_init_sound,
	macos_close_sound
};

void __init_sound ()
{
	snd = &sound_macos;
}

static void fill_sound (SndChannel *chan, SndCommand *cmd_passed)
{
    UINT32 fill_me, play_me;
    SndCommand cmd;

    fill_me = cmd_passed->param2;
    play_me = !fill_me;

    header.samplePtr = (Ptr)buffer[play_me];
    cmd.cmd = bufferCmd;
    cmd.param1 = 0;
    cmd.param2 = (long) &header;

    SndDoCommand (chan, &cmd, 0);

    memset (buffer[fill_me], 0, BUFFER_SIZE << 1);

    play_sound();
    mix_sound();
    
    memcpy (buffer[fill_me], sarienbuf, BUFFER_SIZE << 1);

    cmd.cmd = callBackCmd;
    cmd.param1 = 0;
    cmd.param2 = play_me;

    SndDoCommand (chan, &cmd, 0);    
}

static int macos_init_sound (SINT16 *b)
{
  SndCallBackUPP cb;
  SndCommand cmd;

  report ("sound_macos: written by jeremy@astra.mb.ca\n");

  sarienbuf = b;

  header.numChannels = 1;
  header.sampleSize = 16;
  header.sampleRate = rate22khz;
  header.numFrames = BUFFER_SIZE;
  header.encode = cmpSH;

  buffer[0] = malloc (BUFFER_SIZE << 1);
  buffer[1] = malloc (BUFFER_SIZE << 1);
  if (!buffer[0] || !buffer[1])
  {
    report ("sound_macos: Out of memory allocating %d bytes\n", BUFFER_SIZE << 1);
    return (-1);
  }
 
  cb = NewSndCallBackUPP (fill_sound);

  if ( SndNewChannel (&channel, sampledSynth, initMono, cb) != noErr)
  {
    report ("sound_macos: Unable to create channel");
    return (-1);
  }

  cmd.cmd = callBackCmd;
  cmd.param2 = 0;
  SndDoCommand (channel, &cmd, 0);

  return 0;
}

static void macos_close_sound ()
{
  SndDisposeChannel (channel, true);

  free (buffer[0]);
  free (buffer[1]);
}


