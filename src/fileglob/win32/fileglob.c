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
#ifdef NATIVE_WIN32
#include <io.h>
#else
#include <dir.h>
#endif

#include "sarien.h"
#include "agi.h"


int __file_exists (char *fname)
{
	int rc;
	struct _finddata_t fdata;

	fdata.attrib = _A_NORMAL | _A_ARCH | _A_RDONLY;
	rc = (0 >= _findfirst (fname, &fdata));

	return rc;
}


char* __file_name (char *fname)
{
	int rc, f;
	struct _finddata_t fdata;

	_D (("(\"%s\")", fname));
	fdata.name[0] = 0;
	fdata.attrib = _A_NORMAL;
	f = rc = _findfirst((char*)fname, &fdata);
	while (rc == 0) {
		rc = _findnext(f, &fdata);
		if(rc==0) {
			strlwr (fdata.name);
			if (strstr (fdata.name, "dir.") != NULL)
			    rc = 1;
		}
	}

	return strdup (fdata.name);
}


void fixpath (int flag, char *fname)
{
	/* _D (("(%d, %s)", flag, fname)); */

	strcpy (path, game.dir);

	if(*path && (path[strlen (path)-1]!='\\' && path[strlen(path)-1] != '/'))
	{
		if(path[strlen(path)-1]==':')
			strcat(path, ".\\");
		else
			strcat(path, "\\");
	}
		    
	if(flag==1)
		strcat (path, game.name);

	strcat (path, fname);
}


char *get_current_directory ()
{
    return ".";
}

