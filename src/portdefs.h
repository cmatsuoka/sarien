/* $Id$ */

/*
 * This file contains port-specific definitions. Port makefiles may
 * set a different include path to override definitions in sarien.h
 */


/*
 * Features
 */

#include <stdlib.h>
#include <assert.h>
typedef signed int Err;

/*
 * Memory allocation
 */

/* Environment variable containing the path name for the users's
 * private files ($HOME in Unix, %USERPROFILE% in Win32)
 * DATADIR conflicts with ObjIdl.h in win32 SDK, renamed to DATA_DIR
 */
#ifdef WIN32
#  define HOMEDIR "USERPROFILE"
#  define DATA_DIR "Sarien"
#else
#  define HOMEDIR "HOME"
#  define DATA_DIR ".sarien"
#endif


/*
 * Data types
 */
typedef unsigned char	UINT8;
typedef signed char	SINT8;
typedef unsigned short	UINT16;
typedef signed short	SINT16;
typedef unsigned int	UINT32;
typedef signed int	SINT32;

/*
 * Version and other definitions
 */
#ifdef NATIVE_WIN32
#  define INLINE __inline
#  define VERSION __TIMESTAMP__
#endif

