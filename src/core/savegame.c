/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

/*
 * Savegame support by Vasyl Tsvirkunov <vasyl@pacbell.net>
 */

#ifndef PALMOS

#include <stdio.h>
#include <string.h>
#include <assert.h>

#ifndef __MPW__
#  ifndef __DICE__
#    include <sys/stat.h>
#  endif
#  include <sys/types.h>
#endif
#ifdef WIN32
#  include <direct.h>
#endif

#include "sarien.h"
#include "agi.h"
#include "graphics.h"
#include "sprite.h"
#include "text.h"
#include "savegame.h"

#if defined(__DICE__) || defined(WIN32)
#  define MKDIR(a,b) mkdir(a)
#else
#  define MKDIR(a,b) mkdir(a,b)
#endif

/* Image stack support */
struct image_stack_element {
	UINT8 type;
	UINT8 pad;
	SINT16 parm1;
	SINT16 parm2;
	SINT16 parm3;
	SINT16 parm4;
	SINT16 parm5;
	SINT16 parm6;
	SINT16 parm7;
};

#define IMAGE_STACK_SIZE 32
static struct image_stack_element image_stack[IMAGE_STACK_SIZE];
static int image_stack_pointer = 0;

void clear_image_stack(void)
{
	image_stack_pointer = 0;
}

void record_image_stack_call(UINT8 type, SINT16 p1, SINT16 p2, SINT16 p3, SINT16 p4, SINT16 p5, SINT16 p6, SINT16 p7)
{
	struct image_stack_element* pnew;

	image_stack_pointer++;
	assert (image_stack_pointer < IMAGE_STACK_SIZE);
	pnew = &image_stack[image_stack_pointer];

	pnew->type = type;
	pnew->parm1 = p1;
	pnew->parm2 = p2;
	pnew->parm3 = p3;
	pnew->parm4 = p4;
	pnew->parm5 = p5;
	pnew->parm6 = p6;
	pnew->parm7 = p7;
}

void replay_image_stack_call(UINT8 type, SINT16 p1, SINT16 p2, SINT16 p3, SINT16 p4, SINT16 p5, SINT16 p6, SINT16 p7)
{
	switch(type)
	{
	case ADD_PIC:
		agi_load_resource(rPICTURE, p1);
		decode_picture(p1, p2);
		break;
	case ADD_VIEW:
		agi_load_resource(rVIEW, p1);
		add_to_pic(p1, p2, p3, p4, p5, p6, p7);
		break;
	}
}

/* */

static char* strSig = "Sarien:";

void write_uint8(FILE* f, SINT8 val)
{
	fwrite(&val, 1, 1, f);
}

void write_sint16(FILE* f, SINT16 val)
{
	static UINT8 buf[2];
	buf[0] = (UINT8)((val>>8)&255);
	buf[1] = (UINT8)(val&255);
	fwrite(buf, 1, 2, f);
}

void write_uint16(FILE* f, UINT16 val)
{
	static UINT8 buf[2];
	buf[0] = (UINT8)((val>>8)&255);
	buf[1] = (UINT8)(val&255);
	fwrite(buf, 1, 2, f);
}

UINT8 read_uint8(FILE* f)
{
	static UINT8 buf[1];
	fread(buf, 1, 1, f);
	return buf[0];
}

UINT16 read_uint16(FILE* f)
{
	static UINT8 buf[2];
	fread(buf, 1, 2, f);
	return (buf[0]<<8)|buf[1];
}

SINT16 read_sint16(FILE* f)
{
	static UINT8 buf[2];
	fread(buf, 1, 2, f);
	return (SINT16)((buf[0]<<8)|buf[1]);
}


void write_string(FILE* f, char* s)
{
	write_sint16(f, (SINT16)strlen(s));
	fwrite(s, 1, strlen(s), f);
}

void read_string(FILE* f, char* s)
{
	SINT16 size = read_sint16(f);
	fread(s, 1, size, f);
	s[size] = (char)0;
}

void write_bytes(FILE* f, char* s, SINT16 size)
{
	fwrite(s, 1, size, f);
}

void read_bytes(FILE* f, char* s, SINT16 size)
{
	fread(s, 1, size, f);
}

int save_game(char* s, char* d)
{
	SINT16 i;
	struct image_stack_element* ptr = image_stack;
	FILE* f=fopen(s, "wb");

	if(!f)
		return err_BadFileOpen;

	write_bytes(f, strSig, 8);
	write_string(f, d);

	write_sint16(f, (SINT16)game.state);
	/* game.name */
	write_string(f, game.id);
	/* game.crc */

	for(i=0; i<MAX_FLAGS; i++)
		write_uint8(f, game.flags[i]);
	for(i=0; i<MAX_VARS; i++)
		write_uint8(f, game.vars[i]);

	write_sint16(f, (SINT16)game.horizon);
	write_sint16(f, (SINT16)game.line_status);
	write_sint16(f, (SINT16)game.line_user_input);
	write_sint16(f, (SINT16)game.line_min_print);
	/* game.cursor_pos */
	/* game.input_buffer */
	/* game.echo_buffer */
	/* game.keypress */
	write_sint16(f, (SINT16)game.input_mode);
	write_sint16(f, (SINT16)game.lognum);

	write_sint16(f, (SINT16)game.player_control);
	write_sint16(f, (SINT16)game.quit_prog_now);
	write_sint16(f, (SINT16)game.status_line);
	write_sint16(f, (SINT16)game.clock_enabled);
	write_sint16(f, (SINT16)game.exit_all_logics);
	write_sint16(f, (SINT16)game.picture_shown);
	write_sint16(f, (SINT16)game.has_prompt);
	write_sint16(f, (SINT16)game.game_flags);

	write_sint16(f, (SINT16)game.alt_pri);
	for(i=0; i<_HEIGHT; i++)
		write_uint8(f, game.pri_table[i]);

	/* game.msg_box_ticks */
	/* game.block */
	/* game.window */
	/* game.has_window */

	write_sint16(f, (SINT16)game.gfx_mode);
	write_uint8(f, game.cursor_char);
	write_sint16(f, (SINT16)game.color_fg);
	write_sint16(f, (SINT16)game.color_bg);

	/* game.hires (#ifdef USE_HIRES) */
	/* game.sbuf */

	/* game.ego_words */
	/* game.num_ego_words */

	write_sint16(f, (SINT16)game.num_objects);
	for(i=0; i<(SINT16)game.num_objects; i++)
		write_sint16(f, (SINT16)object_get_location(i));

	/* game.ev_keyp */
	/* game.ev_scan */
	for(i=0; i<MAX_WORDS1; i++)
		write_string(f, game.strings[i]);

	/* record info about loaded resources */
	for(i=0; i<MAX_DIRS; i++)
	{
		write_uint8(f, game.dir_logic[i].flags);
		write_sint16(f, (SINT16)game.logics[i].sIP);
		write_sint16(f, (SINT16)game.logics[i].cIP);
	}
	for(i=0; i<MAX_DIRS; i++)
		write_uint8(f, game.dir_pic[i].flags);
	for(i=0; i<MAX_DIRS; i++)
		write_uint8(f, game.dir_view[i].flags);
	for(i=0; i<MAX_DIRS; i++)
		write_uint8(f, game.dir_sound[i].flags);

	/* game.pictures */
	/* game.logics */
	/* game.views */
	/* game.sounds */

	for(i=0; i<MAX_VIEWTABLE; i++)
	{
		struct vt_entry* v = &game.view_table[i];

		write_uint8(f, v->step_time);
		write_uint8(f, v->step_time_count);
		write_uint8(f, v->entry);
		write_sint16(f, v->x_pos);
		write_sint16(f, v->y_pos);
		write_uint8(f, v->current_view);

		/* v->view_data */

		write_uint8(f, v->current_loop);
		write_uint8(f, v->num_loops);

		/* v->loop_data */

		write_uint8(f, v->current_cel);
		write_uint8(f, v->num_cels);

		/* v->cel_data */
		/* v->cel_data_2 */
		
		write_sint16(f, v->x_pos2);
		write_sint16(f, v->y_pos2);

		/* v->s */

		write_sint16(f, v->x_size);
		write_sint16(f, v->y_size);
		write_uint8(f, v->step_size);
		write_uint8(f, v->cycle_time);
		write_uint8(f, v->cycle_time_count);
		write_uint8(f, v->direction);

		write_uint8(f, v->motion);
		write_uint8(f, v->cycle);
		write_uint8(f, v->priority);

		write_uint16(f, v->flags);
		
		write_uint8(f, v->parm1);
		write_uint8(f, v->parm2);
		write_uint8(f, v->parm3);
		write_uint8(f, v->parm4);
	}

/* Save image stack */
	
	for (i = 0; i < image_stack_pointer; i++) {
		ptr = &image_stack[i];
		write_uint8(f, ptr->type);
		write_sint16(f, ptr->parm1);
		write_sint16(f, ptr->parm2);
		write_sint16(f, ptr->parm3);
		write_sint16(f, ptr->parm4);
		write_sint16(f, ptr->parm5);
		write_sint16(f, ptr->parm6);
		write_sint16(f, ptr->parm7);
	}
	write_uint8(f, 0);

	fclose(f);

	return err_OK;
}

int load_game(char* s)
{
	int i;
	UINT8 t;
	SINT16 parm[7];
	char sig[8];
	char id[8];
	char description[256];
	FILE* f=fopen(s, "rb");

	if(!f)
		return err_BadFileOpen;

	read_bytes(f, sig, 8);
	if(strncmp(sig, strSig, 8))
	{
		fclose(f);
		return err_BadFileOpen;
	}

	read_string(f, description);

	game.state = read_sint16(f);
	/* game.name - not saved */
	read_string(f, id);
	if(strcmp(id, game.id))
	{
		fclose(f);
		return err_BadFileOpen;
	}
	/* game.crc - not saved */

	for(i=0; i<MAX_FLAGS; i++)
		game.flags[i] = read_uint8(f);
	for(i=0; i<MAX_VARS; i++)
		game.vars[i] = read_uint8(f);

	game.horizon = read_sint16(f);
	game.line_status = read_sint16(f);
	game.line_user_input = read_sint16(f);
	game.line_min_print = read_sint16(f);

	/* These are never saved */
	game.cursor_pos = 0;
	game.input_buffer[0] = 0;
	game.echo_buffer[0] = 0;
	game.keypress = 0;

	game.input_mode = read_sint16(f);
	game.lognum = read_sint16(f);

	game.player_control = read_sint16(f);
	game.quit_prog_now = read_sint16(f);
	game.status_line = read_sint16(f);
	game.clock_enabled = read_sint16(f);
	game.exit_all_logics = read_sint16(f);
	game.picture_shown = read_sint16(f);
	game.has_prompt = read_sint16(f);
	game.game_flags = read_sint16(f);

	game.alt_pri = read_sint16(f);
	for(i=0; i<_HEIGHT; i++)
		game.pri_table[i] = read_uint8(f);

	if(game.has_window)
		close_window();
	game.msg_box_ticks = 0;
	game.block.active = FALSE;
	/* game.window - fixed by close_window() */
	/* game.has_window - fixed by close_window() */

	game.gfx_mode = read_sint16(f);
	game.cursor_char = read_uint8(f);
	game.color_fg = read_sint16(f);
	game.color_bg = read_sint16(f);

	/* game.hires (#ifdef USE_HIRES) - rebuilt from image stack */
	/* game.sbuf - rebuilt from image stack */

	/* game.ego_words - fixed by clean_input */
	/* game.num_ego_words - fixed by clean_input */

	game.num_objects = read_sint16(f);
	for(i=0; i<(SINT16)game.num_objects; i++)
		object_set_location(i, read_sint16(f));

	/* Those are not serialized */
	for (i = 0; i < MAX_DIRS; i++) {
		game.ev_scan[i].occured = FALSE;
		game.ev_keyp[i].occured = FALSE;
	}

	for(i=0; i<MAX_WORDS1; i++)
		read_string(f, game.strings[i]);

	for(i=0; i<MAX_DIRS; i++)
	{
		if(read_uint8(f) & RES_LOADED)
			agi_load_resource(rLOGIC, i);
		else if(!(game.dir_logic[i].flags & RES_CACHED))
			agi_unload_resource(rLOGIC, i);
		game.logics[i].sIP = read_sint16(f);
		game.logics[i].cIP = read_sint16(f);
	}
	for(i=0; i<MAX_DIRS; i++)
	{
		if(read_uint8(f) & RES_LOADED)
			agi_load_resource(rPICTURE, i);
		else if(!(game.dir_pic[i].flags & RES_CACHED))
			agi_unload_resource(rPICTURE, i);
	}
	for(i=0; i<MAX_DIRS; i++)
	{
		if(read_uint8(f) & RES_LOADED)
			agi_load_resource(rVIEW, i);
		else if(!(game.dir_view[i].flags & RES_CACHED))
			agi_unload_resource(rVIEW, i);
	}
	for(i=0; i<MAX_DIRS; i++)
	{
		if(read_uint8(f) & RES_LOADED)
			agi_load_resource(rSOUND, i);
		else if(!(game.dir_sound[i].flags & RES_CACHED))
			agi_unload_resource(rSOUND, i);
	}

	/* game.pictures - loaded above */
	/* game.logics - loaded above */
	/* game.views - loaded above */
	/* game.sounds - loaded above */

	for(i=0; i<MAX_VIEWTABLE; i++)
	{
		struct vt_entry* v = &game.view_table[i];

		v->step_time = read_uint8(f);
		v->step_time_count = read_uint8(f);
		v->entry = read_uint8(f);
		v->x_pos = read_sint16(f);
		v->y_pos = read_sint16(f);
		v->current_view = read_uint8(f);

		/* v->view_data - fixed below  */

		v->current_loop = read_uint8(f);
		v->num_loops = read_uint8(f);

		/* v->loop_data - fixed below  */

		v->current_cel = read_uint8(f);
		v->num_cels = read_uint8(f);

		/* v->cel_data - fixed below  */
		/* v->cel_data_2 - fixed below  */
		
		v->x_pos2 = read_sint16(f);
		v->y_pos2 = read_sint16(f);

		/* v->s - fixed below */

		v->x_size = read_sint16(f);
		v->y_size = read_sint16(f);
		v->step_size = read_uint8(f);
		v->cycle_time = read_uint8(f);
		v->cycle_time_count = read_uint8(f);
		v->direction = read_uint8(f);

		v->motion = read_uint8(f);
		v->cycle = read_uint8(f);
		v->priority = read_uint8(f);

		v->flags = read_uint16(f);
		
		v->parm1 = read_uint8(f);
		v->parm2 = read_uint8(f);
		v->parm3 = read_uint8(f);
		v->parm4 = read_uint8(f);
	}

/* Fix some pointers in viewtable */
	for(i=0; i<MAX_VIEWTABLE; i++)
	{
		struct vt_entry* v = &game.view_table[i];

		if(game.dir_view[v->current_view].offset == _EMPTY)
			continue;

		if(!(game.dir_view[v->current_view].flags & RES_LOADED))
			agi_load_resource(rVIEW, v->current_view);

		set_view(v, v->current_view); /* Fix v->view_data */
		set_loop(v, v->current_loop); /* Fix v->loop_data */
		set_cel(v, v->current_cel);   /* Fix v->cel_data */
		v->cel_data_2 = v->cel_data;
		v->s = NULL; /* not sure if it is used... */
	}

	erase_both();

/* Clear input line */
	clear_screen(0);
	write_status();

/* Recreate background from saved image stack */
	clear_image_stack();
	while((t = read_uint8(f)) != 0)
	{
		for(i=0; i<7; i++)
			parm[i] = read_sint16(f);
		replay_image_stack_call(t, parm[0], parm[1], parm[2], parm[3], parm[4], parm[5], parm[6]);
	}

	if(ferror(f) || feof(f))
	{
		fclose(f);
		return err_BadFileOpen;
	}

	fclose(f);

	setflag(F_restore_just_ran, TRUE);

	game.has_prompt = 0; /* force input line repaint if necessary*/
	clean_input();
	
	erase_both();
	blit_both();
	commit_both();
	show_pic();
	do_update();

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

	sprintf (path, "%s/" DATA_DIR "/%s/%08d.sav",
		home, game.id, slot);
	_D (_D_WARN "file is [%s]", path);
	
	save_game(path, desc);

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
	
	sprintf (path, "%s/" DATA_DIR "/%s/%08d.sav",
		home, game.id, slot);

	stop_sound();
	if((rc = load_game (path)) == err_OK)
		message_box ("Game loaded.");
	else
		message_box ("Error loading game.");

	return rc;
}

#endif /* PALMOS */

/* end: savegame.c */

