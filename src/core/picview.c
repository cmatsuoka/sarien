/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#include "sarien.h"

#ifdef OPT_PICTURE_VIEWER

#include <stdio.h>
#include <string.h>
#include <ctype.h>
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
		key = do_poll_keyboard ();
		if (key) break;
	}

	return key;
}


int view_pictures ()
{
	int ec = err_OK;
	char x[64];
	int i, pic = 0, dir = 1;
	int mode;

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
		
		_D ("picture = %d", pic);
		if ((ec = agi_load_resource (rPICTURE, pic)) != err_OK)
			continue;

		sprintf (x, "Picture:%3i     [drawing]      Show: %3s",
			pic, 0 /*opt.showscreendraw*/ ? " on" : "off");
		print_text (x, 0, 0, 0, strlen (x) + 1, 0, 15);

		/* decodes the raw data to useable form */
		decode_picture (pic, TRUE);
		show_pic ();
		put_screen ();
		
update_statusline:
		sprintf (x, "Picture:%3i                    Show: %3s",
			pic, 0 /*opt.showscreendraw*/ ? " on" : "off");
		print_text (x, 0, 0, 0, strlen (x) + 1, 0, 15);
		sprintf (x, "V:Visible    P:Priority    +:Next -:Prev");
		print_text (x, 0, 0, 23, strlen (x) + 1, 15, 0);
		sprintf (x, "R:Redraw     D:Show drawing       Q:Quit");
		print_text (x, 0, 0, 24, strlen (x) + 1, 15, 0);

		while (42) {
			decode_picture (pic, TRUE);
    			switch (tolower (picviewer_get_key() & 0xff)) {
    			case 'q':
				goto end_view;
    			case 'v':
				debug.priority = 0;
				show_pic ();
				put_screen ();
    				break;
    			case 'p':
				debug.priority = 1;
				show_pic ();
				put_screen ();
    				break;
			case 'd':
				/*opt.showscreendraw = !opt.showscreendraw;*/
				goto update_statusline;
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

#endif /* OPT_PICTURE_VIEWER */

