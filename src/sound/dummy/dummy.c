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

static int dummy2_init_sound (SINT16 *buffer);
static void dummy2_close_sound (void);

static struct sound_driver sound_dummy2 = {
	"Dummy sound driver",
	dummy2_init_sound,
	dummy2_close_sound,
};


void __init_sound ()
{
	snd = &sound_dummy2;
}

static int dummy2_init_sound (SINT16 *buffer)
{
	report ("sound_dummy: sound output disabled\n");
	return -1;
}

static void dummy2_close_sound (void)
{
}


