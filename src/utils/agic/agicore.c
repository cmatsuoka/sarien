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

/* Core Management - maintains an expandable array for code generation */
/* Author: Jewel of Mars */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "agic.h"

#define SIZE_CORE_START		4096
#define SIZE_CORE_GROW		4096

BYTE *bCore = NULL;
int CoreSize = 0;
PTR PC = 0;

int InitCore(void)
{
    if (bCore != NULL)
	return FAILURE;

    bCore = malloc(SIZE_CORE_START);

    if (bCore == NULL)
	return FAILURE;

    CoreSize = SIZE_CORE_START;

    return SUCCESS;
}

void DumpCore(FILE * file)
{
    fputc(PC & 255, file);
    fputc((PC >> 8) & 255, file);
    fwrite(bCore, 1, PC, file);
    return;
}

int FreeCore(void)
{
    if (bCore == NULL)
	return FAILURE;

    free(bCore);
    CoreSize = 0;

    return SUCCESS;
}

int GrowCore(void)
{
    if (bCore == NULL)
	return FAILURE;

    bCore = realloc(bCore, CoreSize + SIZE_CORE_GROW);
    if (bCore == NULL) {
	CoreSize = 0;
	return FAILURE;
    }
    CoreSize += SIZE_CORE_GROW;
    return SUCCESS;
}

void SetCoreByte(PTR ptr, BYTE b)
{
    while (ptr >= CoreSize) {
	if (GrowCore() == FAILURE)
	    FatalError("could not grow core");
    }

    bCore[ptr] = b;
    return;
}

void Output(BYTE b)
{
    SetCoreByte(PC, b);
    PC++;
}

void OutputWord(WORD b)
{
    SetCoreWord(PC, b);
    PC += 2;
    return;
}

void SetCoreWord(PTR ptr, WORD b)
{
    SetCoreByte(ptr, (BYTE) (b & 255));
    SetCoreByte((PTR) (ptr + 1), (BYTE) ((b >> 8) & 255));
    return;
}

PTR GetPC(void)
{
    return PC;
}
