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
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "sarien.h"
#include "agi.h"

extern struct sarien_options opt;


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
	return !!game.flags[n];
}


void setflag (int n, int v)
{
	game.flags[n] = !!v;
}


void flipflag (int n)
{
	game.flags[n] = !game.flags[n];
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

