/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999,2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#ifndef __AGI_LOGIC
#define __AGI_LOGIC

#ifdef __cplusplus
extern "C"{
#endif

struct agi_logic {
	UINT8 *data;		/* loaded logic is here */
	int sIP;		/* saved IP */
	int cIP;		/* current IP */
	int size;		/* size of data */
	int num_texts;		/* number of messages */
	char **texts;		/* message list */
};

int	decode_logic	(int);
void	unload_logic	(int);

#ifdef __cplusplus
};
#endif

#endif
