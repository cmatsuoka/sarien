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

/* Compiler Error Control */
/* Author: Jewel of Mars */

/* Change log:
 *
 * JAF 12/26/97 - Added more error messages
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "agic.h"
#include "agierr.h"

typedef unsigned int ERROR_TYPE;
#define CERR_FATAL_ERROR		0
#define CERR_ERROR				1
#define CERR_WARNING			2

#define MAX_ERROR_LENGTH		128

typedef struct tagErrorInfo ERROR_INFO;
struct tagErrorInfo {
    ERROR_ID id;
    ERROR_TYPE type;
    char text[MAX_ERROR_LENGTH];
};

ERROR_INFO errors[NUM_ERRORS] =
{
    {CERR_UNKNOWN_CHAR, CERR_WARNING, "Unknown character "},
    {CERR_SYNTAX_ERROR, CERR_ERROR, "Syntax error "},
    {CERR_BAD_LABEL, CERR_ERROR, "Could not resolve label "},
    {CERR_NO_STRING_SPACE, CERR_ERROR, "Too many strings"},
    {CERR_STRING_USED, CERR_ERROR, "String already used"},
    {CERR_BAD_TOKEN, CERR_ERROR, "Could not resolve token "},
    {CERR_BAD_OBJECT, CERR_ERROR, "Could not resolve object "},
    {CERR_BAD_FUNCTION, CERR_ERROR, "Could not resolve function "},
	{CERR_UNENDED_STRING, CERR_ERROR, "Unterminated string constant"},
	{CERR_OUT_OF_MEMORY, CERR_FATAL_ERROR, "Out of memory " },
	{CERR_BAD_STRING_CODE, CERR_ERROR, "Bad string control code " },
	{CERR_UNENDED_SYMBOL, CERR_ERROR, "Unterminated symbol" },
	{CERR_BAD_ARG_TYPE, CERR_WARNING, "Bad type: " },
	{CERR_BAD_ARG_NUM, CERR_WARNING, "Bad number of arguments: " },
	{CERR_WRONG_EQUALS, CERR_WARNING, "Conditionals should use ==, not =" }
};

static int nErrors = 0;
static int nWarnings = 0;

void CompilerFailure(ERROR_ID id, char *sz)
{
    switch (errors[id].type) {
    case CERR_WARNING:
	printf("%s(%i): Warning C%03i: %s", GetCurrentFile(), GetCurrentLine(), id, errors[id].text);

	if (sz != NULL)
	    printf(sz);

	printf("\n");

	nWarnings++;
	break;

    case CERR_ERROR:
	printf("%s(%i): Error C%03i: %s", GetCurrentFile(), GetCurrentLine(), id, errors[id].text);
	if (sz != NULL)
	    printf(sz);

	printf("\n");

	nErrors++;
	break;

    case CERR_FATAL_ERROR:
	printf("%s(%i): Fatal Error C%03i: %s", GetCurrentFile(), GetCurrentLine(), id, errors[id].text);
	if (sz != NULL)
	    printf(sz);

	printf("\nAborting...\n");
	exit(0);
	break;
    }

    return;
}

void PrintErrorStats(void)
{
    if (nErrors == 0)
	printf("No errors.  ");
    else if (nErrors == 1)
	printf("1 error.  ");
    else
	printf("%i errors.  ", nErrors);

    if (nWarnings == 0)
	printf("No warnings.\n");
    else if (nWarnings == 1)
	printf("1 warning.\n");
    else
	printf("%i warnings.\n", nWarnings);
}

BOOL CompiledOkay(void)
{
    if (nErrors == 0)
	return TRUE;

    return FALSE;
}

void FatalError(char *sz)
{
    printf("Internal Fatal Error: %s\n", sz);
    exit(0);
}

void Error(int n, char *sz)
{
    printf("Error N%03i: %s\n", n, sz);
}
