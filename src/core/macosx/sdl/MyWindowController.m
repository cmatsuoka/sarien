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

#import "MyWindowController.h"
#include <stdio.h>
#include <string.h>

char **gArgv;    
int gArgc;

BOOL SelectDir = NO;
NSOpenPanel * panel;

//This method adds a string to the command line
int Add_To_CommandLine(char * Source)
{
    char * cp_temp;
    int i_size,i;
    const char ** temp_argv;

    //Step 1: Check the arguments
    if (Source == NULL)
	return -1;

    //Step 2: Build the cp_temp
    i_size = (strlen(Source));
    cp_temp = malloc(sizeof(char * )*(i_size + 1));
    if (cp_temp == NULL)
	return -1;
    strcpy(cp_temp, Source);

    //Step 3: Increment the Argc
    gArgc++;
    //Step 4: Reallocate Argv
    temp_argv = (char**) malloc (sizeof(char * ) * (gArgc+1));
    if (temp_argv == NULL)
	return -1;

    //Step 5: Copy the Source to the end of Argv list
    for(i=0; i< gArgc-1; i++)
	temp_argv[i] = gArgv[i];
    temp_argv[gArgc-1] = cp_temp;

    //Step 6: Copy temp_argv to gArgv
    gArgv = temp_argv;
    //Done: Wasn't that easy ;-)
    return 0;
}

@implementation MyWindowController
    int status;

- (IBAction)EmulateSoundCheck:(id)sender
{
    if ([E_PopUp isEnabled] == YES)
	[E_PopUp setEnabled:NO];
    else
	[E_PopUp setEnabled:YES];
}

- (IBAction)ForceEmuCheck:(id)sender
{
    if ([v_PopUp isEnabled] == YES)
	[v_PopUp setEnabled:NO];
    else
	[v_PopUp setEnabled:YES];
}

- (IBAction)QuitButton:(id)sender
{
    exit(0);
}

- (IBAction)RunButton:(id)sender
{
    char  * s_temp;
    
    //	This line is added because the real argv is out of scope.
    //  Todo: Fix this to use the real application name aka *argv[0]
    Add_To_CommandLine("Sarien_for_OS_X_using_SDL");
    
    if([a_Box state] == NSOnState)
	Add_To_CommandLine("-a");
    if([A_Box state] == NSOnState)
	Add_To_CommandLine("-A");
    if([C_Box state] == NSOnState)
	Add_To_CommandLine("-C");
    if([d_Box state] == NSOnState)
	Add_To_CommandLine("-d");
    if([E_Box state] == NSOnState)
    {
	strcpy(s_temp, "-E ");
	strcat(s_temp, [[E_PopUp titleOfSelectedItem] cString]);
	Add_To_CommandLine(s_temp);
    }
    if([F_Box state] == NSOnState)
	Add_To_CommandLine("-F");
    if([g_Box state] == NSOnState)
	Add_To_CommandLine("-g");
    if([H_Box state] == NSOnState)
	Add_To_CommandLine("-H 1");
    if([L_Box state] == NSOnState)
	Add_To_CommandLine("-L");
    if([m_Box state] == NSOnState)
	Add_To_CommandLine("-m");
    if([n_Box state] == NSOnState)
	Add_To_CommandLine("-n");
    if([o_Box state] == NSOnState)
	Add_To_CommandLine("-o");
    if([p_Box state] == NSOnState)
	Add_To_CommandLine("-p");
    if([r_Box state] == NSOnState)
	Add_To_CommandLine("-r");
    if([S_Box state] == NSOnState)
    {
	strcpy(s_temp, "-S ");
	strcat(s_temp, [[S_TextField stringValue] cString]);
	Add_To_CommandLine(s_temp);
    }    
    if([V_Box state] == NSOnState)
	Add_To_CommandLine("-V");
    if([v_Box state] == NSOnState)
    {
	strcpy(s_temp, "-v ");
	strcat(s_temp, [[v_PopUp titleOfSelectedItem] cString]);
	Add_To_CommandLine(s_temp);
    }
    if (SelectDir == YES)
    {
	Add_To_CommandLine((char *)[[CurrentDir stringValue] cString]);
    }

    //Close the Option Window
    [super close];
    
    /* Hand off to main application code */
    status = SDL_main (gArgc, gArgv);

    /* We're done, thank you for playing */
    exit(status);
    
}

- (IBAction)WindowSizeCheck:(id)sender
{
    if ([S_TextField isEnabled] == YES)
	[S_TextField setEnabled:NO];
    else
	[S_TextField setEnabled:YES];    
}

- (IBAction)ChooseDirButton:(id)sender
{
    panel = [NSOpenPanel openPanel];
    [panel setCanChooseFiles: NO];
    [panel setCanChooseDirectories: YES];
    [panel setDirectory:NSHomeDirectory()];

    if ([panel runModal] == NSOKButton)
    {
	SelectDir = YES;
	[CurrentDir setStringValue:[panel directory]];
    }
    else
    {
	[CurrentDir setStringValue:@"Current Directory"];	
	SelectDir = NO;
    }
}
@end

