/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#ifndef __AGI_VIEW_H
#define __AGI_VIEW_H

#ifdef __cplusplus
extern "C"{
#endif

struct view_cel {
	UINT8	height;
	UINT8	width;
	UINT8	transparency;
	UINT8	mirror_loop;
	UINT8	mirror;
	UINT8	*data;
};

struct view_loop {
	int num_cels;
	struct view_cel *cel;
};

struct agi_view {
	int num_loops;
	struct view_loop *loop;
	char *descr;
	UINT8 *rdata;
};

struct agi_view_table {
	UINT8		x_pos;
	UINT8		y_pos;

	UINT8		*bg_scr;
	UINT8		*bg_pri;
	UINT16		bg_x;
	UINT16		bg_y;
	UINT16		bg_x_size;
	UINT16		bg_y_size;

	UINT8		direction;
	UINT8		motion;
	UINT8		priority;

	UINT8		step_time;
	UINT8		step_size;
	UINT8		step_time_count;

	UINT8		cycle_time;
	UINT8		cycle_status;
	UINT8		cycle_time_count;

#define VT_VIEW(x)   views[(x).current_view]  
#define VT_LOOP(x)   VT_VIEW(x).loop[(x).current_loop]
#define VT_CEL(x)    VT_LOOP(x).cel[(x).current_cel]
#define VT_WIDTH(x)  VT_CEL(x).width
#define VT_HEIGHT(x) VT_CEL(x).height

	UINT8		current_view;
	UINT8		current_loop;
	UINT8		current_cel;

#define DRAWN		0x0001
#define IGNORE_BLOCKS	0x0002
#define FIXED_PRIORITY	0x0004
#define IGNORE_HORIZON	0x0008
#define UPDATE		0x0010
#define CYCLING		0x0020
#define ANIMATED	0x0040
#define MOTION		0x0080
#define ON_WATER	0x0100
#define IGNORE_OBJECTS	0x0200
#define ON_LAND		0x0800
#define FIX_LOOP	0x2000
	UINT16		flags;

	UINT8		parm1;
	UINT8		parm2;
	UINT8		parm3;
	UINT8		parm4;
};

struct view_list {
	struct view_list *up;
	struct view_list *down;
	struct agi_view_table *vt;
};


void	init_view_table	(void);
void	unload_view	(int);
int	decode_view	(int);
void	add_view_table	(int, int);
void	set_loop	(int, int);
void	set_cel		(int, int);
void	add_to_pic	(int, int, int, int, int, int, int);
void	reset_views	(void);
void	reset_view	(int);
void	calc_direction	(int);
void	draw_obj	(int);

#ifdef __cplusplus
};
#endif

#endif

