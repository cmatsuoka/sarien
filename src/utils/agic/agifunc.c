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

/* AGI Function Symbol management */
/* Author: Jewel of Mars */

/* Change log:
 *
 * JAF 12/26/97 - Replaced centre.posn with center.posn in function list.
 *                Added working type checking
 *                Added the missing command return
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "agic.h"
#include "agierr.h"

#define MAX_NAME_LENGTH    20
#define NUM_TEST_COMMANDS  19
#define NUM_AGI_COMMANDS   169

typedef struct {
    char commandName[MAX_NAME_LENGTH];
    int numArgs;
    int argTypeMask;
} agiCommandType;

const agiCommandType testCommands[NUM_TEST_COMMANDS] =
{
    {"", 0, 0x00},
    {"equaln", 2, 0x80},
    {"equalv", 2, 0xC0},
    {"lessn", 2, 0x80},
    {"lessv", 2, 0xC0},
    {"greatern", 2, 0x80},
    {"greaterv", 2, 0xC0},
    {"isset", 1, 0x00},
    {"issetv", 1, 0x80},
    {"has", 1, },
    {"obj.in.room", 2, 0x00},
    {"posn", 5, 0x00 },
    {"controller", 1, 0x00},
    {"have.key", 0, 0x00},
    {"said", 5, 0x00},
    {"compare.strings", 2, 0x00},
    {"obj.in.box", 5, 0x00},
    {"center.posn", 5, 0x00},
    {"right.posn", 5, 0x00}
};

const agiCommandType agiCommands[NUM_AGI_COMMANDS] =
{
    {"return", 0, 0x00},
    {"increment", 1, 0x80},
    {"decrement", 1, 0x80},
    {"assignn", 2, 0x80},
    {"assignv", 2, 0xC0},
    {"addn", 2, 0x80},
    {"addv", 2, 0xC0},
    {"subn", 2, 0x80},
    {"subv", 2, 0xC0},
    {"lindirectv", 2, 0xC0},
    {"rindirect", 2, 0xC0},
    {"lindirectn", 2, 0x80},
    {"set", 1, 0x00},
    {"reset", 1, 0x00},
    {"toggle", 1, 0x00},
    {"set.v", 1, 0x80},
    {"reset.v", 1, 0x80},
    {"toggle.v", 1, 0x80},
    {"new.room", 1, 0x00},
    {"new.room.v", 1, 0x80},
    {"load.logics", 1, 0x00},
    {"load.logics.v", 1, 0x80},
    {"call", 1, 0x00},
    {"call.v", 1, 0x80},
    {"load.pic", 1, 0x80},
    {"draw.pic", 1, 0x80},
    {"show.pic", 0, 0x00},
    {"discard.pic", 1, 0x80},
    {"overlay.pic", 1, 0x80},
    {"show.pri.screen", 0, 0x00},
    {"load.view", 1, 0x00},
    {"load.view.v", 1, 0x80},
    {"discard.view", 1, 0x00},
    {"animate.obj", 1, 0x00},
    {"unanimate.all", 0, 0x00},
    {"draw", 1, 0x00},
    {"erase", 1, 0x00},
    {"position", 3, 0x00},
    {"position.v", 3, 0x60},
    {"get.posn", 3, 0x60},
    {"reposition", 3, 0x60},
    {"set.view", 2, 0x00},
    {"set.view.v", 2, 0x40},
    {"set.loop", 2, 0x00},
    {"set.loop.v", 2, 0x40},
    {"fix.loop", 1, 0x00},
    {"release.loop", 1, 0x00},
    {"set.cel", 2, 0x00},
    {"set.cel.v", 2, 0x40},
    {"last.cel", 2, 0x40},
    {"current.cel", 2, 0x40},
    {"current.loop", 2, 0x40},
    {"current.view", 2, 0x40},
    {"number.of.loops", 2, 0x40},
    {"set.priority", 2, 0x00},
    {"set.priority.v", 2, 0x40},
    {"release.priority", 1, 0x00},
    {"get.priority", 2, 0x40},
    {"stop.update", 1, 0x00},
    {"start.update", 1, 0x00},
    {"force.update", 1, 0x00},
    {"ignore.horizon", 1, 0x00},
    {"observe.horizon", 1, 0x00},
    {"set.horizon", 1, 0x00},
    {"object.on.water", 1, 0x00},
    {"object.on.land", 1, 0x00},
    {"object.on.anything", 1, 0x00},
    {"ignore.objs", 1, 0x00},
    {"observe.objs", 1, 0x00},
    {"distance", 3, 0x20},
    {"stop.cycling", 1, 0x00},
    {"start.cycling", 1, 0x00},
    {"normal.cycle", 1, 0x00},
    {"end.of.loop", 2, 0x00},
    {"reverse.cycle", 1, 0x00},
    {"reverse.loop", 2, 0x00},
    {"cycle.time", 2, 0x40},
    {"stop.motion", 1, 0x00},
    {"start.motion", 1, 0x00},
    {"step.size", 2, 0x40},
    {"step.time", 2, 0x40},
    {"move.obj", 5, 0x00},
    {"move.obj.v", 5, 0x70},
    {"follow.ego", 3, 0x00},
    {"wander", 1, 0x00},
    {"normal.motion", 1, 0x00},
    {"set.dir", 2, 0x40},
    {"get.dir", 2, 0x40},
    {"ignore.blocks", 1, 0x00},
    {"observe.blocks", 1, 0x00},
    {"block", 4, 0x00},
    {"unblock", 0, 0x00},
    {"get", 1, 00},
    {"get.v", 1, 0x80},
    {"drop", 1, 0x00},
    {"put", 2, 0x00},
    {"put.v", 2, 0x40},
    {"get.room.v", 2, 0xC0},
    {"load.sound", 1, 0x00},
    {"sound", 2, 00},
    {"stop.sound", 0, 0x00},
    {"print", 1, 00},
    {"print.v", 1, 0x80},
    {"display", 3, 0x00},
    {"display.v", 3, 0xE0},
    {"clear.lines", 3, 0x00},
    {"text.screen", 0, 0x00},
    {"graphics", 0, 0x00},
    {"set.cursor.char", 1, 0x00},
    {"set.text.attribute", 2, 0x00},
    {"shake.screen", 1, 0x00},
    {"configure.screen", 3, 0x00},
    {"status.line.on", 0, 0x00},
    {"status.line.off", 0, 0x00},
    {"set.string", 2, 0x00},
    {"get.string", 5, 0x00},
    {"word.to.string", 2, 0x00},
    {"parse", 1, 0x00},
    {"get.num", 2, 0x40},
    {"prevent.input", 0, 0x00},
    {"accept.input", 0, 0x00},
    {"set.key", 3, 0x00},
    {"add.to.pic", 7, 0x00},
    {"add.to.pic.v", 7, 0xFE},
    {"status", 0, 0x00},
    {"save.game", 0, 0x00},
    {"restore.game", 0, 0x00},
    {"init.disk", 0, 0x00},
    {"restart.game", 0, 0x00},
    {"show.obj", 1, 0x00},
    {"random", 3, 0x20},
    {"program.control", 0, 0x00},
    {"player.control", 0, 0x00},
    {"obj.status.v", 1, 0x80},
    {"quit", 1, 0x00},
    {"show.mem", 0, 0x00},
    {"pause", 0, 0x00},
    {"echo.line", 0, 0x00},
    {"cancel.line", 0, 0x00},
    {"init.joy", 0, 0x00},
    {"toggle.monitor", 0, 0x00},
    {"version", 0, 0x00},
    {"script.size", 1, 0x00},
    {"set.game.id", 1, 0x00},
    {"log", 1, 0x00},
    {"set.scan.start", 0, 0x00},
    {"reset.scan.start", 0, 0x00},
    {"reposition.to", 3, 0x00},
    {"reposition.to.v", 3, 0x60},
    {"trace.on", 0, 0x00},
    {"trace.info", 3, 0x00},
    {"print.at", 4, 0x00},
    {"print.at.v", 4, 0x80},
    {"discard.view.v", 1, 0x80},
    {"clear.text.rect", 5, 0x00},
    {"set.upper.left", 2, 0x00},
    {"set.menu", 1, 0x00},
    {"set.menu.item", 2, 0x00},
    {"submit.menu", 0, 0x00},
    {"enable.item", 1, 0x00},
    {"disable.item", 1, 0x00},
    {"menu.input", 0, 0x00},
    {"show.obj.v", 1, 0x80},
    {"open.dialogue", 0, 0x00},
    {"mul.n", 2, 0x80},
    {"mul.v", 2, 0xC0},
    {"div.n", 2, 0x80},
    {"div.v", 2, 0xC0},
    {"close.window", 0, 0x00}
};

static void TypeCheckList( const agiCommandType* list, BYTE bCommand, int nArgs, int nArgType );

BYTE FindAGICommand(char *sz)
{
    int i;

    for (i = 0; i < NUM_AGI_COMMANDS; i++)
	if (STRCMP(agiCommands[i].commandName, sz) == 0)
	    return i;

    CompilerFailure(CERR_BAD_FUNCTION, sz);

    return 0;
}

BYTE FindTestCommand(char *sz)
{
    int i;

    for (i = 0; i < NUM_TEST_COMMANDS; i++)
	if (STRCMP(testCommands[i].commandName, sz) == 0)
	    return i;

    CompilerFailure(CERR_BAD_FUNCTION, sz);

    return 0;
}

/* #define min(a,b)	( ((a)<(b)) ? (a) : (b) ) */

void TypeCheckList( const agiCommandType* list, BYTE bCommand, int nArgs, int nArgType )
	{
	char szBuffer[256];
	int i;

	if (list[bCommand].numArgs != nArgs)
		{
		sprintf( szBuffer, "%s expected %i", list[bCommand].commandName, list[bCommand].numArgs );
		CompilerFailure( CERR_BAD_ARG_NUM, szBuffer );
		}

	for( i=0; i<min(list[bCommand].numArgs, nArgs); i++ )
		{
		if ( (list[bCommand].argTypeMask & (1 << (7-i))) !=
			(nArgType & (1 << (7-i))) )
			{
			sprintf( szBuffer, "%s expects %s as arg %i", list[bCommand].commandName,
				((list[bCommand].argTypeMask & (1 << (7-i))) != 0) ? "var" : "non-var",
				i+1 );
			CompilerFailure( CERR_BAD_ARG_TYPE, szBuffer );
			}
		}

	return;
	}


void TypeCheckAGICommand( BYTE bCommand, int nArgs, int nArgType )
	{
	TypeCheckList( agiCommands, bCommand, nArgs, nArgType );
	}

void TypeCheckTestCommand( BYTE bCommand, int nArgs, int nArgType )
	{
	TypeCheckList( testCommands, bCommand, nArgs, nArgType );
	}
