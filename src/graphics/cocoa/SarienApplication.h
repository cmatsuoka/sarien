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

#import <Cocoa/Cocoa.h>
#include "sarien.h"
#include "graphics.h"

extern int gamemain(int argc, char** argv);

@interface SarienApplication : NSApplication
{
}

- (void)finishLaunching;
- (void)gameThread:(id)info;

- (void)selectDidEnd:(NSWindow *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo;

@end
