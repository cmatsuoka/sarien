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



void __init_sound ()
{
	snd = &sound_hpux;
}


static int hpux_init_sound (SINT16 *b)
{
}


static void hpux_close_sound ()
{
}


static void dump_buffer ()
{
}

