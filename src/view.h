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

/**
 * AGI view resource structure.
 */
struct agi_view {
	int num_loops;
	struct view_loop *loop;
	char *descr;
	UINT8 *rdata;
};

/**
 * AGI view table entry
 */
struct vt_entry {
	UINT8		step_time;
	UINT8		step_time_count;
	UINT8		entry;
	SINT16		x_pos;
	SINT16		y_pos;
	UINT8		current_view;
	struct agi_view	*view_data;
	UINT8		current_loop;
	UINT8		num_loops;
	struct view_loop *loop_data;
	UINT8		current_cel;
	UINT8		num_cels;
	struct view_cel	*cel_data;
	struct view_cel	*cel_data_2;
	SINT16		x_pos2;
	SINT16		y_pos2;
	void		*s;
	SINT16		x_size;
	SINT16		y_size;
	UINT8		step_size;
	UINT8		cycle_time;
	UINT8		cycle_time_count;
	UINT8		direction;

#define MOTION_NORMAL		0
#define MOTION_WANDER		1
#define	MOTION_FOLLOW_EGO	2
#define	MOTION_MOVE_OBJ		3
	UINT8		motion;

#define	CYCLE_NORMAL		0
#define CYCLE_END_OF_LOOP	1
#define	CYCLE_REV_LOOP 		2
#define	CYCLE_REVERSE		3
	UINT8		cycle;

	UINT8		priority;

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
#define UPDATE_POS	0x0400
#define ON_LAND		0x0800
#define DONTUPDATE	0x1000
#define FIX_LOOP	0x2000
#define DIDNT_MOVE	0x4000
#define	ADJ_EGO_XY	0x8000
	UINT16		flags;

	UINT8		parm1;
	UINT8		parm2;
	UINT8		parm3;
	UINT8		parm4;
}; /* struct vt_entry */

#define for_each_vt_entry(x) \
	for (x = game.view_table; (x) < &game.view_table[MAX_VIEWTABLE]; (x)++)
#define if_is_ego_view(x) \
	if ((x) == game.view_table)

/* Motion */
void    check_all_motions (void);
void    move_obj	(struct vt_entry *);
void	in_destination	(struct vt_entry *);
void	fix_position	(int);
void	update_position	(void);

/* View table management */
void	set_cel		(struct vt_entry *, int);
void	set_loop	(struct vt_entry *, int);
void	set_view	(struct vt_entry *, int);
void	start_update	(struct vt_entry *);
void	stop_update	(struct vt_entry *);
void	update_viewtable(void);

void	unload_view	(int);
int	decode_view	(int);
void	add_to_pic	(int, int, int, int, int, int, int);
void	draw_obj	(int);

#ifdef __cplusplus
};
#endif

#endif /* __AGI_VIEW_H */
