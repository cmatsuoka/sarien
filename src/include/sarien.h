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

/* Features */
#define USE_CONSOLE
#define USE_PCM_SOUND
#define USE_IIGS_SOUND
#define USE_HIRES
#define USE_COMMAND_LINE
#define AGDS_SUPPORT
#define OPT_LIST_OBJECTS
#define OPT_PICTURE_VIEWER
#define OPT_LIST_DICT

#ifdef PALMOS
#  include <PalmOS.h>
#  undef USE_CONSOLE
#  undef USE_PCM_SOUND
#  undef USE_HIRES
#  undef USE_COMMAND_LINE
#  undef AGDS_SUPPORT
#  undef OPT_LIST_OBJECTS
#  undef OPT_PICTURE_VIEWER
#  undef OPT_LIST_DICT
#else
#  include <stdlib.h>
#endif

#ifdef FAKE_PALMOS
#  undef USE_CONSOLE
#  undef USE_PCM_SOUND
#  undef USE_HIRES
#  undef USE_COMMAND_LINE
#  undef AGDS_SUPPORT
#  undef OPT_LIST_OBJECTS
#  undef OPT_PICTURE_VIEWER
#  undef OPT_LIST_DICT
#endif

/*
 * From the Turbo C FAQ:
 *
 * Q. I have a working program that dynamically allocates memory
 *    using malloc() or calloc() in small data models (tiny, small,
 *    and medium). When I compile this program in large data models
 *    (compact, large, and huge), my program hangs.
 * A. Make sure that you have #include <alloc.h> in your program.
 */
#ifdef __TURBOC__
#  include <alloc.h>
#  undef USE_CONSOLE
#  undef USE_PCM_SOUND
#  undef USE_HIRES
#  undef AGDS_SUPPORT
#  undef OPT_LIST_OBJECTS
#  undef OPT_PICTURE_VIEWER
#  undef OPT_LIST_DICT
#endif

/*
 * From hall_j@sat.mot.com (Joseph Hall)
 * Date: Mon, 6 Jun 1994 18:48:45 GMT
 *
 * (...)
 * MPW's malloc(), on the other hand, was busted around 3.0 or 3.1.
 * Do some malloc-ing and some free-ing and eventually it would crash.
 * I don't know whether that was ever fixed, though I reported it.
 * I suggest that you use NewPtr() on the Mac to obtain memory, then
 * manage it with a memory allocator of your own.
 */
#ifdef __MPW__
#  include <Memory.h>
#  define malloc(x)	((void *)NewPtr (x))
#  define calloc(x,s)	((void *)NewPtrClear ((x) * (s)))
#  define free(x)	DisposePtr ((Ptr)(x))
#  undef USE_HIRES
#  undef USE_COMMAND_LINE
#endif


#ifndef USE_PCM_SOUND
#  undef USE_IIGS_SOUND
#endif

#include "console.h"

#ifdef DMALLOC
#  include <dmalloc.h>
#endif

#if defined (NATIVE_WIN32)
#  define INLINE __inline
#elif !defined (INLINE)
#  define INLINE
#endif

/* Environment variable containing the path name for the users's
 * private files ($HOME in Unix, %USERPROFILE% in Win32)
 * DATADIR conflicts with ObjIdl.h in win32 SDK, renamed to DATA_DIR 
 */
#if defined (WIN32) || defined (__MSDOS__)
#  define HOMEDIR "USERPROFILE"
#  define DATA_DIR "Sarien"
#else
#  define HOMEDIR "HOME"
#  define DATA_DIR ".sarien"
#endif

#ifdef PALMOS
   typedef UInt8	UINT8;
   typedef UInt16	UINT16;
   typedef UInt32	UINT32;
   typedef Int8	SINT8;
   typedef Int16	SINT16;
   typedef Int32	SINT32;
#  define malloc(x) MemPtrNew(x)
#  define free(x) MemPtrFree(x)
#else
   typedef unsigned char	UINT8;
   typedef signed char		SINT8;
   typedef unsigned short	UINT16;
   typedef signed short		SINT16;
#  ifdef __MSDOS__
     typedef unsigned long	UINT32;
     typedef signed long	SINT32;
#  else
     typedef unsigned int	UINT32;
     typedef signed int		SINT32;
#  endif
#endif

#ifndef FALSE
#  define FALSE		0
#  define TRUE		(!FALSE)
#endif

#define	TITLE		"Sarien"

#ifdef NATIVE_WIN32
#  define VERSION __TIMESTAMP__
#endif


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
#define MAX_WORDS1	24
#define	MAX_WORDS2	40
#ifndef MAX_PATH
#define MAX_PATH	260
#endif

#define	_EMPTY		0xfffff
#define	EGO_OWNED	0xff

#define	CRYPT_KEY_SIERRA	"Avis Durgan"
#define CRYPT_KEY_AGDS		"Alex Simkin"

#ifdef FANCY_BOX
#define	MSG_BOX_COLOUR	0x07		/* Grey */
#define MENU_BG		0x07		/* Grey */
#else
#define	MSG_BOX_COLOUR	0x0f		/* White */
#define MENU_BG		0x0f		/* White */
#endif
#define MSG_BOX_TEXT	0x00		/* Black */
#define MSG_BOX_LINE	0x04		/* Red */
#define MENU_FG		0x00		/* Black */
#define MENU_LINE	0x00		/* Black */
#define STATUS_FG	0x00		/* Black */
#define	STATUS_BG	0x0f		/* White */

#define DISABLE_COPYPROTECTION		/* only works on some games */


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
#  ifdef _D
#    undef _D
#  endif
void _D(char *, ...);
#endif /* _TRACE */

extern	UINT8	*exec_name;

int	parse_cli	(int, char **);
int	run_game	(void);


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

int main_cycle (void);
int view_pictures (void);

struct mouse {
	int button;
	unsigned int x;
	unsigned int y;
};


extern struct mouse mouse;



extern	volatile UINT32	clock_ticks;
extern	volatile UINT32 clock_count;
extern	volatile UINT32	msg_box_secs2;

int	init_machine	(int, char **);
int	deinit_machine	(void);
int	file_isthere	(char *fname);	/* Allegro has file_exists() */
char*	file_name	(char *fname);

char*	fixpath		(int flag, char *fname);


/* from motion.c */
extern int get_direction (int x, int y, int x0, int y0, int s);
/* from text.c */
extern int message_box (char *s);

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


int get_app_dir (char *app_dir, unsigned int size);
char* get_config_file(void);

void	list_games	(void);
int	v2id_game	(void);
int	v3id_game	(void);

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

/**
 * Command-line options.
 */
struct sarien_options {
	int forceload;		/**< force loading of all resources */
	int cache;		/**< cache loaded resources */
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
#ifdef MITSHM
	int mitshm;		/**< use shared memory extension */
#endif
#ifdef XF86DGA
	int dga;		/**< use XFree86 DGA extension */
#endif
#ifdef USE_HIRES
	int hires;		/**< use hi-res pictures */
#endif
	int soundemu;		/**< sound emulation mode */
	int gfxhacks;		/**< enable graphics driver optimizations */
	int agimouse;		/**< AGI Mouse 1.0 emulation */
};

extern struct sarien_options opt;

#ifdef USE_CONSOLE
extern struct sarien_debug debug;
#endif

#ifdef __cplusplus
};
#endif

#endif /* __SARIEN_H */

