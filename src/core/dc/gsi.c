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

////////////////////////////////////////////////////////////////////////
// file_size
//
// Gets size of a file
// in: file descriptor
// out: file size
////////////////////////////////////////////////////////////////////////
int file_size(int f)
{
	int num_bytes = 0;

	fs_seek(f, 0, SEEK_END);
	num_bytes = fs_tell(f);
	fs_seek(f, 0, SEEK_SET);
	printf("file size: %d bytes\r\n", num_bytes);

	return num_bytes;
}

int load_gsi(IMAGE *img, char *szFile)
{
	uint32 f = 0;
	GSIHEADER gsih;

	printf("Opening %s\r\n", szFile);

	f = fs_open(szFile, O_RDONLY);

	if(f == 0)
	{
		printf("Couldn't open file: %s\r\n", szFile);
		return -1;
	}
	else
	{
		fs_read(f, &gsih, sizeof(GSIHEADER));

		if(gsih.version == 1)
		{
			gsih.bpp = gsih.height;
			gsih.height = gsih.width;
			gsih.width = gsih.data_size;
			gsih.data_size = gsih.format;
			gsih.format = FORMAT_RGB565;
		}
		
		printf("format=%d\r\n", (int)gsih.format);
		printf("data_size=%d\r\n", (int)gsih.data_size);
		printf("width=%d\r\n", (int)gsih.width);
		printf("height=%d\r\n", (int)gsih.height);
		printf("bpp=%d\r\n", (int)gsih.bpp);

		img->w = gsih.width;
		img->h = gsih.height;
		img->bpp = gsih.bpp;

		img->data = malloc(gsih.data_size);
		fs_read(f, img->data, gsih.data_size);
		fs_close(f);
		return 0;
	}
}

void free_gsi(IMAGE *img)
{
	free(img->data);
}
