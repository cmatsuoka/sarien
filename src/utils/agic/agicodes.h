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

/*
   ** agicodes.h
 */

#ifndef _agicodes_h_
#define _agicodes_h_

/* TC = Test Commands */
/* AC = AGI Commands */
/* HC = High Commands */

#define	TC_EQUALN		1
#define	TC_EQUALV		2
#define	TC_LESSN		3
#define	TC_LESSV		4
#define	TC_GREATERN		5
#define	TC_GREATERV		6
#define TC_HAS			9
#define TC_SAID			14
#define TC_ISSET		7

#define AC_INC			1
#define AC_DEC			2
#define AC_ASSIGNN		3
#define AC_ASSIGNV		4
#define AC_ADDN			5
#define AC_ADDV			6
#define AC_SUBN			7
#define AC_SUBV			8

#define AC_LINDIRECTV	9
#define AC_RINDIRECT	10
#define AC_LINDIRECTN	11

#define AC_MULN			164
#define AC_MULV			165
#define AC_DIVN			166
#define AC_DIVV			167

#define HC_IF			0xFF
#define HC_GOTO			0xFE
#define HC_ELSE			0xFE
#define HC_NOT			0xFD
#define HC_OR			0xFC

#endif				/* _agicodes_h_ */
