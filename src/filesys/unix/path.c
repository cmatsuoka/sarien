/* Sarien - A Sierra AGI resource interpreter engine
 * Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *
 * $Id$
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; see docs/COPYING for further details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
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
report (":%s\n", app_dir);

	return x ? 0 : -1;
}

char* get_config_file (void)
{
	struct stat st;
	static char rc_path[MAX_PATH];

#ifdef HAVE_SNPRINTF
	snprintf (rc_path, MAX_PATH - 1, "%s/.sarienrc", getenv ("HOME"));
#else
	sprintf (rc_path, "%s/.sarienrc", getenv ("HOME"));
#endif

	if (stat (rc_path, &st) < 0)
		strcpy (rc_path, "/etc/sarien.conf");

	return rc_path;
}

char *get_current_directory ()
{
	return (".");
}

