#import <Cocoa/Cocoa.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/time.h>

#include "sarien.h"
#include "graphics.h"
#include "main.h"

unsigned int sarienPalette[32];
SarienView*  sarienView;

struct gfx_driver gfx_cocoa =
{
    cocoa_init,
    cocoa_deinit,
    cocoa_put_block,
    cocoa_put_pixels,
    cocoa_timer,
    cocoa_keypress,
    cocoa_get_key
};

int init_machine (int argc, char **argv)
{
    gfx = &gfx_cocoa;
    return err_OK;
}

int deinit_machine (void)
{
    return err_OK;
}

int cocoa_init(void)
{
    int            i, j;
    unsigned char* c = (unsigned char*)sarienPalette;

    for (i = 0, j = 0; i < 32; i++)
    {
        *c++ = 255;
        *c++ = palette[j++] << 2;
        *c++ = palette[j++] << 2;
        *c++ = palette[j++] << 2;
    }

    return err_OK;
}

int cocoa_deinit(void)
{
    return err_OK;
}

void cocoa_put_block(int x, int y, int w, int h)
{
    [sarienView lock];
    [sarienView setNeedsDisplay: YES];
    [sarienView unlock];
}

void cocoa_put_pixels(int x, int y, int w, UINT8 *p)
{
    unsigned int* pixels;

    [sarienView lock];
    pixels = &[sarienView screenContent][x + (y * GFX_WIDTH)];
    
    while (w--)
    {
        *pixels++ = sarienPalette[*p++];
    }
    [sarienView unlock];
}

int cocoa_keypress(void)
{
    return [sarienView hasPendingKey];
}

int cocoa_get_key(void)
{
    return [sarienView getKey];
}

void cocoa_timer(void)
{
    static double msec = 0.0;
    struct timeval tv;
    struct timezone tz;
    double m;
	
    gettimeofday(&tv, &tz);
    m = 1000.0 * tv.tv_sec + tv.tv_usec / 1000.0;

    while (m - msec < 42)
    {
        usleep(5000);
        gettimeofday(&tv, &tz);
        m = 1000.0 * tv.tv_sec + tv.tv_usec / 1000.0;
    }

    msec = m; 
}

int    __argc;
char** __argv;

int main(int argc, const char *argv[])
{
    /* For some obscure reasons, when double clicked in the finder, 
       argc and argv can't be parsed by the GNU's getopt library. */
    __argc = 1;
    __argv = malloc(sizeof(*__argv) * (__argc + 1));
    __argv[0] = strdup("sarien"); 
    __argv[1] = NULL;
    
    if (argv == NULL)
    {
        exit(0);
    }
    
    return NSApplicationMain(argc, argv);
}
