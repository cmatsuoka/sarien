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
#include "pvrutils.h"

/* Linear/iterative twiddling algorithm from Marcus' tatest */
#define TWIDTAB(x) ((x&1)|((x&2)<<1)|((x&4)<<2)|((x&8)<<3)|((x&16)<<4)| \
	((x&32)<<5)|((x&64)<<6)|((x&128)<<7)|((x&256)<<8)|((x&512)<<9)|((x&1024)<<10))
#define TWIDOUT(x, y) ( (TWIDTAB((y))) | ((TWIDTAB((x))) << 1) )

/* Twiddle function -- copies from a source rectangle in SH-4 ram to a
   destination texture in PVR ram. Areas outside the source texture will
   be filled with bgcol. */
void txr_twiddle_copy(const uint16 *src, uint32 srcw, uint32 srch,
		uint32 dest, uint32 destw, uint32 desth, uint16 bgcol) {
	int	x, y;
	uint16	*vtex;
	uint16	val;
	
	vtex = (uint16*)ta_txr_map(dest);
	
	for (y=0; y<desth; y++)
	{
		for (x=0; x<destw; x++)
		{
			if (x >= srcw || y >= srch)
				val = bgcol;
			else
				val = src[y*srcw+x];
			vtex[TWIDOUT(x,y)] = val;
		}
	}
}

/* Twiddle function -- copies from a source rectangle in SH-4 ram to a
   destination texture in PVR ram. The image will be scaled to the texture
   size. */
void txr_twiddle_scale(const uint16 *src, uint32 srcw, uint32 srch,
		uint32 dest, uint32 destw, uint32 desth) {
	int	x, y, srcx, srcy;
	uint16	*vtex;
	uint16	val;
	float	scalex, scaley;
	
	vtex = (uint16*)ta_txr_map(dest);
	scalex = srcw * 1.0f / destw;
	scaley = srch * 1.0f / desth;
	
	for (y=0; y<desth; y++) {
		for (x=0; x<destw; x++) {
			srcx = (int)(x*scalex);
			srcy = (int)(y*scaley);
			val = src[srcy*srcw+srcx];
			vtex[TWIDOUT(x,y)] = val;
		}
	}
}

/* Commits a dummy polygon (for unused lists). Specify the polygon
   type (opaque/translucent). */
void pvr_dummy_poly(int type) {
	poly_hdr_t poly;
	ta_poly_hdr_col(&poly, type);
	ta_commit_poly_hdr(&poly);
}

/* Commits an entire blank frame. Assumes two lists active (opaque/translucent) */
void pvr_blank_frame() {
	ta_begin_render();
	pvr_dummy_poly(TA_OPAQUE);
	ta_commit_eol();
	pvr_dummy_poly(TA_TRANSLUCENT);
	ta_commit_eol();
	ta_finish_frame();
}

void pvr_clear_texture_ram()
{
	uint16 *vtex;

	vtex = (uint16*)ta_txr_map(0);
	memset(vtex, 0, 1024*1024*2);
}