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

/* Goto Label Management */
/* Author: Jewel of Mars */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "agic.h"
#include "agierr.h"

struct tagResolved;
typedef struct tagResolved {
    char *sz;
    PTR ptr;
    struct tagResolved *next, *prev;
} LABEL;

LABEL *ResolvedHead = NULL;
LABEL *UnresolvedHead = NULL;

void AddLabel(LABEL ** list, char *sz, PTR ptr);
void RemoveLabel(LABEL ** list, LABEL * label);
void FreeList(LABEL ** list);

void AddLabel(LABEL ** list, char *sz, PTR ptr)
{
    LABEL *labelNew;

    labelNew = malloc(sizeof(LABEL));
    if (labelNew == NULL)
	FatalError("could not allocate label memory");

    labelNew->sz = sz;
    labelNew->ptr = ptr;

    if (*list != NULL)
	(*list)->prev = labelNew;

    labelNew->prev = NULL;
    labelNew->next = *list;
    *list = labelNew;

    return;
}

void RemoveLabel(LABEL ** list, LABEL * label)
{
    if (label->prev != NULL)
	label->prev->next = label->next;

    if (label->next != NULL)
	label->next->prev = label->prev;

    if ((label->prev == NULL) && (label->next == NULL))
	*list = NULL;

    free(label);

    return;
}

void FreeList(LABEL ** list)
{
    LABEL *labelCurrent;
    LABEL *labelNext;

    labelCurrent = *list;
    while (labelCurrent != NULL) {
	labelNext = labelCurrent->next;
	free(labelCurrent->sz);
	free(labelCurrent);
	labelCurrent = labelNext;
    }

    *list = NULL;

    return;
}

void OutputLabel(char *sz)
{
    LABEL *resolved;

    for (resolved = ResolvedHead; resolved != NULL; resolved = resolved->next) {
	if (STRCMP(resolved->sz, sz) == 0) {
	    OutputWord(resolved->ptr - (GetPC() + 2));
	    free(sz);
	    return;
	}
    }

    AddLabel(&UnresolvedHead, sz, GetPC());
    OutputWord(0xCDAB);
    return;
}

void ResolveLabel(char *sz)
{
    LABEL *labelCurrent;
    LABEL *labelNext;

    labelCurrent = UnresolvedHead;
    while (labelCurrent != NULL) {
	labelNext = labelCurrent->next;

	if (STRCMP(labelCurrent->sz, sz) == 0) {
	    SetCoreWord(labelCurrent->ptr, GetPC() - (labelCurrent->ptr + 2));
	    RemoveLabel(&UnresolvedHead, labelCurrent);
	}
	labelCurrent = labelNext;
    }

    AddLabel(&ResolvedHead, sz, GetPC());
    return;
}

void PrintUnresolvedLabels()
{
    LABEL *unresolved;

    for (unresolved = UnresolvedHead; unresolved != NULL; unresolved = unresolved->next)
	CompilerFailure(CERR_BAD_LABEL, unresolved->sz);

    return;
}

void FreeLabels()
{
    FreeList(&UnresolvedHead);
    FreeList(&ResolvedHead);
}
