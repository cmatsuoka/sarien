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
#include <dos.h>

#include "sarien.h"
#include "agi.h"


int file_isthere (char *fname)
{
	struct find_t fdata;
	return !_dos_findfirst (fname, _A_NORMAL | _A_ARCH | _A_RDONLY, &fdata);
}


char* file_name (char *fname)
{
	int rc;
	struct find_t fdata;
	short l;

	_D ("(\"%s\")", fname);
	fdata.name[0] = 0;
	rc = _dos_findfirst((char*)fname, _A_NORMAL, &fdata);
	while (rc == 0) {
		rc = _dos_findnext(&fdata);
		if(rc == 0) {
			l=strlen (fdata.name);
			strlwr (fdata.name);
			if (strstr (fdata.name, "dir.")!=NULL)
			    rc = 1;
		}
	}

	return strdup (fdata.name);
}



char *fixpath (int flag, char *fname)
{
	static char path[MAX_PATH];
	char *p;

    	strcpy (path, game.dir);

	if (*path && (path[strlen(path)-1]!='\\' && path[strlen(path)-1] != '/'))
	{
		if(path[strlen(path)-1]==':')
			strcat(path, "./");
		else
			strcat(path, "/");
	}

	if (flag==1)
		strcat (path, game.name);

	strcat (path, fname);

	p = path;

	while(*p) {
		if (*p=='\\')
		    *p='/';
		p++;
	}

	return path;
}


char *get_current_directory ()
{
	return ".";
}

