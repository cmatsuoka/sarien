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

/* AGI resources */
#include "view.h"
#include "picture.h"
#include "logic.h"
#include "sound.h"

#define WIN_TO_PIC_X(x) ((x) / 2)
#define WIN_TO_PIC_Y(y) ((y) < 8 ? 999 : (y) >= (8 + _HEIGHT) ? 999 : (y) - 8)

/**
 * AGI variables.
 */
enum {
	V_cur_room = 0,			/* 0 */
	V_prev_room,
	V_border_touch_ego,
	V_score,
	V_border_code,
	V_border_touch_obj,		/* 5 */
	V_ego_dir,
	V_max_score,
	V_free_pages,
	V_word_not_found,
	V_time_delay,                   /* 10 */
	V_seconds,
	V_minutes,
	V_hours,
	V_days,
	V_joystick_sensitivity,         /* 15 */
	V_ego_view_resource,
	V_agi_err_code,
	V_agi_err_code_info,
	V_key,
	V_computer,                     /* 20 */
	V_window_reset,
	V_soundgen,
	V_volume,
	V_max_input_chars,
	V_sel_item,                     /* 25 */
	V_monitor
};

/**
 * AGI flags
 */
enum {
	F_ego_water = 0,		/* 0 */
	F_ego_invisible,
	F_entered_cli,
	F_ego_touched_p2,
	F_said_accepted_input,
	F_new_room_exec,		/* 5 */
	F_restart_game,
	F_script_blocked,
	F_joy_sensitivity,
	F_sound_on,
	F_debugger_on,			/* 10 */
	F_logic_zero_firsttime,
	F_restore_just_ran,
	F_status_selects_items,
	F_menus_work,
	F_output_mode,			/* 15 */
	F_auto_restart
};

struct agi_event {
	UINT16 data;
	UINT8 occured;
};

struct agi_object {
	int location;
	char *name;
};

struct agi_word {
	int id;
	char *word;
};


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

struct agi_block {
	int active;
	int x1, y1;
	int x2, y2;
	UINT8 *buffer;	/* used for window background */
};

#define EGO_VIEW_TABLE	0
#define	HORIZON		36
#define _WIDTH		160
#define _HEIGHT		168

/**
 * AGI game structure.
 * This structure contains all global data of an AGI game executed
 * by the interpreter.
 */
struct agi_game {
#define STATE_INIT	0x00
#define STATE_LOADED	0x01
#define STATE_RUNNING	0x02
	int state;		/**< state of the interpreter */

	char name[8];		/**< lead in id (e.g. `GR' for goldrush) */
	char id[8];		/**< game id */
	char dir[MAX_PATH];	/**< game dir */
	UINT32 crc;		/**< game CRC */

	/* game flags and variables */
	UINT8 flags[MAX_FLAGS];	/**< 256 1-bit flags */
	UINT8 vars[MAX_VARS];	/**< 256 variables */

	/* internal variables */
	int horizon;		/**< horizon y coordinate */
	int line_status;	/**< line number to put status on */
	int line_user_input;	/**< line to put user input on */
	int line_min_print;	/**< num lines to print on */
	int cursor_pos;		/**< column where the input cursor is */
	UINT8 input_buffer[40];	/**< buffer for user input */
	UINT8 echo_buffer[40];	/**< buffer for echo.line */
	int keypress;
#define INPUT_NORMAL	0x01
#define INPUT_GETSTRING	0x02
#define INPUT_MENU	0x03
#define INPUT_NONE	0x04
	int input_mode;		/**< keyboard input mode */
	int input_enabled;	/**< keyboard input enabled */
	int lognum;		/**< current logic number */

	/* internal flags */
	int player_control;	/**< player is in control */
	int quit_prog_now;	/**< quit now */
	int status_line;	/**< status line on/off */
	int clock_enabled;	/**< clock is on/off */
	int exit_all_logics;	/**< break cycle after new.room */
	int picture_shown;	/**< show.pic has been issued */
	int has_prompt;		/**< input prompt has been printed */
#define ID_AGDS		0x00000001
#define ID_AMIGA	0x00000002
	int game_flags;		/**< Sarien options flags */

	UINT8 pri_table[_HEIGHT];	/**< priority table */

	/* windows */
	UINT32 msg_box_ticks;	/**< timed message box tick counter */
	struct agi_block block;
	struct agi_block window;
	int has_window;

	/* graphics & text*/
	int gfx_mode;
	char cursor_char;
	unsigned int color_fg;
	unsigned int color_bg;
	UINT8 *sbuf;			/**< 160x168 AGI screen buffer */
#ifdef USE_HIRES
	UINT8 *hires;			/**< 320x168 hi-res buffer */
#endif

	/* player command line */
	struct agi_word ego_words[MAX_WORDS];
	int num_ego_words;

	unsigned int num_objects;

	struct agi_event ev_keyp[MAX_DIRS];	/**< keyboard keypress events */
	char strings[MAX_STRINGS + 1][MAX_STRINGLEN];	/**< strings */

	/* directory entries for resources */
	struct agi_dir dir_logic[MAX_DIRS];
	struct agi_dir dir_pic[MAX_DIRS];
	struct agi_dir dir_view[MAX_DIRS];
	struct agi_dir dir_sound[MAX_DIRS];

	/* resources */
	struct agi_picture pictures[MAX_DIRS];	/**< AGI picture resources */
	struct agi_logic logics[MAX_DIRS];	/**< AGI logic resources */
	struct agi_view views[MAX_DIRS];	/**< AGI view resources */
	struct agi_sound sounds[MAX_DIRS];	/**< AGI sound resources */

	/* view table */
	struct vt_entry view_table[MAX_VIEWTABLE];

	SINT32 ver;				/**< detected game version */

	int simple_save;			/**< select simple savegames */
};

/**
 *
 */
struct agi_loader {
	int version;
	int int_version;
	int (*init)(void);
	int (*deinit)(void);
	int (*detect_game)(char *);
	int (*load_resource)(int, int);
	int (*unload_resource)(int, int);
	int (*load_objects)(char *);
	int (*load_words)(char *);
};


extern struct agi_game game;

int	agi_init		(void);
int	agi_deinit		(void);
int	agi_version		(void);
int	agi_get_release		(void);
void	agi_set_release		(int);
int	agi_detect_game		(char *);
int	agi_load_resource	(int, int);
int	agi_unload_resource	(int, int);
void	agi_unload_resources	(void);

/* words */
int	show_words	(void);
int	load_words	(char *);
void	unload_words	(void);
int	find_word (char *word, int *flen);
void	dictionary_words(char *);

/* objects */
int	show_objects	(void);
int	load_objects	(char *fname);
int	alloc_objects	(int);
void	unload_objects	(void);
char*	object_name	(unsigned int);
int	object_get_location (unsigned int);
void	object_set_location (unsigned int, int);

void	new_input_mode (int);
void	old_input_mode (void);

int     run_logic               (int);

#ifdef __cplusplus
};
#endif

#endif /* __AGI_H */

