
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

/* AGI Token Management */
/* Author: Jewel of Mars */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "agic.h"
#include "agierr.h"

struct tagToken;
typedef struct tagToken {
    WORD w;
    char *sz;
    struct tagToken *next;
} TOKEN;

TOKEN *TokenHead;
void AddToken(char *sz, WORD w);

void AddToken(char *sz, WORD w)
{
    TOKEN *token;
    token = malloc(sizeof(TOKEN));
    if (token == NULL)
	FatalError("could not allocate token memory");

    token->sz = sz;
    token->w = w;
    token->next = TokenHead;
    TokenHead = token;

    return;
}

/*
   This is the "ExtractWordsTok" routine of Martin Tillenius, slightly modified
   for use in this particular application
 */

void LoadTokens(char *szFilename)
{
    int i;
    int same;
    char letter;
    char w_name[50];
    int w_nr;
    int index[26];
    FILE *in;

    in = fopen(szFilename, "rb");

    if (in == NULL)
	FatalError("Could not open token file");

    for (i = 0; i < 26; index[i++] = (fgetc(in) << 8) + fgetc(in));

    for (i = 0; i < 26; i++) {
	/* i+65 is the current letter we are working on */
	if (index[i] != 0) {
	    fseek(in, index[i], SEEK_SET);
	    same = fgetc(in);
	    do {
		do {
		    letter = fgetc(in);
		    w_name[same++] = (letter ^ 0x7f) & 0x7f;
		} while (!feof(in) && ((letter & 0x80) == 0));

		w_name[same] = 0;
		w_nr = (fgetc(in) << 8) + fgetc(in);
		/* w_nr is the ID, w_name holds the name */
		AddToken(strdup(w_name), (WORD) w_nr);
		same = fgetc(in);
	    } while (!feof(in) && (same != 0));
	}
    }

    fclose(in);

    return;
}

void FreeTokens(void)
{
    TOKEN *token, *next;

    token = TokenHead;
    while (token != NULL) {
	next = token->next;
	free(token->sz);
	free(token);
	token = next;
    }

    return;
}

WORD GetTokenIndex(char *sz)
{
    TOKEN *token;

    if (STRCMP(sz, "anyword") == 0)
	return 1;

    for (token = TokenHead; token != NULL; token = token->next) {
	if (STRCMP(token->sz, sz) == 0)
	    return token->w;
    }

    CompilerFailure(CERR_BAD_TOKEN, sz);
    return 0;
}
