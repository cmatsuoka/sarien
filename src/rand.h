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

SINT32	get_rnd_seed	(void);
void	set_rnd_seed	(void);
SINT32	rnd		(SINT32 maxrnd);

#ifdef __cplusplus
};
#endif

#endif /* __AGI_RANDOM_H */
