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
#include "gfx_agi.h"
#include "view.h"
#include "picture.h"
#include "console.h"


struct agi_view_table view_table[MAX_VIEWTABLE];/* 32 animated objects/views */
struct agi_view	views[MAX_DIRS];		/* max views */


/* FIXME: ugh. any way to eliminate this kludge? */
UINT8 old_prio = 0;


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
	int h, w, c, l, d;
	UINT8 *p;

	p = vc->data;
	memset (p, vc->transparency,
		vc->height * vc->width);

	if (vc->width == 0 || vc->height == 0)
		return;

	for (d = h = 0; h < vc->height; h++) {
		w = 0;
		p = vc->data + (h * vc->width);

		while(42) {
			if ((l = data[d++]) == 0)
				break;
			c = (l >> 4) & 0xf;
			l = l & 0xf;
			memset (p, c, l);
			p += l;
		}
	}
}


void init_view_table ()
{
	int i;

	_D ("()");
	for (i = 0; i < MAX_VIEWTABLE; i++) {
		memset (&view_table[i], 0, sizeof (struct agi_view_table));
		view_table[i].step_time = 1;
		view_table[i].step_time_count = 1;
		view_table[i].step_size = 1;
		view_table[i].cycle_time = 1;
		view_table[i].cycle_time_count = 1;
		view_table[i].cycle_status = CYCLE_NORMAL;
	}
}


void reset_view (int n)
{
	if (view_table[n].bg_scr) {
		free (view_table[n].bg_scr);
		view_table[n].bg_scr = NULL;
	}
	if (view_table[n].bg_pri) {
		free (view_table[n].bg_pri);
		view_table[n].bg_pri = NULL;
	}

	/*view_table[n].flags &= ~(UPDATE | ANIMATED);*/
	view_table[n].flags = 0;
	view_table[n].step_time = 1;
	view_table[n].step_time_count = 1;
	view_table[n].step_size = 1;
	view_table[n].cycle_time = 1;
	view_table[n].cycle_time_count = 1;
	view_table[n].cycle_status = CYCLE_NORMAL;
}


void reset_views ()
{
	int i;

	for (i = 0; i < MAX_DIRS; i++) 
		agi_unload_resource (rVIEW, i);

	for (i = 0; i < MAX_VIEWTABLE; i++)
		reset_view (i);
}


/* unloads ALL data in a VIEW, not a VIEWTABLE entry */
void unload_view (int n)
{
	int x, y;

	if (~game.dir_view[n].flags & RES_LOADED)
		return;

	/* free all the loops */
	for (x = 0; x < views[n].num_loops; x++) {
		for (y = 0; y < views[n].loop[x].num_cels; y++)
			if (views[n].loop[x].cel[y].data != NULL)
				free (views[n].loop[x].cel[y].data);

		if (views[n].loop[x].cel != NULL)
			free (views[n].loop[x].cel);
	}

	free (views[n].loop);
	free (views[n].descr);
	free (views[n].rdata);

	game.dir_view[n].flags &= ~RES_LOADED;
}


void add_view_table (int entry, int vw)
{
	_D ("(%d, %d)", entry, vw);

	_D ("F5 = %d", getflag (5));
	/* To prevent Larry explosion in room 11 after hooker */
	if (~game.dir_view[vw].flags & RES_LOADED) {
		report ("Parachute deployed: view %d not loaded\n", vw);
		agi_load_resource (rVIEW, vw);
	}

	if (entry == 0) {
		setvar (V_ego_view_resource, vw);
	}

	if (view_table[entry].current_view != vw || getflag(F_new_room_exec)) {	
		view_table[entry].current_view = vw;

		view_table[entry].bg_scr = NULL;
		view_table[entry].bg_pri = NULL;
		view_table[entry].motion = MOTION_NORMAL;
		view_table[entry].cycle_status = CYCLE_NORMAL;

		/* Loop numbers should be retained, this checks if a loop number
		 * is sane for the given view
		 */
		if (view_table[entry].current_loop >= views[vw].num_loops)
			view_table[entry].current_loop=0;

		/* Hmm... */
		if (views[vw].num_loops >= 1 && views[vw].num_loops <= 3) {
			switch(view_table[entry].direction) {
			case 0:
			case 1:
			case 5:
				break;
			case 2:
			case 3:
			case 4:
				set_loop (entry, 0);
				break;
			case 6:
			case 7:
			case 8:
				set_loop (entry, 1);
				break;
			}
		} else if (views[vw].num_loops == 4) {
			switch(view_table[entry].direction) {
			case 0:
				/*
				 * WRONG: set_loop (entry, 0);
				 *
				 * In this case the loop number is retained
				 * from the previous object
				 */
				break;
			case 1:
				set_loop (entry, 3);
				break;
			case 2:
			case 3:
			case 4:
				set_loop (entry, 0);
				break;
			case 5:
				set_loop (entry, 2);
				break;
			case 6:
			case 7:
			case 8:
				set_loop (entry, 1);
				break;
			}
		} else {
			/* CM: Can't set loop to zero here, it produces
			 * the SSSRA bug.
			 *
			 * WRONG: set_loop (entry, 0);
			 */
		}

		/* Sanity check */
		if (view_table[entry].priority < 4)
			view_table[entry].priority = 4;
	}

#if 0
	/* CM: The two following lines were testing current_loop and cur_cel
	 *     against junk values. Fixed to test against the current view
	 *     and loop data.
	 */
	if (view_table[entry].current_loop >= view_table[entry].view->num_loops)
		view_table[entry].current_loop = 0;

	if (view_table[entry].current_cel > view_table[entry].view->loop[view_table[entry].current_loop].num_cels)
		view_table[entry].current_cel = 0;

	set_loop (entry, 0);
	set_cel  (entry, view_table[entry].cur_cel);
#endif
}


void set_cel (int entry, int cel)
{
	if (cel >= VT_LOOP(view_table[entry]).num_cels) {
		report ("Oops! attempt to set cel(=%d) > num_cels(=%d)\n",
			cel, VT_LOOP(view_table[entry]).num_cels);
		cel = 0;
	}

	view_table[entry].current_cel = cel;
}


void set_loop (int entry, int loop)
{
	if (loop >= VT_VIEW(view_table[entry]).num_loops)
		loop = 0;

	if (view_table[entry].current_loop != loop || getflag(F_new_room_exec)) {
		view_table[entry].current_loop = loop;
		set_cel (entry, view_table[entry].current_cel);
	}

	/* FR:
	 * cycle_status should be set to CYCLE_NORMAL (fixes taxi in larry 1)
	 */
	view_table[entry].cycle_status = CYCLE_NORMAL;
}


void add_to_pic (int view, int loop, int cel, int x, int y, int priority, int margin)
{
	/* copy a bitmap onto the main screen */
	/* check for cel mirror */
	struct view_cel	*c;
	int x1, y1;

	if (priority == 0)
		priority = old_prio;

	old_prio = priority;

	if ((c = &views[view].loop[loop].cel[cel])) {
		if (y - c->height > 0)
			y -= c->height;
		else
			y = 0;

		agi_put_bitmap (c->data, x, y, c->width, c->height,
			c->transparency & 0xF, priority);

		/* If margin is 0, 1, 2, or 3, the base of the cel is
		 * surrounded with a rectangle of the corresponding priority.
		 * If margin >= 4, this extra margin is not shown.
		 */
		if (margin < 4) {
			/* add rectangle around object, don't clobber control
			 * info in xdata
			 */
			for (y1 = y; y1 < c->height; y1++) {
				for (x1 = x; x1 < c->width; x1++) {
					int idx = y1 * _WIDTH + x1;
					if (xdata_data[idx] >= 4)
						xdata_data[idx] = margin;
				}
			}
		}
	}
}


void calc_direction (int vt)
{
	if (view_table[vt].flags & FIX_LOOP)
		return;

	/* CM: fixes the BAD LOOP error */
	if(~view_table[vt].flags & DRAWN)
		return;

	/* FR: Fixed (see agistudio doc) */
	if (VT_VIEW(view_table[vt]).num_loops < 4) {
		switch (view_table[vt].direction) {
		case 0:
		case 1:
		case 5:
			break;
		case 2:
		case 3:
		case 4:
			set_loop (vt, 0);
			break;
		case 6:
		case 7:
		case 8:
			set_loop (vt, 1);
			break;
		}
	} else if (VT_VIEW(view_table[vt]).num_loops == 4) {
		switch(view_table[vt].direction) {
		case 0:
			break;
		case 1:
			set_loop (vt, 3);
			break;
		case 2:
		case 3:
		case 4:
			set_loop (vt, 0);
			break;
		case 5:
			set_loop (vt, 2);
			break;
		case 6:
		case 7:
		case 8:
			set_loop (vt, 1);
			break;
		}
	}
}


void draw_obj (int vt)
{
	struct agi_view_table *v = &view_table[vt];
	int cel_width, cel_height;

	/* Sanity check */
	set_cel (vt, view_table[vt].current_cel);

	cel_width = VT_WIDTH(view_table[vt]);
	cel_height = VT_HEIGHT(view_table[vt]);




	reposition (vt);




#if 0
	/* DF: CLIPPING (FIXES OP:RECON BUG !! (speach bubbles) */

	/* Breaks sprites moving off screen */
	if (v->x_pos + cel_width > _WIDTH)
		v->x_pos = _WIDTH - cel_width;

	if(v->y_pos + cel_height > _HEIGHT)
	      v->y_pos=_HEIGHT - cel_height;

	/* this also breaks kq2 intro etc!! hmmmm */
	if(v->y_pos + cel_height > 200)	/* _HEIGHT=168, breaks op:recon */
	      v->y_pos=200- cel_height;
#endif

	/* save bg co-ords */
	v->bg_x = v->x_pos;
	v->bg_y = (cel_height > v->y_pos) ?  0 : (v->y_pos - cel_height);
	v->bg_x_size = cel_width;
	v->bg_y_size = cel_height;

	/* copy background (screen) */
	v->bg_scr = malloc (v->bg_x_size * v->bg_y_size);

	/* We can always read from screen2 because agi_put_bitmap always
	 * draws a copy of the image there
	 */
	get_bitmap (v->bg_scr, screen2, v->bg_x, v->bg_y,
		v->bg_x_size, v->bg_y_size);

	/* copy background (priority map) */
	v->bg_pri = (UINT8*)malloc(v->bg_x_size * v->bg_y_size);
	get_bitmap (v->bg_pri, xdata_data, v->bg_x, v->bg_y, v->bg_x_size,
		v->bg_y_size);

	/* FR:
	 * Sierra logo didn't appear in demos because
	 * (v->y_pos - cel_height) < 0 and agi_put_bimap receive
	 * only unsigned values!
	 */
	agi_put_bitmap (VT_CEL(view_table[vt]).data,
		v->x_pos,
		cel_height > v->y_pos ? 0 : v->y_pos - cel_height,
		cel_width,
		cel_height,
		VT_CEL(view_table[vt]).transparency & 0xf,
		v->priority);
}


int decode_view (int resnum)
{
	int loop, cel;
	UINT8 *v, *lptr;
	UINT16 lofs, cofs;
	struct view_loop *vl;
	struct view_cel	*vc;

	_D ("(%d)", resnum);
	v = views[resnum].rdata;

	views[resnum].num_loops = 0;

	if (v == NULL)
		return err_ViewDataError;

	views[resnum].descr = lohi_getword (v + 3) ?
		strdup ((char*)v + lohi_getword (v + 3)) : strdup ("");

	/* if no loops exist, return! */
	views[resnum].num_loops = lohi_getbyte (v + 2);

	if (views[resnum].num_loops == 0)
		return err_NoLoopsInView;

	/* allocate memory for all views */
	views[resnum].loop = calloc(views[resnum].num_loops,
		sizeof(struct view_loop));

	if (views[resnum].loop == NULL)
		return err_NotEnoughMemory;

	/* clean out all our loop data */
	for(loop=0; loop<views[resnum].num_loops; loop++) {
		views[resnum].loop[loop].num_cels = 0;
	}

	/* decode all of the loops in this view */
	lptr = v + 5;		/* first loop address */

	for (loop = 0; loop < views[resnum].num_loops; loop++, lptr += 2) {

		lofs = lohi_getword (lptr);	/* loop header offset */
		vl = &views[resnum].loop[loop];	/* the loop struct */

		vl->num_cels = lohi_getbyte (v + lofs);
		vl->cel = calloc (vl->num_cels, sizeof (struct view_cel));
		if (vl->cel == NULL) {
			free (views[resnum].loop);
			views[resnum].num_loops = 0;
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
    					free (views[resnum].loop[loop].cel[cel].data);
    				for (; loop >= 0; loop--)
    					free(views[resnum].loop[loop].cel);

    				free(views[resnum].loop);
    				views[resnum].num_loops = 0;
    				return err_NotEnoughMemory;
    			}

			decode_cel (vc, v + cofs);

			_D ("mirror=%d, loop=%d", vc->mirror,vc->mirror_loop);
    			if (vc->mirror == 1 && vc->mirror_loop != loop) {
				_D(_D_WARN "mirror_loop = %d", vc->mirror_loop);
    				mirror_cel (vc);
			}
    		} /* cel */
	} /* loop */

	return err_OK;
}

