/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  Dreamcast files Copyright (C) 2002 Brian Peek/Ganksoft Entertainment
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#include <kos.h>
#include "image.h"
#include "gsi.h"
#include "util.h"

void draw_bm_scale(uint32 txr, float scale, float bright, int w, int h, float x, float y, int center)
{
	vertex_ot_t	v;
	poly_hdr_t	hdr;
	float		x1, y1, x2, y2;

	ta_poly_hdr_txr(&hdr, TA_TRANSLUCENT, TA_ARGB1555, w, h, txr, TA_NO_FILTER);
	ta_commit_poly_hdr(&hdr);

	if(center)
	{
		/* Figure out where to draw the bm */
		x1 = (320.0f - w/2 * scale) + x;
		y1 = (240.0f - h/2 * scale) + y;
		x2 = (320.0f + w/2 * scale) + x;
		y2 = (240.0f + h/2 * scale) + y;
	}
	else
	{
		x1 = x;
		y1 = y;
		x2 = x + w;
		y2 = y + h;
	}

	/* - -
	   + - */
	v.flags = TA_VERTEX_NORMAL;
	v.x = x1;
	v.y = y2;
	v.z = 10.0f;
	v.u = 0.0f; v.v = 1.0f;
	v.a = 1.0f; v.r = v.g = v.b = bright;
	v.oa = v.or = v.og = v.ob = 0.0f;
	ta_commit_vertex(&v, sizeof(v));

	/* + -
	   - - */
	v.y = y1;
	v.v = 0.0f;
	ta_commit_vertex(&v, sizeof(v));

	/* - -
	   - + */
	v.x = x2; v.y = y2;
	v.u = v.v = 1.0f;
	ta_commit_vertex(&v, sizeof(v));

	/* - +
	   - - */
	v.flags = TA_VERTEX_EOL;
	v.y = y1;
	v.v = 0.0f;
	ta_commit_vertex(&v, sizeof(v));
}
