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
#include <sys/stat.h>
#include <sys/types.h>

#include "sarien.h"
#include "agi.h"
#include "keyboard.h"
#include "text.h"
#include "opcodes.h"
#include "savegame.h"
#include "iff.h"

static int loading_ok;

#define CENTER -1, -1, -1

#ifdef WIN32

/* Date: Tue, 22 May 2001 16:22:06 +0400
 * From: Igor Nesterov <nest@rtsnet.ru>
 * To: sarien-devel@lists.sourceforge.net
 * Subject: Fw: [sarien-devel] Improved savegames
 * 
 * I remember yet another "ms-compatible solution" common for Win9X and
 * WinNT/2K: ApplicationData directory. Value AppData in registry key
 * "HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Explorer\
 * Shell Folders" hold path to this directory. This maps to "c:\windows\
 * appilcation data" for Win9X and "C:\Documents and Settings\UserName\
 * Application Data for NT
 */

#include <windows.h>

int get_app_dir (char *app_dir, unsigned int size)
{
	const char *szapp = "AppData";
	const char *szpath = "Software\\Microsoft\\Windows\\CurrentVersion\\"
		"Explorer\\Shell Folders";
	HKEY hkey;
	DWORD type;

	if (RegOpenKey (HKEY_CURRENT_USER, szpath, &hkey) != ERROR_SUCCESS)
		return -1;

	if (RegQueryValueEx (hkey, szapp, NULL, &type,
		(unsigned char *)app_dir, &size) != ERROR_SUCCESS)
	{
		RegCloseKey (hkey);
		return -1;
	}

	RegCloseKey (hkey);

	return 0;
}

#else

/* MS-DOS and UNIX */

int get_app_dir (char *app_dir, unsigned int size)
{
	char *x;

	x = getenv (HOMEDIR);
	if (x) {
		strncpy (app_dir, x, size);
	} else {
		x = getenv ("SARIEN");
		if (x)
			strncpy (app_dir, x, size);
	}
	
	return x ? 0 : -1;
}

#endif


/* Words are big-endian */

static void write32 (UINT32 n, FILE *f)
{
	UINT8 size[4];

	size[0] = (n & 0xff000000) >> 24;
	size[1] = (n & 0x00ff0000) >> 16;
	size[2] = (n & 0x0000ff00) >> 8;
	size[3] = (n & 0x000000ff) >> 0;

	fwrite (size, 1, 4, f);
}


static void write16 (UINT16 n, FILE *f)
{
	UINT8 size[2];

	size[0] = (n & 0xff00) >> 8;
	size[1] = (n & 0x00ff) >> 0;

	fwrite (size, 1, 2, f);
}


static void write8 (UINT8 n, FILE *f)
{
	fwrite (&n, 1, 1, f);
}


static void iff_newchunk (char *t, UINT32 s, FILE *f)
{
	fwrite (t, 1, 4, f);
	write32 (s, f);
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
	UINT8 b;
	UINT32 i, s_agid, s_gcrc, s_form, s_desc, s_objs;
	UINT32 s_flag, s_vars, s_stri, s_view;
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
	for (i = 0; i < MAX_WORDS1; s_stri += strlen (game.strings[i++]) + 1);
	s_view = 28;
	s_form = s_agid + s_gcrc + s_desc + s_vars + s_flag + s_stri +
		/* s_vtbl + */ MAX_VIEWTABLE * (8 + s_view) + s_objs;

	iff_newgroup ("FORM", s_form, "IAGI", f);

	/* Game ID */
	if (s_agid) {
		iff_newchunk ("AGID", s_agid, f);
		fwrite (game.id, 1, strlen (game.id) + 1, f);
		s_agid -= strlen (game.id) + 1;
		iff_chunk_pad (s_agid, f);
	}
	
	/* Game CRC */
	iff_newchunk ("GCRC", s_gcrc, f);
	write32 (crc, f);

	/* Save game description */
	iff_newchunk ("DESC", s_desc, f);
	s_desc -= fwrite (d, 1, (strlen (d) + 1), f);
	iff_chunk_pad (s_desc, f);

	/* Variables */
	iff_newchunk ("VARS", s_vars, f);
	write32 (MAX_VARS, f);
	s_vars -= 4;
	for (i = 0; i < MAX_VARS; i++, s_vars--) {
		b = getvar (i);
		write8 (b, f);
	}
	iff_chunk_pad (s_vars, f);

	/* Flags */
	iff_newchunk ("FLAG", s_flag, f);
	write32 (MAX_FLAGS, f);
	s_flag -= 4;
	for (i = 0; i < MAX_FLAGS; i++, s_flag--) {
		b = getflag (i);
		write8 (b, f);
	}
	iff_chunk_pad (s_flag, f);

	/* Strings */
	iff_newchunk ("STRI", s_stri, f);
	write32 (MAX_WORDS1, f);
	s_stri -= 4;
	for (i = 0; i < MAX_WORDS1; i++) {
		fwrite (game.strings[i], 1, strlen (game.strings[i]) + 1, f);
		s_stri -= strlen (game.strings[i]) + 1;
	}
	iff_chunk_pad (s_stri, f);

	/* Save the objects */
	iff_newchunk ("OBJS", s_objs, f);

	write32 (game.num_objects, f);
	for (i = 0; i < game.num_objects; i++) {
		write8 (object_get_location (i), f);
	}

	for (i = 0; i < MAX_VIEWTABLE; i++) {
		struct vt_entry *v = &game.view_table[i];
		iff_newchunk ("VIEW", s_view, f);
		write32 (i, f);
		write8 (v->step_time, f);
		write8 (v->step_time_count, f);
		write8 (v->x_pos, f);
		write8 (v->y_pos, f);
		write8 (v->current_view, f);
		write8 (v->current_loop, f);
		write8 (v->current_cel, f);
		write8 (v->step_size, f);
		write8 (v->cycle_time, f);
		write8 (v->cycle_time_count, f);
		write8 (v->direction, f);
		write8 (v->motion, f);
		write8 (v->cycle, f);
		write8 (v->priority, f);
		write16 (v->flags, f);
		write8 (v->parm1, f);
		write8 (v->parm2, f);
		write8 (v->parm3, f);
		write8 (v->parm4, f);
	}

	fclose (f);

	return err_OK;
}


static void get_view (int size, UINT8 *b)
{
	UINT32 i;
	struct vt_entry *v;

	_D ("(%d, %p)", size, b);

	i = hilo_getdword (b);
 	b += 4;

	if (i > MAX_VIEWTABLE)
		return;

	v = &game.view_table[i];

	v->step_time		= hilo_getbyte (b++);
	v->step_time_count	= hilo_getbyte (b++);
	v->x_pos		= hilo_getbyte (b++);
	v->y_pos		= hilo_getbyte (b++);
	v->current_view		= hilo_getbyte (b++);
	v->current_loop		= hilo_getbyte (b++);
	v->current_cel		= hilo_getbyte (b++);
	v->step_size		= hilo_getbyte (b++);
	v->cycle_time		= hilo_getbyte (b++);
	v->cycle_time_count	= hilo_getbyte (b++);
	v->direction		= hilo_getbyte (b++);
	v->motion		= hilo_getbyte (b++);
	v->cycle		= hilo_getbyte (b++);
	v->priority		= hilo_getbyte (b++);
	v->flags		= hilo_getword(b); b+=2;
	v->parm1		= hilo_getbyte (b++);
	v->parm2		= hilo_getbyte (b++);
	v->parm3		= hilo_getbyte (b++);
	v->parm4		= hilo_getbyte (b++);
}


static void get_stri (int size, UINT8 *buffer)
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


static void get_flag (int size, UINT8 *buffer)
{
	UINT32 i, n;
	UINT8 b;

	_D ("(%d, %p)", size, buffer);

	n = hilo_getdword (buffer);
	buffer += 4;

	for (i = 0; i < n; i++) {
		b = hilo_getbyte (buffer++);
		setflag (i, b);
	}
}


static void get_vars (int size, UINT8 *buffer)
{
	UINT32 i, n;
	UINT8 b;

	_D ("(%d, %p)", size, buffer);

	n = hilo_getdword (buffer);
	buffer += 4;

	for (i = 0; i < n; i++) {
		b = hilo_getbyte (buffer++);
		setvar (i, b);
	}
}


static void get_objs (int size, UINT8 *buffer)
{
	UINT32 i, n;

	_D ("(%d, %p)", size, buffer);

	n = hilo_getdword (buffer);
	buffer += 4;

	for (i = 0; i < n; i++) {
		object_set_location (i, hilo_getbyte (buffer++));
	}
}


static void get_agid (int size, UINT8 *buffer)
{
	_D ("(%d, %p)", size, buffer);

	if (size < strlen (game.id)) {
		loading_ok = 0;
		return;
	}

	if (memcmp (buffer, game.id, strlen (game.id)))
		loading_ok = 0;
}


static void get_gcrc (int size, UINT8 *buffer)
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
		h.form[3] != 'M' || h.id[0] != 'I' || h.id[1] != 'A' ||
		h.id[2] != 'G' || h.id[3] != 'I') {
		fclose (f);
		return err_BadFileOpen;
	}

	/* IFF chunk IDs */
	iff_register ("AGID", get_agid);
	iff_register ("GCRC", get_gcrc);
	iff_register ("FLAG", get_flag);
	iff_register ("VARS", get_vars);
	iff_register ("STRI", get_stri);
	iff_register ("OBJS", get_objs);
	/* iff_register ("VTBL", get_vtbl); */
	iff_register ("VIEW", get_view);

	loading_ok = 1;

	/* Load IFF chunks */
	while (loading_ok && !feof (f))
		iff_chunk (f);

	iff_release ();
	fclose (f);

	/* FIXME */
	//cmd_draw_pic (getvar (V_cur_room));
	/* redraw_sprites (); */
	//game.new_room_num = getvar (V_cur_room);
	//game.ego_in_new_room = TRUE;
	game.exit_all_logics = TRUE;

	return loading_ok ? err_OK : err_BadFileOpen;
}


int savegame_dialog ()
{
	char home[MAX_PATH], path[MAX_PATH];
	char *desc;
	int slot = 0;

	if (get_app_dir (home, MAX_PATH) < 0) {
		message_box ("Couldn't save game.", CENTER);
		return err_BadFileOpen;
	}

	message_box (
		"Multi-slot savegames are under development and"
		"will be available in future versions of Sarien."
		, -1, -1, -1);
	wait_key ();
 	desc = "Save game test";

	snprintf (path, MAX_PATH, "%s/" DATADIR "/", home);
	mkdir (path, 0755);
	snprintf (path, MAX_PATH, "%s/" DATADIR "/%s/", home, game.id);
	_D (_D_WARN "path is [%s]", path);
	mkdir (path, 0711);

	snprintf (path, MAX_PATH, "%s/" DATADIR "/%s/%08d.iff",
		home, game.id, slot);
	_D (_D_WARN "file is [%s]", path);
	
	save_game (path, desc);

	message_box ("Game saved.", CENTER);

	return err_OK;
}


int loadgame_dialog ()
{
	char home[MAX_PATH], path[MAX_PATH];
	int slot = 0;
	int rc;

	if (get_app_dir (home, MAX_PATH) < 0) {
		message_box ("Error loading game.", CENTER);
		return err_BadFileOpen;
	}

	snprintf (path, MAX_PATH, "%s/" DATADIR "/", home);
	mkdir (path, 0755);
	snprintf (path, MAX_PATH, "%s/" DATADIR "/%s/", home, game.id);
	mkdir (path, 0711);
	
	snprintf (path, MAX_PATH, "%s/" DATADIR "/%s/%08d.iff",
		home, game.id, slot);
	if ((rc = load_game (path)) == err_OK)
		message_box ("Gamed loaded.", CENTER);
	else
		message_box ("Error loading game.", CENTER);

	return rc;
}

