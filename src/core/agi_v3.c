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
#include "lzw.h"
#include "picture.h"
#include "view.h"
#include "logic.h"
#include "sound.h"
#include "gfx_agi.h"
#include "console.h"

static int agi_v3_init (void);
static int agi_v3_deinit (void);
static int agi_v3_detect_game (char *);
static int agi_v3_load_resource (int, int);
static int agi_v3_unload_resource (int, int);

extern struct agi_picture pictures[];
extern struct agi_logic logics[];
extern struct agi_view views[];
extern struct agi_sound sounds[];

struct agi_loader agi_v3 = {
	3,
	0,
	agi_v3_init,
	agi_v3_deinit,
	agi_v3_detect_game,
	agi_v3_load_resource,
	agi_v3_unload_resource
};


int agi_v3_detect_game (char *gn)
{
	int ec = err_Unk;
	char x[MAX_PATH], *xname, *path;
	int l;

	_D ("(\"%s\")", gn);
	strncpy (game.dir, gn, MAX_PATH);

	strcpy (x, "*vol.0");
	path = fixpath (GAMEDIR, x);

	_D (_D_WARN "path = %s", path);
	if (file_exists(path)) {
		_D(_D_WARN "getting xname for path = %s", path);
		xname = file_name (path);
		/* remove the DIR from xname */
		l = strlen (xname);
		if (l >= 5)
			l -= 5;
		xname[l] = 0;
		strncpy (game.name, xname, 8);
		_D (_D_WARN "game.name = %s", game.name);

		agi_v3.int_version = 0x3149;	/* setup for 3.002.149 */
		ec = v3id_game();
	} else {
		ec = err_InvalidAGIFile;
	}

	return ec;
}


static int agi_v3_load_dir (struct agi_dir *agid, FILE *fp, UINT32 offs, UINT32 len)
{
	int ec = err_OK;
	UINT8 *mem;
	int i;

	fseek (fp, offs, SEEK_SET);
	if ((mem = malloc (len + 32)) != NULL) {
		fread(mem, 1, len, fp);

		/* set all directory resources to gone */
		for(i = 0; i < MAX_DIRS; i++) {
			agid[i].volume = 0xff;
			agid[i].offset = _EMPTY;
		}

		/* build directory entries */
		for(i = 0; i < len; i += 3) {
			agid[i / 3].volume = hilo_getbyte (mem + i) >> 4;
			agid[i / 3].offset = hilo_getpword (mem+i) & _EMPTY;
		}

		free(mem);
	} else {
		ec = err_NotEnoughMemory;
	}

	return ec;
}


struct agi3vol {
	UINT32 sddr;
	UINT32 len;
};

int agi_v3_init (void)
{
	int ec = err_OK;
#ifndef PALMOS
	struct agi3vol agi_vol3[4];
	int i;
	UINT16 xd[4];
	FILE *fp;
	char *path;

	path = fixpath (GAMEDIR, DIR_);

	if ((fp = fopen(path, "rb")) != NULL) {
		/* build offset table for v3 directory format */
		/* FIXME: not endian aware! */
		fread (&xd, 1, 8, fp);
		fseek (fp, 0, SEEK_END);

		for(i = 0; i < 4; i++)
			agi_vol3[i].sddr = xd[i];

		agi_vol3[0].len = agi_vol3[1].sddr - agi_vol3[0].sddr;
		agi_vol3[1].len = agi_vol3[2].sddr - agi_vol3[1].sddr;
		agi_vol3[2].len = agi_vol3[3].sddr - agi_vol3[2].sddr;
		agi_vol3[3].len = ftell(fp) - agi_vol3[3].sddr;

		if (agi_vol3[3].len > 256 * 3)
			agi_vol3[3].len = 256 * 3;

		fseek(fp, 0, SEEK_SET);

		/* read in directory files */
  		ec = agi_v3_load_dir (game.dir_logic, fp, agi_vol3[0].sddr,
			agi_vol3[0].len);

  		if(ec == err_OK) {
	  		ec = agi_v3_load_dir (game.dir_pic, fp, agi_vol3[1].sddr,
				agi_vol3[1].len);
		}

  		if(ec == err_OK) {
	  		ec = agi_v3_load_dir (game.dir_view, fp, agi_vol3[2].sddr,
				agi_vol3[2].len);
		}

  		if(ec == err_OK) {
	  		ec = agi_v3_load_dir (game.dir_sound, fp, agi_vol3[3].sddr,
				agi_vol3[3].len);
		}
	} else {
		printf ("Failed to open \"%s\"\n", path);
		ec = err_BadFileOpen;
	}
#endif /* !PALMOS */

	return ec;
}


int agi_v3_deinit ()
{
	int ec=err_OK;

#if 0
	/* unload words */
	agi_v3_unload_words();

	/* unload objects */
	agi_v3_unload_objects();
#endif

	return ec;
}


int agi_v3_unload_resource (int restype, int resnum)
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
		unload_sound(resnum);
		break;
	}

	return err_OK;
}


/*
 * This function does noting but load a raw resource into memory,
 * if further decoding is required, it must be done by another
 * routine.
 *
 * NULL is returned if unsucsessful.
 */

UINT8* agi_v3_load_vol_res (struct agi_dir *agid)
{
#ifndef PALMOS
	UINT8 x[MAX_PATH], *data = NULL, *comp_buffer, *path;
	FILE *fp;

	_D ("(%p)", agid);
	sprintf (x, "vol.%i", agid->volume);
	path = fixpath (GAMEDIR, x);

	if (agid->offset != _EMPTY && (fp = fopen((char*)path, "rb")) != NULL) {
		fseek (fp, agid->offset, SEEK_SET);
		fread (&x, 1, 7, fp);

		if (hilo_getword(x) != 0x1234) {
			/* FIXME */
			deinit_video_mode();
			printf("ACK! BAD RESOURCE!!!\n");
			exit(0);
		}

		agid->len = lohi_getword (x + 3);	/* uncompressed size */
		agid->clen = lohi_getword (x + 5);	/* compressed len */

		comp_buffer = calloc (1, agid->clen + 32);
		fread (comp_buffer, 1, agid->clen, fp);

		if (x[2] & 0x80 || agid->len == agid->clen) {
			/* do not decompress */
			data = comp_buffer;

#if 0
			/* CM: added to avoid problems in
			 *     convert_v2_v3_pic() when clen > len
			 *     e.g. Sierra demo 4, first picture
			 *     (Tue Mar 16 13:13:43 EST 1999)
			 */
			agid->len = agid->clen;

			/* Now removed to fix Gold Rush! in demo4 */
#endif
		} else {
			/* it is compressed */
			data = calloc (1, agid->len + 32);
			LZW_expand (comp_buffer, data, agid->len);
			free (comp_buffer);
			agid->flags |= RES_COMPRESSED;
		}

		fclose(fp);
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

int agi_v3_load_resource (int restype, int resnum)
{
	int ec = err_OK;
	UINT8 *data = NULL;

	if (resnum > MAX_DIRS)
		return err_BadResource;

	switch (restype) {
	case rLOGIC:
		/* load resource into memory, decrypt messages at the end
		 * and build the message list (if logic is in memory)
		 */
		if (~game.dir_logic[resnum].flags & RES_LOADED) {
			/* if logic is already in memory, unload it */
			agi_v3.unload_resource (rLOGIC, resnum);

			/* load raw resource into data */
			data = agi_v3_load_vol_res (&game.dir_logic[resnum]);
			logics[resnum].data=data;

			/* uncompressed logic files need to be decrypted */
			if (data != NULL) {
				/* resloaded flag gets set by decode logic */
				/* needed to build string table */
				ec = decode_logic(resnum);
				logics[resnum].sIP=2;
			} else {
				ec=err_BadResource;
			}

			/*logics[resnum].sIP=2;*/	/* saved IP = 2 */
			/*logics[resnum].cIP=2;*/	/* current IP = 2 */

			logics[resnum].cIP = logics[resnum].sIP;
       		}

		/* if logic was cached, we get here */
		/* reset code pointers incase it was cached */

		/* we start at 2 to skip the size of the logic */
		/*logics[resnum].sIP=2;*/	/* saved IP = 2 */
		/*logics[resnum].cIP=2;*/	/* current IP = 2 */
		logics[resnum].cIP=logics[resnum].sIP;
		break;
	case rPICTURE:
		/* if picture is currently NOT loaded *OR* cacheing is off,
		 * unload the resource (caching==off) and reload it
		 */
		if (~game.dir_pic[resnum].flags & RES_LOADED) {
			agi_v3.unload_resource (rPICTURE, resnum);
			data = agi_v3_load_vol_res (&game.dir_pic[resnum]);
			if (data != NULL) {
				data = convert_v2_v3_pic (data,
					game.dir_pic[resnum].len);
				pictures[resnum].rdata = data;
				game.dir_pic[resnum].flags |= RES_LOADED;
			} else {
				ec=err_BadResource;
			}
		}
		break;
	case rSOUND:
		if (game.dir_sound[resnum].flags & RES_LOADED)
			break;

		if ((data = agi_v3_load_vol_res (&game.dir_sound[resnum])) != NULL) {
			sounds[resnum].rdata = data;
			game.dir_sound[resnum].flags |= RES_LOADED;
			decode_sound (resnum);
		} else {
			ec = err_BadResource;
		}
		break;
	case rVIEW:
		/* Load a VIEW resource into memory...
		 * Since VIEWS alter the view table ALL the time can we
		 * cache the view? or must we reload it all the time?
		 */
		/* load a raw view from a VOL file into data */
		if (game.dir_view[resnum].flags & RES_LOADED)
			break;

		agi_v3.unload_resource (rVIEW, resnum);
		if ((data = agi_v3_load_vol_res (&game.dir_view[resnum])) != NULL) {
			views[resnum].rdata = data;
			game.dir_view[resnum].flags |= RES_LOADED;
			ec = decode_view(resnum);
		} else {
			ec = err_BadResource;
		}
		break;
	default:
		ec = err_BadResource;
		break;
	}

	return ec;
}

