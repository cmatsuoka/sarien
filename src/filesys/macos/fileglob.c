/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999,2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#include <stdlib.h>
#include <string.h>

#include "sarien.h"
#include "agi.h"


#ifdef __MPW__

int mkdir (char *s, int n)
{
}

#endif


static char *match (char *s, int n)
{
}


int file_isthere (char *fname)
{
	return match (fname, 0) != NULL;
}


char* file_name (char *fname)
{
	return match (fname, 0);
}


char *fixpath (int flag, char *fname)
{
	static char path[MAX_PATH];
	char *s;

	_D ("(flag = %d, fname = \"%s\")", flag, fname);
	_D ("game.dir = %s", game.dir);

	strcpy (path, game.dir);
	if (*path && path[strlen (path) - 1] != '/')
		strcat (path, "/");
	if (flag)
		strcat (path, game.name);

	strcat (path, fname);

	if (path[strlen (path) - 1] == '.')
		path[strlen (path) - 1] = 0;

	if ((s = match (path, 1)) != NULL)
		strcpy (path, s);
	else
		path[0] = 0;
	
	_D ("fixed path = %s", path);

	return path;
}


char *get_current_directory ()
{
	return (".");
}

