/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#ifndef __AGI_H
#define __AGI_H

#ifdef __cplusplus
extern "C"{
#endif


struct agi_event {
	UINT8	event;
	UINT8	occured;
	UINT16	data;
};

struct agi_word {
	int id;
	char *word;
};

int	show_words	(void);
int	load_words	(char *);
void	unload_words	(void);
int	find_word	(char *);
void	dictionary_words(char *);


struct agi_dir {
	UINT8  volume;
	UINT32 offset;
	UINT32 len;
	UINT32 clen;
	UINT8  flags;
	/* 0 = not in mem, can be freed
	 * 1 = in mem, can be released
	 * 2 = not in mem, cant be released
	 * 3 = in mem, cant be released
	 * 0x40 = was compressed
	 */
};

/* This structure should contain everything we would want to
 * put in an agi "snapshot" savegame. Later in the cleanup process
 * we'll remove individual global variables and use this struct instead.
 */
struct agi_game {
	char name[8];		/* lead in id (eg, goldrush GR */
	char dir[8];		/* game dir (for v3 games, eg GR<dir> */
	char id[8];		/* game id */	

	/* game flags and variables */
	UINT8 flags[MAX_FLAGS];		/* 256 flags (byte is easier to debug! */
	UINT8 vars[MAX_VARS];			/* 256 variables */

	/* internal variables */
	int horizon;		/* horizon marker */
	int line_status;	/* line num to put status on */
	int line_user_input;	/* line to put user input on */
	int line_min_print;	/* num lines to print on */
	int message_box_key;	/* message box keypress */
	int new_room_num;

	/* internal flags */
	int ego_in_new_room;	/* new room flag */
	int control_mode;	/* who's in control */
	int quit_prog_now;	/* quit now */
	int status_line;	/* status line on/off */
	int allow_kyb_input;	/* allow keyboard input */
	int clock_enabled;	/* clock is on/off */
	int exit_all_logics;
#define ID_AGDS		0x00000001
#define ID_AMIGA	0x00000002
	int game_flags;		/* game flags!! (important) */

	/* directory entries for resources */
	struct agi_dir dir_logic[MAX_DIRS];
	struct agi_dir dir_pic[MAX_DIRS];
	struct agi_dir dir_view[MAX_DIRS];
	struct agi_dir dir_sound[MAX_DIRS];

	/* player command line */
	struct agi_word ego_words[MAX_WORDS];
	int num_ego_words;

	int num_objects;

	struct agi_event events[MAX_DIRS];	/* keyboard events */

	char strings[MAX_WORDS1][MAX_WORDS2];	/* strings */
#if 0
	/* resources */
	struct agi_picture pictures[MAX_DIRS];
	struct agi_logic logics[MAX_DIRS];
	struct agi_view views[MAX_DIRS];
	struct agi_object *objects;

	/* view table */
	struct agi_view_table view_table[MAX_VIEWTABLE];
#endif
};

extern struct agi_game game;

struct agi_loader {
	int version;
	int int_version;
	int (*init)(void);
	int (*deinit)(void);
	int (*detect_game)(UINT8 *);
	int (*load_resource)(int, int);
	int (*unload_resource)(int, int);
};


extern	UINT8	path[];

int	agi_init	(void);
int	agi_deinit	(void);

#ifdef __cplusplus
};
#endif
#endif
