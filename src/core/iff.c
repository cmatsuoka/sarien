/* Extended Module Player
 * Copyright (C) 1997 Claudio Matsuoka and Hipolito Carraro Jr
 * $Id$
 *
 * This file is part of the Extended Module Player and is distributed
 * under the terms of the GNU General Public License. See doc/COPYING
 * for more information.
 */

#include <stdio.h>
#include <string.h>

#include "sarien.h"
#include "iff.h"

static struct iff_info *iff_head = NULL;

void iff_chunk (FILE *f)
{
	UINT32 size;
	UINT8 s[4];
	char id[8] = "";

	if (fread (id, 1, 4, f) != 4)
		return;
	if (fread (s, 1, 4, f) != 4)
		return;
	size = ((UINT32)s[0] << 24) | ((UINT32)s[1] << 16) |
		((UINT32)s[2] << 8) | s[3];

	_D (_D_WARN "%c%c%c%c %d", id[0], id[1], id[2], id[3], size);
	iff_process (id, size, f);
}


void iff_register (char *id, void (*loader) ())
{
	struct iff_info *f;

	f = malloc (sizeof (struct iff_info));
	strcpy (f->id, id);
	f->loader = loader;
	if (!iff_head) {
		iff_head = f;
		f->prev = NULL;
	} else {
		struct iff_info *i;
		for (i = iff_head; i->next; i = i->next) {}
		i->next = f;
		f->prev = i;
	}
	f->next = NULL;
}


void iff_release ()
{
	struct iff_info *i;

	for (i = iff_head; i->next; i = i->next) {}
	while (i->prev) {
		i = i->prev;
		free (i->next);
		i->next = NULL;
	}
	free (iff_head);
	iff_head = NULL;
}


int iff_process (char *id, long size, FILE * f)
{
	char *buffer;
	struct iff_info *i;

	if (size == 0)
		return 0;

	if ((buffer = malloc (size)) == NULL)
		return -1;
	fread (buffer, 1, size, f);
	for (i = iff_head; i; i = i->next) {
		if (id && !strncmp (id, i->id, 4)) {
			i->loader (size, buffer);
			break;
		}
	}
	free (buffer);
	return 0;
}

