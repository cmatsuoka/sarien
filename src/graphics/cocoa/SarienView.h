/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2002 Stuart George and Claudio Matsuoka
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

#define KEY_QUEUE_SIZE 32

@interface SarienView : NSView
{
    unsigned int *screen;
    CGImageRef    screenImage;
    NSLock       *mutex;

    unichar key_queue[KEY_QUEUE_SIZE];
    int     key_queue_start;
    int     key_queue_end;
}

- (unsigned int*)screenContent;

- (id)initWithFrame:(NSRect)frameRect;
- (void)dealloc;

- (BOOL)acceptsFirstResponder;
- (void)keyDown:(NSEvent*)event;

- (BOOL)hasPendingKey;
- (int)getKey;

- (void)displayIfNeeded;
- (BOOL)needsDisplay;

- (void)lock;
- (void)unlock;

@end
