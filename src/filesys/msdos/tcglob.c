/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999,2002 Stuart George and Claudio Matsuoka
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
#include <dir.h>

#include "sarien.h"
#include "agi.h"


 
/*
 * We want a large stack for recursive flood fills and stuff
 */
extern unsigned _stklen = 32000U;


int file_isthere (char *fname)
{
	struct ffblk fdata;
	return !findfirst (fname, &fdata, FA_ARCH | FA_RDONLY);
}


char* file_name (char *fname)
{
	int rc;
	struct ffblk fdata;

	fdata.ff_name[0] = 0;
	rc = findfirst (fname, &fdata, FA_ARCH | FA_RDONLY);

	while (rc == 0) {
		rc = _dos_findnext(&fdata);
		if(rc == 0) {
			strlwr (fdata.ff_name);
			if (strstr (fdata.ff_name, "dir.") != NULL)
				rc = 1;
		}
	}

	return strdup (fdata.ff_name);
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

/* end: tcglob.c */

