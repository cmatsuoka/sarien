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
#include <string.h>	/* for memcpy() */
#include "sarien.h"
#include "list.h"
#include "agi.h"
#include "sprite.h"
#include "graphics.h"
#include "text.h"

/**
 * Sprite structure. 
 * This structure holds information on visible and priority data of
 * a rectangular area of the AGI screen. Sprites are chained in two
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
#ifdef USE_HIRES
	UINT8 *hires;			/**< buffer for hi-res background */
#endif
};    


/*
 * Blitter functions
 */

#ifdef USE_HIRES

static int blit_hires_cel (int x, int y, int spr, struct view_cel *c)
{
	UINT8 *q = NULL;
	UINT8 *h0, *h;
	int i, j, t;
	int epr, pr;		/* effective and real priorities */
	int hidden = TRUE;

	h0 = &game.hires[(x + y * _WIDTH) * 2];
	q = c->data;
	t = c->transparency;
	spr <<= 4;

	for (i = 0; i < c->height; i++) {
		h = h0;
		for (j = c->width * 2; j; j--, h++) {
			/* Check if we're on a control line */
			if ((pr = *h & 0xf0) < 0x30) {
				UINT8 *p1;
				/* Yes, get effective priority going down */
				for (p1 = h; (epr = *p1 & 0xf0) < 0x30; p1 += _WIDTH * 2) {
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
				*h = (pr < 0x30 ? pr : spr) | *q;
				hidden = FALSE;
			}
			q += (j & 1);
		}
		h0 += _WIDTH * 2;
	}

	return hidden;
}

#endif

static int blit_cel (int x, int y, int spr, struct view_cel *c)
{
	UINT8 *p0, *p, *q = NULL;
	int i, j, t;
	int epr, pr;		/* effective and real priorities */
	int hidden = TRUE;

#ifdef USE_HIRES
	if (opt.hires)
		blit_hires_cel (x, y, spr, c);
#endif

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
				for (p1 = p; (epr = *p1 & 0xf0) < 0x30; p1 += _WIDTH) {
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
				hidden = FALSE;
			}
		}
		p0 += _WIDTH;
	}

	return hidden;
}

static void objs_savearea (struct sprite *s)
{
	int y;
	UINT8 *p0, *q;
#ifdef USE_HIRES
	UINT8 *h0, *k;
#endif

	if (s->y_pos >= _HEIGHT)
		return;

	p0 = &game.sbuf[s->x_pos + s->y_pos * _WIDTH];
	q = s->buffer;
#ifdef USE_HIRES
	h0 = &game.hires[(s->x_pos + s->y_pos * _WIDTH) * 2];
	k = s->hires;
#endif
	for (y = 0; y < s->y_size; y++) {
		memcpy (q, p0, s->x_size);
		q += s->x_size;
		p0 += _WIDTH;
#ifdef USE_HIRES
		memcpy (k, h0, s->x_size * 2);
		k += s->x_size * 2;
		h0 += _WIDTH * 2;
#endif
	}
}

static void objs_restorearea (struct sprite *s)
{
	int y, offset;
	UINT8 *p0, *q;
#ifdef USE_HIRES
	UINT8 *h0, *k;
#endif

	if (s->y_pos >= _HEIGHT)
		return;

	p0 = &game.sbuf[s->x_pos + s->y_pos * _WIDTH];
	q = s->buffer;
#ifdef USE_HIRES
	h0 = &game.hires[(s->x_pos + s->y_pos * _WIDTH) * 2];
	k = s->hires;
#endif
	offset = game.line_min_print * CHAR_LINES;
	for (y = 0; y < s->y_size; y++) {
		memcpy (p0, q, s->x_size);
		put_pixels_a (s->x_pos, s->y_pos + y + offset, s->x_size, p0);
		q += s->x_size;
		p0 += _WIDTH;
#ifdef USE_HIRES
		memcpy (h0, k, s->x_size * 2);
		if (opt.hires) {
			put_pixels_hires (s->x_pos * 2,
				s->y_pos + y + offset, s->x_size * 2, h0);
		}
		k += s->x_size * 2;
		h0 += _WIDTH * 2;
#endif
	}
}


/*
 * Sprite management functions
 */

static LIST_HEAD(spr_upd_head);
static LIST_HEAD(spr_nonupd_head);

/**
 * Condition to determine whether a sprite will be in the 'updating' list.
 */
static int test_updating (struct vt_entry *v)
{
	return (v->flags & (ANIMATED|UPDATE|DRAWN)) ==
		(ANIMATED|UPDATE|DRAWN);
}

/**
 * Condition to determine whether a sprite will be in the 'non-updating' list.
 */
static int test_not_updating (struct vt_entry *v)
{
	return (v->flags & (ANIMATED|UPDATE|DRAWN)) == (ANIMATED|DRAWN);
}

/**
 * Convert sprite priority to y value.
 */
static INLINE int prio_to_y (int p)
{
	int i;

	if (game.alt_pri) {		/* set.pri.base used */
		if (p == 0)
			return -1;
		for (i = 168; i; i--) {
			if (game.pri_table[i] < p)
				return i;
		}
	}

	return (p - 5) * 12 + 48;
}

/**
 * Create and initialize a new sprite structure.
 */
static struct sprite *new_sprite (struct vt_entry *v)
{
	struct sprite *s;

	s = malloc (sizeof (struct sprite));
	if (s == NULL)
		abort ();
	s->v = v;	/* link sprite to associated view table entry */
	s->x_pos = v->x_pos;
	s->y_pos = v->y_pos - v->y_size + 1;
	s->x_size = v->x_size;
	s->y_size = v->y_size;
	s->buffer = malloc (s->x_size * s->y_size);
#ifdef USE_HIRES
	s->hires = malloc (s->x_size * s->y_size * 2);
#endif
	v->s = s;	/* link view table entry to this sprite */

	return s;
}

/**
 * Insert sprite in the specified sprite list.
 */
static void spr_addlist (struct list_head *head, struct vt_entry *v)
{
	struct sprite *s;

	s = new_sprite (v);
	list_add_tail (&s->list, head);
}

/**
 * Sort sprites from lower y values to build a sprite list.
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
			y_val[i] = v->flags & UPDATE ? v->y_pos :
				prio_to_y (v->priority);
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

/**
 * Build list of updating sprites.
 */
static struct list_head *build_upd_blitlist ()
{
	return build_list (&spr_upd_head, test_updating);
}

/**
 * Build list of non-updating sprites.
 */
static struct list_head *build_nonupd_blitlist ()
{
	return build_list (&spr_nonupd_head, test_not_updating);
}

/**
 * Clear the given sprite list.
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

/**
 * Copy sprites from the pic buffer to the screen buffer, and check if
 * sprites of the given list have moved.
 */
static void commit_sprites (struct list_head *head)
{
	struct list_head *h;

	list_for_each (h, head, next) {
		struct sprite *s = list_entry (h, struct sprite, list);
		int x1, y1, x2, y2, w, h;

		w = (s->v->cel_data->width > s->v->cel_data_2->width) ?
			s->v->cel_data->width : s->v->cel_data_2->width;

		h = (s->v->cel_data->height > s->v->cel_data_2->height) ?
			s->v->cel_data->height : s->v->cel_data_2->height;

		s->v->cel_data_2 = s->v->cel_data;

		if (s->v->x_pos < s->v->x_pos2) {
			x1 = s->v->x_pos;
			x2 = s->v->x_pos2 + w - 1;
		} else {
			x1 = s->v->x_pos2;
			x2 = s->v->x_pos + w - 1;
		}

		if (s->v->y_pos < s->v->y_pos2) {
			y1 = s->v->y_pos - h + 1;
			y2 = s->v->y_pos2;
		} else {
			y1 = s->v->y_pos2 - h + 1;
			y2 = s->v->y_pos;
		}

		commit_block (x1, y1, x2, y2);

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

#ifdef USE_CONSOLE
	if (debug.statusline)
		write_status ();
#endif
}

/**
 * Erase all sprites in the given list.
 */
static void erase_sprites (struct list_head *head)
{
	struct list_head *h;

	list_for_each (h, head, prev) {
		struct sprite *s = list_entry (h, struct sprite, list);
		objs_restorearea (s);
	}

	free_list (head);
}

/**
 * Blit all sprites in the given list.
 */
static void blit_sprites (struct list_head *head)
{
	struct list_head *h = NULL;
	int hidden;

	list_for_each (h, head, next) {
		struct sprite *s = list_entry (h, struct sprite, list);
		objs_savearea (s);
		hidden = blit_cel (s->x_pos, s->y_pos, s->v->priority,
			s->v->cel_data);
		if (s->v->entry == 0) {		/* if ego, update f1 */
			setflag (F_ego_invisible, hidden);
		}
	}
}


/*
 * Public functions
 */


void commit_upd_sprites ()
{
	commit_sprites (&spr_upd_head);
}

void commit_nonupd_sprites ()
{
	commit_sprites (&spr_nonupd_head);
}

/* check moves in both lists */
void commit_both ()
{
	commit_upd_sprites ();
	commit_nonupd_sprites ();
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
 * This cel is not a sprite, it can't be moved or removed.
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
	struct view_cel	*c = NULL;
	int x1, y1, x2, y2, y3;
	UINT8 *p1, *p2;

	_D ("v=%d, l=%d, c=%d, x=%d, y=%d, p=%d, m=%d",
		view, loop, cel, x, y, pri, mar); 

	if (pri == 0)
		pri = 8;		/* ??!? */

	c = &game.views[view].loop[loop].cel[cel];

	x1 = x;
	y1 = y - c->height + 1;
	x2 = x + c->width - 1;
	y2 = y;

	if (x1 < 0) x1 = 0;
	if (y1 < 0) y1 = 0;
	if (x2 >= _WIDTH) x2 = _WIDTH - 1;
	if (y2 >= _HEIGHT) y2 = _HEIGHT - 1;

	erase_both ();

	_D (_D_WARN "blit_cel (%d, %d, %d, c)", x, y, pri);
	blit_cel (x1, y1, pri, c);

	/* If margin is 0, 1, 2, or 3, the base of the cel is
	 * surrounded with a rectangle of the corresponding priority.
	 * If margin >= 4, this extra margin is not shown.
	 */
	if (mar < 4) {
		/* add rectangle around object, don't clobber control
		 * info in priority data. The box extends to the end of
		 * its priority band!
		 */
		y3 = (y2 / 12) * 12;

		p1 = &game.sbuf[x1 + y3 * _WIDTH];
		p2 = &game.sbuf[x2 + y3 * _WIDTH];

		for (y = y3; y <= y2; y++) {
			if ((*p1 >> 4) >= 4) *p1 = (mar << 4) | (*p1 & 0x0f);
			if ((*p2 >> 4) >= 4) *p2 = (mar << 4) | (*p2 & 0x0f);
			p1 += _WIDTH;
			p2 += _WIDTH;
		}

		p1 = &game.sbuf[x1 + y3 * _WIDTH];
		p2 = &game.sbuf[x1 + y2 * _WIDTH];
		for (x = x1; x <= x2; x++) {
			if ((*p1 >> 4) >= 4) *p1 = (mar << 4) | (*p1 & 0x0f);
			if ((*p2 >> 4) >= 4) *p2 = (mar << 4) | (*p2 & 0x0f);
			p1++;
			p2++;
		}
	}

	blit_both ();

	commit_block (x1, y1, x2, y2);
}

/**
 * Show object and description
 * This function shows an object from the player's inventory, displaying
 * a message box with the object description.
 * @param n  Number of the object to show
 */
void show_obj (n)
{
	struct view_cel *c;
	struct sprite s;
	int x1, y1, x2, y2;

	agi_load_resource (rVIEW, n);
	if (!(c = &game.views[n].loop[0].cel[0]))
		return;
	
	x1 = (_WIDTH - c->width) / 2;
	y1 = 120;
	x2 = x1 + c->width - 1;
	y2 = y1 + c->height - 1;

	s.x_pos = x1;
	s.y_pos = y1;
	s.x_size = c->width;
	s.y_size = c->height;
	s.buffer = malloc (s.x_size * s.y_size);
#ifdef USE_HIRES
	s.hires = malloc (s.x_size * s.y_size * 2);
#endif

	objs_savearea (&s);
	blit_cel (x1, y1, s.x_size, c);
	commit_block (x1, y1, x2, y2);
	message_box (game.views[n].descr);
	objs_restorearea (&s);
	commit_block (x1, y1, x2, y2);

	free (s.buffer);
}

void commit_block (int x1, int y1, int x2, int y2)
{
	int i, w, offset;
	UINT8 *q;
#ifdef USE_HIRES
	UINT8 *h;
#endif

	if (!game.picture_shown)
		return;

	/* Clipping */
	if (x1 < 0) x1 = 0;
	if (x2 < 0) x2 = 0;
	if (y1 < 0) y1 = 0;
	if (y2 < 0) y2 = 0;
	if (x1 >= _WIDTH) x1 = _WIDTH - 1;
	if (x2 >= _WIDTH) x2 = _WIDTH - 1;
	if (y1 >= _HEIGHT) y1 = _HEIGHT - 1;
	if (y2 >= _HEIGHT) y2 = _HEIGHT - 1;

	/* _D ("%d, %d, %d, %d", x1, y1, x2, y2); */

	w = x2 - x1 + 1;
	q = &game.sbuf[x1 + _WIDTH * y1];
#ifdef USE_HIRES
	h = &game.hires[(x1 + _WIDTH * y1) * 2];
#endif
	offset = game.line_min_print * CHAR_LINES;
	for (i = y1; i <= y2; i++) {
		put_pixels_a (x1, i + offset, w, q);
		q += _WIDTH;
#ifdef USE_HIRES
		if (opt.hires) {
			put_pixels_hires (x1 * 2, i + offset, w * 2, h);
		}
		h += _WIDTH * 2;
#endif
	}

	flush_block_a (x1, y1 + offset, x2, y2 + offset);
}

/* end: sprite.c */
