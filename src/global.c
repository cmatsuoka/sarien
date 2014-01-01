/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2003 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#include <stdio.h>
#include <string.h>
#include "sarien.h"
#include "agi.h"

/**
 *
 */
UINT8 hilo_getbyte (UINT8 *mem)
{
	return mem[0];
}

/**
 *
 */
UINT16 hilo_getword (UINT8 *mem)
{
	return ((UINT16)mem[0] << 8) + (mem[1]);
}

/**
 *
 */
UINT32 hilo_getpword (UINT8 *mem)
{
	return ((UINT32)mem[0] << 16) + ((UINT32)mem[1] << 8) + (mem[2]);
}

/**
 *
 */
UINT32 hilo_getdword (UINT8 *mem)
{
	return ((UINT32)mem[0] << 24) + ((UINT32)mem[1] << 16) +
		((UINT32)mem[2] << 8) + (mem[3]);
}

/**
 *
 */
UINT8 lohi_getbyte (UINT8 *mem)
{
	return mem[0];
}

/**
 *
 */
UINT16 lohi_getword (UINT8 *mem)
{
	return ((UINT16)mem[1] << 8) + (mem[0]);
}

/**
 *
 */
UINT32 lohi_getpword (UINT8 *mem)
{
	return ((UINT32)mem[2] << 16) + ((UINT32)mem[1] << 8) + (mem[0]);
}

/**
 *
 */
UINT32 lohi_getdword (UINT8 *mem)
{
	return ((UINT32)mem[3] << 24) + ((UINT32)mem[2] << 16) +
		((UINT32)mem[1] << 8) + (mem[0]);

}

/**
 *
 */
int getflag (int n)
{
	UINT8 *set = (UINT8 *)&game.flags;

	set += n >> 3;
	return (*set & (1 << (n & 0x07))) != 0;
}

/**
 *
 */
void setflag (int n, int v)
{
	UINT8 *set = (UINT8 *)&game.flags;

	set += n >> 3;
	if (v)
		*set |= 1 << (n & 0x07);		/* set bit  */
	else
		*set &= ~(1 << (n & 0x07));		/* clear bit */
}

/**
 *
 */
void flipflag (int n)
{
	UINT8 *set = (UINT8 *)&game.flags;

	set += n >> 3;
	*set ^= 1 << (n & 0x07);			/* flip bit */
}

/**
 *
 */
void setvar (int var, int val)
{
	game.vars[var] = val;
}

/**
 *
 */
int getvar (int var)
{
	return game.vars[var];
}

/**
 *
 */
void decrypt (UINT8 *mem, int len)
{
	UINT8 *key;
	int i;

	key = opt.agds ? (UINT8*)CRYPT_KEY_AGDS : (UINT8*)CRYPT_KEY_SIERRA;

	for (i = 0; i < len; i++)
		*(mem + i) ^= *(key + (i % 11));
}

/* end: global.c */

