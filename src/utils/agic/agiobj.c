/*
 *  AGI Logic Compiler
 *  Copyright (C) 1997, Floating Hills Software.
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
 *
 *  Send comments, ideas, and criticism to greekcat@hotmail.com
 *
 */

/* AGI Object Management */
/* Author: Jewel of Mars */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "agic.h"
#include "agierr.h"

#define MAX_OBJECTS	256
char *objects[MAX_OBJECTS];
int TotalObjects;
char *szObjectCore;

int GetFileLength(FILE * file);
int GetFileLength(FILE * file)
{
    int len;

    fseek(file, 0L, SEEK_END);
    len = (int) ftell(file);
    fseek(file, 0L, SEEK_SET);

    return len;
}

void LoadObjects(char *szFilename)
{
    FILE *file;
    int len;
    int i;

    if (NULL == (file = fopen(szFilename, "rb")))
	FatalError("Could not open object file");

    len = GetFileLength(file);
    szObjectCore = malloc(len);
    if (szObjectCore == NULL) {
	fclose(file);
	FatalError("Could not allocate object index");
    }
    if (fread(szObjectCore, 1, len, file) != len) {
	fclose(file);
	FatalError("Could not load object file");
    }
    EncryptBlock(szObjectCore, len);
    fclose(file);

    TotalObjects = (szObjectCore[0] | (szObjectCore[1] << 8)) / 3;

    for (i = 0; i < TotalObjects; i++)
	objects[i] = szObjectCore + 3 + (szObjectCore[3 + 3 * i] & 0xFF) + ((szObjectCore[4 + 3 * i] & 0xFF) << 8);

    return;
}

void FreeObjects(void)
{
    free(szObjectCore);
}

char *GetObjectName(BYTE b)
{
    return objects[b];
}

BYTE GetObjectIndex(char *sz)
{
    int i;

    for (i = 0; i < TotalObjects; i++) {
	if (STRCMP(objects[i], sz) == 0)
	    return (BYTE) i;
    }

    CompilerFailure(CERR_BAD_OBJECT, sz);
    return 0;
}
