/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2002 Stuart George and Claudio Matsuoka
 *
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 *
 *  MacOS X SDL port by Benjamin Zale <junior_aepi@users.sourceforge.net>
 */

#import <Cocoa/Cocoa.h>
#import "MyWindowController.h"

@interface SDLMain : NSObject
{
}
- (void) applicationDidFinishLaunching: (NSNotification *) note;
- (void)quit:(id)sender;
- (void) setupWorkingDirectory;
@end

