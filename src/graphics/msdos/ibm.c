/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
//#include <conio.h>

#ifdef __WATCOMC__
#include <i86.h>
#endif

#include "sarien.h"
#include "graphics.h"

#define KEY_PGUP	0x4A2D	/* keypad + */
#define KEY_PGDN	0x4E2B  /* keypad - */
#define KEY_HOME	0x352F  /* keypad / */
#define KEY_END		0x372A  /* keypad * */

#define __outp(a, b)	outp(a, b)
#define move_memory(a, b, c) memmove((char*)a, (char*)b, (UINT32)c)

#ifdef __WATCOMC__
void DebugBreak(void);
#pragma aux DebugBreak = "int 3" parm[];
#endif


extern struct gfx_driver *gfx;
extern struct sarien_options opt;

UINT8	*exec_name;
UINT8	*screen_buffer;

void	(__interrupt __far *prev_08)	(void);
void	__interrupt __far new_timer	(void);
static int	pc_init_vidmode	(void);
static int	pc_deinit_vidmode	(void);
static void	pc_put_block		(int, int, int, int);
static void	pc_put_pixels		(int, int, int, UINT8 *);
static void	pc_dummy		(void);
static int	pc_get_key		(void);
static int	pc_keypress		(void);


#define TICK_SECONDS 18

static struct gfx_driver GFX_ibm = {
	pc_init_vidmode,
	pc_deinit_vidmode,
	pc_put_block,
	pc_put_pixels,
	pc_dummy,
	pc_keypress,
	pc_get_key
};

static void pc_dummy ()
{
	/* dummy */
	static UINT32 cticks = (SINT32)-1;

	while(cticks==clock_ticks);
	cticks=clock_ticks;
}


int init_machine (int argc, char **argv)
{
	gfx = &GFX_ibm;

	exec_name = (UINT8*)strdup(argv[0]);

	screen_buffer = (UINT8*)malloc (GFX_WIDTH*GFX_HEIGHT);

	clock_count=0;
	clock_ticks=0;

	prev_08 = _dos_getvect(0x08);
	_dos_setvect(0x08, new_timer);

	return err_OK;
}

int deinit_machine ()
{
	free (exec_name);
	free (screen_buffer);
	_dos_setvect (0x08, prev_08);

	return err_OK;
}

static int pc_init_vidmode ()
{
	union REGS r;
	int i;

	memset (&r, 0x0, sizeof(union REGS));
#ifdef __WATCOMC__
	r.w.ax = 0x13;
	int386 (0x10, &r, &r);
	__outp(0x3C8, 0l);
	for (i = 0; i < 16 * 3; i++)
		__outp (0x3c9, palette[i]);
#endif
	
#ifdef __TURBOC__
	r.x.ax = 0x13;
	int86 (0x10, &r, &r);
	outportb (0x3c8, 0);
	for (i = 0; i < 16 * 3; i++)
		outportb (0x3c9, palette[i]);
#endif




	return err_OK;
}


static int pc_deinit_vidmode ()
{
	union REGS r;

	memset (&r, 0x0, sizeof(union REGS));

#ifdef __WATCOMC__
	r.w.ax = 0x03;
	int386 (0x10, &r, &r);
#endif

#ifdef __TURBOC__
	r.x.ax = 0x03;
	int86 (0x10, &r, &r);
#endif

	return err_OK;
}


/* blit a block onto the screen */
static void pc_put_block (int x1, int y1, int x2, int y2)
{
	int i, h;

	if (x1 >= GFX_WIDTH)
		x1 = GFX_WIDTH - 1;
	if (y1 >= GFX_HEIGHT)
		y1 = GFX_HEIGHT - 1;
	if (x2 >= GFX_WIDTH)
		x2 = GFX_WIDTH - 1;
	if (y2 >= GFX_HEIGHT)
		y2 = GFX_HEIGHT - 1;

	h = y2 - y1 + 1;
	for (i = 0; i < h; i++) {
		memcpy ((UINT8*)0xA0000 + 320 * (y1 + i) + x1,
			screen_buffer + 320 * (y1 + i) + x1, x2 - x1 + 1);
	}
}


static void pc_put_pixels(int x, int y, int w, UINT8 *p)
{
	UINT8 *s = &screen_buffer[y * 320 + x];
	while (w--) *s++ = *p++;
}


static int pc_keypress ()
{
	return !!kbhit();
}


static int pc_get_key ()
{
	union REGS r;
	UINT16 key;

	memset (&r, 0, sizeof(union REGS));
#ifdef __WATCOMC__
	int386 (0x16, &r, &r);
	switch (key = r.w.ax)
#endif
#ifdef __TURBOC__
	int86 (0x16, &r, &r);
	switch (key = r.x.ax)
#endif
	
	{
		case KEY_PGDN:
		case KEY_PGUP:
		case KEY_HOME:
		case KEY_END:
			break;
		default:
			if(r.h.al == 0)
				key = r.h.ah << 8;
			else
				key = r.h.al;
			break;
	}

	return key;
}

/* WATCOM HATES timer routines... */
/* lucky we call no other routines inside our timer */
/* coz SS!=DS and watcom wants SS==DS but it aint inside a timer! */

void __interrupt __far new_timer (void)
{
//	union REGS	r;
//	UINT16		key;
	static UINT32 msg_box_ticks = 0;

	clock_ticks++;

	_chain_intr(prev_08);
}
