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

typedef int OPTYPE;
#define OP_VAR		0
#define OP_AND		1
#define OP_OR		2

#define DATA_LENGTH		32

typedef struct tagBooleanExp BOOLEAN_EXP;
struct tagBooleanExp {
    OPTYPE optype;
    BOOL not;
    BOOLEAN_EXP *left;
    BOOLEAN_EXP *right;
    int length;
    BYTE data[DATA_LENGTH];
};

void FreeBoolean(BOOLEAN_EXP * exp);

BOOLEAN_EXP *BooleanCommand(void);
BOOLEAN_EXP *BooleanAnd(BOOLEAN_EXP * a, BOOLEAN_EXP * b);
BOOLEAN_EXP *BooleanOr(BOOLEAN_EXP * a, BOOLEAN_EXP * b);
BOOLEAN_EXP *BooleanNot(BOOLEAN_EXP * a);

void BooleanReduceNots(BOOLEAN_EXP * a);
void BooleanReduceDeMorgan(BOOLEAN_EXP * a);
BOOL IsFullyReduced(BOOLEAN_EXP * a);
void BooleanFullyReduce(BOOLEAN_EXP * a);

void DumpBoolean(BOOLEAN_EXP * exp);
