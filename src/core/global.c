/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999,2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "sarien.h"
#include "agi.h"
#include "view.h"
#include "text.h"

extern struct agi_loader *loader;
extern struct sarien_options opt;
extern struct gfx_driver *gfx;


UINT8 hilo_getbyte (UINT8 *mem)
{
	return mem[0];
}


UINT16 hilo_getword (UINT8 *mem)
{
	return (mem[0]<<8) + (mem[1]);
}


UINT32 hilo_getpword (UINT8 *mem)
{
	return (mem[0]<<16) + (mem[1]<<8) + (mem[2]);
}


UINT32 hilo_getdword (UINT8 *mem)
{
	return (mem[0]<<24) + (mem[1]<<16) + (mem[2]<<8) + (mem[3]);
}


UINT8 lohi_getbyte (UINT8 *mem)
{
	return mem[0];
}


UINT16 lohi_getword (UINT8 *mem)
{
	return (mem[1]<<8) + (mem[0]);
}


UINT32 lohi_getpword (UINT8 *mem)
{
	return (mem[2]<<16) + (mem[1]<<8) + (mem[0]);
}


UINT32 lohi_getdword (UINT8 *mem)
{
	return (mem[3]<<24) + (mem[2]<<16) + (mem[1]<<8) + (mem[0]);

}


int getflag (int n)
{
	UINT8 *set = (UINT8*)&game.flags;

	set += n >> 3;
	return (*set & (1 << (n & 0x07))) != 0;
}


void setflag (int n, int v)
{
	UINT8 *set = (UINT8*)&game.flags;

	set += n >> 3;
	if (v)
		*set |= 1 << (n & 0x07);		/* set bit  */
	else
		*set &= ~(1 << (n & 0x07));		/* clear bit*/
}


void flipflag (int n)
{
	UINT8 *set = (UINT8*)&game.flags;

	set += n >> 3;
	*set ^= 1 << (n & 0x07);			/* flip bit */
}


void setvar (int var, int val)
{
	game.vars[var] = val;
}


int getvar (int var)
{
    return game.vars[var];
}


void decrypt (UINT8 *mem, int len)
{
	UINT8 *key;
	int i;

	key = opt.agds ? (UINT8*)CRYPT_KEY_AGDS : (UINT8*)CRYPT_KEY_SIERRA;

	for (i = 0; i < len; i++)
		*(mem + i) ^= *(key + (i % 11));
}


/* unload all logic resources */
void unload_resources ()
{
	int i;

	for(i = 0; i < MAX_DIRS; i++) {
		dir_view[i].flags &= ~RES_CACHED;	/* clear cache flag */
		loader->unload_resource (rVIEW, i);	/* free view */

		dir_pic[i].flags &= ~RES_CACHED;	/* clear cache flag */
		loader->unload_resource (rPICTURE, i);	/* free resource */

		dir_logic[i].flags &= ~RES_CACHED;	/* clear cache flag */
		loader->unload_resource (rLOGIC, i);	/* free resource */

		dir_sound[i].flags &= ~RES_CACHED;	/* clear cache flag */
		loader->unload_resource (rSOUND, i);	/* free resource */
	}
}


void new_room_resources ()
{
	int x;

	for (x = 0; x < MAX_DIRS; x++) {
		/* FR: 
		 * According to the specs, only the logic resources need to
		 * be freed (and now freeing the view resources will corrupt
		 * the program anyway)
		 *
		 * loader->unload_resource(rVIEW, x);
		 * loader->unload_resource(rPICTURE, x);
		 */
		loader->unload_resource(rLOGIC, x);
	}

	for (x = 0; x < MAX_VIEWTABLE; x++)
		reset_view(x);
}


UINT8 o_status = 0;	/* FIXME */

void update_status_line (int force)
{
	char x[64];
	static int o_score = 255, o_max_score = 0, o_sound = FALSE,
		o_status = FALSE;

	/* If it's already there and we're not forcing, don't write */
   	if (!force && o_status == status_line &&
		o_score == getvar (V_score) && 
		o_max_score == getvar (V_max_score),
		o_sound == getflag (F_sound_on))
		return;

	o_score = getvar (V_score);
	o_max_score = getvar (V_max_score);
   	o_sound = getflag (F_sound_on);

	/* Jump out here if the status line is invisible and was invisible
	 * the last time. If the score or sound has changed, there is no
         * reason to re-erase the status line here. Allow force, though...
         */

	if (!force && o_status == status_line && !status_line)
		return;

   	o_status = status_line;

	if (line_min_print == 0)
		return;

	if (!status_line) {
		//print_status ("                                        ");
	} else {
		sprintf (x, " Score:%i of %03i", o_score, getvar(V_max_score));
		print_status ("%-17s             Sound:%s ", x,
			o_sound ? "On " : "Off");
	}
}

