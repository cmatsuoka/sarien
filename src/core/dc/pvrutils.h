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

#ifndef __PVRUTILS_H
#define __PVRUTILS_H

/* Twiddle function -- copies from a source rectangle in SH-4 ram to a
   destination texture in PVR ram. */
void txr_twiddle_copy(const uint16 *src, uint32 srcw, uint32 srch,
		uint32 dest, uint32 destw, uint32 desth, uint16 bkg);

/* Twiddle function -- copies from a source rectangle in SH-4 ram to a
   destination texture in PVR ram. The image will be scaled to the texture
   size. */
void txr_twiddle_scale(const uint16 *src, uint32 srcw, uint32 srch,
		uint32 dest, uint32 destw, uint32 desth);

/* Commits a dummy polygon (for unused lists). Specify the polygon
   type (opaque/translucent). */
void pvr_dummy_poly(int type);

/* Commits an entire blank frame. Assumes two lists active (opaque/translucent) */
void pvr_blank_frame();
void pvr_clear_texture_ram();

#endif	/* __PVRUTILS_H */
