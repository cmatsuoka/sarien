/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#include "sarien.h"
#include "agi.h"
#include "rand.h"

static int dir_table[9] = {
	8, 1, 2,
        7, 0, 3,
	6, 5, 4
};

static int check_step (int delta, int step)
{
	return (-step >= delta) ? 0 : (step <= delta) ? 2 : 1;
}

static int get_direction (int x, int y, int x0, int y0, int s)
{
	return dir_table [check_step(x0 - x, s) + 3 * check_step(y0 - y, s)];
}

static int check_block (int x, int y)
{
	if (x <= game.block.x1 || x >= game.block.x2)
		return FALSE;

	if (y <= game.block.y1 || y >= game.block.y2)
		return FALSE;

	return TRUE;
}

static void changepos (struct vt_entry *v)
{
	int b, x, y;

	x = v->x_pos;
	y = v->y_pos;
	b = check_block (x, y);

	switch (v->direction) {
	case 1:
		y -= v->step_size;
		break;
	case 2:
		x += v->step_size;
		y -= v->step_size;
		break;
	case 3:
		x += v->step_size;
		break;
	case 4:
		x += v->step_size;
		y += v->step_size;
		break;
	case 5:
		y += v->step_size;
		break;
	case 6:
		x -= v->step_size;
		y += v->step_size;
		break;
	case 7:
		x -= v->step_size;
		break;
	case 8:
		x -= v->step_size;
		y -= v->step_size;
		break;
	}

	if (check_block (x, y) == b) {
		v->flags &= ~MOTION;
	} else {
		v->flags |= MOTION;
		v->direction = 0;
		if_is_ego_view (v)
			game.vars[V_ego_dir] = 0;
	}
}

static void motion_wander (struct vt_entry *v)
{
	if (v->parm1--) {
		if (~v->flags & DIDNT_MOVE)
			return;
	} 

	v->direction = rnd (9);
	
	if_is_ego_view (v) {
		game.vars[V_ego_dir] = v->direction;
		while (v->parm1 < 6) {
			v->parm1 = rnd (51);	/* huh? */
		}
	}
}

static void motion_followego (struct vt_entry *v)
{
	int ego_x, ego_y;
	int obj_x, obj_y;
	int dir;
	int k;

	ego_x = game.view_table[0].x_pos + game.view_table[0].x_size / 2;
	ego_y = game.view_table[0].y_pos;

	obj_x = v->x_pos + v->x_size / 2;
	obj_y = v->y_pos;

	/* Get direction to reach ego */
	dir = get_direction (obj_x, obj_y, ego_x, ego_y, v->parm1);

	/* Already at ego coordinates */
	if (dir == 0) {
		v->direction = 0;
		v->motion = MOTION_NORMAL;
		setflag (v->parm2, TRUE);
		return;
	}

	if (v->parm3 == 0xff) {
		v->parm3 = 0;
	} else if (v->flags & DIDNT_MOVE) {
		int d;

		while ((v->direction = rnd (9)) == 0);

		d = (abs (ego_y - obj_y) + abs (ego_x - obj_x)) / 2 + 1;

		if (d <= v->step_size) {
			v->parm3 = v->step_size;
			return;
		}

		while ((v->parm3 = rnd (d)) < v->step_size);
		return;
	}

	if (v->parm3 != 0) {

		/* this is ugly and I dont know why this works, but other line
		   does not! */

		k=v->parm3;
		k-=v->step_size;
		v->parm3=k;

		if((SINT8)v->parm3<0)
		/* DF : watcom complained about lvalue */
		/* if ( ((SINT8)v->parm3 -= v->step_size) < 0) */
			v->parm3 = 0;
	} else {
		v->direction = dir;
	}
}

static void motion_moveobj (struct vt_entry *v)
{
	v->direction = get_direction (v->x_pos, v->y_pos, v->parm1,
		v->parm2, v->step_size);

	/* Update V6 if ego */
	if_is_ego_view (v)
		game.vars[V_ego_dir] = v->direction;

	if (v->direction == 0)
		in_destination (v);
}

static void check_motion (struct vt_entry *v)
{
	switch (v->motion) {
	case MOTION_WANDER:
		motion_wander (v);
		break;
	case MOTION_FOLLOW_EGO:
		motion_followego (v);
		break;
	case MOTION_MOVE_OBJ:
		motion_moveobj (v);
		break;
	}

	if ((!game.block.active || ~v->flags & IGNORE_BLOCKS) && v->direction)
		changepos (v);
}

 
/*
 * Public functions
 */

/**
 *
 */
void check_all_motions ()
{
	struct vt_entry *v;

	for_each_vt_entry (v) {
		if ((v->flags & (ANIMATED|UPDATE|DRAWN)) ==
			(ANIMATED|UPDATE|DRAWN) && v->step_time_count == 1)
		{
			check_motion (v);
		}
      }
}

/**
 *
 */
void in_destination (struct vt_entry *v)
{
	v->step_size = v->parm3;
	setflag (v->parm4, TRUE);
	v->motion = MOTION_NORMAL;
	if_is_ego_view (v)
		game.player_control = TRUE;
}

/**
 * Wrapper for static function motion_moveobj().
 * This function is used by cmd_move_object() for the first motion cycle
 * after setting the motion mode to MOTION_MOVE_OBJ.
 */
void move_obj (struct vt_entry *v)
{
	motion_moveobj (v);
}

/* end: motion.c */

