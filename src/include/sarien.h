/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#ifndef __SARIEN_H
#define __SARIEN_H

#ifdef HAVE_GETOPT_LONG
#define ELIDE_CODE
#endif

#if defined (NATIVE_WIN32)
#define INLINE __forceinline
#elif !defined (INLINE)
#define INLINE
#endif

typedef unsigned char	UINT8;
typedef unsigned short	UINT16;
#ifndef WIN32
typedef unsigned long	UINT32;
#endif
typedef signed char	SINT8;
typedef signed short	SINT16;
typedef signed long	SINT32;

#ifndef FALSE
#define FALSE		0
#define TRUE		(!FALSE)
#endif

#define	TITLE		"Sarien"

#ifdef NATIVE_WIN32
#define VERSION "Win32 native experimental version"
#endif

#define DIR		"dir."

#define LOGDIR		"logdir"
#define PICDIR		"picdir"
#define VIEWDIR		"viewdir"
#define	SNDDIR		"snddir"
#define OBJECTS		"object"
#define WORDS		"words.tok"

#define	MAX_DIRS	256
#define	MAX_VARS	256
#define	MAX_FLAGS	256
#define MAX_VIEWTABLE	64
#define MAX_WORDS	20
#define MAX_WORDS1	24
#define	MAX_WORDS2	40

#define	_EMPTY		0xfffff
#define	EGO_OWNED	0xff

#define	CRYPT_KEY_SIERRA	"Avis Durgan"
#define CRYPT_KEY_AGDS		"Alex Simkin"

#define	MSG_BOX_COLOUR	0x0f		/* White */
#define MSG_BOX_TEXT	0x00		/* Black */
#define MSG_BOX_LINE	0x04		/* Red */
#define MENU_BG		0x0f		/* White bg */
#define MENU_FG		0x00		/* Black fg */
#define MENU_LINE	0x00		/* Black line */
#define	STATUS_BG	0x0f		/* White */
#define STATUS_FG	0x00		/* Blue */
#define STATUS_FG_CLEAN	0x00		/* Black */


#define AGDS_SUPPORT			/* enable support for AGDS games */
#define	DISABLE_COPYPROTECTION		/* only works on some games */
#define EGO_VIEW_TABLE	0
#define	HORIZON		36
#define _WIDTH		160
#define _HEIGHT		168

#ifdef _TRACE
#define _D(y) do {							\
	printf (__PRETTY_FUNCTION__ " [%s:%d] ", __FILE__, __LINE__);	\
	printf y;							\
	printf ("\n");							\
} while (0)
#else
#define _D(y)
#endif

#ifdef __cplusplus
extern "C"{
#endif

extern	UINT8	*exec_name;

int	parse_cli	(int, char **);


UINT8	lohi_getbyte	(UINT8 *);
UINT16	lohi_getword	(UINT8 *);
UINT32	lohi_getpword	(UINT8 *);
UINT32	lohi_getdword	(UINT8 *);
UINT8	hilo_getbyte	(UINT8 *);
UINT16	hilo_getword	(UINT8 *);
UINT32	hilo_getpword	(UINT8 *);
UINT32	hilo_getdword	(UINT8 *);

int	getflag		(int);
void	setflag		(int, int);
void	flipflag	(int);
int	getvar		(int);
void	setvar		(int, int);

void	decrypt		(UINT8 *mem, int len);
//void	unload_resources(void);
void	release_sprites	(void);
//void	new_room_resources(void);
void	update_status_line(int);

void main_cycle (int);

#ifdef __cplusplus
};
#endif



extern	volatile UINT32	clock_ticks;
extern	volatile UINT32 clock_count;
extern	volatile UINT32	msg_box_secs2;

int	init_machine	(int, char **);
int	deinit_machine	(void);
int	__file_exists	(char *fname);
char*	__file_name	(char *fname);

void	fixpath		(int flag, char *fname);


extern	void inventory(void);

/*
 * get_current_directory() returns the current working
 * directory name in a platform independent manner. On
 * Unix, DOS, and Win32, it just returns a "." string.
 * On MacOS, there is no concept of a "." directory, so
 * more detailed (and platform dependent) code would be
 * needed there. Hence the need for abstraction.
 *
 * Return values from this function should be considered
 * read-only, static, and non-thread safe. Assume that
 * each call to this function will overwrite the previously
 * returned data.
 *
 * Implementations of this function are in the fileglob directory.
 */
char* get_current_directory (void);

extern	void list_games(void);

#ifdef __cplusplus
extern "C"{
#endif

int v2id_game (void);
int v3id_game (void);

#ifdef __cplusplus
};
#endif


enum {
	NO_GAMEDIR = 0,
	GAMEDIR
};

enum {
	V_cur_room = 0,
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
	V_Volume,
	V_max_input_chars,
	V_sel_item,                     /* 25 */
	V_monitor
};

enum {
	F_ego_water = 0,
	F_ego_invis,
	F_entered_cli,
	F_ego_touched_p2,
	F_said_accepted_input,
	F_new_room_exec,				/* 5 */
	F_restart_game,
	F_script_blocked,
	F_joy_sensitivity,
	F_sound_on,
	F_debugger_on,					/* 10 */
	F_logic_zeron_firsttime,
	F_restore_just_ran,
	F_status_selects_items,
	F_menus_work,					/* 14 */
	F_output_mode
};

enum error {
	err_OK = 0,
	err_DoNothing,
	err_BadCLISwitch,
	err_InvalidAGIFile,
	err_BadFileOpen,
	err_NotEnoughMemory,
	err_BadResource,
	err_UnknownAGIVersion,
	err_RestartGame,
	err_NoLoopsInView,
	err_ViewDataError,
	err_NoGameList,

	err_Unk = 127
};

enum RESOURCES {
	rLOGIC = 1,
	rSOUND,
	rVIEW,
	rPICTURE
};

enum {
	gRUN_GAME = 1,
	gSHOW_WORDS,
	gSHOW_OBJECTS,
	gVIEW_PICTURES,
	gCRC,
	gLIST_GAMES
};

enum {
	RES_LOADED = 1,
	RES_CACHED = 2,
	RES_COMPRESSED = 0x40
};

enum {
	lCOMMAND_MODE = 1,
	lTEST_MODE
};

enum {
	eKEY_PRESS = 1,
	eSCAN_CODE,
	eMENU_PRESS
};

enum {
	CONTROL_PLAYER = 1,
	CONTROL_PROGRAM
};

enum {
	GFX_MODE = 1,
	TXT_MODE
};

enum {
	SAVE = 1,
	NO_SAVE
};

enum {
	LINES = 1,
	NO_LINES,
	BX_SAVE = 0x10
};

enum {
	MOTION_NORMAL = 1,
	MOTION_WANDER,
	MOTION_FOLLOW_EGO,
	MOTION_MOVE_OBJ
};

enum {
	CYCLE_NORMAL = 1,
	CYCLE_END_OF_LOOP,
	CYCLE_REV_LOOP, 
	CYCLE_REV
};

struct game_id_list {
	struct game_id_list *next;
	UINT32 version;
	UINT32 crc;
	char *gName;
	char *switches;
};


struct sarien_options {
	int forceload;		/* force loading of all resources */
	int cache;		/* cache loaded resources */
	int gamerun;		/* status for game */
	int pcjrgfx;		/* PCJr graphics */
	int showscreendraw;	/* show screen drawing */
	int showkeypress;
	int emuversion;
#ifndef NO_DEBUG
	int debug;
#endif
	int scale;		/* window scale factor */
	int agds;		/* enable AGDS mode */
	int amiga;		/* enable Amiga mode */
	int fullscreen;		/* use full screen mode if available */
	int nosound;		/* disable sound */
#ifdef MITSHM
	int mitshm;		/* use shared memory extension */
#endif
#ifdef XF86DGA
	int dga;		/* use XFree86 DGA extension */
#endif
	int soundemu;		/* sound emulation mode */
};

#endif
