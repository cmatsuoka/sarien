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

@interface SarienWindow : NSWindow
{
    int maxScale;
    int scale;
}

- (void)awakeFromNib;
- (void)close;

- (void)setScale:(int)newScale;
- (IBAction)setScale1x:(id)sender;
- (IBAction)setScale2x:(id)sender;
- (IBAction)setScale3x:(id)sender;
- (IBAction)setScale4x:(id)sender;

@end

