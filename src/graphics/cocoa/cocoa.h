/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 *
 *  MacOS X port by Richard Houle <richard.houle@sandtechnology.com>
 */

#import "SarienView.h"

extern int    __argc;
extern char** __argv;

extern SarienView* sarienView;

int  cocoa_init      (void);
int  cocoa_deinit    (void);
void cocoa_put_block (int, int, int, int);
void cocoa_put_pixels(int, int, int, UINT8 *);
int  cocoa_keypress  (void);
int  cocoa_get_key   (void);
void cocoa_timer     (void);

extern struct gfx_driver *gfx;

