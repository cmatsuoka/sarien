/* Sarien - A Sierra AGI resource interpreter engine
 * Copyright (C) 1999 Dark Fiber, (C) 1999,2001 Claudio Matsuoka
 *  
 * $Id$
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; see docs/COPYING for further details.
 */

/* Coded with a little help from Ruda Moura's Xvidmode */


#include <X11/Xlib.h>
#include <X11/extensions/xf86dga.h>
#include <X11/extensions/xf86vmode.h>

static XF86VidModeModeInfo **modeinfo;
static char *framebuffer, *buffer;
static int bytes_per_line;
static int bytes_per_pixel;


static int	dga_init_vidmode	(void);
static int	dga_deinit_vidmode	(void);
static void	dga_put_block		(int, int, int, int);
static void	dga_put_pixel8		(int, int, int);
static void	dga_put_pixel15		(int, int, int);
static void	dga_put_pixel16		(int, int, int);
static void	dga_put_pixel24		(int, int, int);


static int dga_init_vidmode ()
{
	int i, vm_max, m320x200 = 0;
	int width, height, bank_size, ram;
	int major, minor, event_base, error_base, flags; 

	if (!XF86DGAQueryExtension (display, &event_base, &error_base)) {
		fprintf (stderr, "\tdga: DGA extension not found\n");
		return err_Unk;
	}

	if (!XF86DGAQueryVersion (display, &major, &minor)) {
		fprintf (stderr, "\tdga: can't determine DGA version\n");
		return err_Unk;
	}

	fprintf (stderr, "\tdga: XF86 DGA extension version %d.%d\n",
		major, minor);

	if (!XF86DGAQueryDirectVideo (display, screen, &flags)) {
		fprintf (stderr, "\tdga: can't query DGA DirectVideo\n");
		return err_Unk;
	}

	if (~flags & XF86DGADirectPresent) {
		fprintf (stderr, "\tdga: DirectVideo support not present\n");
		return err_Unk;
	}

	if (!XF86VidModeQueryVersion (display, &major, &minor)) {
		fprintf (stderr, "\tdga: can't determine VidMode version\n");
		return err_Unk;
	}

	fprintf (stderr, "\tdga: XF86 VidMode extension version %d.%d\n",
		major, minor);

	if (depth != 8 && depth != 15 && depth != 16 && depth != 24) {
		fprintf (stderr, "\tdga: unsupported depth (%d bpp)\n", depth);
		return err_Unk;
	}
			
	XF86VidModeGetAllModeLines (display, screen, &vm_max, &modeinfo); 

	for (i = 0; i < vm_max; i++) {
		if (modeinfo[i]->hdisplay == 320 &&
			modeinfo[i]->vdisplay == 200) {
			m320x200 = i;
			break;
		}
	}

	if (i == vm_max) {
		fprintf (stderr, "\tdga: can't set resolution to 320x200\n");
		return err_Unk;
	}

	XF86DGAGetViewPortSize (display, screen, &width, &height);

	XF86DGAGetVideo (display, screen, &framebuffer, &bytes_per_line,
		&bank_size, &ram);

	bytes_per_pixel = (depth == 15 ? 16 : depth) >> 3;

	fprintf (stderr, "\tdga: mode is %dx%d (%dx%d bytes per line)\n",
		width, height, bytes_per_line, bytes_per_pixel);

	bytes_per_line *= bytes_per_pixel;

	fprintf (stderr, "\tdga: %dK ram, %dK bank, framebuffer @ %p\n",
		ram, bank_size >> 10, framebuffer);

	if (ram != bank_size >> 10) {
		if (bytes_per_line * 200 > bank_size) {
			fprintf (stderr, "\tdga: framebuffer is not linear\n");
			return err_Unk;
		}
		XF86DGASetVidPage (display, screen, 0);
	}

	if ((buffer = malloc (320 * 200 * bytes_per_pixel)) == NULL) {
		fprintf (stderr, "\tdga: can't alloc buffer\n");
		return err_Unk;
	}

	opt.mitshm = FALSE;

	signal (SIGSEGV, kill_mode);
	signal (SIGQUIT, kill_mode);
	signal (SIGFPE, kill_mode);
	signal (SIGTERM, kill_mode);
	signal (SIGINT, kill_mode);

	/* init_vidmode */
	gfx->deinit_video_mode = dga_deinit_vidmode;
	gfx->put_block = dga_put_block;
	/* poll_timer */

	switch (depth) {
	case 8:
		gfx->put_pixel = dga_put_pixel8;
		break;
	case 15:
		gfx->put_pixel = dga_put_pixel15;
		break;
	case 16:
		gfx->put_pixel = dga_put_pixel16;
		break;
	case 24:
		gfx->put_pixel = dga_put_pixel24;
		break;
	}

	attribute_mask = CWOverrideRedirect;
	attributes.override_redirect = True;
	window = XCreateWindow (display, root, 0, 0, 320, 200, 0,
		depth, InputOutput, CopyFromParent, attribute_mask,
		&attributes);
	XMapWindow (display, window);
	XSelectInput (display, window, KeyPressMask | KeyReleaseMask);
	XSetInputFocus (display, window, RevertToNone, CurrentTime);
	XGrabKeyboard (display, window, True, GrabModeAsync, GrabModeAsync,
		CurrentTime);
	XGrabPointer (display, window, True, 0, GrabModeAsync, GrabModeAsync,
		window, None, CurrentTime);
	XFlush (display);
		
	XF86VidModeSwitchToMode (display, screen, modeinfo[m320x200]);
	XFlush (display);

	XF86DGADirectVideo (display, screen, XF86DGADirectGraphics);

	memset (framebuffer, depth > 8 ? 0 : BlackPixel (display, screen),
		height * width * bytes_per_pixel);

	XF86DGASetViewPort (display, screen, 0, 0);
	XFlush (display);

	return err_OK;
}


static int dga_deinit_vidmode ()
{
	XF86DGADirectVideo (display, window, 0);

	XUngrabKeyboard (display, CurrentTime);
	XUngrabPointer (display, CurrentTime);
	XSelectInput (display, window, 0);
	XFlush (display);

	XDestroyWindow (display, window);
	XF86VidModeSwitchToMode (display, screen, modeinfo[0]);
	XFlush (display);

	XCloseDisplay (display);

	XFree (modeinfo);
	//free (buffer2);
	free (buffer);

	return err_OK;
}


/* put a block onto the screen */
static void dga_put_block (int x1, int y1, int x2, int y2)
{
	int i, h;

	if (x1 >= GFX_WIDTH)
		x1 = GFX_WIDTH - 1;
	if (y1 >= GFX_HEIGHT)
		y1 = GFX_HEIGHT - 1;
	if (x2 >= GFX_WIDTH)
		x2 = GFX_WIDTH - 1;
	if (y2 >= GFX_HEIGHT)
		y2 = GFX_HEIGHT - 1;

	h = y2 - y1 + 1;

	for (i = 0; i < h; i++)
		memcpy (framebuffer + bytes_per_line * (y1 + i) +
			bytes_per_pixel * x1,
			buffer + bytes_per_pixel * (320 * (y1 + i) + x1),
			bytes_per_pixel * (x2 - x1 + 1));
}


static void dga_put_pixel8 (int x, int y, int c)
{
	/*video_buffer[y * 320 + x] = (c & 0xFF);*/
}


static void dga_put_pixel15 (int x, int y, int c)
{
	/*video_buffer[y * 320 + x] = (c & 0xFF);*/
}


static void dga_put_pixel16 (int x, int y, int c)
{
	*(UINT16 *)(buffer + bytes_per_pixel * (320 * y + x)) =
		rgb_palette[c];
}


static void dga_put_pixel24 (int x, int y, int c)
{
	/*video_buffer[y * 320 + x] = (c & 0xFF);*/
}

