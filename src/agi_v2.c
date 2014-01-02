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
#include "sarien.h"
#include "agi.h"

static int agi_v2_init (void);
static int agi_v2_deinit (void);
static int agi_v2_detect_game (char *);
static int agi_v2_load_resource (int, int);
static int agi_v2_unload_resource (int, int);
static int agi_v2_load_objects(char *);
static int agi_v2_load_words(char *);

struct agi_loader agi_v2 = {
	2,
	0,
	agi_v2_init,
	agi_v2_deinit,
	agi_v2_detect_game,
	agi_v2_load_resource,
	agi_v2_unload_resource,
	agi_v2_load_objects,
	agi_v2_load_words
};


static int agi_v2_detect_game (char *gn)
{
	strncpy (game.dir, gn, MAX_PATH);
	_D (_D_WARN "game.dir = %s", game.dir);

	if (	!file_isthere (fixpath (NO_GAMEDIR, LOGDIR)) ||
		!file_isthere (fixpath (NO_GAMEDIR, PICDIR)) ||
		!file_isthere (fixpath (NO_GAMEDIR, SNDDIR)) ||
		!file_isthere (fixpath (NO_GAMEDIR, VIEWDIR)))
	{
		return err_InvalidAGIFile;
	}

	agi_v2.int_version = 0x2917;		/* setup for 2.917 */

	return v2id_game ();
}


static int agi_v2_load_dir (struct agi_dir *agid, char *fname)
{
	FILE *fp;
	UINT8 *mem;
	UINT32 flen;
	unsigned int i;
	char *path;

	path = fixpath (NO_GAMEDIR, fname);
	report ("Loading directory: %s\n", path);

	if ((fp = fopen (path, "rb")) == NULL) {
		return err_BadFileOpen;
	}

	fseek (fp, 0, SEEK_END);
	flen = ftell (fp);
	fseek (fp, 0, SEEK_SET);

	if ((mem = malloc (flen + 32)) == NULL) {
		fclose (fp);
		return err_NotEnoughMemory;
	}

	fread (mem, 1, flen, fp);

	/* set all directory resources to gone */
	for (i = 0; i < MAX_DIRS; i++) {
		agid[i].volume = 0xff;
		agid[i].offset = _EMPTY;
	}

	/* build directory entries */
	for (i = 0; i < flen; i += 3) {
		agid[i / 3].volume = hilo_getbyte (mem + i) >> 4;
		agid[i / 3].offset = hilo_getpword (mem + i) & (UINT32)_EMPTY;
		/* _D ("%d: volume %d, offset 0x%05x", i / 3,
			agid[i / 3].volume, agid[i / 3].offset); */
	}

	free (mem);
	fclose (fp);

	return err_OK;
}


static int agi_v2_init ()
{
	int ec = err_OK;

	/* load directory files */
	ec = agi_v2_load_dir (game.dir_logic, LOGDIR);
	if (ec == err_OK)
		ec = agi_v2_load_dir (game.dir_pic, PICDIR);
	if (ec == err_OK)
		ec = agi_v2_load_dir (game.dir_view, VIEWDIR);
	if (ec == err_OK)
		ec = agi_v2_load_dir (game.dir_sound, SNDDIR);

	return ec;
}


static int agi_v2_deinit ()
{
	int ec = err_OK;

#if 0
	/* unload words */
	agi_v2_unload_words ();

	/* unload objects */
	agi_v2_unload_objects ();
#endif

	return ec;
}

static int agi_v2_unload_resource (int t, int n)
{
	/* _D ("unload resource"); */

	switch (t) {
	case rLOGIC:
		unload_logic (n);
		break;
	case rPICTURE:
		unload_picture (n);
		break;
	case rVIEW:
		unload_view (n);
		break;
	case rSOUND:
		unload_sound (n);
		break;
	}

	return err_OK;
}


/*
 * This function does noting but load a raw resource into memory,
 * if further decoding is required, it must be done by another
 * routine. NULL is returned if unsucsessfull.
 */

static UINT8* agi_v2_load_vol_res (struct agi_dir *agid)
{
	UINT8 *data = NULL;
	char x[MAX_PATH], *path;
	FILE *fp;
	unsigned int sig;

	sprintf (x, "vol.%i", agid->volume);
	path = fixpath (NO_GAMEDIR, x);
	_D ("path = %s", path);

	if (agid->offset != _EMPTY && (fp = fopen (path, "rb")) != NULL) {
		_D ("loading resource at offset %d", agid->offset);
		if (fseek (fp, agid->offset, SEEK_SET) < 0) {
			agid->offset = _EMPTY;
			return NULL;
		}
		fread (&x, 1, 5, fp);
		if ((sig = hilo_getword ((UINT8*)x)) == 0x1234) {
			agid->len = lohi_getword ((UINT8*)x + 3);
			data = calloc (1, agid->len + 32);
			if (data != NULL) {
				fread (data, 1, agid->len, fp);
			} else {
				abort ();
			}
		} else {
#if 0
			/* FIXME: call some panic handler instead of
			 *        deiniting directly
			 */
			deinit_video_mode ();
#endif
			report ("Error: bad signature %04x\n", sig);
			/* fprintf (stderr, "ACK! BAD RESOURCE!!!\n"); */
			return 0;
		}
		fclose (fp);
	} else {
		/* we have a bad volume resource */
		/* set that resource to NA */
		agid->offset = _EMPTY;
	}

	return data;
}


/*
 * Loads a resource into memory, a raw resource is loaded in
 * with above routine, then further decoded here.
 */
int agi_v2_load_resource (int t, int n)
{
	int ec = err_OK;
	UINT8 *data = NULL;

	_D (_D_WARN "(t = %d, n = %d)", t, n);
	if (n > MAX_DIRS)
		return err_BadResource;

	switch (t) {
	case rLOGIC:
		if (~game.dir_logic[n].flags & RES_LOADED) {
			_D (_D_WARN "loading logic resource %d", n);
			agi_v2.unload_resource (rLOGIC, n);

			/* load raw resource into data */
			data = agi_v2_load_vol_res (&game.dir_logic[n]);

			game.logics[n].data = data;
			ec = data ? decode_logic (n) : err_BadResource;

			game.logics[n].sIP = 2;
       		}

    		/* if logic was cached, we get here */
    		/* reset code pointers incase it was cached */

   		game.logics[n].cIP = game.logics[n].sIP;
		break;
	case rPICTURE:
		/* if picture is currently NOT loaded *OR* cacheing is off,
		 * unload the resource (caching == off) and reload it
		 */

		_D (_D_WARN "loading picture resource %d", n);
		if (game.dir_pic[n].flags & RES_LOADED)
			break;

		/* if loaded but not cached, unload it */
		/* if cached but not loaded, etc */
		agi_v2.unload_resource (rPICTURE, n);
		data = agi_v2_load_vol_res (&game.dir_pic[n]);

		if (data != NULL) {
			game.pictures[n].rdata = data;
			game.dir_pic[n].flags |= RES_LOADED;
		} else {
			ec = err_BadResource;
		}
		break;
	case rSOUND:
		_D (_D_WARN "loading sound resource %d", n);
		if (game.dir_sound[n].flags & RES_LOADED)
			break;

		data = agi_v2_load_vol_res (&game.dir_sound[n]);

		if (data != NULL) {
			game.sounds[n].rdata = data;
			game.dir_sound[n].flags |= RES_LOADED;
			decode_sound (n);
		} else {
			ec = err_BadResource;
		}
		break;
	case rVIEW:
		/* Load a VIEW resource into memory...
		 * Since VIEWS alter the view table ALL the time
		 * can we cache the view? or must we reload it all
		 * the time?
		 */
		if (game.dir_view[n].flags & RES_LOADED)
			break;

		_D (_D_WARN "loading view resource %d", n);
    		agi_v2.unload_resource (rVIEW, n);
    		data = agi_v2_load_vol_res (&game.dir_view[n]);
    		if (data) {
    			game.views[n].rdata = data;
    			game.dir_view[n].flags |= RES_LOADED;
    			ec = decode_view (n);
    		} else {
    			ec=err_BadResource;
		}
		break;
	default:
		ec = err_BadResource;
		break;
	}

	return ec;
}

static int agi_v2_load_objects(char *fname)
{
	return load_objects(fname);
}

static int agi_v2_load_words(char *fname)
{
	return load_words(fname);
}
