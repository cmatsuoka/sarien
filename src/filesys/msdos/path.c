/* Sarien - A Sierra AGI resource interpreter engine
 * Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *
 * $Id$
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; see docs/COPYING for further details.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sarien.h"
#include "agi.h"


int get_app_dir (char *app_dir, unsigned int size)
{
	char *x;

	x = getenv (HOMEDIR);
	_D ("HOMEDIR = %s", x);
	if (x) {
		strncpy (app_dir, x, size);
	} else {
		x = getenv ("SARIEN");
		_D ("SARIEN = %s", x);
		if (x)
			strncpy (app_dir, x, size);
	}

	_D ("app_dir = %s", app_dir);
	
	return x ? 0 : -1;
}

char* get_config_file(void)
{
	static char ini_path[MAX_PATH];
	char *q;

	if (getenv ("SARIEN") != NULL) {
		sprintf(ini_path, "%s/%s", getenv("SARIEN"), "sarien.ini");
	} else {
		strcpy (ini_path, exec_name);
		q = strchr(ini_path, 0x0);
		q--;

		while((*q!='\\' && *q!='/') && q>ini_path)
			q--;

		if(q!=ini_path)
			*q=0x0;

		strcat(ini_path, "/sarien.ini");
	}

	return (char*)ini_path;
}

char *get_current_directory ()
{
	return ".";
}

