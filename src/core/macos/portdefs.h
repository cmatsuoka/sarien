/* $Id$ */

/*
 * This file contains MacOS port-specific definitions.
 */


#include <stdlib.h>
#include <assert.h>

/*
 * Features
 */
#undef USE_COMMAND_LINE
#undef OPT_LIST_OBJECTS
#undef OPT_PICTURE_VIEWER
#undef OPT_LIST_DICT

#define HOMEDIR "HOME"
#define DATA_DIR ".sarien"

/*
 * Memory management
 */

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

#define malloc(x)	my_malloc(x)
#define calloc(x,s)	my_calloc(x,s)
#define realloc(x,s)	my_realloc(x,s)
#define free(x)		my_free(x)


/*
 * Data types
 */

typedef unsigned char	UINT8;
typedef signed char	SINT8;
typedef unsigned short	UINT16;
typedef signed short	SINT16;
typedef unsigned int	UINT32;
typedef signed int	SINT32;


