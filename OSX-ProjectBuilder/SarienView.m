#import "SarienView.h"
#include "sarien.h"
#include "graphics.h"
#include "keyboard.h"
#include "main.h"

@implementation SarienView

- (id)initWithFrame:(NSRect)frameRect
{
    CGDataProviderRef provider;
    CGColorSpaceRef   colors;
    unsigned int*     c;
    unsigned int      i, size;
    
    [super initWithFrame:frameRect];
    mutex = [[NSLock alloc] init];
    
    colors   = CGColorSpaceCreateDeviceRGB();
    size     = GFX_WIDTH * GFX_HEIGHT * 4;
    screen   = (unsigned int*)malloc(size);
    provider = CGDataProviderCreateWithData(NULL, screen, size, NULL);
    
    c = screen;
    for (i = 0; i < (GFX_WIDTH * GFX_HEIGHT); i++)
    {
#if defined(__BIG_ENDIAN__)
        *c++ = 0xff000000; // ARGB -> 100% Opaque pixel entirely black.
#elif defined(__LITTLE_ENDIAN__)
        *c++ = 0x000000ff; // BGRA -> 100% Opaque pixel entirely black.
#else
#error You must compile with either __LITTLE_ENDIAN__ or __BIG_ENDIAN__ flag.
#endif
    }
    
    screenImage = CGImageCreate(
                    GFX_WIDTH,
                    GFX_HEIGHT,
                    8,
                    32,
                    GFX_WIDTH * 4,
                    colors,
                    kCGImageAlphaFirst,
                    provider,
                    NULL,
                    0,
                    kCGRenderingIntentDefault);

    CGDataProviderRelease(provider);
    CGColorSpaceRelease(colors);
    
    sarienView = self;
    return self;
}

- (void)dealloc
{
    CGImageRelease(screenImage);
    free(screen);
    [mutex release];
    [super dealloc];
}

- (void)displayIfNeeded
{
    [self setNeedsDisplay: YES];
    [super displayIfNeeded];
}

- (BOOL)needsDisplay
{
    return YES;
}

- (void)drawRect:(NSRect)aRect
{
    CGContextRef context;
    CGRect       rc;
    NSRect       bnd;

    [mutex lock];
    context = [[NSGraphicsContext graphicsContextWithWindow: [self window]] graphicsPort];
    bnd     = [self frame];
    rc      = CGRectMake(bnd.origin.x, bnd.origin.y, aRect.size.width, aRect.size.height);

    CGContextDrawImage(context, rc, screenImage);
    [mutex unlock];
}

- (unsigned int*)screenContent
{
    return screen;
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (void)keyDown:(NSEvent*)event
{
    NSString* characters;
    int       i, length;
    
    characters = [event characters];
    length     = [characters length];
    
    [mutex lock];
    for (i = 0; i < length; i++)
    {
        NSLog(@"Key Down: %d", [characters characterAtIndex: i]);

        key_queue[key_queue_end++] = [characters characterAtIndex: i];
        key_queue_end %= KEY_QUEUE_SIZE;
    }
    [mutex unlock];
}

- (BOOL)hasPendingKey
{
    return key_queue_start != key_queue_end;
}

- (int)getKey
{
    unichar k;
    
    while (true)
    {
        while (key_queue_start == key_queue_end)
        {
            cocoa_timer();
        }

        [mutex lock];
        k = key_queue[key_queue_start++];
        key_queue_start %= KEY_QUEUE_SIZE;
        [mutex unlock];

        switch (k)
        {
        case NSUpArrowFunctionKey:
            return KEY_UP;
        case NSDownArrowFunctionKey:
            return KEY_DOWN;
        case NSLeftArrowFunctionKey:
            return KEY_LEFT;
        case NSRightArrowFunctionKey:
            return KEY_RIGHT;
        case 127:
        case NSDeleteFunctionKey:
        case NSDeleteCharFunctionKey:
            return KEY_BACKSPACE;
        case NSHomeFunctionKey:
            return KEY_UP_LEFT;
        case NSPageUpFunctionKey:
            return KEY_UP_RIGHT;
        case NSEndFunctionKey:
            return KEY_DOWN_LEFT;
        case NSPageDownFunctionKey:
            return KEY_DOWN_RIGHT;
        case NSExecuteFunctionKey:
            return KEY_ENTER;
	case NSF1FunctionKey:
            return 0x3b00;
        case NSF2FunctionKey:
            return 0x3c00;
        case NSF3FunctionKey:
            return 0x3d00;
        case NSF4FunctionKey:
            return 0x3e00;
        case NSF5FunctionKey:
            return 0x3f00;
        case NSF6FunctionKey:
            return 0x4000;
        case NSF7FunctionKey:
            return 0x4100;
        case NSF8FunctionKey:
            return 0x4200;
        case NSF9FunctionKey:
            return 0x4300;
        case NSF10FunctionKey:
            return 0x4400;
        default:
            return k;
        }
    }
    
    return 0;
}

- (void)lock
{
    [mutex lock];
}

- (void)unlock
{
    [mutex unlock];
}

@end