/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999,2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "sarien.h"
#include "agi.h"
#include "gfx.h"
#include "keyboard.h"
#include "rand.h"
#include "objects.h"
#include "picture.h"
#include "view.h"
#include "logic.h"
#include "menu.h"
#include "console.h"


/* A bunch of global variables. Ick. */

UINT8		*gname=NULL;		/* lead in id (eg, goldrush GR */
UINT8		*gdir=NULL;		/* game dir (for v3 games, eg GR<dir> */
UINT8		*gid=NULL;		/* game id */
UINT8		path[1024];		/* holds expanded paths for files */
UINT8		horizon;		/* horizon marker */
struct agi_loader *loader;		/* loader */
struct agi_dir	dir_logic[MAX_DIRS];	/* directory entries for logics */
struct agi_dir	dir_pic[MAX_DIRS];	/* directory entries for pictures */
struct agi_dir	dir_view[MAX_DIRS];	/* directory entries for views */
struct agi_dir	dir_sound[MAX_DIRS];	/* directory entries for sounds */
UINT8		flags[MAX_FLAGS/CHAR_BIT];	/* 256 bit flags */
UINT8		vars[MAX_VARS];		/* 256 byte variables */
UINT16		ego_in_new_room;	/* new room flag */
UINT8		control_mode;		/* who's in control */
UINT8		quit_prog_now;		/* quit now */
UINT8		status_line;		/* status line on/off */
UINT8		line_status;		/* line num to put status on */
UINT8		line_user_input;	/* line to put user input on */
UINT8		line_min_print;		/* num lines to print on */
UINT8		allow_kyb_input;	/* allow keyboard input */
UINT8		clock_enabled;		/* clock is on/off */
//UINT8		timed_message_box;	/* timed message box */
UINT16		message_box_key;	/* message box keypress */
UINT32		game_flags;		/* game flags!! (important) */


volatile UINT32	msg_box_secs2;		/* message box timeout in sec/2 */

//UINT8	*exec_name;			/* copy of argv[0] */

extern struct sarien_options opt;
extern struct agi_picture pictures[];
extern struct agi_logic logics[];
extern struct agi_view views[];
extern struct agi_object *objects;
extern struct agi_events events[];
extern UINT8 *font, font_english[], font_russian[];

int agi_init ()
{
	int ec, i;

	gid = NULL;			/* clean out GAME ID */

#if 0
	/* clean out game info */
	memset (&game_info, 0, sizeof(struct game_id_list));
#endif

	/* set the font */
	font= opt.agds ? font_russian : font_english;

	/* reset all flags to false and all variables to 0 */
	for (i = 0; i < MAX_FLAGS; i++)
		setflag (i, 0);
	for (i = 0; i < MAX_VARS; i++)
		setvar(i, 0);

	/* clear all logics, pictures, views and events */
	memset (&logics, 0, MAX_DIRS * sizeof (struct agi_logic));
	memset (&pictures, 0, MAX_DIRS * sizeof (struct agi_picture));
	memset (&views, 0x0, MAX_DIRS * sizeof (struct agi_view));
	memset (&events, 0x0, MAX_DIRS * sizeof (struct agi_event));

	init_words ();
	init_view_table ();
	set_rnd_seed ();
	init_menus ();

	/* clear string buffer */
	for (i = 0; i < MAX_WORDS1; i++)
		strings[i][0] = 0;

	/* setup emulation */

	report ("Emulating Sierra AGI v");
	switch (loader->int_version>>12) {
	case 2:
		report ("%x.%03x\n",
			(int)(loader->int_version >> 12) & 0xF,
			(int)(loader->int_version) & 0xFFF);
		break;
	case 3:
		report ("%x.002.%03x\n",
			(int)(loader->int_version >> 12) & 0xF,
			(int)(loader->int_version) & 0xFFF);
		break;
	}

	game_flags |= opt.amiga ? ID_AMIGA : 0;
	game_flags |= opt.agds ? ID_AGDS : 0;

	if (game_flags & ID_AMIGA)
		printf ("Amiga padded game detected\n");

	if (game_flags & ID_AGDS)
		printf ("AGDS mode enabled\n");

	screen_mode=GFX_MODE;
	/*allow_kyb_input=__FALSE;*/
	txt_fg=0xF;
	txt_bg=0x0;

	ec = loader->init ();		/* load vol files, etc */

	if(ec == err_OK)
		ec=load_objects((UINT8*)OBJECTS);

	/* CM: ec= commented out, demogs has no words.tok */
	if(ec == err_OK)
		/*ec =*/ load_words((UINT8*)WORDS);

	/* FIXME: load IIgs instruments and samples */
	/* load_instruments("kq.sys16"); */

	/* Load logic 0 into memory, set cache flag for logic 0 */
	if(ec == err_OK) {
		ec = loader->load_resource(rLOGIC, 0);
		dir_logic[0].flags |= RES_CACHED;	/* keep this one cached */
	}

	/* if cached, enable caching options */
	if (opt.cache) {
		for(i=0; i<MAX_DIRS; i++) {
			dir_logic[i].flags |= RES_CACHED;
			dir_pic[i].flags |= RES_CACHED;
			dir_view[i].flags |= RES_CACHED;
			dir_sound[i].flags |= RES_CACHED;
		}
	}

	/* if forced, load all cacheable objects */
	if (opt.forceload && ec == err_OK) {
		for(i = 0; i < MAX_DIRS; i++) {
			printf ("Force loading cached resource: Logic %4i\r", i);
			fflush (stdout);
			loader->load_resource (rLOGIC, i);
		}
		printf ("\n");

		for(i = 0; i < MAX_DIRS; i++) {
			printf ("Force loading cached resource: Picture %4i\r", i);
			fflush (stdout);
			loader->load_resource (rPICTURE, i);
		}
		printf ("\n");

		for(i = 0; i < MAX_DIRS; i++) {
			printf ("Force loading cached resource: View %4i\r", i);
			fflush(stdout);
			loader->load_resource (rVIEW, i);
		}
		printf("\n");
	}

	return ec;
}


int agi_deinit ()
{
	int ec;

	reset_graphics ();		/* clean out video memory */
	clean_input ();			/* remove all words from memory */

	/* release game ID if one is present */
	if (gid != NULL) {
		free(gid);
		gid=NULL;
	}

#if 0
	/* release game name if present */
	/* ack. v3 needs this no restart.. */
	if(gname!=NULL) {
		free(gname);
		gname=NULL;
	}
#endif

	deinit_menus ();		/* unload the menus */
	unload_resources ();		/* unload resources in memory */
	ec = loader->deinit ();
	unload_objects();
	unload_words();

	ego_in_new_room = 0;
	control_mode = 0;
	quit_prog_now = 0;
	status_line = FALSE;
	line_status = 0;
	line_user_input = 0;
	line_min_print = 0;
	allow_kyb_input = 0;
	clock_enabled = 0;
	//timed_message_box = 0;
	message_box_key = 0;

	return ec;
}


char *agi_printf (char *msg, int lognum)
{
	static char x[256], y[256];
	char z[16], *p;
	int xx, xy;

	/* turn a AGI string into a real string */
	p = x;

	for (*p = xx = xy = 0; *msg; ) {
		switch(*msg) {
		case '%':
			msg++;
			switch(*msg++) {
			case 'v':
				xx = atoi((char*)msg);
				while (isdigit(*msg)!=0) msg++;
				sprintf((char*)z, "%03i", getvar(xx));

				xy=99;
				if(*msg=='|') {
					msg++;
					xy=atoi((char*)msg);
					while(isdigit(*msg)!=0) msg++;
				}
				xx=0;
				if(xy==99) {
					/* remove all leading 0' */
					/* dont remove the 3rd zero if 000 */
					while(z[xx]=='0' && xx<2) xx++;
				}
				else
					xx=3-xy;
				strcat(p, z + xx);
				break;
			case '0':
				strcat(p, objects[atol(msg)-1].name);
				break;
			case 'g':
				strcat(p, logics[0].texts[atol(msg)-1]);
				break;
			case 'w':
				strcat(p, ego_words[atol(msg)-1].word);
				break;
			case 's':
				strcat(p, strings[atol(msg)]);
				break;
			case 'm':
				strcat(p, logics[lognum].texts[atol(msg)-1]);
				break;
			default:
				break;
			}

			while(isdigit(*msg)!=0) msg++;
			while(*p!=0x0) p++;
			break;

		default:
			*p=*msg;
			msg++;
			p++;
			*p=0x0;
			break;
		}
	}

	p = x;
	if (strchr (x, '%') != NULL) {
		strcpy (y, x);
		p = agi_printf (y, lognum);
	}

	return p;
}

