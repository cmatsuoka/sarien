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

/* Compiler Error Management */

typedef unsigned int ERROR_ID;
#define CERR_UNKNOWN_CHAR			0
#define CERR_SYNTAX_ERROR			1
#define CERR_BAD_LABEL				2
#define CERR_NO_STRING_SPACE		3
#define CERR_STRING_USED			4
#define CERR_BAD_TOKEN				5
#define CERR_BAD_OBJECT				6
#define CERR_BAD_FUNCTION			7
#define CERR_UNENDED_STRING			8
#define CERR_OUT_OF_MEMORY			9
#define CERR_BAD_STRING_CODE		10
#define CERR_UNENDED_SYMBOL			11
#define CERR_BAD_ARG_TYPE			12
#define CERR_BAD_ARG_NUM			13
#define CERR_WRONG_EQUALS			14

#define NUM_ERRORS					15

void CompilerFailure(ERROR_ID id, char *sz);
void PrintErrorStats(void);
BOOL CompiledOkay(void);

void FatalError(char *sz);
void Error(int n, char *sz);
