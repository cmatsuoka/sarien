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

#import "SDL.h"
#import "SDLMain.h"
#import <sys/param.h> /* for MAXPATHLEN */
#import <unistd.h>
#import <Cocoa/Cocoa.h>

static int    gArgc;
static char  **gArgv;
static BOOL   gFinderLaunch;


/* The main class of the application, the application's delegate */
@implementation SDLMain
- (void)quit:(id)sender
{

	Uint32 subsystem_mask = SDL_INIT_VIDEO;
	if (SDL_WasInit(subsystem_mask) == subsystem_mask) {
		/* Post a SDL_QUIT event */
		SDL_Event event;

		event.type = SDL_QUIT;
		SDL_PushEvent(&event);
	} else {
		exit(0);
	}
}


/* Set the working directory to the .app's parent directory */
- (void) setupWorkingDirectory
{
	char parentdir[MAXPATHLEN];
	char *c;
    
	strncpy (parentdir, gArgv[0], sizeof(parentdir));
	c = (char*) parentdir;

	while (*c != '\0')		/* go to end */
		c++;
    
	while (*c != '/')		/* back up to parent */
		c--;
    
	*c++ = '\0';			/* cut off last part (binary name) */

	if (chdir (parentdir) != 0)	/* chdir to the binary app's parent */
		exit(0);   

	if (chdir ("../../../") != 0)	/* chdir to the .app's parent */
		exit(0); 
}


/* Called when the internal event loop has just started running */
- (void) applicationDidFinishLaunching: (NSNotification *) note
{
	/* Set the working directory to the .app's parent directory */
	[self setupWorkingDirectory];
    
	if (gFinderLaunch == YES) {
		MyWindowController * pController  = [[MyWindowController alloc]
			initWithWindowNibName:@"CommandLine"];

		/* this forces the nib to be loaded */
		NSWindow * pWindow = [pController window];
	
		[pController showWindow:self];
		[pWindow makeKeyAndOrderFront:self];
	} else {
		/* Hand off to main application code */
		int status = SDL_main (gArgc, gArgv);

		/* We're done, thank you for playing */
		exit(status);
	}
}
@end


#ifdef main
#  undef main
#endif

/* Main entry point to executable - should *not* be SDL_main! */
int main (int argc, char **argv)
{
	/* Copy the arguments into a global variable */
	int i;
    
	/* This is passed if we are launched by double-clicking */
	if ( argc >= 2 && strncmp (argv[1], "-psn", 4) == 0 ) {
		// Finder Launch
		gArgc = 1;
		gFinderLaunch = YES;
	} else {
		gArgc = argc;
		gFinderLaunch = NO;
	}

	gArgv = (char**) malloc (sizeof(*gArgv) * (gArgc+1));
	assert (gArgv != NULL);
	for (i = 0; i < gArgc; i++)
        	gArgv[i] = argv[i];
	gArgv[i] = NULL;

	NSApplicationMain (argc, argv);
	return 0;
}

