/*
	svgalib.h
*/

#ifndef __SVGALIB_H__
#define __SVGALIB_H__

/*
 * FUNCTIONS
 */

static UINT8 svgalib_key_init (void);
static void svgalib_key_close (void);
static void svgalib_key_flush (void);
static void svgalib_key_update (void);
static void svgalib_key_handler (int scancode, int press);

static UINT16 init_vidmode (void);
static UINT16 deinit_vidmode (void);
static void put_block (UINT16 x1, UINT16 y1, UINT16 x2, UINT16 y2);
static inline void _put_pixel (UINT16 x, UINT16 y, UINT16 c);

static void new_timer (void);

#endif	/* __SVGALIB_H__ */
