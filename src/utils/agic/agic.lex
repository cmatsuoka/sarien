%x string
%x symbol
%x preproc

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

/* Change log:
 *
 * JAF 12/26/97 - Both -h and -? now give help
 *                Symbols of arbitrary length are now recognized
 *                Added more sophisticated string handling to the lexer
 *                Added the #message to the lexer
 *                The lexer recognizes return / The lexer now ignores it
 *                The lexer now processes the indirection star
 *                The lexer now uses | to bracket arbitrary symbols
 *                Removed redundant processing of has
 *
 */

/* UGLY-KLUDGE: Need to match whitespace after reserved words
   to prevent them from being eaten as symbols */

/* Lexical Analyzer */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "agic.h"
#include "boolean.h"
#include "y.tab.h"
#include "agierr.h"

unsigned int nLineNumber = 1;
char szFile[_MAX_PATH];

char* szString;
int nStringLen;
int nMaxStringLen;

void AddToString( char* sz )
	{
	if ((strlen(sz) + nStringLen) > (nMaxStringLen - 1))
		{
		szString = (char*)realloc( szString, strlen(sz) + nStringLen + 1 );
		if (szString == NULL)
			CompilerFailure( CERR_OUT_OF_MEMORY, "for resizing string buffer" );
		nMaxStringLen = strlen(sz) + nStringLen + 1;
		}
	
	strcpy( &(szString[nStringLen]), sz );
	nStringLen += strlen(sz);

	return;
	}
%}

%%

^"# "						BEGIN(preproc);
<preproc>[0-9]+				nLineNumber = atoi(yytext);
<preproc>" "
<preproc>"\""[^"]*			{ strncpy( szFile, yytext+1, 128 ); while( input() != '\n' ); BEGIN(INITIAL); }

"["[^\n]*
[ \t]+
\n							nLineNumber++;

"--"						return DEC;
"++"						return INC;

^[A-Za-z][A-Za-z0-9]*":"	{ *(yytext+strlen(yytext)-1) = 0; ResolveLabel( strdup(yytext) ); }

"defstr"					return DEFSTR;
"#message"					return MESSAGE;

"goto"[ ]*					return GOTO;
"if"[ ]*					return IF;
"else"[ ]*					return ELSE;
"said"[ ]*					return SAID;
"{"							return '{';
"}"							return '}';
";"							return ';';
"("							return '(';
")"							return ')';
","							return ',';
"&&"						return AND;
"||"						return OR;
"!"							return NOT;

"*"							return '*';
"="							return '=';
"+="						return ADDEQL;
"-="						return SUBEQL;
"*="						return MULEQL;
"/="						return DIVEQL;

"!="						return CMP_NEQ;
"=="						return CMP_EQ;
"<"							return CMP_LT;
">"							return CMP_GT;
"<="						return CMP_LTE;
">="						return CMP_GTE;

[0-9]+						{ yylval.num = atoi(yytext); return NUM; }
v[0-9]+						{ yylval.num = atoi(yytext+1); return VAR; }
f[0-9]+						{ yylval.num = atoi(yytext+1); return FLAG; }
[A-Za-z]+[. A-Za-z]*[A-Za-z0-9]*  { yylval.text = strdup(yytext); /* printf( "SYMBOL(%s)\n", yytext ); */ return SYMBOL; }

"|"							BEGIN(symbol);
<symbol>\n					{ BEGIN(INITIAL); CompilerFailure( CERR_UNENDED_SYMBOL, NULL ); yylval.text = strdup(yytext); return SYMBOL; }
<symbol>[^\n|]*				{ yylval.text = strdup(yytext); return SYMBOL; }
<symbol>"|"					BEGIN(INITIAL);

"\""						{
	BEGIN(string);

	szString = (char *)malloc( INITIAL_STRING_BUFFER );
	if (szString == NULL)
		CompilerFailure( CERR_OUT_OF_MEMORY, "for initial string buffer" );

	nMaxStringLen = INITIAL_STRING_BUFFER;
	nStringLen = 0;
	}

<string>"\""				{
	BEGIN(INITIAL);

	szString[nStringLen] = 0;
	yylval.text = szString;

	return STRING;
	}

<string>\n					{
	BEGIN(INITIAL);

	CompilerFailure( CERR_UNENDED_STRING, NULL );
	
	szString[nStringLen] = 0;
	yylval.text = szString;

	return STRING;
	}

<string>[^"\n\\]*			{ AddToString( yytext ); }
<string>"\\n"				{ AddToString( "\n" ); }
<string>"\\\\"				{ AddToString( "\\" ); }
<string>"\\\""				{ AddToString( "\"" ); }
<string>\\[^n\\\"]			{ CompilerFailure( CERR_BAD_STRING_CODE, yytext ); }

<*>.|\\n					{ CompilerFailure( CERR_UNKNOWN_CHAR, yytext ); }

%%

int GetCurrentLine(void)
{
    return nLineNumber;
}

char *GetCurrentFile(void)
{
    return szFile;
}

yywrap(void)
{
    return 1;
}

void yyparse(void);

void DisplayHelp(void)
{
    printf("Usage: AGIC input [-h] [-q] [-o objects] [-t tokens] [-e output]\n\n");

    printf("       input         loads source code from file \"input\"\n");
    printf("       -h            help\n");
    printf("       -o objects    loads the object list from file \"objects\"\n");
    printf("       -t tokens     loads the dictionary from file \"tokens\"\n");
    printf("       -e output     outputs compiled logic image to file \"output\"\n\n");

    printf("AGIC could not have been created without the work of the AGI community.\n");
    printf("The information and source code published by others has been indispensable\n");
    printf("in creating this software.  Floating Hills Software hopes that this software\n");
    printf("proves as useful to you.\n");
}

int main(int argc, char *argv[])
{
    char szObjectsFile[_MAX_PATH], szTokensFile[_MAX_PATH], szOutputFile[_MAX_PATH];
    FILE *input, *output;
    int i;
	int ErrorLevel = 1;

    printf(PROGRAM_NAME);
    printf(FREE_BANNER);

    strcpy(szObjectsFile, OBJ_FILE);
    strcpy(szTokensFile, TOK_FILE);
    strcpy(szOutputFile, "LOGIC.OUT");

    i = 1;
    while (i < argc) {
	if ((argv[i][0] == '-') || (argv[i][0] == '/')) {
	    switch (argv[i][1]) {
	    case 'o':
	    case 'O':
		i++;
		strcpy(szObjectsFile, argv[i]);
		break;

	    case 't':
	    case 'T':
		i++;
		strcpy(szTokensFile, argv[i]);
		break;

	    case 'e':
	    case 'E':
		i++;
		strcpy(szOutputFile, argv[i]);
		break;

		case '?':
	    case 'h':
	    case 'H':
		DisplayHelp();
		return;

	    default:
		Error(1, "Unknown switch");
	    }
	} else {
	    if (szFile[0] == 0)
		strcpy(szFile, argv[i]);
	    else
		FatalError("Too many files to compile");
	}

	i++;
    }

    if (szFile[0] == 0)
	FatalError("No filename given for compile");

    yyin = input = fopen(szFile, "rt");

    if (input == NULL)
	FatalError("Could not open input file");

    InitCore();
    InitStrings();

    LoadObjects(szObjectsFile);
    LoadTokens(szTokensFile);

    yyparse();

    fclose(input);

    FreeTokens();
    FreeObjects();

    PrintUnresolvedLabels();
    FreeLabels();

    PrintErrorStats();

    if (CompiledOkay()) {
	output = fopen(szOutputFile, "wb");

	if (output == NULL)
	    FatalError("Could not open output file");

	DumpCore(output);
	DumpStrings(output);
	fclose(output);
	printf("Produced file %s successfully.\n", szOutputFile);
	ErrorLevel = 0;
    }
    FreeCore();
    FreeStrings();

    return ErrorLevel;
}
