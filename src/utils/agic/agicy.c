
/*  A Bison parser, made from agi.y
    by GNU Bison version 1.28  */

#define YYBISON 1  /* Identify Bison output.  */

#define	NUM	257
#define	VAR	258
#define	FLAG	259
#define	STRING	260
#define	SYMBOL	261
#define	IF	262
#define	ELSE	263
#define	GOTO	264
#define	CMP_EQ	265
#define	CMP_LT	266
#define	CMP_GT	267
#define	CMP_NEQ	268
#define	CMP_LTE	269
#define	CMP_GTE	270
#define	ADDEQL	271
#define	SUBEQL	272
#define	MULEQL	273
#define	DIVEQL	274
#define	DEFSTR	275
#define	MESSAGE	276
#define	SAID	277
#define	INC	278
#define	DEC	279
#define	AND	280
#define	OR	281
#define	NOT	282

#line 1 "agi.y"


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

#line 73 "agi.y"
typedef union
	{
	int num;
	char* text;
	BOOLEAN_EXP* bool;
	} YYSTYPE;
#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		152
#define	YYFLAG		-32768
#define	YYNTBASE	37

#define YYTRANSLATE(x) ((unsigned)(x) <= 282 ? yytranslate[x] : 55)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,    29,
    30,    35,     2,    36,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,    32,     2,
    31,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    33,     2,    34,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     3,     4,     5,     6,
     7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
    17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
    27,    28
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     1,     4,    11,    15,    18,    19,    26,    31,    36,
    42,    48,    54,    58,    62,    67,    72,    77,    82,    87,
    92,    97,   102,   108,   111,   112,   119,   120,   121,   132,
   133,   134,   140,   143,   145,   149,   153,   157,   160,   162,
   166,   170,   174,   178,   182,   186,   190,   194,   198,   202,
   206,   210,   214,   218,   219,   225,   226,   232,   233,   235,
   239,   241,   243,   247,   248,   250,   252,   254,   256,   258,
   260,   264,   265,   267,   269,   271,   273
};

static const short yyrhs[] = {    -1,
    38,    37,     0,    21,    29,     3,    30,    31,    45,     0,
    22,     3,    45,     0,     1,    32,     0,     0,     1,    33,
    39,    37,    34,    43,     0,     4,    31,     3,    32,     0,
     4,    31,     4,    32,     0,    35,     4,    31,     3,    32,
     0,    35,     4,    31,     4,    32,     0,     4,    31,    35,
     4,    32,     0,     4,    24,    32,     0,     4,    25,    32,
     0,     4,    17,     3,    32,     0,     4,    17,     4,    32,
     0,     4,    18,     3,    32,     0,     4,    18,     4,    32,
     0,     4,    19,     3,    32,     0,     4,    19,     4,    32,
     0,     4,    20,     3,    32,     0,     4,    20,     4,    32,
     0,    10,    29,     7,    30,    32,     0,     7,    32,     0,
     0,     7,    29,    40,    53,    30,    32,     0,     0,     0,
     8,    29,    41,    46,    30,    42,    33,    37,    34,    43,
     0,     0,     0,     9,    44,    33,    37,    34,     0,     6,
    45,     0,     6,     0,    29,    46,    30,     0,    46,    26,
    46,     0,    46,    27,    46,     0,    28,    46,     0,     5,
     0,     4,    31,     3,     0,     4,    31,     4,     0,     4,
    11,     3,     0,     4,    11,     4,     0,     4,    14,     3,
     0,     4,    14,     4,     0,     4,    13,     3,     0,     4,
    13,     4,     0,     4,    12,     3,     0,     4,    12,     4,
     0,     4,    16,     3,     0,     4,    16,     4,     0,     4,
    15,     3,     0,     4,    15,     4,     0,     0,     7,    29,
    47,    51,    30,     0,     0,    23,    29,    48,    49,    30,
     0,     0,    50,     0,    49,    36,    50,     0,     7,     0,
    52,     0,    51,    36,    52,     0,     0,     4,     0,     3,
     0,     5,     0,     7,     0,    45,     0,    54,     0,    53,
    36,    54,     0,     0,     4,     0,     5,     0,     7,     0,
     3,     0,    45,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   100,   101,   104,   105,   106,   107,   108,   109,   110,   111,
   112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
   122,   123,   124,   125,   126,   127,   128,   129,   130,   133,
   134,   136,   139,   140,   143,   144,   145,   146,   147,   148,
   149,   150,   151,   152,   153,   154,   155,   156,   157,   158,
   159,   160,   161,   162,   163,   164,   165,   168,   169,   170,
   173,   176,   177,   178,   181,   182,   183,   184,   185,   188,
   189,   190,   193,   194,   195,   196,   197
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","NUM","VAR",
"FLAG","STRING","SYMBOL","IF","ELSE","GOTO","CMP_EQ","CMP_LT","CMP_GT","CMP_NEQ",
"CMP_LTE","CMP_GTE","ADDEQL","SUBEQL","MULEQL","DIVEQL","DEFSTR","MESSAGE","SAID",
"INC","DEC","AND","OR","NOT","'('","')'","'='","';'","'{'","'}'","'*'","','",
"logic","command","@1","@2","@3","@4","orelse","@5","strexpr","expr","@6","@7",
"sargs","sargument","bargs","bargument","args","argument", NULL
};
#endif

static const short yyr1[] = {     0,
    37,    37,    38,    38,    38,    39,    38,    38,    38,    38,
    38,    38,    38,    38,    38,    38,    38,    38,    38,    38,
    38,    38,    38,    38,    40,    38,    41,    42,    38,    43,
    44,    43,    45,    45,    46,    46,    46,    46,    46,    46,
    46,    46,    46,    46,    46,    46,    46,    46,    46,    46,
    46,    46,    46,    47,    46,    48,    46,    49,    49,    49,
    50,    51,    51,    51,    52,    52,    52,    52,    52,    53,
    53,    53,    54,    54,    54,    54,    54
};

static const short yyr2[] = {     0,
     0,     2,     6,     3,     2,     0,     6,     4,     4,     5,
     5,     5,     3,     3,     4,     4,     4,     4,     4,     4,
     4,     4,     5,     2,     0,     6,     0,     0,    10,     0,
     0,     5,     2,     1,     3,     3,     3,     2,     1,     3,
     3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
     3,     3,     3,     0,     5,     0,     5,     0,     1,     3,
     1,     1,     3,     0,     1,     1,     1,     1,     1,     1,
     3,     0,     1,     1,     1,     1,     1
};

static const short yydefact[] = {     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     5,
     6,     0,     0,     0,     0,     0,     0,     0,    25,    24,
    27,     0,     0,     0,     0,     2,     0,     0,     0,     0,
     0,     0,     0,     0,     0,    13,    14,     0,     0,     0,
    72,     0,     0,     0,    34,     4,     0,     0,    15,    16,
    17,    18,    19,    20,    21,    22,     8,     9,     0,    76,
    73,    74,    75,    77,     0,    70,     0,    39,     0,     0,
     0,     0,     0,     0,     0,    33,     0,     0,    30,    12,
     0,     0,     0,     0,     0,     0,     0,     0,     0,    54,
    56,    38,     0,     0,     0,    28,    23,     0,    10,    11,
    31,     7,    26,    71,    42,    43,    48,    49,    46,    47,
    44,    45,    52,    53,    50,    51,    40,    41,    64,    58,
    35,    36,    37,     0,     3,     0,    66,    65,    67,    68,
    69,     0,    62,    61,     0,    59,     0,     0,    55,     0,
    57,     0,     0,     0,    63,    60,    30,    32,    29,     0,
     0,     0
};

static const short yydefgoto[] = {    26,
     9,    27,    41,    42,   124,   102,   126,    64,    73,   119,
   120,   135,   136,   132,   133,    65,    66
};

static const short yypact[] = {     7,
   -28,    53,   -16,    35,    38,    66,    71,   111,     2,-32768,
-32768,    22,    42,    89,    94,    84,    86,    15,-32768,-32768,
-32768,   112,   117,   115,    91,-32768,    31,    92,    93,    95,
    97,    99,   101,   103,   107,-32768,-32768,   109,   113,   119,
    76,    26,   114,   116,   115,-32768,    96,   118,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   121,-32768,
-32768,-32768,-32768,-32768,     4,-32768,    45,-32768,   105,   120,
    26,    26,    21,   122,   124,-32768,   125,   126,   123,-32768,
   127,    76,    98,   100,   102,   104,   106,   108,   110,-32768,
-32768,-32768,    64,    26,    26,-32768,-32768,   115,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,    82,   129,
-32768,-32768,-32768,   128,-32768,   130,-32768,-32768,-32768,-32768,
-32768,    32,-32768,-32768,    33,-32768,    31,    31,-32768,    82,
-32768,   129,   131,   132,-32768,-32768,   123,-32768,-32768,   140,
   142,-32768
};

static const short yypgoto[] = {     0,
-32768,-32768,-32768,-32768,-32768,   -21,-32768,   -23,   -51,-32768,
-32768,-32768,   -14,-32768,   -10,-32768,    61
};


#define	YYLAST		166


static const short yytable[] = {   150,
    46,    -1,     1,    10,    11,     2,    -1,     1,     3,     4,
     2,     5,    19,     3,     4,    20,     5,    38,    39,    92,
    93,    76,     6,     7,    28,    29,    48,     6,     7,    67,
    68,     1,    69,    81,     2,    -1,     8,     3,     4,    82,
     5,     8,   122,   123,    30,    31,    94,    95,    70,    40,
    96,     6,     7,    71,    72,    83,    84,    85,    86,    87,
    88,   139,   141,    21,    -1,     8,    22,   140,   142,    12,
    13,    14,    15,    24,   125,    89,    16,    17,    60,    61,
    62,    45,    63,    18,   127,   128,   129,    45,   130,    94,
    95,    32,    33,   121,    23,   131,    34,    35,    77,    78,
   105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
   115,   116,   117,   118,    25,    36,   131,    37,    43,    44,
    45,    47,    59,    49,    50,   149,    51,   146,    52,   145,
    53,   101,    54,    90,    55,   134,   143,   144,    56,   151,
    57,   152,   104,    74,    58,    75,     0,     0,    91,     0,
     0,    79,    80,    97,    98,     0,    99,   100,   103,     0,
   137,     0,   138,     0,   147,   148
};

static const short yycheck[] = {     0,
    24,     0,     1,    32,    33,     4,     0,     1,     7,     8,
     4,    10,    29,     7,     8,    32,    10,     3,     4,    71,
    72,    45,    21,    22,     3,     4,    27,    21,    22,     4,
     5,     1,     7,    30,     4,    34,    35,     7,     8,    36,
    10,    35,    94,    95,     3,     4,    26,    27,    23,    35,
    30,    21,    22,    28,    29,    11,    12,    13,    14,    15,
    16,    30,    30,    29,    34,    35,    29,    36,    36,    17,
    18,    19,    20,     3,    98,    31,    24,    25,     3,     4,
     5,     6,     7,    31,     3,     4,     5,     6,     7,    26,
    27,     3,     4,    30,    29,   119,     3,     4,     3,     4,
     3,     4,     3,     4,     3,     4,     3,     4,     3,     4,
     3,     4,     3,     4,     4,    32,   140,    32,     7,     3,
     6,    31,     4,    32,    32,   147,    32,   142,    32,   140,
    32,     9,    32,    29,    32,     7,   137,   138,    32,     0,
    32,     0,    82,    30,    32,    30,    -1,    -1,    29,    -1,
    -1,    34,    32,    32,    31,    -1,    32,    32,    32,    -1,
    33,    -1,    33,    -1,    34,    34
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/share/bison/bison.simple"
/* This file comes from bison-1.28.  */

/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

#ifndef YYSTACK_USE_ALLOCA
#ifdef alloca
#define YYSTACK_USE_ALLOCA
#else /* alloca not defined */
#ifdef __GNUC__
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc) || defined (__sgi) || (defined (__sun) && defined (__i386))
#define YYSTACK_USE_ALLOCA
#include <alloca.h>
#else /* not sparc */
/* We think this test detects Watcom and Microsoft C.  */
/* This used to test MSDOS, but that is a bad idea
   since that symbol is in the user namespace.  */
#if (defined (_MSDOS) || defined (_MSDOS_)) && !defined (__TURBOC__)
#if 0 /* No need for malloc.h, which pollutes the namespace;
	 instead, just don't use alloca.  */
#include <malloc.h>
#endif
#else /* not MSDOS, or __TURBOC__ */
#if defined(_AIX)
/* I don't know what this was needed for, but it pollutes the namespace.
   So I turned it off.   rms, 2 May 1997.  */
/* #include <malloc.h>  */
 #pragma alloca
#define YYSTACK_USE_ALLOCA
#else /* not MSDOS, or __TURBOC__, or _AIX */
#if 0
#ifdef __hpux /* haible@ilog.fr says this works for HPUX 9.05 and up,
		 and on HPUX 10.  Eventually we can turn this on.  */
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#endif /* __hpux */
#endif
#endif /* not _AIX */
#endif /* not MSDOS, or __TURBOC__ */
#endif /* not sparc */
#endif /* not GNU C */
#endif /* alloca not defined */
#endif /* YYSTACK_USE_ALLOCA not defined */

#ifdef YYSTACK_USE_ALLOCA
#define YYSTACK_ALLOC alloca
#else
#define YYSTACK_ALLOC malloc
#endif

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	goto yyacceptlab
#define YYABORT 	goto yyabortlab
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    { yychar = (token), yylval = (value);			\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { yyerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYPURE
#define YYLEX		yylex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, &yylloc, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval, &yylloc)
#endif
#else /* not YYLSP_NEEDED */
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval)
#endif
#endif /* not YYLSP_NEEDED */
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/
#endif

int yynerrs;			/*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int yydebug;			/*  nonzero means print parse trace	*/
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

/* Define __yy_memcpy.  Note that the size argument
   should be passed with type unsigned int, because that is what the non-GCC
   definitions require.  With GCC, __builtin_memcpy takes an arg
   of type size_t, but it can handle unsigned int.  */

#if __GNUC__ > 1		/* GNU C and GNU C++ define this.  */
#define __yy_memcpy(TO,FROM,COUNT)	__builtin_memcpy(TO,FROM,COUNT)
#else				/* not GNU C or C++ */
#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (to, from, count)
     char *to;
     char *from;
     unsigned int count;
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#else /* __cplusplus */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (char *to, char *from, unsigned int count)
{
  register char *t = to;
  register char *f = from;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif
#endif

#line 217 "/usr/share/bison/bison.simple"

/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
#ifdef __cplusplus
#define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#else /* not __cplusplus */
#define YYPARSE_PARAM_ARG YYPARSE_PARAM
#define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
#endif /* not __cplusplus */
#else /* not YYPARSE_PARAM */
#define YYPARSE_PARAM_ARG
#define YYPARSE_PARAM_DECL
#endif /* not YYPARSE_PARAM */

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
#ifdef YYPARSE_PARAM
int yyparse (void *);
#else
int yyparse (void);
#endif
#endif

int
yyparse(YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int yychar1 = 0;		/*  lookahead token as an internal (translated) token number */

  short	yyssa[YYINITDEPTH];	/*  the state stack			*/
  YYSTYPE yyvsa[YYINITDEPTH];	/*  the semantic value stack		*/

  short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE yylsa[YYINITDEPTH];	/*  the location stack			*/
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;
  int yyfree_stacks = 0;

#ifdef YYPURE
  int yychar;
  YYSTYPE yylval;
  int yynerrs;
#ifdef YYLSP_NEEDED
  YYLTYPE yylloc;
#endif
#endif

  YYSTYPE yyval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/

  int yylen;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YYLSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YYLSP_NEEDED
      YYLTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YYLSP_NEEDED
      /* This used to be a conditional around just the two extra args,
	 but that might be undefined if yyoverflow is a macro.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yystacksize);
#else
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yystacksize);
#endif

      yyss = yyss1; yyvs = yyvs1;
#ifdef YYLSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  yyerror("parser stack overflow");
	  if (yyfree_stacks)
	    {
	      free (yyss);
	      free (yyvs);
#ifdef YYLSP_NEEDED
	      free (yyls);
#endif
	    }
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
#ifndef YYSTACK_USE_ALLOCA
      yyfree_stacks = 1;
#endif
      yyss = (short *) YYSTACK_ALLOC (yystacksize * sizeof (*yyssp));
      __yy_memcpy ((char *)yyss, (char *)yyss1,
		   size * (unsigned int) sizeof (*yyssp));
      yyvs = (YYSTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yyvsp));
      __yy_memcpy ((char *)yyvs, (char *)yyvs1,
		   size * (unsigned int) sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yylsp));
      __yy_memcpy ((char *)yyls, (char *)yyls1,
		   size * (unsigned int) sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YYLSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

  goto yybackup;
 yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Reading a token: ");
#endif
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

#if YYDEBUG != 0
      if (yydebug)
	{
	  fprintf (stderr, "Next token is %d (%s", yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
#endif
	  fprintf (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  if (yylen > 0)
    yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (yydebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


  switch (yyn) {

case 3:
#line 104 "agi.y"
{ AddString( yyvsp[-3].num-1, yyvsp[0].text ); ;
    break;}
case 4:
#line 105 "agi.y"
{ AddString( yyvsp[-1].num-1, yyvsp[0].text ); ;
    break;}
case 5:
#line 106 "agi.y"
{ yyerrok; ;
    break;}
case 6:
#line 107 "agi.y"
{ yyerrok; Output(HC_IF); Output(HC_IF); Push(GetPC()); OutputWord(0); ;
    break;}
case 7:
#line 108 "agi.y"
{  ;
    break;}
case 8:
#line 109 "agi.y"
{ Output(AC_ASSIGNN); Output(yyvsp[-3].num); Output(yyvsp[-1].num); ;
    break;}
case 9:
#line 110 "agi.y"
{ Output(AC_ASSIGNV); Output(yyvsp[-3].num); Output(yyvsp[-1].num); ;
    break;}
case 10:
#line 111 "agi.y"
{ Output(AC_LINDIRECTN); Output(yyvsp[-3].num); Output(yyvsp[-1].num); ;
    break;}
case 11:
#line 112 "agi.y"
{ Output(AC_LINDIRECTV); Output(yyvsp[-3].num); Output(yyvsp[-1].num); ;
    break;}
case 12:
#line 113 "agi.y"
{ Output(AC_RINDIRECT); Output(yyvsp[-4].num); Output(yyvsp[-1].num); ;
    break;}
case 13:
#line 114 "agi.y"
{ Output(AC_INC); Output(yyvsp[-2].num); ;
    break;}
case 14:
#line 115 "agi.y"
{ Output(AC_DEC); Output(yyvsp[-2].num); ;
    break;}
case 15:
#line 116 "agi.y"
{ Output(AC_ADDN); Output(yyvsp[-3].num); Output(yyvsp[-1].num); ;
    break;}
case 16:
#line 117 "agi.y"
{ Output(AC_ADDV); Output(yyvsp[-3].num); Output(yyvsp[-1].num); ;
    break;}
case 17:
#line 118 "agi.y"
{ Output(AC_SUBN); Output(yyvsp[-3].num); Output(yyvsp[-1].num); ;
    break;}
case 18:
#line 119 "agi.y"
{ Output(AC_SUBV); Output(yyvsp[-3].num); Output(yyvsp[-1].num); ;
    break;}
case 19:
#line 120 "agi.y"
{ Output(AC_MULN); Output(yyvsp[-3].num); Output(yyvsp[-1].num); ;
    break;}
case 20:
#line 121 "agi.y"
{ Output(AC_MULV); Output(yyvsp[-3].num); Output(yyvsp[-1].num); ;
    break;}
case 21:
#line 122 "agi.y"
{ Output(AC_DIVN); Output(yyvsp[-3].num); Output(yyvsp[-1].num); ;
    break;}
case 22:
#line 123 "agi.y"
{ Output(AC_DIVV); Output(yyvsp[-3].num); Output(yyvsp[-1].num); ;
    break;}
case 23:
#line 124 "agi.y"
{ Output(HC_GOTO); OutputLabel(yyvsp[-2].text); ;
    break;}
case 24:
#line 125 "agi.y"
{ HandleNakedSymbol( yyvsp[-1].text ); ;
    break;}
case 25:
#line 126 "agi.y"
{ Output( (bCommand = FindAGICommand( yyvsp[-1].text )) ); free(yyvsp[-1].text); args = 0; argstype = 0; ;
    break;}
case 26:
#line 127 "agi.y"
{ if (bCommand != 0) TypeCheckAGICommand( bCommand, args, argstype ); ;
    break;}
case 27:
#line 128 "agi.y"
{ Output(HC_IF); ;
    break;}
case 28:
#line 129 "agi.y"
{ BooleanFullyReduce(yyvsp[-1].bool); DumpBoolean(yyvsp[-1].bool); FreeBoolean(yyvsp[-1].bool); Output(HC_IF); Push(GetPC()); OutputWord(0); ;
    break;}
case 30:
#line 133 "agi.y"
{ IfFixup = Pop(); SetCoreWord( IfFixup, GetPC() - IfFixup - 2 ); ;
    break;}
case 31:
#line 134 "agi.y"
{ IfFixup = Pop(); Output(HC_GOTO); Push(GetPC()); OutputWord(0);
											SetCoreWord( IfFixup, GetPC() - IfFixup - 2 ); ;
    break;}
case 32:
#line 136 "agi.y"
{ IfFixup = Pop(); SetCoreWord( IfFixup, GetPC() - IfFixup - 2 ); ;
    break;}
case 33:
#line 139 "agi.y"
{ yyval.text = JoinString( yyvsp[-1].text, yyvsp[0].text ); ;
    break;}
case 34:
#line 140 "agi.y"
{ yyval.text = yyvsp[0].text; ;
    break;}
case 35:
#line 143 "agi.y"
{ yyval.bool = yyvsp[-1].bool; ;
    break;}
case 36:
#line 144 "agi.y"
{ yyval.bool = BooleanAnd( yyvsp[-2].bool, yyvsp[0].bool ); ;
    break;}
case 37:
#line 145 "agi.y"
{ yyval.bool = BooleanOr( yyvsp[-2].bool, yyvsp[0].bool ); ;
    break;}
case 38:
#line 146 "agi.y"
{ yyval.bool = BooleanNot( yyvsp[0].bool ); ;
    break;}
case 39:
#line 147 "agi.y"
{ yyval.bool = bool = BooleanCommand(); bool->data[0] = TC_ISSET; bool->data[1] = yyvsp[0].num; bool->length = 2; ;
    break;}
case 40:
#line 148 "agi.y"
{ CompilerFailure( CERR_WRONG_EQUALS, NULL ); yyval.bool = bool = BooleanCommand(); bool->data[0] = TC_EQUALN; bool->data[1] = yyvsp[-2].num; bool->data[2] = yyvsp[0].num; bool->length = 3; ;
    break;}
case 41:
#line 149 "agi.y"
{ CompilerFailure( CERR_WRONG_EQUALS, NULL ); yyval.bool = bool = BooleanCommand(); bool->data[0] = TC_EQUALV; bool->data[1] = yyvsp[-2].num; bool->data[2] = yyvsp[0].num; bool->length = 3; ;
    break;}
case 42:
#line 150 "agi.y"
{ yyval.bool = bool = BooleanCommand(); bool->data[0] = TC_EQUALN; bool->data[1] = yyvsp[-2].num; bool->data[2] = yyvsp[0].num; bool->length = 3; ;
    break;}
case 43:
#line 151 "agi.y"
{ yyval.bool = bool = BooleanCommand(); bool->data[0] = TC_EQUALV; bool->data[1] = yyvsp[-2].num; bool->data[2] = yyvsp[0].num; bool->length = 3; ;
    break;}
case 44:
#line 152 "agi.y"
{ yyval.bool = bool = BooleanNot( BooleanCommand() ); bool->data[0] = TC_EQUALN; bool->data[1] = yyvsp[-2].num; bool->data[2] = yyvsp[0].num; bool->length = 3; ;
    break;}
case 45:
#line 153 "agi.y"
{ yyval.bool = bool = BooleanCommand(); bool->data[0] = TC_EQUALV; bool->data[1] = yyvsp[-2].num; bool->data[2] = yyvsp[0].num; bool->length = 3; ;
    break;}
case 46:
#line 154 "agi.y"
{ yyval.bool = bool = BooleanCommand(); bool->data[0] = TC_GREATERN; bool->data[1] = yyvsp[-2].num; bool->data[2] = yyvsp[0].num; bool->length = 3; ;
    break;}
case 47:
#line 155 "agi.y"
{ yyval.bool = bool = BooleanCommand(); bool->data[0] = TC_GREATERV; bool->data[1] = yyvsp[-2].num; bool->data[2] = yyvsp[0].num; bool->length = 3; ;
    break;}
case 48:
#line 156 "agi.y"
{ yyval.bool = bool = BooleanCommand(); bool->data[0] = TC_LESSN; bool->data[1] = yyvsp[-2].num; bool->data[2] = yyvsp[0].num; bool->length = 3; ;
    break;}
case 49:
#line 157 "agi.y"
{ yyval.bool = bool = BooleanCommand(); bool->data[0] = TC_LESSV; bool->data[1] = yyvsp[-2].num; bool->data[2] = yyvsp[0].num; bool->length = 3; ;
    break;}
case 50:
#line 158 "agi.y"
{ yyval.bool = ConstructBooleanOr( yyvsp[-2].num, yyvsp[0].num, TC_GREATERN, TC_EQUALN ); ;
    break;}
case 51:
#line 159 "agi.y"
{ yyval.bool = ConstructBooleanOr( yyvsp[-2].num, yyvsp[0].num, TC_GREATERV, TC_EQUALV ); ;
    break;}
case 52:
#line 160 "agi.y"
{ yyval.bool = ConstructBooleanOr( yyvsp[-2].num, yyvsp[0].num, TC_LESSN, TC_EQUALN ); ;
    break;}
case 53:
#line 161 "agi.y"
{ yyval.bool = ConstructBooleanOr( yyvsp[-2].num, yyvsp[0].num, TC_LESSV, TC_LESSV ); ;
    break;}
case 54:
#line 162 "agi.y"
{ yyval.bool = bool = BooleanCommand(); bCommand = bool->data[0] = FindTestCommand( yyvsp[-1].text ); args = 0; argstype = 0; free(yyvsp[-1].text); ;
    break;}
case 55:
#line 163 "agi.y"
{ yyval.bool = yyvsp[-2].bool; bool->length = args+1; if (bCommand != 0) TypeCheckTestCommand( bCommand, args, argstype ); ;
    break;}
case 56:
#line 164 "agi.y"
{ yyval.bool = bool = BooleanCommand(); bool->data[0] = TC_SAID; args = 0; ;
    break;}
case 57:
#line 165 "agi.y"
{ yyval.bool = yyvsp[-2].bool; bool->data[1] = args; bool->length = 2*args + 2; ;
    break;}
case 61:
#line 173 "agi.y"
{ WORD w; w = GetTokenIndex(yyvsp[0].text); free(yyvsp[0].text); args++; bool->data[2*args] = w&0xFF; bool->data[2*args+1] = (w<<8) & 0xFF; ;
    break;}
case 65:
#line 181 "agi.y"
{ bool->data[args+1] = yyvsp[0].num; argstype |= 1 << (7-args); args++; ;
    break;}
case 66:
#line 182 "agi.y"
{ bool->data[args+1] = yyvsp[0].num; args++; ;
    break;}
case 67:
#line 183 "agi.y"
{ bool->data[args+1] = yyvsp[0].num; args++; ;
    break;}
case 68:
#line 184 "agi.y"
{ bool->data[args+1] = GetObjectIndex(yyvsp[0].text); args++; ;
    break;}
case 69:
#line 185 "agi.y"
{ bool->data[args+1] = GetStringID( yyvsp[0].text ); args++; ;
    break;}
case 73:
#line 193 "agi.y"
{ Output( yyvsp[0].num ); argstype |= 1 << (7-args); args++; ;
    break;}
case 74:
#line 194 "agi.y"
{ Output( yyvsp[0].num ); args++; ;
    break;}
case 75:
#line 195 "agi.y"
{ Output( GetObjectIndex(yyvsp[0].text) ); args++; ;
    break;}
case 76:
#line 196 "agi.y"
{ Output( yyvsp[0].num ); args++; ;
    break;}
case 77:
#line 197 "agi.y"
{ Output( GetStringID( yyvsp[0].text ) ); args++; ;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 543 "/usr/share/bison/bison.simple"

  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YYLSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = yylloc.first_line;
      yylsp->first_column = yylloc.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
	  for (x = (yyn < 0 ? -yyn : 0);
	       x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (yyn < 0 ? -yyn : 0);
		       x < (sizeof(yytname) / sizeof(char *)); x++)
		    if (yycheck[x + yyn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, yytname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      yyerror(msg);
	      free(msg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror("parse error");
    }

  goto yyerrlab1;
yyerrlab1:   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

yyerrhandle:

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;

 yyacceptlab:
  /* YYACCEPT comes here.  */
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
  return 0;

 yyabortlab:
  /* YYABORT comes here.  */
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
  return 1;
}
#line 202 "agi.y"


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
