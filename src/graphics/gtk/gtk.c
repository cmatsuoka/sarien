/* Sarien - A Sierra AGI resource interpreter engine
 * Copyright (C) 1999-2002 Stuart George and Claudio Matsuoka
 *  
 * $Id$
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; see docs/COPYING for further details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include "sarien.h"
#include "graphics.h"
#include "keyboard.h"

extern struct sarien_options opt;
extern struct gfx_driver *gfx;

static int __argc;
static char **__argv;

static int scale;
static GtkWidget *imagearea;
static GdkImage *image;
GdkVisual *visual;
static int drv_palette[32];


#define KEY_QUEUE_SIZE 16

static int key_queue[KEY_QUEUE_SIZE];
static int key_queue_start = 0;
static int key_queue_end = 0;

#define key_enqueue(k) do { key_queue[key_queue_end++] = (k); \
	key_queue_end %= KEY_QUEUE_SIZE; } while (0)
#define key_dequeue(k) do { (k) = key_queue[key_queue_start++]; \
	key_queue_start %= KEY_QUEUE_SIZE; } while (0)


static int	init_vidmode	(void);
static int	deinit_vidmode	(void);
static void	drv_put_block	(int, int, int, int);
static void	_putpixels	(int, int, int, UINT8 *);
static int	drv_keypress	(void);
static int	drv_get_key	(void);
static void	drv_timer	(void);

static struct gfx_driver gfx_drv = {
	init_vidmode,
	deinit_vidmode,
	drv_put_block,
	_putpixels,
	drv_timer,
	drv_keypress,
	drv_get_key
};

#define ASPECT_RATIO(x) ((x) * 6 / 5)


static GtkItemFactoryEntry menu_items[] = {
	{ "/_File",        NULL,         NULL, 	        0, "<Branch>" },
	{ "/File/Quit",    "<control>Q", gtk_main_quit, 0, NULL },
	{ "/_View",	   NULL,         NULL,          0, "<LastBranch>" },
        { "/View/Palette", "<control>P", NULL,          0, NULL },
	{ "/View/sep1",    NULL,         NULL,          0, "<Separator>" },
        { "/View/Hires",   "<control>H", NULL,          0, NULL },
	{ "/View/sep2",    NULL,         NULL,          0, "<Separator>" },
        { "/View/Size",    "<control>S", NULL,          0, NULL },
        { "/View/Ratio",   "<control>R", NULL,          0, NULL },
	{ "/_Help",        NULL,         NULL,          0, "<LastBranch>" },
	{ "/_Help/About",  NULL,         NULL,          0, NULL }
};



/* ===================================================================== */

/* Slow generic routine. */

static void _putpixels (int x, int y, int w, UINT8 *p)
{
	int cp;
	register int i, j;

	if (w == 0)
		return;

	x *= scale;
	y *= scale;
	while (w--) {
		cp = drv_palette[*p++];
		for (i = 0; i < scale; i++) {
			for (j = 0; j < scale; j++)
				gdk_image_put_pixel (image, x + j, y + i, cp);
		}
		x += scale;
	}
}

static void _putpixels_fixratio (int x, int y, int w, UINT8 *p)
{
	if (y > 0 && ASPECT_RATIO (y) - 1 != ASPECT_RATIO (y - 1))
		_putpixels (x, ASPECT_RATIO(y) - 1, w, p);
	_putpixels (x, ASPECT_RATIO(y), w, p); 
}

/* ===================================================================== */
/* Gtk functions */

static void process_events ()
{
	gtk_main_iteration_do (FALSE);
}

static guint pixel_from_rgb (GdkVisual *visual, guint r, guint g, guint b)
{
	return  ((r >> (16 - visual->red_prec))   << visual->red_shift) |
		((g >> (16 - visual->green_prec)) << visual->green_shift) |
		((b >> (16 - visual->blue_prec))  << visual->blue_shift);
}

static void set_palette (UINT8 *pal, int scol, int numcols)
{
	int i;

	for (i = scol; i < scol + numcols; i++) {
		drv_palette[i] = pixel_from_rgb (visual, (int)pal[0] << 10,
			(int)pal[1] << 10, (int)pal[2] << 10);
		pal += 3;
	}
}


static void get_main_menu (GtkWidget *window, GtkWidget **menubar)
{
	GtkItemFactory *item_factory;
	GtkAccelGroup *accel_group;
	gint nmenu_items = sizeof (menu_items) / sizeof (menu_items[0]);

	accel_group = gtk_accel_group_new ();

	/* accelerator group */
	item_factory = gtk_item_factory_new (GTK_TYPE_MENU_BAR,
		"<main>", accel_group);

	/* generate the menu items */
	gtk_item_factory_create_items (item_factory, nmenu_items,
		menu_items, NULL);

	/* Attach the new accelerator group to the window. */
	gtk_window_add_accel_group (GTK_WINDOW (window), accel_group);

	/* Return the actual menu bar created by the item factory. */ 
	*menubar = gtk_item_factory_get_widget (item_factory, "<main>");
}

static gint delete_event (GtkWidget *widget, GdkEvent *event, gpointer data)
{
	/* If you return FALSE in the "delete_event" signal handler,
	 * GTK will emit the "destroy" signal. Returning TRUE means
	 * you don't want the window to be destroyed.
	 * This is useful for popping up 'are you sure you want to quit?'
	 * type dialogs.
	 */

	g_print ("delete event occurred\n");

	/* Change TRUE to FALSE and the main window will be destroyed with
	 * a "delete_event".
	 */

	return TRUE;
}

static void destroy (GtkWidget *widget, gpointer data)
{
	gtk_main_quit ();
}

static int init_vidmode ()
{
	GtkWidget *window;
	GtkWidget *main_vbox;
	GtkWidget *menubar;
 
	gtk_init (&__argc, &__argv);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	g_signal_connect (G_OBJECT (window), "delete_event",
		G_CALLBACK (delete_event), NULL);
	g_signal_connect (G_OBJECT (window), "destroy",
		G_CALLBACK (destroy), NULL);

	gtk_window_set_title (GTK_WINDOW (window), "GtkSarien " VERSION);

	main_vbox = gtk_vbox_new (FALSE, 1);
	gtk_container_set_border_width (GTK_CONTAINER (main_vbox), 1);
	gtk_container_add (GTK_CONTAINER (window), main_vbox);

	get_main_menu (window, &menubar);
	gtk_box_pack_start (GTK_BOX (main_vbox), menubar, FALSE, TRUE, 0);

	visual = gdk_visual_get_system ();
	image = gdk_image_new (GDK_IMAGE_FASTEST, visual, GFX_WIDTH * scale,
		opt.fixratio ? ASPECT_RATIO (GFX_HEIGHT * scale) :
		GFX_HEIGHT * scale);

	imagearea = gtk_image_new_from_image (image, NULL);
	gtk_box_pack_start (GTK_BOX (main_vbox), imagearea, FALSE, TRUE, 0);
	/* gtk_widget_set_double_buffered (imagearea, FALSE); */
  
	gtk_widget_show_all (window);

	set_palette (palette, 0, 32);

	gfx->put_pixels = opt.fixratio ? _putpixels_fixratio : _putpixels;

	return err_OK;
}


static int deinit_vidmode ()
{
	gtk_main_quit ();
	return err_OK;
}


/* put a block onto the screen */
static void drv_put_block (int x1, int y1, int x2, int y2)
{
	if (x1 >= GFX_WIDTH)  x1 = GFX_WIDTH  - 1;
	if (y1 >= GFX_HEIGHT) y1 = GFX_HEIGHT - 1;
	if (x2 >= GFX_WIDTH)  x2 = GFX_WIDTH  - 1;
	if (y2 >= GFX_HEIGHT) y2 = GFX_HEIGHT - 1;

	if (scale > 1) {
		x1 *= scale;
		y1 *= scale;
		x2 = (x2 + 1) * scale - 1;
		y2 = (y2 + 1) * scale - 1;
	}

	if (opt.fixratio) {
		y1 = ASPECT_RATIO (y1);
		y2 = ASPECT_RATIO (y2);
	}

	/* The +28 offset doesn't make any sense */
	gtk_widget_queue_draw_area (imagearea, x1 + 1, y1 + 28, x2 - x1 + 1, y2 - y1 + 1);
}


static int drv_keypress ()
{
	process_events ();
	return key_queue_start != key_queue_end;
}


static int drv_get_key ()
{
	UINT16 k;

	while (key_queue_start == key_queue_end)	/* block */
		drv_timer ();

	key_dequeue(k);

	return k;
}


static void drv_timer ()
{
	struct timeval tv;
	struct timezone tz;
	static double msec = 0.0;
	double m;
	
	gettimeofday (&tv, &tz);
	m = 1000.0 * tv.tv_sec + tv.tv_usec / 1000.0;

	while (m - msec < 42) {
		usleep (5000);
		gettimeofday (&tv, &tz);
		m = 1000.0 * tv.tv_sec + tv.tv_usec / 1000.0;
	}
	msec = m; 

	process_events ();
}


/*
 * Public functions
 */

int init_machine (int argc, char **argv)
{
	gfx = &gfx_drv;

	__argc = argc;
	__argv = argv;
	scale = opt.scale;
	if (scale < 1)
		scale = 1;
	if (scale > 2)
		scale = 2;

	return err_OK;
}

int deinit_machine (void)
{
	return err_OK;
}

/* end: gtk.c */
