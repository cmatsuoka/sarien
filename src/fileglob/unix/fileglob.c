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
#include <glob.h>

#include "sarien.h"
#include "agi.h"


int __file_exists (char *fname)
{
	int rc;
	glob_t pglob;

	_D ("(\"%s\")", fname);
	rc = glob (fname, GLOB_ERR | GLOB_NOSORT, NULL, &pglob);
	if (!rc && pglob.gl_pathc <= 0)
		rc = -1;

	globfree (&pglob);

	return rc;
}


char* __file_name (char *fname)
{
	int rc;
	glob_t pglob;
	char *s, *r = NULL;

	_D ("(\"%s\")", fname);
	rc = glob (fname, GLOB_ERR | GLOB_NOSORT, NULL, &pglob);
	if (!rc) {
		r = strdup (pglob.gl_pathv[0]);
		_D ("found name \"%s\"", r);
	}

	globfree (&pglob);

	if ((s = strrchr (r, '/')))
		r = s + 1;

	return r;
}


char *fixpath (int flag, char *fname)
{
	static char path[MAX_PATH];

	_D ("(flag = %d, fname = \"%s\")", flag, fname);

	strcpy (path, game.dir);
	if(*path && path[strlen((char*)path)-1]!='/')
		strcat((char*)path, "/");
		    
	if (flag)
		strcat (path, game.name);

	strcat (path, fname);
	if (path[strlen (path) - 1] == '.')
		path[strlen (path) - 1] = 0;

	return path;
}


char *get_current_directory ()
{
	return (".");
}

