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

@interface MyWindowController : NSWindowController
{
    IBOutlet id A_Box;
    IBOutlet id a_Box;
    IBOutlet id C_Box;
    IBOutlet id d_Box;
    IBOutlet id E_Box;
    IBOutlet id E_PopUp;
    IBOutlet id F_Box;
    IBOutlet id g_Box;
    IBOutlet id H_Box;
    IBOutlet id L_Box;
    IBOutlet id m_Box;
    IBOutlet id n_Box;
    IBOutlet id o_Box;
    IBOutlet id p_Box;
    IBOutlet id r_Box;
    IBOutlet id S_Box;
    IBOutlet id S_TextField;
    IBOutlet id V_Box;
    IBOutlet id v_Box;
    IBOutlet id v_PopUp;
    IBOutlet id CurrentDir;
}
- (IBAction)EmulateSoundCheck:(id)sender;
- (IBAction)ForceEmuCheck:(id)sender;
- (IBAction)QuitButton:(id)sender;
- (IBAction)RunButton:(id)sender;
- (IBAction)WindowSizeCheck:(id)sender;
- (IBAction)ChooseDirButton:(id)sender;
@end
