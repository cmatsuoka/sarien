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
#include <string.h>
#include "sarien.h"
#include "agi.h"
#include "gfx_base.h"
#include "picture.h"
#include "sound.h"
#include "console.h"
#include "text.h"


volatile UINT32 clock_ticks;
volatile UINT32 clock_count;
struct sarien_options opt;

static UINT8 mode;

extern UINT8 *cur_font, font_english[];

#ifndef _TRACE
INLINE void _D (char *s, ...) { }
#endif


static int view_pictures ()
{
	int ec = err_OK;
	char x[64];
	int i, pic = 0, dir = 1;

	mode = 'v';

	for (i = 0; ec == err_OK; i = 1) {
		while (game.dir_pic[pic].offset == _EMPTY) {
			pic += dir;
			if (pic < 0)
				pic = MAX_DIRS - 1;

			if (pic > MAX_DIRS - 1) {
				pic = 0;
				if (i == 0) {		/* no pics? */
					ec = 1;
					fprintf (stderr, "No pictures found\n");
					goto end_view;
				}
			}
		}
		
		if ((ec = agi_load_resource (rPICTURE, pic)) != err_OK)
			continue;

		sprintf (x, "Picture:%3i     [drawing]     Show: %3s",
			pic, 0 /*opt.showscreendraw*/ ? " on" : "off");
		print_text (x, 0, 4, 190, strlen (x) + 1, 15, 0);

		/* decodes the raw data to useable form */
		decode_picture (pic);
		show_buffer (mode);

update_statusline:
		sprintf (x, "V:Vis C:Con P:Pri X:P+C   +:Next -:Prev");
		print_text (x, 0, 4, 170, strlen (x) + 1, 15, 0);
		sprintf (x, "R:Redraw      D:Show toggle      Q:Quit");
		print_text (x, 0, 4, 180, strlen (x) + 1, 15, 0);
		sprintf (x, "Picture:%3i                   Show: %3s",
			pic, opt.showscreendraw ? " on" : "off");
		print_text (x, 0, 4, 190, strlen (x) + 1, 15, 0);

		put_screen ();

		while (42) {
			decode_picture (pic);
    			switch (tolower (get_key() & 0xFF)) {
    			case 'q':
				goto end_view;
    			case 'v':
				show_buffer (mode = 'v');
    				break;
    			case 'p':
    			case 'z':
				show_buffer (mode = 'p');
 				break;
    			case 'c':
				show_buffer (mode = 'c');
				break;
			case 'd':
				opt.showscreendraw = !opt.showscreendraw;
				goto update_statusline;
    			case 'x':
				show_buffer (mode = 'x');
  				break;
			case 'r':
				goto next_pic;
    			case '+':
    				
 				if (pic < MAX_DIRS - 1)
    					pic++;
    				else
    					pic = 0;
    				dir = 1;
				goto next_pic;
    			case '-':
    				
    				if (pic > 0)
    					pic--;
    				else
    					pic = MAX_DIRS - 1;
    				i = 0;
    				dir = -1;
				goto next_pic;
    			}
    		}
next_pic:
    		agi_unload_resource (rPICTURE, pic);
    		
	}

end_view:
	return ec;
}



int main(int argc, char *argv[])
{
	int ec;

	/* we must do this before _ANYTHING_ else if using allegro!! */
#ifdef HAVE_ALLEGRO
	allegro_init ();
#endif

	printf(
TITLE " " VERSION " - A Sierra AGI resource interpreter engine.\n"
"Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka\n"
"Portions Copyright (C) 1998 Lance Ewing, (C) 1999 Felipe Rosinha\n"
#ifndef HAVE_GETOPT_LONG
"Portions Copyright (C) 1989-1997 Free Software Foundation, Inc.\n"
#endif
"\n"
"This program is free software; you can redistribute it and/or modify it\n"
"under the terms of the GNU General Public License, version 2 or later,\n"
"as published by the the Free Software Foundation.\n"
"\n");

	opt.scale = 2;

	init_machine (argc, argv);
	screen_mode = GFX_MODE;

	cur_font = font_english;

	ec = agi_detect_game (argc > 1 ? argv[1] : get_current_directory ());
	if (ec != err_OK) {
		ec = err_InvalidAGIFile;
		goto bail_out;
	}

	printf("AGI v%i game detected.\n", agi_version ());

	if (init_video () != err_OK) {
		ec = err_Unk;
		goto bail_out;
	}

    	ec = view_pictures();

    	agi_deinit ();
	deinit_video ();

bail_out:
	if (ec == err_OK || ec == err_DoNothing) {
		deinit_machine ();
		exit (ec);
	}

	printf ("Error %04i: ", ec);

	switch (ec) {
	case err_BadCLISwitch:
		printf("Bad CLI switch.\n");
		break;
	case err_InvalidAGIFile:
		printf("Invalid or inexistent AGI file.\n");
		break;
	case err_BadFileOpen:
		printf("Unable to open file.\n");
		break;
	case err_NotEnoughMemory:
		printf("Not enough memory.\n");
		break;
	case err_BadResource:
		printf("Error in resource.\n");
		break;
	case err_UnknownAGIVersion:
		printf("Unknown AGI version.\n");
		break;
	case err_NoGameList:
		printf("No game ID List was found!\n");
		break;
	}
	printf("\nUse parameter -h to list the command line options\n");

	deinit_machine ();

	return ec;
}


void show_buffer (int mode)
{
	switch (mode) {
	case 'x':
		put_block_buffer (xdata_data);
		break;
	case 'c':
	case 'p':
		break;
	case 'v':
	default:
		dump_screenX ();
		break;
	}
	put_screen ();
}


