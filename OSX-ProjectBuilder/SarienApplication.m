#import "SarienApplication.h"

#include <unistd.h>
#include "main.h"

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
