/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  Dreamcast files Copyright (C) 2002 Brian Peek/Ganksoft Entertainment
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

/*
 * This file contains port-specific definitions. Port makefiles may
 * set a different include path to override definitions in sarien.h
 */

#define DC_BASE_PATH	"/cd"
#define DC_GFX_PATH		"/cd/gfx"
#define VMU_PATH		"/vmu/%s/SDC-%s-%d"
#define UNKNOWN_GAME	"Unknown"
#undef USE_COMMAND_LINE
char g_gamename[255];
static char g_vmu_port[2];

#include <stdlib.h>

typedef unsigned char	UINT8;
typedef signed char		SINT8;
typedef unsigned short	UINT16;
typedef signed short		SINT16;
typedef unsigned int	UINT32;
typedef signed int		SINT32;
