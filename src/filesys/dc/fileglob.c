/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  Dreamcast files Copyright (C) 2002 Brian Peek/Ganksoft Entertainment
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

#include "sarien.h"
#include "agi.h"

char cfg_path[255] = "/rd/sarien.cfg";

static char *match (char *p, int f)
{
	char *slash, *dir, *pattern, s[MAX_PATH];
//	DIR *d;
//	struct dirent *e;
	int i, j;
	static char path[MAX_PATH];
	uint32 fd;
	dirent_t *de;

	*path = 0;

	strcpy (s, p);
	slash = strrchr (s, '/');
	if (slash)
	{
		*slash = 0;
		pattern = slash + 1;
		dir = s; 
	}
	else
	{
		pattern = s;
		dir = ".";
	}
	
	_D ("dir=%s, pattern=%s", dir, pattern);

	/* empty pattern given */
	if (!*pattern)
		return 0;

	//if ((d = opendir (dir)) == NULL)
	if((fd = fs_open(dir, O_RDONLY | O_DIR)) == NULL)
	{
		_D("%s not found\n", dir);
		return NULL;
	}

//	while ((e = readdir (d)) != NULL) {
	while((de = fs_readdir(fd)) != NULL)
	{
		_D("name=%s", de->name);
		/* check backwards */

		if(de->name[strlen(de->name)-1] == '.')
			de->name[strlen(de->name)-1] = '\0';

		i = strlen (de->name) - 1;
		j = strlen (pattern) - 1;

		for (; i >= 0 && j >= 0; i--, j--)
		{
			if (pattern[j] == '*')
			{
				i = j = -1;
				break;
			}
			if (tolower (de->name[i]) != tolower (pattern[j]))
				break;
		}

		/* exact match */
		if (i < 0 && j <= 0) {
			if (f) {
				strcpy (path, dir);
				strcat (path, "/");
				strcat (path, de->name);
			} else {
				strcpy (path, de->name);
			}
			fs_close(fd);
			return path;
		}
	}
	fs_close(fd);
	return NULL;
}

int file_isthere(char *fname)
{
	_D ("(fname=%s)", fname);
	return match (fname, 0) != NULL;
}

int get_app_dir (char *app_dir, unsigned int size)
{
	strcpy(app_dir, "/vmu");
	return 0;
}

char* get_config_file(void)
{
	return cfg_path;
}

char* file_name (char *fname)
{
	_D("(fname=%s)", fname);
	return match (fname, 0);
}


char *fixpath (int flag, char *fname)
{
	static char path[MAX_PATH];
	char *s;

	_D("(flag = %d, fname = \"%s\")", flag, fname);
	_D("game.dir = %s, game.name = %s", game.dir, game.name);
	
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


char *get_current_directory ()
{
	return ".";
}

