/*
 *  Sarien AGI :: Copyright (C) 1998 Dark Fiber
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

#ifndef __AGI_RANDOM
#define __AGI_RANDOM

#ifdef __cplusplus
extern "C"{
#endif

					/* m = 2^31 - 1 */
#define	RNG_M		2147483647L
#define	RNG_A		48271L
					/* m div a */
#define	RNG_Q		127773L
					/* m mod a */
#define	RNG_R		2836L

SINT32	get_rnd_seed	(void);
void	set_rnd_seed	(void);
void	set_xrnd_seed	(SINT32 seedval);
SINT32	xrnd		(void);
SINT32	rnd		(SINT32 maxrnd);

#ifdef __cplusplus
};
#endif

#endif
