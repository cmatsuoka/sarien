/*
 *  Sarien AGI :: Copyright (C) 1999 Dark Fiber
 *
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
 */

#ifndef __IBM_PORT
#define __IBM_PORT

#ifdef __cplusplus
extern "C"{
#endif

extern	UINT8	*exec_name;
extern	UINT8	*screen_buffer;
extern	volatile UINT32	clock_ticks;
extern	volatile UINT32 clock_count;
extern	volatile UINT32	msg_box_ticks;

extern	UINT16	init_machine(int, char **);
extern	UINT16	deinit_machine(void);
extern	UINT16	setup_machine(void);

extern	UINT8	is_keypress(void);
extern	UINT16	get_keypress(void);

#ifdef __cplusplus
};
#endif
#endif
