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
#include <ctype.h>
#include <sys/types.h>
#include <dirent.h>

#include "sarien.h"
#include "agi.h"


/* poor man's glob()
 *
 * For our needs, using glob() is overkill. Not all platforms have
 * glob, and glob performs case-sensitive matches we don't want.
 */
static char *match (char *p, int f)
{
	char *slash, *dir, *pattern, s[MAX_PATH];
	DIR *d;
	struct dirent *e;
	int i, j;
	static char path[MAX_PATH];

	*path = 0;

	strcpy (s, p);
#ifdef WIN32			/* Use backslash in Cygwin (bug #540848) */
	slash = strrchr (s, '\\');
#else
	slash = strrchr (s, '/');
#endif
	if (slash) {
		*slash = 0;
		pattern = slash + 1;
		dir = s; 
	} else {
		pattern = s;
		dir = ".";
	}
	
	_D ("dir=%s, pattern=%s", dir, pattern);

	/* empty pattern given */
	if (!*pattern)
		return 0;

	if ((d = opendir (dir)) == NULL)
		return NULL;

	while ((e = readdir (d)) != NULL) {
		/* check backwards */
		i = strlen (e->d_name) - 1;
		j = strlen (pattern) - 1;

		for (; i >= 0 && j >= 0; i--, j--) {
			if (pattern[j] == '*') {
				i = j = -1;
				break;
			}
			if (tolower (e->d_name[i]) != tolower (pattern[j]))
				break;
		}

		/* exact match */
		if (i < 0 && j <= 0) {
			if (f) {
				strcpy (path, dir);
#ifdef WIN32
				strcat (path, "\\");
#else
				strcat (path, "/");
#endif
				strcat (path, e->d_name);
			} else {
				strcpy (path, e->d_name);
			}
			closedir (d);
			return path;
		}
	}

	closedir (d);

	return NULL;
}


int file_isthere (char *fname)
{
	_D ("(fname=%s)", fname);
	return match (fname, 0) != NULL;
}


char* file_name (char *fname)
{
	_D ("(fname=%s)", fname);
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
	_D ("path = %s", path);

	if ((s = match (path, 1)) != NULL) {
		strcpy (path, s);
	} else {
		/* For amiga games */
		strcat (path, "s");
		if ((s = match (path, 1)) != NULL) {
			strcpy (path, s);
		} else {
			path[0] = 0;
		}
	}
	
	_D ("fixed path = %s", path);

	return path;
}

