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

static int dummy2_init_sound (SINT16 *buffer);
static void dummy2_close_sound (void);
/* static void dummy2_dump_buffer (void); */

static struct sound_driver sound_dummy2=
{
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
	fprintf (stderr, "sound_dummy: sound output disabled\n");
	return -1;
}

static void dummy2_close_sound (void)
{
}


/*
static void dummy2_dump_buffer (void)
{
}
*/

