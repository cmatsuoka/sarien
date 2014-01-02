/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2003 Stuart George and Claudio Matsuoka
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

extern int decode_objects(UINT8* mem, UINT32 flen);


static struct agi_object *objects;		/* objects in the game */


int alloc_objects (int n)
{
	if ((objects = calloc (n, sizeof(struct agi_object))) == NULL)
    		return err_NotEnoughMemory;

	return err_OK;
}

int decode_objects (UINT8* mem, UINT32 flen)
{
	unsigned int i, so, padsize;

	padsize = game.game_flags & ID_AMIGA ? 4 : 3;

	game.num_objects = 0;
	objects = NULL;

	/* check if first pointer exceeds file size
	 * if so, its encrypted, else it is not
	 */

	if (lohi_getword (mem) > flen) {
		report ("Decrypting objects... ");
		decrypt (mem, flen);
		report ("done.\n");
	}

	/* alloc memory for object list
	 * byte 3 = number of animated objects. this is ignored.. ??
	 */
	if (lohi_getword(mem) / padsize >= 256) {
#ifdef AGDS_SUPPORT
    		/* die with no error! AGDS game needs not to die to work!! :( */
		return err_OK;
#else
		/* no AGDS support, die with error */
		return err_BadResource;
#endif
	}

	game.num_objects = lohi_getword(mem) / padsize;
	_D ("num_objects = %d (padsize = %d)", game.num_objects, padsize);

	if (alloc_objects (game.num_objects) != err_OK)
    		return err_NotEnoughMemory;

    	/* build the object list */
    	for (i = 0, so = padsize; i < game.num_objects; i++, so += padsize) {
		int offset;

		(objects + i)->location = lohi_getbyte (mem + so + 2);
		offset = lohi_getword (mem + so) + padsize;

    		if (offset < flen) {
			(objects + i)->name = strdup (mem + offset);
	    	} else {
	    		printf ("ERROR: object %i name beyond object filesize! "
				"(%04x > %04x)\n", i, offset, flen);
	    		(objects + i)->name = strdup ("");
	    	}
    	}
	report ("Reading objects: %d objects read.\n", game.num_objects);

	return err_OK;

}

int load_objects (char *fname)
{
	FILE *fp;
	UINT32 flen;
	UINT8 *mem;
	char *path;

	objects=NULL;
	game.num_objects = 0;

	_D ("(fname = %s)", fname);
	path = fixpath (NO_GAMEDIR, fname);
	report ("Loading objects: %s\n", path);

	if ((fp = fopen(path, "rb")) == NULL)
		return err_BadFileOpen;

	fseek (fp, 0, SEEK_END);
	flen = ftell (fp);
	fseek (fp, 0, SEEK_SET);

	if ((mem = calloc (1, flen + 32)) == NULL) {
		fclose (fp);
		return err_NotEnoughMemory;
	}

	fread (mem, 1, flen, fp);
	fclose(fp);

	decode_objects(mem, flen);
	free(mem);

	return err_OK;
}

void unload_objects ()
{
	unsigned int i;

	if (objects != NULL) {
		for (i = 0; i < game.num_objects; i++)
			free (objects[i].name);
		free (objects);
	}
}

#ifdef OPT_LIST_OBJECTS
int show_objects ()
{
	unsigned int i;

	printf(" ID   Objects\n");
	for (i = 0; i < game.num_objects; i++)
		printf ("%3i) %3i - %s\n", 1+i, (objects+i)->location, (objects+i)->name);

	printf ("\n%i objects\n", game.num_objects);

	return err_OK;
}
#endif

void object_set_location (unsigned int n, int i)
{
	if (n >= game.num_objects) {
		report ("Error: Can't access object %d.", n);
		return;
	}
	objects[n].location = i;
}

int object_get_location (unsigned int n)
{
	if (n >= game.num_objects) {
		report ("Error: Can't access object %d.", n);
		return 0;
	}
	return objects[n].location;
}

char *object_name (unsigned int n)
{
	if (n >= game.num_objects) {
		report ("Error: Can't access object %d.", n);
		return "";
	}
	return objects[n].name;
}
