/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999 Dark Fiber, (C) 1999,2001 Claudio Matsuoka
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

	_D (("(\"%s\")", fname));
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

	_D (("(\"%s\")", fname));
	rc = glob (fname, GLOB_ERR | GLOB_NOSORT, NULL, &pglob);
	if (!rc) {
		r = strdup (pglob.gl_pathv[0]);
		_D ((": found name \"%s\"", r));
	}

	globfree (&pglob);

	if ((s = strrchr (r, '/')))
		r = s + 1;

	return r;
}


void fixpath (int flag, char *fname)
{
	_D (("(%d, \"%s\")", flag, fname));

	if (gdir != NULL)
		strcpy((char*)path, (char*)gdir);

	if(*path && path[strlen((char*)path)-1]!='/')
		strcat((char*)path, "/");
		    
	if(flag==1)
		strcat((char*)path, (char*)gname);

	strcat((char*)path, (char*)fname);
	if(path[strlen((char*)path)-1]=='.')
		path[strlen((char*)path)-1]=0;
}


UINT8 *get_current_directory(void)
{
    return(".");
}

