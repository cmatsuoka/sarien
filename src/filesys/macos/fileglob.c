/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999,2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#include <AppleEvents.h>
#include <AERegistry.h>
#include <AEObjects.h>
#include <AEPackObject.h>
#include <Errors.h>
//#include <Processes.h>
//#include <QuickDraw.h>
//#include <QDOffscreen.h>
//#include <Palettes.h>
//#include <ImageCompression.h>
//#include <PictUtils.h>
#include <Files.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <Gestalt.h>
#include <TextUtils.h>
#include "sarien.h"
#include "agi.h"



typedef struct {
	int d_VRefNum;
	long int d_DirID;
	int d_index;
} DIR;

struct dirent {
	char d_name[255];
	int d_namlen;
};


int mkdir (char *s, int n)
{
	return 0;
}


static DIR *opendir(const char *path)
{
	char pathname[1664];
	CInfoPBRec search_info;
	DIR *entry;
	int error;

	search_info.hFileInfo.ioNamePtr = 0;

	if ((path != NULL) || (*path != '\0')) {
		if ((path[0] != '.') || (path[1] != '\0')) {
			search_info.hFileInfo.ioNamePtr =
				c2pstr (strcpy(pathname,path));
		}
	}

	search_info.hFileInfo.ioCompletion = 0;
	search_info.hFileInfo.ioVRefNum = 0;
	search_info.hFileInfo.ioFDirIndex = 0;
	search_info.hFileInfo.ioDirID = 0;
	error = PBGetCatInfoSync (&search_info);

	if (error != noErr) {
		errno = error;
		return (DIR *)NULL;
	}

	entry = (DIR *)malloc (sizeof(DIR));
	if (entry == (DIR *)NULL)
		return (DIR *)NULL;

	entry->d_VRefNum = search_info.hFileInfo.ioVRefNum;
	entry->d_DirID = search_info.hFileInfo.ioDirID;
	entry->d_index = 1;

	return entry;
}


static void closedir (DIR *entry)
{
	assert(entry != (DIR *) NULL);
	LiberateMemory ((void **) &entry);
}


static struct dirent *readdir (DIR *entry)
{
	CInfoPBRec search_info;
	int error;
	static struct dirent dir_entry;
	static unsigned char pathname[1664];

	if (entry == (DIR *)NULL)
		return (struct dirent *)NULL;

	search_info.hFileInfo.ioCompletion = 0;
	search_info.hFileInfo.ioNamePtr = pathname;
	search_info.hFileInfo.ioVRefNum = 0;
	search_info.hFileInfo.ioFDirIndex = entry->d_index;
	search_info.hFileInfo.ioDirID = entry->d_DirID;

	error = PBGetCatInfoSync (&search_info);
	if (error != noErr) {
		errno=error;
		return (struct dirent *)NULL;
	}

	entry->d_index++;
	strcpy (dir_entry.d_name,p2cstr (search_info.hFileInfo.ioNamePtr));
	dir_entry.d_namlen = strlen (dir_entry.d_name);
	return (&dir_entry);
}


static char *match (char *p, int f)
{
	char *slash, *dir, *pattern, s[MAX_PATH];
	DIR *d;
	struct dirent *e;
	int i, j;
	static char path[MAX_PATH];

	*path = 0;

	strcpy (s, p);
	slash = strrchr (s, '/');
	if (slash) {
		*slash = 0;
		pattern = slash + 1;
		dir = s; 
	} else {
		pattern = s;
		dir = ".";
	}
	
	_D ("dir=%s, pattern=%s", dir, pattern);

	/* empty pattern given */
	if (!*pattern)
		return 0;

	if ((d = opendir (dir)) == NULL)
		return NULL;

	while ((e = readdir (d)) != NULL) {
		/* check backwards */
		i = strlen (e->d_name) - 1;
		j = strlen (pattern) - 1;

		for (; i >= 0 && j >= 0; i--, j--) {
			if (pattern[j] == '*') {
				i = j = -1;
				break;
			}
			if (tolower (e->d_name[i]) != tolower(pattern[j]))
				break;
		}
		/* exact match */
		if (i < 0 && j < 0) {
			if (f) {
				strcpy (path, dir);
				strcat (path, "/");
				strcat (path, e->d_name);
			} else {
				strcpy (path, e->d_name);
			}
			return path;
		}
	}

	return NULL;
}


int file_isthere (char *fname)
{
	return match (fname, 0) != NULL;
}


char* file_name (char *fname)
{
	return match (fname, 0);
}


char *fixpath (int flag, char *fname)
{
	static char path[MAX_PATH];
	char *s;

	_D ("(flag = %d, fname = \"%s\")", flag, fname);
	_D ("game.dir = %s", game.dir);

	strcpy (path, game.dir);
	if (*path && path[strlen (path) - 1] != '/')
		strcat (path, "/");
	if (flag)
		strcat (path, game.name);

	strcat (path, fname);

	if (path[strlen (path) - 1] == '.')
		path[strlen (path) - 1] = 0;

	if ((s = match (path, 1)) != NULL)
		strcpy (path, s);
	else
		path[0] = 0;
	
	_D ("fixed path = %s", path);

	return path;
}


char *get_current_directory ()
{
	return (".");
}

