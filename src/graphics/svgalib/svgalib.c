/*
 *  Sarien AGI :: Copyright (C) 1998 Dark Fiber
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 *
 * SVGALib port by XoXus <xoxus@usa.net>
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>
#include <unistd.h>

#include <vga.h>
#include <vgakeyboard.h>

#include "sarien.h"
#include "gfx.h"
#include "keyboard.h"
#include "svgalib.h"

static UINT8 *video_buffer;
static void *svgalib_framebuffer;

static int scale = 1;
static int key = 0;
static int key_control = 0;
static int key_alt = 0;
static int __argc;
static char **__argv;


UINT32 clock_ticks;
UINT32 clock_count;

#define TICK_SECONDS 20

static __GFX_DRIVER GFX_svgalib =
{
	init_vidmode,
	deinit_vidmode,
	put_block,
	_put_pixel,
	new_timer,
	keypress,
	get_key
};


static char key_state[128];

static UINT8 svgalib_key_init ()
{
	int err;

	err = atexit (svgalib_key_close);
	if (err) {
		fprintf (stderr, "Couldn't register svgalib_key_close"
							"with atexit!\n");
		perror ("atexit");
		return err_Unk;
	}

	err = keyboard_init ();
	if (err) {
		fprintf (stderr, "Couldn't init. keyboard!\n");
		return err_Unk;
	}

	keyboard_seteventhandler (svgalib_key_handler);
	svgalib_key_flush ();

	return err_OK;
}


static void svgalib_key_close ()
{
	keyboard_setdefaulteventhandler ();
	keyboard_clearstate ();
	keyboard_translatekeys (0);
	keyboard_close ();
}


static void svgalib_key_flush ()
{
	keyboard_update ();
	keyboard_clearstate ();
	memset (key_state, 0, 128);
}


static void svgalib_key_update ()
{
	keyboard_update ();
}


static struct mapping {
	int scancode, ascii;
} key_map[] = {
	{SCANCODE_BACKSPACE, '\b'}, {SCANCODE_TAB, '\t'},

	{SCANCODE_1, '1'}, {SCANCODE_2, '2'}, {SCANCODE_3, '3'},
	{SCANCODE_4, '4'}, {SCANCODE_5, '5'}, {SCANCODE_6, '6'},
	{SCANCODE_7, '7'}, {SCANCODE_8, '8'}, {SCANCODE_9, '9'},
	{SCANCODE_0, '0'},

	{SCANCODE_A, 'a'}, {SCANCODE_B, 'b'}, {SCANCODE_C, 'c'},
	{SCANCODE_D, 'd'}, {SCANCODE_E, 'e'}, {SCANCODE_F, 'f'},
	{SCANCODE_G, 'g'}, {SCANCODE_H, 'h'}, {SCANCODE_I, 'i'},
	{SCANCODE_J, 'j'}, {SCANCODE_K, 'k'}, {SCANCODE_L, 'l'},
	{SCANCODE_M, 'm'}, {SCANCODE_N, 'n'}, {SCANCODE_O, 'o'},
	{SCANCODE_P, 'p'}, {SCANCODE_Q, 'q'}, {SCANCODE_R, 'r'},
	{SCANCODE_S, 's'}, {SCANCODE_T, 't'}, {SCANCODE_U, 'u'},
	{SCANCODE_V, 'v'}, {SCANCODE_W, 'w'}, {SCANCODE_X, 'x'},
	{SCANCODE_Y, 'y'}, {SCANCODE_Z, 'z'},

	{SCANCODE_BRACKET_LEFT, '['}, {SCANCODE_BRACKET_RIGHT, ']'},
	{SCANCODE_MINUS, '-'}, {SCANCODE_EQUAL, '='},
	{SCANCODE_SEMICOLON, ';'}, {SCANCODE_APOSTROPHE, '\''},
	{SCANCODE_GRAVE, '`'}, {SCANCODE_BACKSLASH, '\\'},
	{SCANCODE_COMMA, ','}, {SCANCODE_PERIOD, '.'},
	{SCANCODE_SLASH, '/'}, {SCANCODE_SPACE, ' '},

	{0, 0}
};

#define INRANGE(x,lower,upper)		((x >= lower) && (x <= upper))

static void svgalib_key_handler (int scancode, int press)
{
	int i;

	/* Trim high bytes off (for safety) */
	scancode &= 0x7F;
	press &= 0x7F;

	if (key_state[scancode] == press)
		return;			/* Key held down */

	key_state[scancode] = press;

	if (press == KEY_EVENTRELEASE) {
		/* Key released */
		if (scancode == SCANCODE_LEFTCONTROL)
			key_control &= ~1;
		if (scancode == SCANCODE_RIGHTCONTROL)
			key_control &= ~2;
		if (scancode == SCANCODE_LEFTALT)
			key_alt &= ~1;
		if (scancode == SCANCODE_RIGHTALT)
			key_alt &= ~2;
		return;
	}

	if ((scancode == SCANCODE_LEFTSHIFT) ||
	    (scancode == SCANCODE_RIGHTSHIFT))
		key = 0;
	else if (scancode == SCANCODE_LEFTCONTROL) {
		key_control |= 1;
		key = 0;
	}
	else if (scancode == SCANCODE_RIGHTCONTROL) {
		key_control |= 2;
		key = 0;
	}
	else if (scancode == SCANCODE_LEFTALT) {
		key_alt |= 1;
		key = 0;
	}
	else if (scancode == SCANCODE_RIGHTALT) {
		key_alt |= 2;
		key = 0;
	}
	else if ((scancode == SCANCODE_CURSORUP) ||
	    (scancode == SCANCODE_CURSORBLOCKUP))
		key = KEY_UP;
	else if ((scancode == SCANCODE_CURSORDOWN) ||
	    (scancode == SCANCODE_CURSORBLOCKDOWN))
		key = KEY_DOWN;
	else if ((scancode == SCANCODE_CURSORLEFT) ||
	    (scancode == SCANCODE_CURSORBLOCKLEFT))
		key = KEY_LEFT;
	else if ((scancode == SCANCODE_CURSORRIGHT) ||
	    (scancode == SCANCODE_CURSORBLOCKRIGHT))
		key = KEY_RIGHT;
	else if (scancode == SCANCODE_CURSORUPLEFT)
		key = KEY_UP_LEFT;
	else if (scancode == SCANCODE_CURSORUPRIGHT)
		key = KEY_UP_RIGHT;
	else if (scancode == SCANCODE_CURSORDOWNLEFT)
		key = KEY_DOWN_LEFT;
	else if (scancode == SCANCODE_CURSORDOWNRIGHT)
		key = KEY_DOWN_RIGHT;
	else if ((scancode == SCANCODE_ENTER) ||
	    (scancode == SCANCODE_KEYPADENTER))
		key = KEY_ENTER;
	else if (scancode == SCANCODE_KEYPADPLUS)
		key = '+';
	else if (scancode == SCANCODE_KEYPADMINUS)
		key = '-';
	else if (scancode == SCANCODE_REMOVE)
		key = 0x53;
	else if (scancode == SCANCODE_INSERT)
		key = 0x52;
	else if ((scancode >= SCANCODE_F1) &&
	    (scancode <= SCANCODE_F10)) {
		int ftable[10] = {
			0x3B00, 0x3C00, 0x3D00, 0x3E00, 0x3F00,
			0x4000, 0x4100, 0x4200, 0x4300, 0x4400};
		key = ftable[(scancode - SCANCODE_F1)];
	}
	else if (scancode == SCANCODE_ESCAPE)
		key = KEY_ESCAPE;
	else {
		/* Map scancode to ASCII key */
		for (i=0; ; i++) {
			if (key_map[i].scancode == 0)
				break;
			if (key_map[i].scancode == scancode) {
				key = key_map[i].ascii;
				break;
			}
		}
		if (INRANGE (scancode, SCANCODE_Q, SCANCODE_P) ||
		    INRANGE (scancode, SCANCODE_A, SCANCODE_L) ||
		    INRANGE (scancode, SCANCODE_Z, SCANCODE_M)) {
			if (key_control || key_alt) {
				key &= ~0x20;
				key -= 0x40;
			}
			if (key_alt) {
				key--;
				key = (scancode_table[key] << 8);
			}
		}
	}
}


static void process_events ()
{
	svgalib_key_update ();
}


UINT16 init_machine (int argc, char **argv)
{
	gfx = &GFX_svgalib;

	__argc = argc;
	__argv = argv;
	scale = 1;

	screen_mode = GFX_MODE;

	clock_count = 0;
	clock_ticks = 0;

	return err_OK;
}


UINT16 deinit_machine ()
{
	return err_OK;
}


static UINT16 init_vidmode ()
{
	int err, i;

	fprintf (stderr, "svgalib: SVGAlib support by XoXus\n");

	err = vga_init ();
	if (err) {
		fprintf (stderr, "svgalib: can't initialize svgalib\n");
		return err_Unk;
	}
	err = vga_hasmode (G320x200x256);
	if (err == 0) {
		fprintf (stderr,
			"svgalib: video mode unavailable (320x200x256)\n");
		return err_Unk;
	}

	vga_setmode (G320x200x256);
	svgalib_framebuffer = vga_getgraphmem ();

	/* Set up EGA colors */
	for (i=0; i < 32; i++)
		vga_setpalette (i, palette[i * 3],
			palette[i * 3 + 1], palette[i * 3 + 2]);

	/* Allocate framebuffer */
	video_buffer = (UINT8 *) malloc (320 * 200);
	if (!video_buffer) {
		fprintf (stderr, "svgalib: can't alloc framebuffer\n");
		deinit_vidmode ();
		return err_Unk;
	}

	screen_mode = GFX_MODE;

	/* XoXus: This is a semantics issue, but the keyboard init
			probably shouldn't be done in 'init_vidmode'
	*/
	err = svgalib_key_init ();
	if (err != err_OK) {
		deinit_vidmode ();
		return err_Unk;
	}

	return err_OK;
}


static UINT16 deinit_vidmode ()
{
	vga_setmode (TEXT);
	if (video_buffer)
		free (video_buffer);

	return err_OK;
}


/* put a block onto the screen */
static void put_block (UINT16 x1, UINT16 y1, UINT16 x2, UINT16 y2)
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
	for (i = 0; i < h; i++)
		memcpy (svgalib_framebuffer + 320 * (y1 + i) + x1,
			video_buffer + 320 * (y1 + i) + x1, x2 - x1 + 1);
}


/* put pixel routine */
static void inline _put_pixel (UINT16 x, UINT16 y, UINT16 c)
{
	/* XoXus: FIXME: Is this a 16-bit color? */
	video_buffer[y * 320 + x] = (c & 0xFF);
}


static UINT8 keypress ()
{
	process_events ();
	return !!key;
}

/* XoXus: FIXME: Should get_keypress block? */
static UINT16 get_key ()
{
	UINT16 k;

	process_events ();
	while (key == 0) {
		process_events ();
	}

	k = key;
	key = 0;
	return k;
}


static void new_timer ()
{
	struct timeval tv;
	struct timezone tz;
	static double msec = 0.0;
	static int msg_box_ticks = 0;
	double m, dm;
	
	gettimeofday (&tv, &tz);
	m = 1000.0 * tv.tv_sec + tv.tv_usec / 1000.0;

	dm = m - msec;
	while (dm < 45)
	{
		usleep (5000);
		gettimeofday (&tv, &tz);
		m = 1000.0 * tv.tv_sec + tv.tv_usec / 1000.0;
		dm = m - msec;
	}
	msec = m; 

	process_events ();
}
