
#import "SarienView.h"

extern int    __argc;
extern char** __argv;

extern SarienView* sarienView;

int  cocoa_init      (void);
int  cocoa_deinit    (void);
void cocoa_put_block (int, int, int, int);
void cocoa_put_pixels(int, int, int, UINT8 *);
int  cocoa_keypress  (void);
int  cocoa_get_key   (void);
void cocoa_timer     (void);

extern struct gfx_driver *gfx;

