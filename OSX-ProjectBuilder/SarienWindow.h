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
