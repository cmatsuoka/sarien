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
#include "picture.h"
#include "view.h"
#include "logic.h"
#include "sound.h"
#include "console.h"

static int agi_v2_init (void);
static int agi_v2_deinit (void);
static int agi_v2_detect_game (char *);
static int agi_v2_load_resource (int, int);
static int agi_v2_unload_resource (int, int);
static int agi_v2_load_objects(char *);
static int agi_v2_load_words(char *);

extern struct agi_picture pictures[];
extern struct agi_logic logics[];
extern struct agi_view views[];
extern struct agi_sound sounds[];

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

	if (	!file_exists (fixpath (NO_GAMEDIR, LOGDIR)) ||
		!file_exists (fixpath (NO_GAMEDIR, PICDIR)) ||
		!file_exists (fixpath (NO_GAMEDIR, SNDDIR)) ||
		!file_exists (fixpath (NO_GAMEDIR, VIEWDIR)))
	{
		return err_InvalidAGIFile;
	}

	agi_v2.int_version = 0x2917;		/* setup for 2.917 */
	return v2id_game ();
}


static int agi_v2_load_dir (struct agi_dir *agid, char *fname)
{
#ifndef PALMOS
	FILE *fp;
	UINT8 *mem;
	UINT32 flen;
	int i;
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
	for (i = 0; i < flen; i+=3) {
		agid[i/3].volume = hilo_getbyte (mem+i) >> 4;
		agid[i/3].offset = hilo_getpword (mem+i) & _EMPTY;
	}

	free (mem);
#endif

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


#if 0
static int agi_v2_load_objects (UINT8 *fname)
{
	return load_objects (fname);
}


static int agi_v2_load_words (UINT8* fname)
{
	return load_words (fname);
}


static int agi_v2_unload_objects ()
{
	unload_objects ();
	return err_OK;
}


static int agi_v2_unload_words ()
{
	unload_words ();
	return err_OK;
}
#endif

static int agi_v2_unload_resource (int restype, int resnum)
{
	switch (restype) {
	case rLOGIC:
		unload_logic (resnum);
		break;
	case rPICTURE:
		unload_picture (resnum);
		break;
	case rVIEW:
		unload_view (resnum);
		break;
	case rSOUND:
		unload_sound (resnum);
		break;
	}

	return err_OK;
}


/*
 * This function does noting but load a raw resource into memory,
 * if further decoding is required, it must be done by another
 * routine. NULL is returned if unsucsessfull.
 */

UINT8* agi_v2_load_vol_res (struct agi_dir *agid)
{
#ifndef PALMOS
	UINT8 *data = NULL;
	char x[MAX_PATH], *path;
	FILE *fp;

	_D ("(agi_dir = [offset:%ld, len:%ld])", agid->offset, agid->len);
	sprintf (x, "vol.%i", agid->volume);
	path = fixpath (NO_GAMEDIR, x);
	_D ("path = %s", path);

	if (agid->offset != _EMPTY && (fp = fopen (path, "rb")) != NULL) {
		_D ("loading resource");
		fseek (fp, agid->offset, SEEK_SET);
		fread (&x, 1, 5, fp);
		if (hilo_getword (x) == 0x1234) {
			agid->len = lohi_getword (x + 3);
			data = calloc (1, agid->len + 32);
			if (data != NULL)
				fread (data, 1, agid->len, fp);
		} else {
			/* FIXME: call some panic handler instead of
			 *        deiniting directly
			 */
			deinit_video_mode ();
			fprintf (stderr, "ACK! BAD RESOURCE!!!\n");
			exit (0);
		}
		fclose (fp);
	} else {
		/* we have a bad volume resource */
		/* set that resource to NA */
		agid->offset = _EMPTY;
	}

	return data;
#endif
}


/*
 * Loads a resource into memory, a raw resource is loaded in
 * with above routine, then further decoded here.
 */
int agi_v2_load_resource (int restype, int resnum)
{
	int ec = err_OK;
	UINT8 *data = NULL;

	_D (_D_WARN "(restype = %d, resnum = %d)", restype, resnum);
	if (resnum > MAX_DIRS)
		return err_BadResource;

	switch (restype) {
	case rLOGIC:
		if (~game.dir_logic[resnum].flags & RES_LOADED) {
			_D (_D_WARN "loading logic resource %d", resnum);
			agi_v2.unload_resource (rLOGIC, resnum);
			/* load raw resource into data */
			data = agi_v2_load_vol_res (&game.dir_logic[resnum]);

			ec = (logics[resnum].data = data) ?
				decode_logic (resnum) : err_BadResource;

			logics[resnum].sIP = 2;
       		}

    		/* if logic was cached, we get here */
    		/* reset code pointers incase it was cached */

		/*logics[resnum].sIP=2;*/	/* saved IP = 2 */
   		/*logics[resnum].cIP=2;*/	/* current IP = 2 */

   		logics[resnum].cIP = logics[resnum].sIP;
		break;
	case rPICTURE:
		/* if picture is currently NOT loaded *OR* cacheing is off,
		 * unload the resource (caching == off) and reload it
		 */

		_D (_D_WARN "loading picture resource %d", resnum);
		if (game.dir_pic[resnum].flags & RES_LOADED)
			break;

		/* if loaded but not cached, unload it */
		/* if cached but not loaded, etc */
		agi_v2.unload_resource (rPICTURE, resnum);
		data = agi_v2_load_vol_res (&game.dir_pic[resnum]);

		if (data != NULL) {
			pictures[resnum].rdata = data;
			game.dir_pic[resnum].flags |= RES_LOADED;
		} else {
			ec = err_BadResource;
		}
		break;
	case rSOUND:
		_D (_D_WARN "loading sound resource %d", resnum);
		if (game.dir_sound[resnum].flags & RES_LOADED)
			break;

		data = agi_v2_load_vol_res (&game.dir_sound[resnum]);

		if (data != NULL) {
			sounds[resnum].rdata = data;
			game.dir_sound[resnum].flags |= RES_LOADED;
			decode_sound (resnum);
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
		if (game.dir_view[resnum].flags & RES_LOADED)
			break;

		_D (_D_WARN "loading view resource %d", resnum);
    		agi_v2.unload_resource (rVIEW, resnum);
    		if ((data = agi_v2_load_vol_res (&game.dir_view[resnum]))) {
    			views[resnum].rdata = data;
    			game.dir_view[resnum].flags |= RES_LOADED;
    			ec = decode_view (resnum);
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
