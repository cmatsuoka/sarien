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

#include "sarien.h"
#include "agi.h"
#include "logic.h"

struct agi_logic logics[MAX_DIRS];


/* decodes messages into message list from raw logic files */
int decode_logic (int resnum)
{
	int ec = err_OK;
	int mstart, mend, mc;
	char *m0;

	/* decrypt messages at end of logic + build message list */

	m0 = logics[resnum].data;

	mstart = lohi_getword (m0) + 2;
	mc = lohi_getbyte (m0 + mstart);
	mend = lohi_getword (m0 + mstart + 1);
	m0 += mstart + 3;			/* cover header info */
	mstart = mc << 1;

	/* if the logic was not compressed, decrypt the text messages
	 * only if there are more than 0 messages
	 */
	if ((~game.dir_logic[resnum].flags & RES_COMPRESSED) && mc > 0)
		decrypt (m0 + mstart, mend - mstart);	/* decrypt messages */

	/* build message list */
	m0 = logics[resnum].data;
	mstart = lohi_getword (m0) + 2;			/* +2 covers pointer */
	logics[resnum].num_texts = lohi_getbyte (m0 + mstart);

	/* resetp logic pointers */
	logics[resnum].sIP = 2;
	logics[resnum].cIP = 2;
	logics[resnum].size = lohi_getword (m0) + 2;	 /* logic end pointer */

	/* allocate list of pointers to point into our data */
	logics[resnum].texts = calloc (1 + logics[resnum].num_texts,
		sizeof (char*));

	/* cover header info */
	m0 += mstart+3;

	if (logics[resnum].texts != NULL) {
		/* move list of strings into list to make real pointers */
		for(mc=0; mc<logics[resnum].num_texts; mc++) {
			mend=lohi_getword(m0+mc*2);
			logics[resnum].texts[mc] = mend ?  m0+mend-2 : "";
		}
		/* set loaded flag now its all completly loaded */
		game.dir_logic[resnum].flags|=RES_LOADED;
	} else {
		/* unload data
		 * blah DF YA WANKER!!@!@# frag. i'm so dumb. not every logic
		 * has text
		 */
		free (logics[resnum].data);
		ec = err_NotEnoughMemory;
	}

	return ec;
}


void unload_logic (int resnum)
{
	if (game.dir_logic[resnum].flags & RES_LOADED) {
		free (logics[resnum].data);
		if (logics[resnum].num_texts)
			free (logics[resnum].texts);
		logics[resnum].num_texts = 0;
		game.dir_logic[resnum].flags &= ~RES_LOADED;
	}
	/* if cached, we end up here */
	logics[resnum].sIP = 2;
	logics[resnum].cIP = 2;
}

