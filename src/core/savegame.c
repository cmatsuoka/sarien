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
#include <string.h>

#include "sarien.h"
#include "agi.h"
#include "gfx.h"
#include "keyboard.h"	/* remove later */
#include "view.h"
#include "objects.h"
#include "opcodes.h"
#include "savegame.h"
#include "iff.h"

static int loading_ok;

extern struct agi_view_table view_table[];
extern struct agi_object *objects;
extern int num_objects;


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
	UINT32 s_flag, s_vars, /* s_vtbl, */ s_stri, s_view;
	UINT32 crc = 0x12345678;	/* FIXME */

	_D (("(\"%s\", \"%s\")", s, d));

	f = fopen (s, "w");

	/* IFF chunk sizes */
	s_agid = strlen (game.id) + 1;
	s_gcrc = 4;
	s_desc = WORD_ALIGN (strlen (d) + 1);
	s_vars = WORD_ALIGN (MAX_VARS) + 4;
	s_flag = WORD_ALIGN (MAX_FLAGS) + 4;
	s_objs = num_objects + 4;	/* 1 byte / object */
	s_stri = 4;
	for (i = 0; i < MAX_WORDS1; s_stri += strlen (strings[i++]) + 1);
	/* s_vtbl = 4; */
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
		fwrite (strings[i], 1, strlen (strings[i]) + 1, f);
		s_stri -= strlen (strings[i]) + 1;
	}
	iff_chunk_pad (s_stri, f);

	/* Save the objects */
	iff_newchunk ("OBJS", s_objs, f);

	write32 (num_objects, f);
	for (i = 0; i < num_objects; i++) {
		write8 (objects[i].location, f);
	}

	for (i = 0; i < MAX_VIEWTABLE; i++) {
		iff_newchunk ("VIEW", s_view, f);
		write32 (i, f);
		write8 (view_table[i].step_time, f);
		write8 (view_table[i].step_time_count, f);
		write8 (view_table[i].x_pos, f);
		write8 (view_table[i].y_pos, f);
		write8 (view_table[i].current_view, f);
		write8 (view_table[i].current_loop, f);
		write8 (view_table[i].num_loops, f);
		write8 (view_table[i].cur_cel, f);
		write8 (view_table[i].num_cels, f);
		write8 (view_table[i].x_size, f);
		write8 (view_table[i].y_size, f);
		write8 (view_table[i].step_size, f);
		write8 (view_table[i].cycle_time, f);
		write8 (view_table[i].cycle_time_count, f);
		write8 (view_table[i].direction, f);
		write8 (view_table[i].motion, f);
		write8 (view_table[i].cycle_status, f);
		write8 (view_table[i].priority, f);
		write16 (view_table[i].flags, f);
		write8 (view_table[i].parm1, f);
		write8 (view_table[i].parm2, f);
		write8 (view_table[i].parm3, f);
		write8 (view_table[i].parm4, f);
	}

	fclose (f);

	return err_OK;
}


static void get_view (int size, UINT8 *buffer)
{
	UINT32 i;

	_D (("(%d, %p)", size, buffer));

	i = hilo_getdword (buffer);
 	buffer += 4;

	if (i > MAX_VIEWTABLE)
		return;

	view_table[i].step_time = hilo_getbyte (buffer++);
	view_table[i].step_time_count = hilo_getbyte (buffer++);
	view_table[i].x_pos = hilo_getbyte (buffer++);
	view_table[i].y_pos = hilo_getbyte (buffer++);
	view_table[i].current_view = hilo_getbyte (buffer++);
	view_table[i].current_loop = hilo_getbyte (buffer++);
	view_table[i].num_loops = hilo_getbyte (buffer++);
	view_table[i].cur_cel = hilo_getbyte (buffer++);
	view_table[i].num_cels = hilo_getbyte (buffer++);
	view_table[i].x_size = hilo_getbyte (buffer++);
	view_table[i].y_size = hilo_getbyte (buffer++);
	view_table[i].step_size = hilo_getbyte (buffer++);
	view_table[i].cycle_time = hilo_getbyte (buffer++);
	view_table[i].cycle_time_count = hilo_getbyte (buffer++);
	view_table[i].direction = hilo_getbyte (buffer++);
	view_table[i].motion = hilo_getbyte (buffer++);
	view_table[i].cycle_status = hilo_getbyte (buffer++);
	view_table[i].priority = hilo_getbyte (buffer++);
	/*view_table[i].flags = hilo_getword ((UINT8*)(((UINT16*)buffer)++));*/
	view_table[i].flags = hilo_getword(buffer);
	buffer+=2;
	view_table[i].parm1 = hilo_getbyte (buffer++);
	view_table[i].parm2 = hilo_getbyte (buffer++);
	view_table[i].parm3 = hilo_getbyte (buffer++);
	view_table[i].parm4 = hilo_getbyte (buffer++);
}


static void get_stri (int size, UINT8 *buffer)
{
	UINT32 i, j, n;
	UINT8 b;

	_D (("(%d, %p)", size, buffer));

	n = hilo_getdword (buffer);
 	buffer += 4;

	for (i = 0; i < n; i++) {
		for (j = 0, b = 1; b; j++) {
			b = hilo_getbyte (buffer++);
			strings[i][j] = b;
		}
	}
}


static void get_flag (int size, UINT8 *buffer)
{
	UINT32 i, n;
	UINT8 b;

	_D (("(%d, %p)", size, buffer));

	n = hilo_getdword (buffer);
	buffer += 4;

	for (i = 0; i < n; i++)
	{
		b = hilo_getbyte (buffer++);
		setflag (i, b);
	}
}


static void get_vars (int size, UINT8 *buffer)
{
	UINT32 i, n;
	UINT8 b;

	_D (("(%d, %p)", size, buffer));

	n = hilo_getdword (buffer);
	buffer += 4;

	for (i = 0; i < n; i++)
	{
		b = hilo_getbyte (buffer++);
		setvar (i, b);
	}
}


static void get_objs (int size, UINT8 *buffer)
{
	UINT32 i, n;

	n = hilo_getdword (buffer);
	buffer += 4;

	for (i = 0; i < n; i++) {
		objects[i].location = hilo_getbyte (buffer++);
	}
}


static void get_agid (int size, UINT8 *buffer)
{
	_D (("(%d, %p)", size, buffer));

	if (size < strlen (game.id)) {
		loading_ok = 0;
		return;
	}

	if (memcmp (buffer, game.id, strlen (game.id)))
		loading_ok = 0;
}


static void get_gcrc (int size, UINT8 *buffer)
{
	_D (("(%d, %p)", size, buffer));
}


int load_game (char *s)
{
	FILE *f;
	struct iff_header h;

	_D (("(\"%s\")", s));
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

	cmd_draw_pic (getvar (V_cur_room));
	redraw_sprites ();
	new_room_num = getvar (V_cur_room);
	game.ego_in_new_room = TRUE;
	exit_all_logics = TRUE;

	return loading_ok ? err_OK : err_BadFileOpen;
}

