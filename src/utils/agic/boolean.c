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

/* Boolean Algebra Toolkit */
/* Author: Jewel of Mars */

#include <stdio.h>
#include <stdlib.h>
#include "agic.h"
#include "boolean.h"

void DumpCommands(BOOLEAN_EXP * exp)
{
    int i;

    if (exp == NULL)
	return;

    DumpCommands(exp->right);

    if (exp->not)
	Output(HC_NOT);

    if (exp->optype == OP_VAR) {
	for (i = 0; i < exp->length; i++)
	    Output(exp->data[i]);
    }
    DumpCommands(exp->left);

    return;
}

void DumpBoolean(BOOLEAN_EXP * exp)
{
    if (exp == NULL)
	return;

    switch (exp->optype) {
    case OP_OR:
	Output(HC_OR);
	DumpCommands(exp->right);
	DumpCommands(exp->left);
	Output(HC_OR);
	break;

    case OP_VAR:
	DumpCommands(exp);
	break;

    case OP_AND:
	DumpBoolean(exp->right);
	DumpBoolean(exp->left);
	break;
    }

    return;
}

void FreeBoolean(BOOLEAN_EXP * exp)
{
    if (exp == NULL)
	return;

    FreeBoolean(exp->right);
    FreeBoolean(exp->left);
    free(exp);

    return;
}

BOOLEAN_EXP *BooleanCommand(void)
{
    BOOLEAN_EXP *boolean;

    boolean = malloc(sizeof(BOOLEAN_EXP));

    boolean->right = NULL;
    boolean->left = NULL;
    boolean->length = 0;
    boolean->optype = OP_VAR;
    boolean->not = FALSE;

    return boolean;
}

BOOLEAN_EXP *BooleanAnd(BOOLEAN_EXP * a, BOOLEAN_EXP * b)
{
    BOOLEAN_EXP *boolean;

    boolean = malloc(sizeof(BOOLEAN_EXP));

    boolean->right = a;
    boolean->left = b;
    boolean->length = 0;
    boolean->optype = OP_AND;
    boolean->not = FALSE;

    return boolean;
}

BOOLEAN_EXP *BooleanOr(BOOLEAN_EXP * a, BOOLEAN_EXP * b)
{
    BOOLEAN_EXP *boolean;

    boolean = malloc(sizeof(BOOLEAN_EXP));

    boolean->right = a;
    boolean->left = b;
    boolean->length = 0;
    boolean->optype = OP_OR;
    boolean->not = FALSE;

    return boolean;
}

BOOLEAN_EXP *BooleanNot(BOOLEAN_EXP * a)
{
    a->not = !a->not;

    return a;
}

void BooleanReduceNots(BOOLEAN_EXP * a)
{
    if (a == NULL)
	return;

    if ((a->not) && (a->optype != OP_VAR)) {
	if (a->optype == OP_OR)
	    a->optype = OP_AND;
	else
	    a->optype = OP_OR;

	a->not = FALSE;

	if (a->left != NULL)
	    a->left->not = !a->left->not;

	if (a->right != NULL)
	    a->right->not = !a->right->not;
    }
    BooleanReduceNots(a->left);
    BooleanReduceNots(a->right);

    return;
}

BOOLEAN_EXP *BooleanClone(BOOLEAN_EXP * a)
{
    BOOLEAN_EXP *clone;

    if (a == NULL)
	return NULL;

    clone = malloc(sizeof(BOOLEAN_EXP));

    memcpy(clone, a, sizeof(BOOLEAN_EXP));
    clone->right = BooleanClone(a->right);
    clone->left = BooleanClone(a->left);

    return clone;
}

void BooleanDeMorganTranspose(BOOLEAN_EXP * or, BOOLEAN_EXP * and, BOOLEAN_EXP * c)
{
    BOOLEAN_EXP *a, *b;
    BOOLEAN_EXP *clone;

    a = and->right;
    b = and->left;
    clone = BooleanClone(c);

    and->left = NULL;
    and->right = NULL;
    FreeBoolean(and);

    or->optype = OP_AND;
    or->left = BooleanOr(a, c);
    or->right = BooleanOr(b, clone);

    return;
}

void BooleanReduceDeMorgan(BOOLEAN_EXP * a)
{
    if (a == NULL)
	return;

    BooleanReduceDeMorgan(a->left);
    BooleanReduceDeMorgan(a->right);

    if (a->optype == OP_OR) {
	if (a->right != NULL)
	    if (a->right->optype == OP_AND)
		BooleanDeMorganTranspose(a, a->right, a->left);

	if (a->left != NULL)
	    if (a->left->optype == OP_AND)
		BooleanDeMorganTranspose(a, a->left, a->right);
    }
    return;
}

BOOL IsFullyReduced(BOOLEAN_EXP * a)
{
    if (a == NULL)
	return TRUE;

    if (a->optype == OP_OR) {
	if (a->right != NULL)
	    if (a->right->optype == OP_AND)
		return FALSE;

	if (a->left != NULL)
	    if (a->left->optype == OP_AND)
		return FALSE;
    }
    return (IsFullyReduced(a->right)) && (IsFullyReduced(a->left));
}

void BooleanFullyReduce(BOOLEAN_EXP * a)
{
    BooleanReduceNots(a);

    while (!IsFullyReduced(a))
	BooleanReduceDeMorgan(a);

    return;
}
