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
