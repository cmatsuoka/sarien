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

#include <stdio.h>
#include "sarien.h"
#include "sound.h"

int dummy_init_sound (SINT16 *buffer);
void dummy_close_sound (void);
void dummy_dump_buffer (void);

SOUND_DRIVER sound_dummy=
{
	"Dummy sound driver",
	dummy_init_sound,
	dummy_close_sound,
};


int dummy_init_sound (SINT16 *buffer)
{
	fprintf (stderr, "\tsound_dummy: sound output disabled\n");
	return 0;
}


void dummy_close_sound (void)
{
}

