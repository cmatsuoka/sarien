/*
 *  Sarien AGI :: Copyright (C) 1999 Dark Fiber 
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

#include <time.h>

#include "sarien.h"
#include "rand.h"

SINT32	rnd_seed;

void set_rnd_seed(void)
{
	set_xrnd_seed(time(NULL));
}

SINT32 get_rnd_seed(void)
{
	return rnd_seed;
}

void set_xrnd_seed(SINT32 seedval)
{
	rnd_seed=(seedval%(RNG_M-1))+1;
}

SINT32 xrnd(void)
{
	SINT32 low, high, test;

	high = rnd_seed / RNG_Q;
	low = rnd_seed % RNG_Q;
	test = RNG_A * low - RNG_R * high;

	return rnd_seed = test > 0 ? test : test + RNG_M;
}

SINT32 rnd(SINT32 maxrnd)
{
	return maxrnd ? xrnd() % maxrnd : xrnd();
}
