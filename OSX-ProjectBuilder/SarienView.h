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
