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

struct agi_picture {
	UINT32	flen;			/* size of raw data */
	UINT8	*rdata;			/* raw vector image data */
	UINT8	*sdata;			/* screen bitmap image */
	UINT8	*pdata;			/* priority bitmap image */
	UINT8	*cdata;			/* control lines image */
	UINT8	*xdata;			/* combined p+c images */
};

/* kludge */
extern UINT8	sprite_data[];
extern UINT8	screen_data[];
extern UINT8	screen2[];
extern UINT8	xdata_data[];
extern UINT8	control_data[];
extern UINT8	priority_data[];
extern UINT8	pic_clear_flag;


int	decode_picture (int);
int	unload_picture (int);

void	dump_screen (int);
void	dump_pri (int);
void	dump_con (int);
void	dump_x (int);

void	dump_pri_screen (void);
void	dump_con_screen (void);
void	dump_x_screen (void);
void	dump_screenX (void);
void	dump_screen2 (void);
void	put_block_buffer (UINT8 *, int, int, int, int);
UINT8* 	convert_v2_v3_pic (UINT8 *data, UINT32 len);
void 	reset_graphics(void);

#ifdef __cplusplus
};
#endif
#endif
