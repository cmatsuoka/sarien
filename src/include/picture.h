/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#ifndef __AGI_PICTURE_H
#define __AGI_PICTURE_H

#ifdef __cplusplus
extern "C"{
#endif

/**
 * AGI picture resource.
 */
struct agi_picture {
	UINT32	flen;			/**< size of raw data */
	UINT8	*rdata;			/**< raw vector image data */
};

int	decode_picture	(int, int);
int	unload_picture	(int);
void	show_pic	(void);
UINT8* 	convert_v3_pic	(UINT8 *data, UINT32 len);

#ifdef __cplusplus
};
#endif

#endif /* __AGI_PICTURE_H */
