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

#include "agicodes.h"

/* I think we should call collections of resources SAR files like
   TAR files on Unix...I think that's pretty clever */
#define NEW_FILETYPE	"Sierra Adventure Resource -- SAR"

#define PROGRAM_NAME "AGI Logic Compiler version 1.42\nCopyright 1997, Floating Hills Software.\n\n"
#define FREE_BANNER "This is free software, and you are welcome to redistribute\nit under certain conditions -- see COPYING for details.\n\n"

#ifndef HAVE_STRICMP
#define stricmp strcasecmp
#endif

#define TOK_FILE	"WORDS.TOK"
#define OBJ_FILE	"OBJECT"

#define SUCCESS		1
#define TRUE		1

#define FAILURE		0
#define FALSE		0

#define INITIAL_STRING_BUFFER		256

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned short PTR;
typedef BYTE BOOL;

#define STRCMP(x,y)		(stricmp((x),(y)))

/*** From agigoto.c ***/
void OutputLabel(char *sz);
void ResolveLabel(char *sz);
void PrintUnresolvedLabels();
void FreeLabels();

/*** From agicore.c ***/
int InitCore(void);
void DumpCore(FILE * file);
int FreeCore(void);

void Output(BYTE b);
void SetCoreByte(PTR ptr, BYTE b);
void OutputWord(WORD b);
void SetCoreWord(PTR ptr, WORD b);

PTR GetPC(void);

/*** From agic.lex ***/
int GetCurrentLine(void);
char *GetCurrentFile(void);

/*** From agistr.c ***/
void AddString(BYTE bID, char *sz);
void DumpStrings(FILE * file);
BYTE GetStringID(char *sz);
void InitStrings(void);
void FreeStrings(void);

/*** From agiavis.c ***/
void EncryptBlock(char *buffer, int len);

/*** From agiobj.c ***/
void LoadObjects(char *szFilename);
void FreeObjects(void);
char *GetObjectName(BYTE b);
BYTE GetObjectIndex(char *sz);

/*** From agitok.c ***/
void LoadTokens(char *szFilename);
void FreeTokens(void);
WORD GetTokenIndex(char *sz);

/*** From agifunc.c ***/
BYTE FindAGICommand(char *sz);
BYTE FindTestCommand(char *sz);
void TypeCheckAGICommand( BYTE bCommand, int nArgs, int nArgType );
void TypeCheckTestCommand( BYTE bCommand, int nArgs, int nArgType );
