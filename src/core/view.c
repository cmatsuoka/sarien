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
#include <assert.h>
#include "sarien.h"
#include "agi.h"
#include "sprite.h"


static void mirror_cel (struct view_cel *vc)
{
	UINT8 *p;
	int x, y;

	p = vc->data;

	for (y = 0; y < vc->height; y++) {
		int z = vc->width - 1;
		int w = y * vc->width;

		x = 0;
		do {
			int temp = p[w + x];
			p[w + x] = p[w + z];
			p[w + z] = temp;
		} while (++x <= --z);
	}
}

static void decode_cel (struct view_cel *vc, UINT8 *data)
{
	int h, c, l, d;
	UINT8 *p;

	p = vc->data;
	memset (p, vc->transparency, vc->height * vc->width);

	if (vc->width == 0 || vc->height == 0)
		return;

	for (d = h = 0; h < vc->height; h++) {
		p = vc->data + (h * vc->width);

		while (42) {
			if ((l = data[d++]) == 0)
				break;
			c = (l >> 4) & 0xf;
			l = l & 0xf;
			memset (p, c, l);
			p += l;
		}
	}
}

static void _set_cel (struct vt_entry *v, int n)
{
	v->current_cel = n;
	v->cel_data = &v->loop_data->cel[n];
	v->x_size = v->cel_data->width;
	v->y_size = v->cel_data->height;
}

static void _set_loop (struct vt_entry *v, int n)
{
	_D (_D_WARN "vt entry #%d, loop = %d", v->entry, n);
	v->current_loop = n;
	v->loop_data = &game.views[v->current_view].loop[n];
	v->num_cels = v->loop_data->num_cels;
	if (v->current_cel >= v->num_cels)
		v->current_cel = 0;
}

static void update_view (struct vt_entry *v)
{
	int cel, last_cel;

	if (v->flags & DONTUPDATE) {
		v->flags &= ~DONTUPDATE;
		return;
	}

	cel = v->current_cel;
	last_cel = v->num_cels - 1;

	switch (v->cycle) {
	case CYCLE_NORMAL:
		if (++cel > last_cel)
			cel = 0;
		break;
	case CYCLE_END_OF_LOOP:
		if (cel < last_cel) {
			if (++cel != last_cel)
				break;
			setflag (v->parm1, TRUE);
			v->flags &= ~CYCLING;
			v->direction = 0;
			v->cycle = CYCLE_NORMAL;
		}
		break;
	case CYCLE_REV_LOOP:
		if (cel == 0) {
			setflag (v->parm1, TRUE);
			v->flags &= ~CYCLING;
			v->direction = 0;
			v->cycle = CYCLE_NORMAL;
		} else {
			cel--;
		}
		break;
	case CYCLE_REVERSE:
		if (cel == 0) {
			cel = last_cel;
		} else {
			cel--;
		}
		break;
	}

	set_cel (v, cel);
}

/*
 * Public functions
 */

/**
 * Decode an AGI view resource.
 * This function decodes the raw data of the specified AGI view resource
 * and fills the corresponding views array element.
 * @param n number of view resource to decode
 */
int decode_view (int n)
{
	int loop, cel;
	UINT8 *v, *lptr;
	UINT16 lofs, cofs;
	struct view_loop *vl;
	struct view_cel	*vc;

	_D ("(%d)", n);
	v = game.views[n].rdata;

	assert (v != NULL);

	game.views[n].descr = lohi_getword (v + 3) ?
		strdup ((char*)v + lohi_getword (v + 3)) : strdup ("");

	/* if no loops exist, return! */
	if ((game.views[n].num_loops = lohi_getbyte (v + 2)) == 0)
		return err_NoLoopsInView;

	/* allocate memory for all views */
	game.views[n].loop =
		calloc (game.views[n].num_loops, sizeof(struct view_loop));

	if (game.views[n].loop == NULL)
		return err_NotEnoughMemory;

	/* decode all of the loops in this view */
	lptr = v + 5;		/* first loop address */

	for (loop = 0; loop < game.views[n].num_loops; loop++, lptr += 2) {
		lofs = lohi_getword (lptr);	/* loop header offset */
		vl = &game.views[n].loop[loop];	/* the loop struct */

		vl->num_cels = lohi_getbyte (v + lofs);
		_D(_D_WARN "view %d, num_cels = %d", n, vl->num_cels);
		vl->cel = calloc (vl->num_cels, sizeof (struct view_cel));
		if (vl->cel == NULL) {
			free (game.views[n].loop);
			game.views[n].num_loops = 0;
			return err_NotEnoughMemory;
		}

    		/* decode the cells */
    		for (cel = 0; cel < vl->num_cels; cel++) {
    			cofs = lofs + lohi_getword (v + lofs + 1 + (cel * 2));
    			vc = &vl->cel[cel];

             		vc->width = lohi_getbyte (v + cofs);
             		vc->height = lohi_getbyte (v + cofs + 1);
             		vc->transparency = lohi_getbyte (v + cofs + 2) & 0xf;
    			vc->mirror_loop = (lohi_getbyte (v + cofs + 2) >>4) & 0x7;
             		vc->mirror = (lohi_getbyte (v + cofs + 2) >> 7) & 0x1;

             		/* skip over width/height/trans|mirror data */
             		cofs += 3;

    			vc->data = malloc ((vc->height + 1) * (vc->width + 1));

    			if (vc->data == NULL) {
    				for (cel--; cel >= 0; cel--)
    					free (game.views[n].loop[loop].cel[cel].data);
    				for (; loop >= 0; loop--)
    					free(game.views[n].loop[loop].cel);

    				free (game.views[n].loop);
    				game.views[n].num_loops = 0;
    				return err_NotEnoughMemory;
    			}

			decode_cel (vc, v + cofs);

    			if (vc->mirror == 1 && vc->mirror_loop != loop)
    				mirror_cel (vc);
    		} /* cel */
	} /* loop */

	return err_OK;
}

/**
 * Unloads all data in a view resource
 * @param n number of view resource
 */
void unload_view (int n)
{
	int x, y;

	if (~game.dir_view[n].flags & RES_LOADED)
		return;

	/* free all the loops */
	for (x = 0; x < game.views[n].num_loops; x++) {
		for (y = 0; y < game.views[n].loop[x].num_cels; y++)
			if (game.views[n].loop[x].cel[y].data != NULL)
				free (game.views[n].loop[x].cel[y].data);

		if (game.views[n].loop[x].cel != NULL)
			free (game.views[n].loop[x].cel);
	}

	free (game.views[n].loop);
	free (game.views[n].descr);
	free (game.views[n].rdata);

	game.dir_view[n].flags &= ~RES_LOADED;
}

/**
 * Set a view table entry to use the specified cel of the current loop.
 * @param v pointer to view table entry
 * @param n number of cel
 */
void set_cel (struct vt_entry *v, int n)
{
	assert (v->view_data != NULL);
	assert (v->num_cels >= n);

	_set_cel (v, n);
	if (v->x_pos + v->x_size > _WIDTH) {
		v->flags |= FLAG10;
		v->x_pos = _WIDTH - v->x_size;
	}
	if (v->y_pos - v->y_size + 1 < 0) {
		v->flags |= FLAG10;
		v->y_pos = v->y_size - 1;
	}
	if (v->y_pos <= game.horizon && (~v->flags & IGNORE_HORIZON)) {
		v->y_pos = game.horizon + 1;
	}
}

/**
 * Set a view table entry to use the specified loop of the current view.
 * @param v pointer to view table entry
 * @param n number of loop
 */
void set_loop (struct vt_entry *v, int n)
{
	assert (v->view_data != NULL);
	assert (v->num_loops >= n);
	_set_loop (v, n);
	set_cel (v, v->current_cel);
}

/**
 * Set a view table entry to use the specified view resource.
 * @param v pointer to view table entry
 * @param n number of AGI view resource
 */
void set_view (struct vt_entry *v, int n)
{
	v->view_data = &game.views[n];
	v->current_view = n;
	v->num_loops = v->view_data->num_loops;
	set_loop (v, v->current_loop >= v->num_loops ? 0 : v->current_loop);
}

/**
 * Set the view table entry as updating.
 * @param v pointer to view table entry
 */
void start_update (struct vt_entry *v)
{
	if (~v->flags & UPDATE) {
		erase_both ();
		v->flags |= UPDATE;
		blit_both ();
	}
}

/**
 * Set the view table entry as non-updating.
 * @param v pointer to view table entry
 */
void stop_update (struct vt_entry *v)
{
	if (v->flags & UPDATE) {
		erase_both ();
		v->flags &= ~UPDATE;
		blit_both ();
	}
}


/* loops to use according to direction and number of loops in
 * the view resource
 */
static int loop_table_2[] = {
	0x04, 0x04, 0x00, 0x00, 0x00, 0x04, 0x01, 0x01, 0x01
};

static int loop_table_4[] = {
	0x04, 0x03, 0x00, 0x00, 0x00, 0x02, 0x01, 0x01, 0x01
};

/**
 * Update view table entries.
 * This function is called at the end of each interpreter cycle
 * to update the view table entries and blit the sprites.
 */
void update_viewtable ()
{
	struct vt_entry *v;
	int i, loop;

	i = 0;
	for_each_vt_entry(v) {
		if ((v->flags & (ANIMATED|UPDATE|DRAWN)) !=
			(ANIMATED|UPDATE|DRAWN))
		{
			continue;
		}

		i++;

		loop = 4;
		if (~v->flags & FIX_LOOP) {
			switch (v->num_loops) {
			case 2:
			case 3:
				loop = loop_table_2[v->direction];
				break;
			case 4:
				loop = loop_table_4[v->direction];
				break;
			}
		}

		if (v->step_time_count == 1 &&
			loop != 4 &&
			loop != v->current_loop)
		{
			set_loop (v, loop);
		}
			
		if (~v->flags & CYCLING)
			continue;

		if (v->cycle_time_count == 0)
			continue;

		if (--v->cycle_time_count == 0) {
			update_view (v);
			v->cycle_time_count = v->cycle_time;
		}
	}

	if (i) {
		erase_upd_sprites ();
		update_position ();
		blit_upd_sprites ();
		checkmove_upd_sprites ();
		game.view_table[0].flags &= ~(ON_WATER|ON_LAND);
	}
}

/* end: view.c */
