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
#include "sarien.h"

void draw_string(uint32 tex_addr, float x, float y, char *string)
{
#define CHAR_W	16.0f
#define CHAR_H	16.0f
#define TEX_U	16.0f
#define TEX_V	16.0f

	vertex_ot_t	v;
	poly_hdr_t	hdr;
	
	char *c;
	int cx;
	int cy;
	float pos = 0.0f;

	c = string;

	ta_poly_hdr_txr(&hdr, TA_OPAQUE, TA_RGB565, 512, 512, tex_addr, TA_NO_FILTER);
	ta_commit_poly_hdr(&hdr);

	while(*c != '\0')
	{
		cx = (*c % 16);
		cy = (*c / 16)-2;

		v.oa = v.or = v.og = v.ob = 0.0f;
		v.a = 1.0f; v.r = v.g = v.b = 1.0f;

		v.x = x + pos;
		v.y = y + CHAR_H;
		v.z = 20.0f;
		v.u = cx / TEX_U;
		v.v = (cy + 1.0f) / TEX_V;
		v.flags = TA_VERTEX_NORMAL;
		ta_commit_vertex(&v, sizeof(v));

		v.x = x + pos;
		v.y = y;
		v.u = cx / TEX_U;
		v.v = cy / TEX_V;
		ta_commit_vertex(&v, sizeof(v));

		v.x = x + pos + CHAR_W;
		v.y = y + CHAR_H;
		v.u = (cx + 1.0f) / TEX_U;
		v.v = (cy + 1.0f) / TEX_V;
		ta_commit_vertex(&v, sizeof(v));

		v.x = x + pos + CHAR_W;
		v.y = y;
		v.u = (cx + 1.0f) / TEX_U;
		v.v = cy / TEX_V;;
		v.flags = TA_VERTEX_EOL;
		ta_commit_vertex(&v, sizeof(v));

		c++;
		pos += CHAR_W;
	}
}
