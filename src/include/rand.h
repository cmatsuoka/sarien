/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#ifndef __AGI_RANDOM_H
#define __AGI_RANDOM_H

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
