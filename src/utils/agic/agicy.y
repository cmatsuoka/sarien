%{

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

/* Parser and Code Generator */

/* Change log:
 *
 * JAF 12/26/97 - Added better error handling.
 *                Added #message as a better form of defstr
 *                Strings may span multiple strings (they will be joined)
 *                Return is now recognized (now as a function)
 *                Uses type checking for AGI and test commands now
 *                Accepts object names as arguments to commands
 *                Added kludgy fix so goto doesn't need parenthesis
 *                Added C-like indirection commands
 *                Removed redundant 'has'
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "agic.h"
#include "agierr.h"
#include "boolean.h"

int args;
int argstype;
BYTE bCommand;

WORD SaidFixup;
WORD IfFixup;

#define MAX_BLOCK_DEPTH			64

int nAddrStack = 0;
PTR ptrAddrStack[MAX_BLOCK_DEPTH];

BOOLEAN_EXP* bool;

void Push(PTR ptr);
PTR Pop(void);

static BOOLEAN_EXP* ConstructBooleanOr( BYTE dataA, BYTE dataB, BYTE opcodeA, BYTE opcodeB );
static char* JoinString( char* sz1, char* sz2 );
static void HandleNakedSymbol( char* sz );

#define YYERROR_VERBOSE
#define YYDEBUG 1
%}

%union
	{
	int num;
	char* text;
	BOOLEAN_EXP* bool;
	}

%token	<num>	NUM
%token	<num>	VAR
%token	<num>	FLAG
%token	<text>	STRING
%token	<text>	SYMBOL
%type	<bool>	expr
%type   <text>  strexpr

%token IF ELSE GOTO
%token CMP_EQ CMP_LT CMP_GT CMP_NEQ CMP_LTE CMP_GTE
%token ADDEQL SUBEQL MULEQL DIVEQL
%token DEFSTR MESSAGE
%token SAID
%token INC DEC AND OR NOT

%left AND OR
%nonassoc NOT
/* Grammar follows */

%%
logic:		/* empty */
		|	command logic
;

command:	DEFSTR '(' NUM ')' '=' strexpr	{ AddString( $3-1, $6 ); }
		|	MESSAGE NUM strexpr			{ AddString( $2-1, $3 ); }
		|	error ';'					{ yyerrok; }
		|   error '{'					{ yyerrok; Output(HC_IF); Output(HC_IF); Push(GetPC()); OutputWord(0); }
			logic '}' orelse			{  }
		|	VAR '=' NUM ';'				{ Output(AC_ASSIGNN); Output($1); Output($3); }
		|	VAR '=' VAR ';'				{ Output(AC_ASSIGNV); Output($1); Output($3); }
		|	'*' VAR '=' NUM ';'			{ Output(AC_LINDIRECTN); Output($2); Output($4); }
		|	'*' VAR '=' VAR ';'			{ Output(AC_LINDIRECTV); Output($2); Output($4); }
		|	VAR '=' '*' VAR ';'			{ Output(AC_RINDIRECT); Output($1); Output($4); }
		|	VAR INC ';'					{ Output(AC_INC); Output($1); }
		|	VAR DEC ';'					{ Output(AC_DEC); Output($1); }
		|	VAR ADDEQL NUM ';'			{ Output(AC_ADDN); Output($1); Output($3); }
		|	VAR ADDEQL VAR ';'			{ Output(AC_ADDV); Output($1); Output($3); }
		|	VAR SUBEQL NUM ';'			{ Output(AC_SUBN); Output($1); Output($3); }
		|	VAR SUBEQL VAR ';'			{ Output(AC_SUBV); Output($1); Output($3); }
		|	VAR MULEQL NUM ';'			{ Output(AC_MULN); Output($1); Output($3); }
		|	VAR MULEQL VAR ';'			{ Output(AC_MULV); Output($1); Output($3); }
		|	VAR DIVEQL NUM ';'			{ Output(AC_DIVN); Output($1); Output($3); }
		|	VAR DIVEQL VAR ';'			{ Output(AC_DIVV); Output($1); Output($3); }
		|	GOTO '(' SYMBOL ')' ';'		{ Output(HC_GOTO); OutputLabel($3); }
		|	SYMBOL ';'					{ HandleNakedSymbol( $1 ); }
		|	SYMBOL '('					{ Output( (bCommand = FindAGICommand( $1 )) ); free($1); args = 0; argstype = 0; }
			args ')' ';'				{ if (bCommand != 0) TypeCheckAGICommand( bCommand, args, argstype ); }
		|	IF '(' 						{ Output(HC_IF); }
			expr ')'					{ BooleanFullyReduce($4); DumpBoolean($4); FreeBoolean($4); Output(HC_IF); Push(GetPC()); OutputWord(0); }
			'{' logic '}' orelse
;

orelse:		/* empty */					{ IfFixup = Pop(); SetCoreWord( IfFixup, GetPC() - IfFixup - 2 ); }
		|	ELSE						{ IfFixup = Pop(); Output(HC_GOTO); Push(GetPC()); OutputWord(0);
											SetCoreWord( IfFixup, GetPC() - IfFixup - 2 ); }
			'{' logic '}'				{ IfFixup = Pop(); SetCoreWord( IfFixup, GetPC() - IfFixup - 2 ); }
;

strexpr:    STRING strexpr				{ $$ = JoinString( $1, $2 ); }
		|	STRING						{ $$ = $1; }
;

expr:		'(' expr ')'				{ $$ = $2; }
		|	expr AND expr				{ $$ = BooleanAnd( $1, $3 ); }
		|	expr OR expr				{ $$ = BooleanOr( $1, $3 ); }
		|	NOT expr					{ $$ = BooleanNot( $2 ); }
		|	FLAG						{ $$ = bool = BooleanCommand(); bool->data[0] = TC_ISSET; bool->data[1] = $1; bool->length = 2; }
		|	VAR '=' NUM					{ CompilerFailure( CERR_WRONG_EQUALS, NULL ); $$ = bool = BooleanCommand(); bool->data[0] = TC_EQUALN; bool->data[1] = $1; bool->data[2] = $3; bool->length = 3; }
		|	VAR '=' VAR					{ CompilerFailure( CERR_WRONG_EQUALS, NULL ); $$ = bool = BooleanCommand(); bool->data[0] = TC_EQUALV; bool->data[1] = $1; bool->data[2] = $3; bool->length = 3; }
		|	VAR CMP_EQ NUM				{ $$ = bool = BooleanCommand(); bool->data[0] = TC_EQUALN; bool->data[1] = $1; bool->data[2] = $3; bool->length = 3; }
		|	VAR CMP_EQ VAR				{ $$ = bool = BooleanCommand(); bool->data[0] = TC_EQUALV; bool->data[1] = $1; bool->data[2] = $3; bool->length = 3; }
		|	VAR CMP_NEQ NUM				{ $$ = bool = BooleanNot( BooleanCommand() ); bool->data[0] = TC_EQUALN; bool->data[1] = $1; bool->data[2] = $3; bool->length = 3; }
		|	VAR CMP_NEQ VAR				{ $$ = bool = BooleanCommand(); bool->data[0] = TC_EQUALV; bool->data[1] = $1; bool->data[2] = $3; bool->length = 3; }
		|	VAR CMP_GT NUM				{ $$ = bool = BooleanCommand(); bool->data[0] = TC_GREATERN; bool->data[1] = $1; bool->data[2] = $3; bool->length = 3; }
		|	VAR CMP_GT VAR				{ $$ = bool = BooleanCommand(); bool->data[0] = TC_GREATERV; bool->data[1] = $1; bool->data[2] = $3; bool->length = 3; }
		|	VAR CMP_LT NUM				{ $$ = bool = BooleanCommand(); bool->data[0] = TC_LESSN; bool->data[1] = $1; bool->data[2] = $3; bool->length = 3; }
		|	VAR CMP_LT VAR				{ $$ = bool = BooleanCommand(); bool->data[0] = TC_LESSV; bool->data[1] = $1; bool->data[2] = $3; bool->length = 3; }
		|	VAR CMP_GTE NUM				{ $$ = ConstructBooleanOr( $1, $3, TC_GREATERN, TC_EQUALN ); }
		|	VAR CMP_GTE VAR				{ $$ = ConstructBooleanOr( $1, $3, TC_GREATERV, TC_EQUALV ); }
		|	VAR CMP_LTE NUM				{ $$ = ConstructBooleanOr( $1, $3, TC_LESSN, TC_EQUALN ); }
		|	VAR CMP_LTE VAR				{ $$ = ConstructBooleanOr( $1, $3, TC_LESSV, TC_LESSV ); }
		|	SYMBOL '(' 					{ $$ = bool = BooleanCommand(); bCommand = bool->data[0] = FindTestCommand( $1 ); args = 0; argstype = 0; free($1); }
			bargs ')'					{ $$ = $<bool>3; bool->length = args+1; if (bCommand != 0) TypeCheckTestCommand( bCommand, args, argstype ); }
		|	SAID '('					{ $$ = bool = BooleanCommand(); bool->data[0] = TC_SAID; args = 0; }
			sargs ')'					{ $$ = $<bool>3; bool->data[1] = args; bool->length = 2*args + 2; }
;

sargs:		/* empty */
		|	sargument
		|	sargs ',' sargument
;

sargument:	SYMBOL						{ WORD w; w = GetTokenIndex($1); free($1); args++; bool->data[2*args] = w&0xFF; bool->data[2*args+1] = (w<<8) & 0xFF; }
;

bargs:		bargument
		|	bargs ',' bargument
        |
;

bargument:	VAR							{ bool->data[args+1] = $1; argstype |= 1 << (7-args); args++; }
		|	NUM							{ bool->data[args+1] = $1; args++; }
		|	FLAG						{ bool->data[args+1] = $1; args++; }
		|	SYMBOL						{ bool->data[args+1] = GetObjectIndex($1); args++; }
		|	strexpr						{ bool->data[args+1] = GetStringID( $1 ); args++; }
;

args:		argument
		|	args ',' argument
        |
;

argument:	VAR							{ Output( $1 ); argstype |= 1 << (7-args); args++; }
        |   FLAG                        { Output( $1 ); args++; }
		|   SYMBOL                      { Output( GetObjectIndex($1) ); args++; }
		|	NUM							{ Output( $1 ); args++; }
		|	strexpr						{ Output( GetStringID( $1 ) ); args++; }
;


/* End of grammar */
%%

yyerror(char *s)
{
    CompilerFailure(CERR_SYNTAX_ERROR, NULL);
}

void Push(PTR ptr)
{
    ptrAddrStack[nAddrStack] = ptr;
    nAddrStack++;
}

PTR Pop(void)
{
    nAddrStack--;
    return ptrAddrStack[nAddrStack];
}

BOOLEAN_EXP *ConstructBooleanOr(BYTE dataA, BYTE dataB, BYTE opcodeA, BYTE opcodeB)
{
    BOOLEAN_EXP *boolA, *boolB;

    boolA = BooleanCommand();
    boolB = BooleanCommand();

    boolA->data[0] = opcodeA;
    boolA->data[1] = dataA;
    boolA->data[2] = dataB;
    boolA->length = 3;

    boolB->data[0] = opcodeB;
    boolB->data[1] = dataA;
    boolB->data[2] = dataB;
    boolB->length = 3;

    return BooleanOr(boolA, boolB);
}

char* JoinString( char* sz1, char* sz2 )
	{
	char *szNew;

	if ((sz1 == NULL) && (sz2 != NULL))
		return sz2;

	if ((sz1 != NULL) && (sz2 == NULL))
		return sz1;

	if ((sz1 == NULL) && (sz2 == NULL))
		FatalError( "Join string given two null arguments" );		
	
	szNew = malloc( strlen(sz1) + strlen(sz2) + 1 );	
	if (szNew == NULL)
		CompilerFailure( CERR_OUT_OF_MEMORY, "for string expression" );
		
	strcpy( szNew, sz1 );
	strcat( szNew, sz2 );
	
	free( sz1 );
	free( sz2 );

	return szNew;
	}

void HandleNakedSymbol( char* sz )
	{
	if (sz[4] != ' ')
		{
		free( sz );
		yyerror( "symbol needs arguments" );
		return;
		}

	sz[4] = 0;
	if (STRCMP( sz, "goto" ) != 0)
		{
		free( sz );
		yyerror( "naked symbol must be goto" );
		return;
		}

	Output( HC_GOTO );
	OutputLabel( strdup( sz+5 ) );
	free( sz );

	return;
	}
