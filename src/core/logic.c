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
#include <string.h>
#include "sarien.h"
#include "agi.h"


/**
 * Decode logic resource
 * This function decodes messages from the specified raw logic resource
 * into a message list.
 * @param n  The number of the logic resource to decode.
 */
int decode_logic (int n)
{
	int ec = err_OK;
	int mstart, mend, mc;
	UINT8 *m0;

	/* decrypt messages at end of logic + build message list */

	/* report ("decoding logic #%d\n", n); */
	m0 = game.logics[n].data;

	mstart = lohi_getword (m0) + 2;
	mc = lohi_getbyte (m0 + mstart);
	mend = lohi_getword (m0 + mstart + 1);
	m0 += mstart + 3;			/* cover header info */
	mstart = mc << 1;

	/* if the logic was not compressed, decrypt the text messages
	 * only if there are more than 0 messages
	 */
	if ((~game.dir_logic[n].flags & RES_COMPRESSED) && mc > 0)
		decrypt (m0 + mstart, mend - mstart);	/* decrypt messages */

	/* build message list */
	m0 = game.logics[n].data;
	mstart = lohi_getword (m0) + 2;			/* +2 covers pointer */
	game.logics[n].num_texts = lohi_getbyte (m0 + mstart);

	/* resetp logic pointers */
	game.logics[n].sIP = 2;
	game.logics[n].cIP = 2;
	game.logics[n].size = lohi_getword (m0) + 2; /* logic end pointer */

	/* allocate list of pointers to point into our data */
	game.logics[n].texts = calloc (1 + game.logics[n].num_texts,
		sizeof (char*));

	/* cover header info */
	m0 += mstart+3;

	if (game.logics[n].texts != NULL) {
		/* move list of strings into list to make real pointers */
		for(mc = 0; mc < game.logics[n].num_texts; mc++) {
			mend = lohi_getword(m0+mc*2);
			game.logics[n].texts[mc] = mend ?
				(char *)m0 + mend - 2 : "";
		}
		/* set loaded flag now its all completly loaded */
		game.dir_logic[n].flags |= RES_LOADED;
	} else {
		/* unload data
		 * blah DF YA WANKER!!@!@# frag. i'm so dumb. not every logic
		 * has text
		 */
		free (game.logics[n].data);
		ec = err_NotEnoughMemory;
	}

	return ec;
}


/**
 * Unload logic resource
 * This function unloads the specified logic resource, freeing any
 * memory chunks allocated for this resource.
 * @param n  The number of the logic resource to unload
 */
void unload_logic (int n)
{
	if (game.dir_logic[n].flags & RES_LOADED) {
		free (game.logics[n].data);
		if (game.logics[n].num_texts)
			free (game.logics[n].texts);
		game.logics[n].num_texts = 0;
		game.dir_logic[n].flags &= ~RES_LOADED;
	}

	/* if cached, we end up here */
	game.logics[n].sIP = 2;
	game.logics[n].cIP = 2;
}

/* end: logic.c */

