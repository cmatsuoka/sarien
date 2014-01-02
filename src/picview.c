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
#include <ctype.h>
#include "sarien.h"

#ifdef OPT_PICTURE_VIEWER

#include "agi.h"
#include "graphics.h"
#include "keyboard.h"
#include "console.h"
#include "text.h"


static int picviewer_get_key ()
{
	int key;

	/* clear key queue */
	while (keypress ()) { get_key (); }

	_D (_D_WARN "waiting...");
	while (42) {
		poll_timer ();
		key = do_poll_keyboard ();
		if (key) break;
	}

	return key;
}


static void screen_dump ()
{
	FILE *f;

	f = fopen ("screen.raw", "wb");
	fwrite (game.sbuf, _HEIGHT, _WIDTH, f);
	fclose (f);
}


int view_pictures ()
{
	int ec = err_OK;
	char x[64];
	int i, pic = 0, dir = 1;

	game.line_min_print = 1;

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
		
		_D ("picture = %d", pic);
		if ((ec = agi_load_resource (rPICTURE, pic)) != err_OK) {
			_D (_D_CRIT "Whoops. bad pic %d", pic);
			ec = err_OK;
			pic += dir;
			goto next_pic;
		}

		print_text ("[drawing]", 0, 16, 0, strlen (x) + 1, 0, 15);

		/* decodes the raw data to useable form */
		decode_picture (pic, TRUE);

		show_pic ();
		put_screen ();
		
update_statusline:
#ifdef USE_HIRES
		sprintf (x, "Picture:%3i                  Hi-res: %3s",
			pic, opt.hires ? " on" : "off");
#else
		sprintf (x, "Picture:%3i                  Hi-res: N/A", pic);
#endif
		print_text (x, 0, 0, 0, strlen (x) + 1, 0, 15);
		sprintf (x, "H:Hi-res    P:Vis/Prio   +:Next   -:Prev");
		print_text (x, 0, 0, 23, strlen (x) + 1, 15, 0);
		sprintf (x, "R:Redraw      D:Screen dump       Q:Quit");
		print_text (x, 0, 0, 24, strlen (x) + 1, 15, 0);

		while (42) {
			decode_picture (pic, TRUE);
    			switch (picviewer_get_key()) {
    			case 'q':
				goto end_view;
#ifdef USE_HIRES
#ifdef USE_MOUSE
			case BUTTON_RIGHT:
#endif
    			case 'h':
				opt.hires = !opt.hires;
				show_pic ();
				put_screen ();
				goto update_statusline;
#endif
    			case 'p':
				debug.priority = !debug.priority;
				show_pic ();
				put_screen ();
    				break;
			case 'd':
				/*opt.showscreendraw = !opt.showscreendraw;*/
				screen_dump();
				goto update_statusline;
			case 'r':
				goto next_pic;
#ifdef USE_MOUSE
			case BUTTON_LEFT:
				if (mouse.x < GFX_WIDTH / 2) 
					goto previous_pic;
#endif
				/* fall through */
    			case '+':
				_D ("next pic");
 				if (pic < MAX_DIRS - 1)
    					pic++;
    				else
    					pic = 0;
    				dir = 1;
				goto next_pic;
    			case '-':
			previous_pic:
				_D ("previous pic");
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

#endif /* OPT_PICTURE_VIEWER */

