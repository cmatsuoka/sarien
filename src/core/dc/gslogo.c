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

IMAGE img;
uint32 size;
uint32 g_tex_addr;
#define Y_OFFS	(20.0f)

void gs_draw_bkg(const float fade, uint32 tex_addr)
{
	vertex_ot_t	v;
	poly_hdr_t	hdr;
	
	ta_poly_hdr_txr(&hdr, TA_OPAQUE, TA_RGB565, 1024, 512, tex_addr, TA_NO_FILTER);
	ta_commit_poly_hdr(&hdr);
	
	/* - -
	   + - */
	v.flags = TA_VERTEX_NORMAL;
	v.x = 0.0f;
	v.y = 512.0f + Y_OFFS;
	v.z = 1.0f;
	v.u = 0.0f; v.v = 1.0f;
	v.a = 1.0f; v.r = v.g = v.b = fade;
	v.oa = v.or = v.og = v.ob = 0.0f;
	ta_commit_vertex(&v, sizeof(v));
	
	/* + -
	   - - */
	v.y = Y_OFFS;
	v.v = 0.0f;
	ta_commit_vertex(&v, sizeof(v));
	
	/* - -
	   - + */
	v.x = 1024.0f;
	v.y = 512.0f + Y_OFFS;
	v.u = 1.0f; v.v = 1.0f;
	ta_commit_vertex(&v, sizeof(v));

	/* - +
	   - - */
	v.flags = TA_VERTEX_EOL;
	v.y = Y_OFFS;
	v.v = 0.0f;
	ta_commit_vertex(&v, sizeof(v));
}

int init_logo(char *file)
{
	if(load_gsi(&img, file) == -1)
	{
		printf("couldn't load image\r\n");
		return -1;
	}

	size = img.w * img.h * 2;
	g_tex_addr = ta_txr_allocate(size);
	ta_txr_load(g_tex_addr, img.data, size);
	free(img.data);
}

void gs_logo(int numframes)
{
	float f;
	int i;

	for (f=0.0f; f<1.0f; f+=0.01f) {
		ta_begin_render();
		gs_draw_bkg(f, g_tex_addr);
		ta_commit_eol();
		pvr_dummy_poly(TA_TRANSLUCENT);
		ta_commit_eol();
		ta_finish_frame();
	}
	
	for (i=0; i < numframes; i++) {
		ta_begin_render();
		gs_draw_bkg(1.0f, g_tex_addr);
		ta_commit_eol();
		pvr_dummy_poly(TA_TRANSLUCENT);
		ta_commit_eol();
		ta_finish_frame();
	}

	for (f=1.0f; f>0.0f; f-=0.01f) {
		ta_begin_render();
		gs_draw_bkg(f, g_tex_addr);
		ta_commit_eol();
		pvr_dummy_poly(TA_TRANSLUCENT);
		ta_commit_eol();
		ta_finish_frame();
	}

	pvr_blank_frame();
	pvr_blank_frame();
	pvr_blank_frame();
}

void shutdown_logo()
{
	ta_txr_release_all();
}
