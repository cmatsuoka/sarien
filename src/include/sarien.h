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

#ifdef __cplusplus
extern "C"{
#endif

#ifdef DMALLOC
#  include <stdlib.h>
#  include <dmalloc.h>
#endif

/* Default features -- can be overriden in portdefs.h */
#define USE_CONSOLE
#define USE_PCM_SOUND
#define USE_IIGS_SOUND
#define USE_HIRES
#define USE_COMMAND_LINE
#define USE_MOUSE
#define AGDS_SUPPORT
#define OPT_LIST_OBJECTS
#define OPT_PICTURE_VIEWER
#define OPT_LIST_DICT

#include "console.h"

#define	TITLE		"Sarien"

#define DIR_		"dir."
#define LOGDIR		"logdir"
#define PICDIR		"picdir"
#define VIEWDIR		"viewdir"
#define	SNDDIR		"snddir"
#define OBJECTS		"object"
#define WORDS		"words.tok"

#define	MAX_DIRS	256
#define	MAX_VARS	256
#define	MAX_FLAGS	(256 >> 3)
#define MAX_VIEWTABLE	64
#define MAX_WORDS	20
#define	MAX_STRINGS	24		/* MAX_STRINGS + 1 used for get.num */
#define MAX_STRINGLEN	40
#ifndef MAX_PATH
#define MAX_PATH	260
#endif

#define	_EMPTY		0xfffff
#define	EGO_OWNED	0xff

#define	CRYPT_KEY_SIERRA	"Avis Durgan"
#define CRYPT_KEY_AGDS		"Alex Simkin"

/* You'll need an ANSI terminal to use these :\ */
#ifdef _TRACE
#  include <stdio.h>
#  ifdef __GNUC__
#    define _D_INFO "\x1b[33m"
#    define _D_CRIT "\x1b[31m"
#    define _D_WARN "\x1b[36m"
#    define _D(args...) do { \
        printf("\x1b[33m" __PRETTY_FUNCTION__ " \x1b[37m[" __FILE__ \
        ":%d] " _D_INFO, __LINE__); printf (args); printf ("\x1b[0m\n"); \
        } while (0)
#  else
#    define _D_INFO "I: "
#    define _D_CRIT "C: "
#    define _D_WARN "W: "
#    define _D fflush(stdout); printf("\n%s:%d: ", __FILE__, __LINE__); printf
#  endif /* __GNUC__ */
#else /* _TRACE */
#  define _D_INFO
#  define _D_CRIT
#  define _D_WARN
#  if defined (__GNUC__) && !defined (NATIVE_MACOSX)
#    define _D(args...)
#  else
#    ifdef _D
#      undef _D
#    endif
     void _D(char *, ...);
#  endif /* __GNUC__ && !NATIVE_MACOSX */
#endif /* _TRACE */


/* Include port-specific definitions
 *
 * In most C compiler implementation, quoted form of #include
 * searches in the same folder first. On the contrary, angle-bracket
 * form starts search in compiler predefined order. So, #include
 * <portdefs.h> would be more correct. --Vasyl
 */
#include <portdefs.h>


#ifndef FALSE
#  define FALSE		0
#  define TRUE		(!FALSE)
#endif

#ifndef INLINE
#  define INLINE
#endif

#ifndef USE_PCM_SOUND
#  undef USE_IIGS_SOUND
#endif

#ifdef FANCY_BOX
#  define	MSG_BOX_COLOUR	0x07		/* Grey */
#  define	MENU_BG		0x07		/* Grey */
#else
#  define	MSG_BOX_COLOUR	0x0f		/* White */
#  define	MENU_BG		0x0f		/* White */
#endif

#define MSG_BOX_TEXT	0x00		/* Black */
#define MSG_BOX_LINE	0x04		/* Red */
#define MENU_FG		0x00		/* Black */
#define MENU_LINE	0x00		/* Black */
#define STATUS_FG	0x00		/* Black */
#define	STATUS_BG	0x0f		/* White */

#ifdef _TRACE
#define DISABLE_COPYPROTECTION		/* only works on some games */
#endif


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
void	release_sprites	(void);
int	main_cycle	(void);
int	view_pictures	(void);
int	parse_cli	(int, char **);
int	run_game	(void);
int	init_machine	(int, char **);
int	deinit_machine	(void);
int	file_isthere	(char *fname);	/* Allegro has file_exists() */
char*	file_name	(char *fname);
char*	fixpath		(int flag, char *fname);
int	get_direction	(int x, int y, int x0, int y0, int s);
void	inventory	(void);
void	list_games	(void);
int	v2id_game	(void);
int	v3id_game	(void);
int v4id_game	(UINT32 ver);
void	update_timer	(void);
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
 * Implementations of this function are in src/filesys/.
 */
char*	get_current_directory (void);
char*	get_config_file	(void);
int	get_app_dir	(char *app_dir, unsigned int size);


enum {
	NO_GAMEDIR = 0,
	GAMEDIR
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

/**
 * AGI resources.
 */
enum {
	rLOGIC = 1,
	rSOUND,
	rVIEW,
	rPICTURE
};

enum {
	RES_LOADED = 1,
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
	GFX_MODE = 1,
	TXT_MODE
};


struct game_id_list {
	struct game_id_list *next;
	UINT32 version;
	UINT32 crc;
	char *gName;
	char *switches;
};

#ifdef USE_MOUSE
struct mouse {
	int button;
	unsigned int x;
	unsigned int y;
};
#endif

/**
 * Command-line options.
 */
struct sarien_options {
#define GAMERUN_RUNGAME 0
#define GAMERUN_PICVIEW 1
#define GAMERUN_WORDS	2
#define GAMERUN_OBJECTS	3
#define GAMERUN_GAMES	4
#define GAMERUN_CRC	5
	int gamerun;		/**< game run mode*/
	int emuversion;		/**< AGI version to emulate */
	int scale;		/**< window scale factor */
	int fixratio;		/**< fix aspect ratio */
	int agds;		/**< enable AGDS mode */
	int amiga;		/**< enable Amiga mode */
	int fullscreen;		/**< use full screen mode if available */
	int nosound;		/**< disable sound */
	int egapal;		/**< use PC EGA palette */
#ifdef MITSHM
	int mitshm;		/**< use shared memory extension */
#endif
#ifdef USE_HIRES
	int hires;		/**< use hi-res pictures */
#endif
	int soundemu;		/**< sound emulation mode */
	int gfxhacks;		/**< enable graphics driver optimizations */
#ifdef USE_MOUSE
	int agimouse;		/**< AGI Mouse 1.0 emulation */
#endif
};

extern  struct sarien_options opt;
extern	UINT8	*exec_name;

extern	volatile UINT32	clock_ticks;
extern	volatile UINT32 clock_count;
extern	volatile UINT32	msg_box_secs2;

#ifdef USE_CONSOLE
extern struct sarien_debug debug;
#endif

#ifdef USE_MOUSE
extern struct mouse mouse;
#endif

#ifdef __cplusplus
};
#endif

#endif /* __SARIEN_H */

