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

#import "SarienApplication.h"

#include <unistd.h>
#include "cocoa.h"

@implementation SarienApplication

- (void)selectDidEnd:(NSWindow *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo
{
    NSLog(@"selectDidEnd");
}

- (void)finishLaunching
{
    NSOpenPanel* panel;

    [super finishLaunching];
    NSLog(@"SarienApplication finishLaunching");

    panel = [NSOpenPanel openPanel];
    [panel setCanChooseFiles: NO];
    [panel setCanChooseDirectories: YES];
    [panel setDirectory:NSHomeDirectory()];
    
    if ([panel runModal])
    {
        chdir([[panel directory] cString]);
        [NSThread detachNewThreadSelector:@selector(gameThread:) toTarget:self withObject:self];
    }
    else
    {
        [NSApp terminate: self];
    }
}

- (void)gameThread:(id)info
{
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

    gamemain(__argc, __argv);
    [pool release];
}

@end
