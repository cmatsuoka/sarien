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

#ifndef _IMAGE_H
#define _IMAGE_H

typedef struct IMAGE
{
	uint16 w, h;
	uint16 bpp;
	uint16 *data;
} IMAGE;

void blt_image(int xpos, int ypos, IMAGE *img, uint16 *vram, uint16 scr_w);
void fade_img(int xpos, int ypos, IMAGE *img, uint16 *vram, uint16 scr_w, uint8 step, uint8 num_steps);
void blt_image_trans(int xpos, int ypos, IMAGE *img, uint16 *vram, uint16 scr_w, uint16 trans);
void blt_fill(int xpos, int ypos, int w, int h, uint16 *vram, uint16 scr_w, uint16 color);
IMAGE *grab_img(int xpos, int ypos, int w, int h, uint16 *vram, uint16 scr_w);
void blt_imageX(int xpos, int ypos, int xstart, int ystart, int w, int h, IMAGE *img, uint16 *vram, uint16 scr_w);

#endif
