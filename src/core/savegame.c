/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */


#ifndef PALMOS

#include <stdio.h>
#include <string.h>
#ifndef __MPW__
#ifndef __DICE__
#include <sys/stat.h>
#endif
#include <sys/types.h>
#endif
#ifdef WIN32
#include <direct.h>
#endif

#include "sarien.h"
#include "agi.h"
#include "keyboard.h"
#include "graphics.h"
#include "text.h"
#include "opcodes.h"
#include "savegame.h"
#include "iff.h"

static int loading_ok;

#if defined(__DICE__) || defined(WIN32)
#  define MKDIR(a,b) mkdir(a)
#else
#  define MKDIR(a,b) mkdir(a,b)
#endif


/* Words are big-endian */

static void write_num (UINT32 n, FILE *f)
{
	UINT8 size[4];

	size[0] = (n & 0xff000000) >> 24;
	size[1] = (n & 0x00ff0000) >> 16;
	size[2] = (n & 0x0000ff00) >> 8;
	size[3] = (n & 0x000000ff) >> 0;

	fwrite (size, 1, 4, f);
}


static void iff_newchunk (char *t, UINT32 s, FILE *f)
{
	fwrite (t, 1, 4, f);
	write_num (s, f);
}


static void iff_newgroup (char *t, UINT32 s, char *n, FILE *f)
{
	iff_newchunk (t, s, f);
	fwrite (n, 1, 4, f);
}


static void iff_chunk_pad (UINT32 i, FILE *f)
{
	UINT8 b = 0x00;

	while (i--)
		fwrite (&b, 1, 1, f);
}


#define WORD_ALIGN(x) (((((x) -1) >> 1) + 1) << 1)

int save_game (char *s, char *d)
{
	FILE *f;
	UINT32 i, s_agid, s_gcrc, s_form, s_desc, s_objs;
	UINT32 s_flag, s_vars, s_stri, s_view, s_spic, s_apic;
	UINT32 crc = 0x12345678;	/* FIXME */

	_D ("(\"%s\", \"%s\")", s, d);

	f = fopen (s, "w");

	/* IFF chunk sizes */
	s_agid = strlen (game.id) + 1;
	s_gcrc = 4;
	s_desc = WORD_ALIGN (strlen (d) + 1);
	s_vars = WORD_ALIGN (MAX_VARS) + 4;
	s_flag = WORD_ALIGN (MAX_FLAGS) + 4;
	s_objs = game.num_objects + 4;	/* 1 byte / object */
	s_stri = 4;
	for (i = 0; i < MAX_WORDS1; s_stri += strlen (game.strings[i++]) + 1) {}
	s_view = 26 * 4;
	s_apic = WORD_ALIGN (_WIDTH * _HEIGHT);
	s_spic = WORD_ALIGN (GFX_WIDTH * GFX_HEIGHT);
	s_form = s_agid + s_gcrc + s_desc + s_vars + s_flag + s_stri +
		MAX_VIEWTABLE * (8 + s_view) + s_objs + s_apic + s_spic;

	iff_newgroup ("FORM", s_form, "FAGI", f);

	/* Game ID */
	if (s_agid) {
		iff_newchunk ("AGID", s_agid, f);
		fwrite (game.id, 1, strlen (game.id) + 1, f);
		s_agid -= strlen (game.id) + 1;
		iff_chunk_pad (s_agid, f);
	}
	
	/* Game CRC */
	iff_newchunk ("GCRC", s_gcrc, f);
	write_num (crc, f);

	/* Save game description */
	iff_newchunk ("DESC", s_desc, f);
	s_desc -= fwrite (d, 1, (strlen (d) + 1), f);
	iff_chunk_pad (s_desc, f);

	/* Variables */
	iff_newchunk ("VARS", s_vars, f);
	write_num (MAX_VARS, f);
	s_vars -= 4;
	fwrite (&game.vars, 1, MAX_VARS, f);
	s_vars -= MAX_VARS;
	iff_chunk_pad (s_vars, f);

	/* Flags */
	iff_newchunk ("FLAG", s_flag, f);
	write_num (MAX_FLAGS, f);
	s_flag -= 4;
	fwrite (&game.flags, 1, MAX_FLAGS, f);
	s_flag -= MAX_FLAGS;
	iff_chunk_pad (s_flag, f);

	/* Strings */
	iff_newchunk ("STRI", s_stri, f);
	write_num (MAX_WORDS1, f);
	s_stri -= 4;
	for (i = 0; i < MAX_WORDS1; i++) {
		fwrite (game.strings[i], 1, strlen (game.strings[i]) + 1, f);
		s_stri -= strlen (game.strings[i]) + 1;
	}
	iff_chunk_pad (s_stri, f);

	/* Save the objects */
	iff_newchunk ("OBJS", s_objs, f);

	write_num (game.num_objects, f);
	for (i = 0; i < game.num_objects; i++) {
		UINT8 x = object_get_location (i);
		fwrite (&x, 1, 1, f);
	}

	for (i = 0; i < MAX_VIEWTABLE; i++) {
		struct vt_entry *v = &game.view_table[i];
		iff_newchunk ("VIEW", s_view, f);
		write_num (v->entry, f);
		write_num (v->step_time, f);
		write_num (v->step_time_count, f);
		write_num (v->x_pos, f);
		write_num (v->y_pos, f);
		write_num (v->current_view, f);
		write_num (v->current_loop, f);
		write_num (v->num_loops, f);
		write_num (v->current_cel, f);
		write_num (v->num_cels, f);
		write_num (v->x_pos2, f);
		write_num (v->y_pos2, f);
		write_num (v->x_size, f);
		write_num (v->y_size, f);
		write_num (v->step_size, f);
		write_num (v->cycle_time, f);
		write_num (v->cycle_time_count, f);
		write_num (v->direction, f);
		write_num (v->motion, f);
		write_num (v->cycle, f);
		write_num (v->priority, f);
		write_num (v->flags, f);
		write_num (v->parm1, f);
		write_num (v->parm2, f);
		write_num (v->parm3, f);
		write_num (v->parm4, f);
	}

	iff_newchunk ("SPIC", s_spic, f);
	fwrite (get_sarien_screen (), GFX_WIDTH, GFX_HEIGHT, f);

	iff_newchunk ("APIC", s_apic, f);
	fwrite (game.sbuf, _WIDTH, _HEIGHT, f);

	fclose (f);

	return err_OK;
}

static UINT32 read_num(UINT8 *x)
{
	UINT32 a;

	a = hilo_getdword (x);
	x += 4;

	return a;
}

static void get_apic (UINT32 size, UINT8 *b)
{
	memcpy (game.sbuf, b, size);
}

static void get_spic (UINT32 size, UINT8 *b)
{
	memcpy (get_sarien_screen (), b, size);
}

static void get_view (UINT32 size, UINT8 *b)
{
	UINT32 i;
	struct vt_entry *v;

	i = read_num (b);
	_D ("(%d, %p) entry = %d", size, b, i);

	if (i > MAX_VIEWTABLE)
		return;

	v = &game.view_table[i];

	v->entry		= i;
	v->step_time		= read_num (b);
	v->step_time_count	= read_num (b);
	v->x_pos		= read_num (b);
	v->y_pos		= read_num (b);
	v->current_view		= read_num (b);
	v->current_loop		= read_num (b);
	v->num_loops		= read_num (b);
	v->current_cel		= read_num (b);
	v->num_cels		= read_num (b);
	v->x_pos2		= read_num (b);
	v->y_pos2		= read_num (b);
	v->x_size		= read_num (b);
	v->y_size		= read_num (b);
	v->step_size		= read_num (b);
	v->cycle_time		= read_num (b);
	v->cycle_time_count	= read_num (b);
	v->direction		= read_num (b);
	v->motion		= read_num (b);
	v->cycle		= read_num (b);
	v->priority		= read_num (b);
	v->flags		= read_num (b);
	v->parm1		= read_num (b);
	v->parm2		= read_num (b);
	v->parm3		= read_num (b);
	v->parm4		= read_num (b);

#if 0
	/* commented to prevent crash with uninitalized entries! */
	set_view (v, v->current_view);
	set_loop (v, v->current_loop);
#endif
}


static void get_stri (UINT32 size, UINT8 *buffer)
{
	UINT32 i, j, n;
	UINT8 b;

	_D ("(%d, %p)", size, buffer);

	n = hilo_getdword (buffer);
 	buffer += 4;

	for (i = 0; i < n; i++) {
		for (j = 0, b = 1; b; j++) {
			b = hilo_getbyte (buffer++);
			game.strings[i][j] = b;
		}
	}
}


static void get_flag (UINT32 size, UINT8 *buffer)
{
	int n;

	_D ("(%d, %p)", size, buffer);

	n = hilo_getdword (buffer);
	buffer += 4;

	memcpy (game.flags, buffer, n);
}


static void get_vars (UINT32 size, UINT8 *buffer)
{
	int n;

	n = hilo_getdword (buffer);
	_D ("(%d, %p) get %d vars", size, buffer, n);
	buffer += 4;

	memcpy (game.vars, buffer, n);

	_D ("done");
}


static void get_objs (UINT32 size, UINT8 *buffer)
{
	UINT32 i, n;

	_D ("(%d, %p)", size, buffer);

	n = hilo_getdword (buffer);
	buffer += 4;

	for (i = 0; i < n; i++) {
		object_set_location (i, hilo_getbyte (buffer++));
	}
}


static void get_agid (UINT32 size, UINT8 *buffer)
{
	_D ("(%d, %p)", size, buffer);

	if (size < strlen (game.id)) {
		loading_ok = 0;
		return;
	}

	if (memcmp (buffer, game.id, strlen (game.id)))
		loading_ok = 0;
}


static void get_gcrc (UINT32 size, UINT8 *buffer)
{
	_D ("(%d, %p)", size, buffer);
}


int load_game (char *s)
{
	FILE *f;
	struct iff_header h;

	_D ("(\"%s\")", s);
	if ((f = fopen (s, "r")) == NULL)
		return err_BadFileOpen;

	fread (&h, 1, sizeof (struct iff_header), f);
	if (h.form[0] != 'F' || h.form[1] != 'O' || h.form[2] != 'R' ||
		h.form[3] != 'M' || h.id[0] != 'F' || h.id[1] != 'A' ||
		h.id[2] != 'G' || h.id[3] != 'I') {
		fclose (f);
		return err_BadFileOpen;
	}

	_D ("registering IFF chunks");

	/* IFF chunk IDs */
	iff_register ("AGID", get_agid);
	iff_register ("GCRC", get_gcrc);
	iff_register ("FLAG", get_flag);
	iff_register ("VARS", get_vars);
	iff_register ("STRI", get_stri);
	iff_register ("OBJS", get_objs);
	iff_register ("VIEW", get_view);
	iff_register ("APIC", get_apic);
	iff_register ("SPIC", get_spic);

	loading_ok = 1;

	/* Load IFF chunks */
	while (loading_ok && !feof (f))
		iff_chunk (f);

	iff_release ();
	fclose (f);

#if 0
	/* FIXME */
	cmd_draw_pic (getvar (V_cur_room));
	/* redraw_sprites (); */
	game.exit_all_logics = TRUE;
#endif

	setflag (F_restore_just_ran, TRUE);

	return err_OK;
}

int savegame_dialog ()
{
	char home[MAX_PATH], path[MAX_PATH];
	char *desc;
	int slot = 0;

	if (get_app_dir (home, MAX_PATH) < 0) {
		message_box ("Couldn't save game.");
		return err_BadFileOpen;
	}

	message_box ("Multi-slot savegames are under development and"
		"will be available in future versions of Sarien.");
 	desc = "Save game test";

	/* DATADIR conflicts with ObjIdl.h in win32 SDK,
		renamed to DATA_DIR */
	sprintf (path, "%s/" DATA_DIR "/", home);

	MKDIR (path, 0755);
	sprintf (path, "%s/" DATA_DIR "/%s/", home, game.id);
	MKDIR (path, 0711);

	sprintf (path, "%s/" DATA_DIR "/%s/%08d.iff",
		home, game.id, slot);
	_D (_D_WARN "file is [%s]", path);
	
	save_game (path, desc);

	message_box ("Game saved.");

	return err_OK;
}


int loadgame_dialog ()
{
	char home[MAX_PATH], path[MAX_PATH];
	int slot = 0;
	int rc;

	if (get_app_dir (home, MAX_PATH) < 0) {
		message_box ("Error loading game.");
		return err_BadFileOpen;
	}

	sprintf (path, "%s/" DATA_DIR "/", home);
	MKDIR (path, 0755);
	sprintf (path, "%s/" DATA_DIR "/%s/", home, game.id);
	MKDIR (path, 0711);
	
	sprintf (path, "%s/" DATA_DIR "/%s/%08d.iff",
		home, game.id, slot);
	if ((rc = load_game (path)) == err_OK) {
		message_box ("Game loaded.");
		/* FIXME: restore at correct point instead of using the
		 *	  new.room hack
		 */
		show_pic ();
		flush_screen ();
		do_update ();
		game.vars[V_border_touch_ego] = 0;
		new_room (game.vars[0]);
		setflag (F_new_room_exec, TRUE);
		game.input_mode = INPUT_NORMAL;
		write_status ();
	} else {
		message_box ("Error loading game.");
	}

	return rc;
}

#endif /* PALMOS */

/* end: savegame.c */

