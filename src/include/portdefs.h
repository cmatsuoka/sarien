/* $Id$ */

/*
 * This file contains port-specific definitions. Port makefiles may
 * set a different include path to override definitions in sarien.h
 */

#ifdef DREAMCAST
#  define DC_BASE_PATH		"/cd"
#  define DC_GFX_PATH		"/cd/gfx"
#  define VMU_PATH		"/vmu/%s/SDC-%s-%d"
#  define UNKNOWN_GAME		"Unknown"
#  undef USE_COMMAND_LINE
char g_gamename[255];
static char g_vmu_port[2];
#endif

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
#  include <assert.h>
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
#  undef FANCY_BOX
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
#  undef USE_PCM_SOUND
#  undef USE_MOUSE
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
#  define realloc(x,s)	(x) /* FIXME */
#  define free(x)	DisposePtr ((Ptr)(x))
#  undef USE_COMMAND_LINE
#  undef OPT_LIST_OBJECTS
#  undef OPT_PICTURE_VIEWER
#  undef OPT_LIST_DICT
#endif

#if defined (NATIVE_WIN32)
#  define INLINE __inline
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

#ifdef NATIVE_WIN32
#  define VERSION __TIMESTAMP__
#endif

#ifdef NATIVE_MACOSX
#  define VERSION "MacOS X native experimental version"
#endif


