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

/* AGI String Management */
/* Author: Jewel of Mars */

/* ARRAY EDITION */
/* Notice to all those who might be reading this:  Right now, the
   code 'band-aids' a major bug, namely, each string index is +1'd
   before it is handed over to the compiler itself.  I do not, as of yet,
   know why this is necessary, but it is apparently what was happening
   with the other 'real' AGI compiler. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "agic.h"
#include "agierr.h"

#define MAX_STRINGS		256
char *strings[MAX_STRINGS];

void DumpEncryptedString(char *sz, FILE * file);

BYTE GetStringID(char *sz)
{
    int i;

    /* If the string already exists, use the original one */
    for (i = 0; i < MAX_STRINGS; i++) {
	if (strings[i] != NULL) {
	    if (STRCMP(strings[i], sz) == 0) {
		free(sz);
		return i + 1;
	    }
	}
    }

    /* Add the string to the list if it doesn't exist */
    for (i = 0; i < MAX_STRINGS; i++) {
	if (strings[i] == NULL) {
	    AddString((BYTE) i, sz);
	    return i + 1;
	}
    }

    CompilerFailure(CERR_NO_STRING_SPACE, NULL);

    return 0;
}

void AddString(BYTE bID, char *sz)
{
    if (strings[bID] != NULL)
	CompilerFailure(CERR_STRING_USED, NULL);

    strings[bID] = sz;

    return;
}

void DumpEncryptedString(char *sz, FILE * file)
{
    static int codedex = 0;
    int len;
    int i;
    char szCode[12] = "Avis Durgan";

    len = strlen(sz);
    for (i = 0; i <= len; i++) {
	fputc(sz[i] ^ szCode[codedex], file);

	codedex++;
	codedex %= 11;
    }

    return;
}

void DumpStrings(FILE * file)
{
    int i;
    long len;
    int TotalStrings;
    BYTE b;

    TotalStrings = 0;
    for (i = 0; i < MAX_STRINGS; i++)
	if (strings[i] != NULL)
	    TotalStrings = i + 1;

    fputc(TotalStrings & 0xFF, file);

    len = (TotalStrings * 2) + 2;
    for (i = 0; i < TotalStrings; i++)
	if (strings[i] != NULL)
	    len += strlen(strings[i]) + 1;

    fputc(len & 0xFF, file);
    fputc((len >> 8) & 0xFF, file);

    len = (TotalStrings * 2) + 2;
    for (i = 0; i < TotalStrings; i++) {
	if (strings[i] != NULL) {
	    fputc(len & 0xFF, file);
	    fputc((len >> 8) & 0xFF, file);
	    len += strlen(strings[i]) + 1;
	} else {
	    fputc(0, file);
	    fputc(0, file);
	}
    }

    for (i = 0; i < TotalStrings; i++)
	if (strings[i] != NULL)
	    DumpEncryptedString(strings[i], file);

    return;
}

void InitStrings(void)
{
    int i;

    for (i = 0; i < MAX_STRINGS; i++)
	strings[i] = NULL;

    return;
}


void FreeStrings(void)
{
    int i;

    for (i = 0; i < MAX_STRINGS; i++)
	free(strings[i]);

    return;
}
