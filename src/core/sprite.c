/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#include <assert.h>
#include "sarien.h"
#include "list.h"
#include "agi.h"
#include "sprite.h"
#include "graphics.h"
#include "text.h"

/**
 * Sprite structure. 
 * This structure holds information on visible and priority data of
 * an rectangular area of the AGI screen. Sprites are chained in two
 * circular lists, one for updating and other for non-updating sprites.
 */
struct sprite {
	struct list_head list;
	struct vt_entry *v;		/**< pointer to view table entry */
	UINT16 x_pos;			/**< x coordinate of the sprite */
	UINT16 y_pos;			/**< y coordinate of the sprite */
	UINT16 x_size;			/**< width of the sprite */
	UINT16 y_size;			/**< height of the sprite */
	UINT8 *buffer;			/**< buffer to store background data */
};    


/*
 * Blitter functions
 */

static void blit_cel (int x, int y, int spr, struct view_cel *c)
{
	UINT8 *p0, *p, *q;
	int i, j, t;
	int epr, pr;		/* effective and real priorities */

	p0 = &game.sbuf[x + y * _WIDTH];
	q = c->data;
	t = c->transparency;
	spr <<= 4;

	for (i = 0; i < c->height; i++) {
		p = p0;
		for (j = c->width; j; j--, p++, q++) {
			/* Check if we're on a control line */
			if ((pr = *p & 0xf0) < 0x30) {
				UINT8 *p1;
				/* Yes, get effective priority going down */
				for (p1 = p; (epr = *p1 & 0xf0) < 0x40; p1 += _WIDTH) {
					if (p1 >= game.sbuf + _WIDTH * _HEIGHT) {
						epr = 0x40;
						break;
					}
				}
			} else {
				epr = pr;
			}
			if (*q != t && spr >= epr) {
				/* Keep control line information visible,
				 * but put our priority over water (0x30)
				 * surface
				 */
				*p = (pr < 0x30 ? pr : spr) | *q;
			}
		}
		put_pixels_a (x, y + i, c->width, p0);
		p0 += _WIDTH;
	}

	if (game.picture_shown)
		flush_block_a (x, y, x + c->width - 1, y + c->height - 1);
}


static void objs_savearea (struct sprite *s)
{
	int y;
	UINT8 *p0, *q;

	p0 = &game.sbuf[s->x_pos + s->y_pos * _WIDTH];
	q = s->buffer;
	for (y = 0; y < s->y_size; y++) {
		memcpy (q, p0, s->x_size);
		q += s->x_size;
		p0 += _WIDTH;
	}
}

static void objs_restorearea (struct sprite *s)
{
	int y;
	UINT8 *p0, *q;

	p0 = &game.sbuf[s->x_pos + s->y_pos * _WIDTH];
	q = s->buffer;
	for (y = 0; y < s->y_size; y++) {
		memcpy (p0, q, s->x_size);
		put_pixels_a (s->x_pos, s->y_pos + y, s->x_size, p0);
		q += s->x_size;
		p0 += _WIDTH;
	}

	if (game.picture_shown) {
		flush_block_a (s->x_pos, s->y_pos, s->x_pos + s->x_size - 1,
			s->y_pos + s->y_size - 1);
	}
}


/*
 * Sprite management functions
 */

static LIST_HEAD(spr_upd_head);
static LIST_HEAD(spr_nonupd_head);


/* condition to determine whether a sprite will be in the
 * 'updating' list
 */
static int test_updating (struct vt_entry *v)
{
	return (v->flags & (ANIMATED|UPDATE|DRAWN)) ==
		(ANIMATED|UPDATE|DRAWN);
}

/* condition to determine whether a sprite will be in the
 * 'non-updating' list
 */
static int test_not_updating (struct vt_entry *v)
{
	return (v->flags & (ANIMATED|UPDATE|DRAWN)) == (ANIMATED|DRAWN);
}

/* convert sprite priority to y value */
static INLINE int prio_to_y (int p)
{
	return (p - 4) * 12 + 48;
}

/* create and initialize a new sprite structure to be added in
 * the sprite list
 */
static struct sprite *new_sprite (struct vt_entry *v)
{
	struct sprite *s;

	s = malloc (sizeof (struct sprite));
	s->v = v;
	s->x_pos = v->x_pos;
	s->y_pos = v->y_pos - v->y_size + 1;
	s->x_size = v->x_size;
	s->y_size = v->y_size;
	s->buffer = malloc (s->x_size * s->y_size);
	v->s = s;

	return s;
}

/* insert the sprite in the given circular list
 */
static void spr_addlist (struct list_head *head, struct vt_entry *v)
{
	struct sprite *s;

	s = new_sprite (v);
	list_add_tail (&s->list, head);
}

/* sort from lower y values to build the list
 */
static struct list_head *
build_list (struct list_head *head, int (*test)(struct vt_entry *))
{
	int i, j, k;
	struct vt_entry *v;
	struct vt_entry *entry[0x100];
	int y_val[0x100];
	int min_y = 0xff, min_index = 0;

	/* fill the arrays with all sprites that satisfy the 'test'
	 * condition and their y values
	 */ 
	i = 0;
	for_each_vt_entry(v) {
		if (test (v)) {
			entry[i] = v;
			y_val[i] = v->flags & UPDATE ?
				prio_to_y (v->priority) : v->y_pos;
			i++;
		}
	}

	/* now look for the smallest y value in the array and put that
	 * sprite in the list
	 */
	for (j = 0; j < i; j++) {
		min_y = 0xff;
		for (k = 0; k < i; k++) {
			if (y_val[k] <= min_y) {
				min_index = k;
				min_y = y_val[k];
			}
		}

		y_val[min_index] = 0xff;
		spr_addlist (head, entry[min_index]);
	}

	return head;
}

/* build list of updating sprites
 */
static struct list_head *build_upd_blitlist ()
{
	return build_list (&spr_upd_head, test_updating);
}

/* build list of non-updating sprites
 */
static struct list_head *build_nonupd_blitlist ()
{
	return build_list (&spr_nonupd_head, test_not_updating);
}

/* clear the given list
 */
static void free_list (struct list_head *head)
{
	struct list_head *h;
	struct sprite *s;

	list_for_each (h, head, next) {
		s = list_entry (h, struct sprite, list);
		list_del (h);
		free (s->buffer);
		if (h->prev != head)
			free (list_entry (h->prev, struct sprite, list));
	}
}

/* check if sprites of the given list have moved
 */
static void checkmove_sprites (struct list_head *head)
{
	struct list_head *h;

	list_for_each (h, head, next) {
		struct sprite *s = list_entry (h, struct sprite, list);

		if (s->v->step_time_count != s->v->step_time)
			continue;

		if (s->v->x_pos == s->v->x_pos2 && s->v->y_pos == s->v->y_pos2){
			s->v->flags |= DIDNT_MOVE;
			continue;
		}

		s->v->x_pos2 = s->v->x_pos;
		s->v->y_pos2 = s->v->y_pos;
		s->v->flags &= ~DIDNT_MOVE;
	}
}

/* erase all sprites in the given list */
static void erase_sprites (struct list_head *head)
{
	struct list_head *h;

	list_for_each (h, head, prev) {
		struct sprite *s = list_entry (h, struct sprite, list);
		objs_restorearea (s);
	}

	free_list (head);
}

/* blit all sprites in the given list */
static void blit_sprites (struct list_head *head)
{
	struct list_head *h;

	list_for_each (h, head, next) {
		struct sprite *s = list_entry (h, struct sprite, list);
		objs_savearea (s);
		blit_cel (s->x_pos, s->y_pos, s->v->priority, s->v->cel_data);
	}
}


/*
 * Public functions
 */


void checkmove_upd_sprites ()
{
	checkmove_sprites (&spr_upd_head);
}

void checkmove_nonupd_sprites ()
{
	checkmove_sprites (&spr_nonupd_head);
}

/* check moves in both lists */
void checkmove_both ()
{
	checkmove_upd_sprites ();
	checkmove_nonupd_sprites ();
}

/**
 * Erase updating sprites.
 * This function follows the list of all updating sprites and restores
 * the visible and priority data of their background buffers back to
 * the AGI screen.
 *
 * @see erase_nonupd_sprites()
 * @see erase_both()
 */
void erase_upd_sprites ()
{
	erase_sprites (&spr_upd_head);
}

/**
 * Erase non-updating sprites.
 * This function follows the list of all non-updating sprites and restores
 * the visible and priority data of their background buffers back to
 * the AGI screen.
 *
 * @see erase_upd_sprites()
 * @see erase_both()
 */
void erase_nonupd_sprites ()
{
	erase_sprites (&spr_nonupd_head);
}

/**
 * Erase all sprites.
 * This function follows the lists of all updating and non-updating
 * sprites and restores the visible and priority data of their background
 * buffers back to the AGI screen.
 *
 * @see erase_upd_sprites()
 * @see erase_nonupd_sprites()
 */
void erase_both ()
{
	erase_upd_sprites ();
	erase_nonupd_sprites ();
}

/**
 * Blit updating sprites.
 * This function follows the list of all updating sprites and blits
 * them on the AGI screen.
 *
 * @see blit_nonupd_sprites()
 * @see blit_both()
 */
void blit_upd_sprites ()
{
	blit_sprites (build_upd_blitlist ());
}

/**
 * Blit updating sprites.
 * This function follows the list of all non-updating sprites and blits
 * them on the AGI screen.
 *
 * @see blit_upd_sprites()
 * @see blit_both()
 */
void blit_nonupd_sprites ()
{
	blit_sprites (build_nonupd_blitlist ());
}

/**
 * Blit all sprites.
 * This function follows the lists of all updating and non-updating
 * sprites and blits them on the AGI screen.
 *
 * @see blit_upd_sprites()
 * @see blit_nonupd_sprites()
 */
void blit_both ()
{
	blit_nonupd_sprites ();
	blit_upd_sprites ();
}

/**
 * Add view to picture.
 * This function is used to implement the add.to.pic AGI command. It
 * copies the specified cel from a view resource on the current picture.
 * This cel is not a sprite, it's can't be moved or removed.
 * @param view  number of view resource
 * @param loop  number of loop in the specified view resource
 * @param cel   number of cel in the specified loop
 * @param x     x coordinate to place the view
 * @param y     y coordinate to place the view
 * @param pri   priority to use
 * @param mar   if < 4, create a margin around the the base of the cel
 */
void add_to_pic (int view, int loop, int cel, int x, int y, int pri, int mar)
{
	struct view_cel	*c;
	int x1, y1;
	UINT8 *p;

	_D ("v=%d, l=%d, c=%d, x=%d, y=%d, p=%d", view, loop, cel, x, y, pri); 
	if (pri == 0)
		pri = 8;		/* ??!? */

	assert ((c = &game.views[view].loop[loop].cel[cel]) != NULL);

	if (y - c->height + 1 > 0)
		y -= c->height - 1;
	else
		y = 0;

	if (x + c->width >= _WIDTH)
		x = _WIDTH - c->width;

	blit_cel (x, y, pri, c);

	/* If margin is 0, 1, 2, or 3, the base of the cel is
	 * surrounded with a rectangle of the corresponding priority.
	 * If margin >= 4, this extra margin is not shown.
	 */
	if (mar < 4) {
		/* add rectangle around object, don't clobber control
		 * info in priority data
		 */
		for (y1 = y; y1 < c->height; y1++) {
			for (x1 = x; x1 < c->width; x1++) {
				int idx = y1 * _WIDTH + x1;
				p = &game.sbuf[idx];
				if ((*p >> 4) >= 4)
					*p = (mar << 4) | (*p & 0x0f);
			}
		}
	}
}

void show_obj (n)
{
	struct view_cel *c;
	struct sprite s;

	agi_load_resource (rVIEW, n);
	if (!(c = &game.views[n].loop[0].cel[0]))
		return;
	
	s.x_pos = (_WIDTH - c->width) / 2;
	s.y_pos = 128;
	s.x_size = c->width;
	s.y_size = c->height;
	s.buffer = malloc (s.x_size * s.y_size);

	objs_savearea (&s);
	blit_cel (s.x_pos, s.y_pos, 15, c);
	message_box (game.views[n].descr);
	objs_restorearea (&s);

	free (s.buffer);
}

/* end: sprite.c */
