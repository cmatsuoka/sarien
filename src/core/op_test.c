/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#include <stdio.h>
#include <string.h>

#include "sarien.h"
#include "agi.h"
#include "keyboard.h"
#include "opcodes.h"
#include "view.h"
#include "logic.h"

extern struct agi_game game;
extern struct agi_logic logics[];
extern struct agi_view views[];
extern struct agi_view_table view_table[];

static UINT8	test_obj_right	(UINT8, UINT8, UINT8, UINT8, UINT8);
static UINT8	test_obj_centre	(UINT8, UINT8, UINT8, UINT8, UINT8);
static UINT8	test_obj_in_box	(UINT8, UINT8, UINT8, UINT8, UINT8);
static UINT8	test_posn	(UINT8, UINT8, UINT8, UINT8, UINT8);
static UINT8	test_said	(UINT8, UINT8 *);
static UINT8	test_controller	(UINT8);
static UINT8	test_keypressed	(void);

#define ip (logics[lognum].cIP)
#define code (logics[lognum].data)

#define test_equal(v1,v2)	(getvar(v1) == (v2))
#define test_less(v1,v2)	(getvar(v1) < (v2))
#define test_greater(v1,v2)	(getvar(v1) > (v2))
#define test_isset(flag)	(getflag (flag))
#define test_has(obj)		(object_get_location (obj) == EGO_OWNED)
#define test_obj_in_room(obj,v)	(object_get_location (obj) == getvar (v))
#define test_compare_strings(s1,s2) (!strcmp(game.strings[s1], game.strings[s2]))


static UINT8 test_keypressed ()
{
	int x = game.keypress;

	game.keypress = 0;
	if (!x) main_cycle ();

	return x;
}


static UINT8 test_controller (UINT8 cont)
{
	int r;

	r = game.events[cont].occured;
	game.events[cont].occured = FALSE;

	return r;
}


static UINT8 test_posn (UINT8 cel, UINT8 x1, UINT8 y1, UINT8 x2, UINT8 y2)
{
	return view_table[cel].x_pos >= x1 &&
		view_table[cel].y_pos >= y1 &&
		view_table[cel].x_pos <= x2 &&
		view_table[cel].y_pos <= y2;
}


static UINT8 test_obj_in_box (UINT8 obj, UINT8 x1, UINT8 y1, UINT8 x2, UINT8 y2)
{
	return view_table[obj].x_pos >= x1 &&
		view_table[obj].y_pos >= y1 &&
		(view_table[obj].x_pos + VT_WIDTH(view_table[obj]) - 1) <= x2 &&
		view_table[obj].y_pos <= y2;
}


/* if obj is in centre of box */
static UINT8 test_obj_centre (UINT8 obj, UINT8 x1, UINT8 y1, UINT8 x2, UINT8 y2)
{
	return (view_table[obj].x_pos + VT_WIDTH(view_table[obj]) / 2) >= x1 &&
		(view_table[obj].x_pos + VT_WIDTH(view_table[obj]) / 2) <= x2 &&
		view_table[obj].y_pos >= y1 &&
		view_table[obj].y_pos <= y2;
}


/* if object N is in right corner */
static UINT8 test_obj_right(UINT8 obj, UINT8 x1, UINT8 y1, UINT8 x2, UINT8 y2)
{
	return (view_table[obj].x_pos + VT_WIDTH(view_table[obj]) - 1) >= x1 &&
		(view_table[obj].x_pos + VT_WIDTH(view_table[obj]) - 1) <= x2 &&
		view_table[obj].y_pos >= y1 &&
		view_table[obj].y_pos <= y2;
}


/* When player has entered something, it is parsed elsewhere */
static UINT8 test_said (UINT8 nwords, UINT8 *cc)
{
	int c, n = game.num_ego_words;
	UINT16 z = 0;

	if (!getflag (F_entered_cli))
		return FALSE;

	/* FR:
	 * I think the reason for the code below is to add some speed....
	 *
	 *	if (nwords != num_ego_words)
	 *		return FALSE;
	 *
	 * In the disco scene in Larry 1 when you type "examine blonde", 
	 * inside the logic is expected ( said("examine", "blonde", "rol") )
	 * where word("rol") = 9999
	 *
	 * According to the interpreter code 9999 means that whatever the
	 * user typed should be correct, but looks like code 9999 means that
	 * if the string is empty at this point, the entry is also correct...
	 * 
	 * With the removal of this code, the behaviour of the scene was
	 * corrected
	 */ 
    	
	for (c = 0; nwords && n; c++, nwords--, n--) {
		z = lohi_getword (cc);
		cc += 2;

		switch (z) {
		case 9999:	/* rest of line (empty string counts to...) */
			nwords=1;
			break;
		case 1:			/* any word */
			break;
		default:
			if (game.ego_words[c].id != z)
				return FALSE;
				break;
			}
	}

	/* The entry string should be entirely parsed, or last word = 9999 */
	if ( ( n ) && ( z != 9999 ) )
		return FALSE;

	/* The interpreter string shouldn't be entirely parsed, but next
	 * word must be 9999
	 */
	if ( (nwords != 0) && (lohi_getword (cc) != 9999) )    
		return FALSE;

	setflag (F_said_accepted_input, TRUE);

	/* FR :
	 * Hack  (Fixes some bugs in larry 1)
	 */
	setflag (F_entered_cli, FALSE);

	return TRUE;
}


int test_if_code (int lognum)
{
	int ec = TRUE, retval = TRUE;
	UINT8	op;
	UINT8	not_test = FALSE;
	UINT8	or_test = FALSE;
	UINT16	last_ip = ip;
	UINT8	p[16];

	while (retval && !game.quit_prog_now) {
#ifdef USE_CONSOLE
		if (debug.enabled && (debug.logic0 || lognum))
			debug_console (lognum, lTEST_MODE, NULL);
#endif

		last_ip = ip;
		op = *(code + ip++);
		memmove (&p, (code + ip), 16);

		switch(op)
		{
		case 0xFF:			/* END IF, TEST TRUE */
			goto end_test;
		case 0xFD:
			not_test = !not_test;
			continue;
		case 0xFC:				/* OR */
			/* if or_test is ON and we hit 0xFC, end of OR, then
			 * or is STILL false so break.
			 */
			if (or_test) {
				ec = FALSE;
				retval = FALSE;
				goto end_test;
			}

			or_test = TRUE;
			continue;

		case 0x00:
			/* return true? */
			goto end_test;
		case 0x01:
			ec = test_equal (p[0], p[1]);
			break;
		case 0x02:
			ec = test_equal (p[0], getvar(p[1]));
			break;
		case 0x03:
			ec = test_less (p[0], p[1]);
			break;
		case 0x04:
			ec = test_less(p[0], getvar(p[1]));
			break;
		case 0x05:
			ec = test_greater (p[0], p[1]);
			break;
		case 0x06:
			ec = test_greater (p[0], getvar (p[1]));
			break;
		case 0x07:
			ec = test_isset (p[0]);
			break;
		case 0x08:
			ec = test_isset (getvar(p[0]));
			break;
		case 0x09:
			ec = test_has (p[0]);
			break;
		case 0x0A:
			ec = test_obj_in_room (p[0], p[1]);
			break;
		case 0x0B:
			ec = test_posn (p[0], p[1], p[2], p[3], p[4]);
			break;
		case 0x0C:
			ec = test_controller (p[0]);
			break;
		case 0x0D:
			ec = test_keypressed ();
			break;
		case 0x0E:
			ec = test_said (p[0], (UINT8*)code + (ip + 1));
			ip = last_ip;
			ip++;		/* skip opcode */
			ip+=p[0]*2;	/* skip num_words*2 */
			ip++;		/* skip num_words opcode */
			break;
		case 0x0F:
			_D(_D_WARN "comparing [%s], [%s]",
				game.strings[p[0]], game.strings[p[1]]);
			ec = test_compare_strings (p[0], p[1]);
			break;
		case 0x10:
			ec = test_obj_in_box (p[0], p[1], p[2], p[3], p[4]);
			break;
		case 0x11:
			ec = test_obj_centre (p[0], p[1], p[2], p[3], p[4]);
			break;
		case 0x12:
			ec = test_obj_right (p[0], p[1], p[2], p[3], p[4]);
			break;
		default:
			ec = FALSE;
			goto end_test;
		}

		if (op <= 0x12)
			ip += logic_names_test[op].num_args;

		/* exchange ec value */
		if (not_test)
			ec = !ec;

		/* not is only enabled for 1 test command */
		not_test = FALSE;

		if (or_test && ec) {
			/* a TRUE inside an OR statement passes
			 * ENTIRE statement scan for end of OR
			 */

		/* CM: test for opcode < 0xfc changed from 'op' to
	 	 *     '*(code+ip)', to avoid problem with the 0xfd (NOT)
		 *     opcode byte. Changed a bad ip += ... ip++ construct.
		 *     This should fix the crash with Larry's logic.0 code:
		 *
		 *     if ((isset(4) ||
		 *          !isset(2) ||
		 *          v30 == 2 ||
                 *          v30 == 1)) {
		 *       goto Label1;
                 *     }
		 *
		 *     The bytecode is: 
		 *     ff fc 07 04 fd 07 02 01 1e 02 01 1e 01 fc ff
		 */

			/* find end of OR */
			while (*(code+ip) != 0xFC) {
				if (*(code + ip) == 0x0E) {	/* said */
					ip++;
					/* cover count + ^words */
					ip += 1 + ((*(code + ip)) * 2);
					continue;
				}

				if (*(code + ip) < 0xFC)
					ip += logic_names_test[*(code +
						ip)].num_args;
				ip++;
			}
			ip++;

			or_test = FALSE;
			retval = TRUE;
		} else {
			retval = or_test ? retval || ec : retval && ec;
		}
	}
end_test:

	/* if false, scan for end of IP? */
	if (retval)
		ip += 2;
	else {
		ip = last_ip;
		while (*(code + ip) != 0xff) {
			if (*(code + ip) == 0x0e) {
				ip++;
				ip += (*(code + ip)) * 2 + 1;
			} else if (*(code + ip) < 0xfc) {
				ip += logic_names_test[*(code + ip++)].num_args;
			} else {
				ip++;
			}
		}
		ip++;	/* skip over 0xFF */
		ip += lohi_getword (code + ip) + 2;
	}

#ifdef USE_CONSOLE
	if (debug.enabled && (debug.logic0 || lognum))
		debug_console (lognum, 0xFF, retval ? "=TRUE" : "=FALSE");
#endif

	return retval;
}

