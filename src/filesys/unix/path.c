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

	return x ? 0 : -1;
}

char* get_config_file (void)
{
	struct stat st;

#ifdef HAVE_SNPRINTF
	snprintf (ini_path, MAX_PATH - 1, "%s/.sarienrc", getenv ("HOME"));
#else
	sprintf (ini_path, "%s/.sarienrc", getenv ("HOME"));
#endif

	if (stat (ini_path, &st) < 0)
		strcpy (ini_path, "/etc/sarien.conf");

	return ini_path;
}

char *get_current_directory ()
{
	return (".");
}

