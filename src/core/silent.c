/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#include <stdio.h>
#include "sarien.h"
#include "agi.h"

static int dummy_init_sound (SINT16 *);
static void dummy_close_sound (void);

struct sound_driver sound_dummy = {
	"Dummy sound driver",
	dummy_init_sound,
	dummy_close_sound
};


static int dummy_init_sound (SINT16 *buffer)
{
	report ("sound_dummy: sound output disabled\n");
	return 0;
}


static void dummy_close_sound ()
{
}


