/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include "sarien.h"
#include "graphics.h"


extern struct gfx_driver *gfx;
extern struct sarien_options opt;


static int	debug_init_vidmode	(void);
static int	debug_deinit_vidmode	(void);
static void	debug_put_block		(int, int, int, int);
static void	debug_put_pixels	(int, int, int, UINT8 *);
static void	debug_timer		(void);
static int	debug_get_key		(void);
static int	debug_keypress		(void);


static struct gfx_driver gfx_debug = {
	debug_init_vidmode,
	debug_deinit_vidmode,
	debug_put_block,
	debug_put_pixels,
	debug_timer,
	debug_keypress,
	debug_get_key
};


#define KEY_QUEUE_SIZE 16

static int key_queue[KEY_QUEUE_SIZE];
static int key_queue_start = 0;
static int key_queue_end = 0;

#define key_enqueue(k) do { key_queue[key_queue_end++] = (k); \
	key_queue_end %= KEY_QUEUE_SIZE; } while (0)
#define key_dequeue(k) do { (k) = key_queue[key_queue_start++]; \
	key_queue_start %= KEY_QUEUE_SIZE; } while (0)

static int ticks;
static FILE *script;

static void process_events ()
{
	static char line[80];
	static int next = -1;
	char *t, *cmd;

	if (next == -1) {
		fgets (line, 80, script);
		if ((t = strchr (line, '#')) != NULL)
			*t = 0;
		_D (_D_WARN "line: %s", line);
		t = strtok (line, " \t");
		next = strtoul (t, NULL, 10);
		_D (_D_CRIT "next: %d", next);
	}

	if (ticks == next) {
		cmd = strtok (NULL, " \t");
	
		if (cmd == NULL) {
		}
		else if (!strcmp (cmd, "key")) {
			int key;
			t = strtok (NULL, "\n");
			key = strtoul (t, NULL, 0);
			printf ("[%06d] key: %02x\n", next, key);
			key_enqueue (key);
		}
		next = -1;
	} else {
		if ((ticks % 1000) == 0) {
			printf ("[%06d] checkpoint (next: %06d)\n", ticks, next);
		} 
	}
	ticks++;
}

int init_machine (int argc, char **argv)
{
	gfx = &gfx_debug;
	return err_OK;
}


int deinit_machine ()
{
	return err_OK;
}


static void debug_timer ()
{
#if 0
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
#endif

	process_events ();
}


static int debug_init_vidmode ()
{
	char *s;

	printf ("init_vidmode\n");
	ticks = 0;
	if ((s = getenv ("SARIEN_SCRIPT")) == NULL)
		s = "script";

	if ((script = fopen (s, "r")) == NULL) {
		printf ("file 'script' not found and SARIEN_SCRIPT undefined\n");
		return - 1;
	}

	printf ("using script: %s\n", s);

	return 0;
}


static int debug_deinit_vidmode ()
{
	printf ("deinit_vidmode\n");
	printf ("total ticks = %d\n", ticks);
	fclose (script);

	return 0;
}


/* blit a block onto the screen */
static void debug_put_block (int x1, int y1, int x2, int y2)
{
	/* printf ("put_block: %d, %d, %d, %d\n", x1, y1, x2, y2); */
}


static void debug_put_pixels(int x, int y, int w, UINT8 *p)
{
	/* printf ("put_pixels: %d, %d, %d\n", x, y, w); */
}


static int debug_keypress ()
{
	process_events ();
	return key_queue_start != key_queue_end;
}


static int debug_get_key ()
{
	UINT16 k;

	while (key_queue_start == key_queue_end)	/* block */
		debug_timer ();

	key_dequeue(k);

	return k;
}

