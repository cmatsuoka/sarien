/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#include "sarien.h"
#include "console.h"
#include "sound.h"

static int amiga_init_sound (SINT16 *buffer);
static void amiga_close_sound (void);

static struct sound_driver sound_amiga = {
	"Dummy sound driver",
	amiga_init_sound,
	amiga_close_sound
};


void __init_sound ()
{
	snd = &sound_amiga;
}

static int amiga_init_sound (SINT16 *buffer)
{
	report ("sound_dummy: sound output disabled\n");
	return -1;
}

static void amiga_close_sound ()
{
}


