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

typedef struct GSIHEADER
{
	char	id[4];
	long	version;
	long	format;
	long	data_size;
	long	width;
	long	height;
	long	bpp;
} GSIHEADER;

enum
{
	FORMAT_ARGB1555,
	FORMAT_RGB565,
};

int load_gsi(IMAGE *img, char *szFile);
int load_gsi_into_mem(char *szFile, IMAGE *img, uint32 max_size);
