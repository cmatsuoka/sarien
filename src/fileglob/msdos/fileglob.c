/*
 *  Sarien AGI :: Copyright (C) 1998 Dark Fiber
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdlib.h>
#include <string.h>
#include <dos.h>

#include "sarien.h"
#include "agi.h"


int __file_exists (char *fname)
{
	struct find_t fdata;
	return _dos_findfirst (fname, _A_NORMAL | _A_ARCH | _A_RDONLY, &fdata);
}


char* __file_name (char *fname)
{
	UINT	rc;
	struct find_t fdata;
	short	l;

	_D (("(\"%s\")", fname));
	fdata.name[0]=0;
	rc = _dos_findfirst((char*)fname, _A_NORMAL, &fdata);
	while (rc == 0) {
		rc = _dos_findnext(&fdata);
		if(rc == 0) {
			l=strlen (fdata.name);
			strlwr (fdata.name);
			if (strstr(fdata.name, (char*)"dir.")!=NULL)
			    rc = 1;
		}
	}

	return strdup (fdata.name);
}



void fixpath (int flag, char *fname)
{
	/* _D (("(%d, \"%s\")", flag, fname)); */
	char *p;

	if (gdir != NULL)
    		strcpy (path, gdir);

	if (*path && (path[strlen(path)-1]!='\\' && path[strlen(path)-1] != '/'))
	{
		if(path[strlen(path)-1]==':')
			strcat(path, "./");
		else
			strcat(path, "/");
	}

	if(flag==1)
		strcat (path, gname);

	strcat (path, fname);

	p = path;

	while(*p) {
		if (*p=='\\')
		    *p='/';
		p++;
	}
}


char *get_current_directory (void)
{
	return ".";
}

