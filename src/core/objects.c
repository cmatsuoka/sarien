/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999,2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "sarien.h"
#include "agi.h"
#include "objects.h"
#include "console.h"

struct agi_object *objects;		/* objects in the game */
int num_objects;

int load_objects (char *fname)
{
	int i, so, padsize;
	FILE *fp;
	UINT32 flen;
	UINT8 *mem;

	padsize = game_flags & ID_AMIGA ? 4 : 3;

	num_objects = 0;
	objects = NULL;

	fixpath (NO_GAMEDIR, fname);
	report ("Loading objects: %s\n", path);

	if ((fp = fopen((char*)path, "rb")) == NULL)
		return err_BadFileOpen;

	fseek (fp, 0, SEEK_END);
	flen = ftell (fp);
	fseek (fp, 0, SEEK_SET);

	if ((mem = calloc (1, flen + 32)) == NULL) {
		fclose (fp);
		return err_NotEnoughMemory;
	}

	fread (mem, 1, flen, fp);

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
		fclose (fp);
		free (mem);
#ifdef AGDS_SUPPORT
    		/* die with no error! AGDS game needs not to die to work!! :( */
		return err_OK;
#else
		/* no AGDS support, die with error */
		return err_BadResource;
#endif
	}

	num_objects = lohi_getword(mem) / padsize;

    	if ((objects = calloc (num_objects, sizeof(struct agi_object))) == NULL) {
		fclose (fp);
		free (mem);
    		return err_NotEnoughMemory;
	}

    	/* build the object list */
    	for (i = 0, so = padsize; i < num_objects; i++, so += padsize) {
		(objects + i)->location = lohi_getbyte (mem + so + 2);
    		if ((lohi_getword (mem + so) + padsize) < flen) {
			(objects+i)->name = strdup (mem +
				(lohi_getword (mem + so) + padsize));
	    	} else {
	    		printf ("ERROR: object %i name beyond object filesize! "
				"(%04x)\n", i, (lohi_getword (mem + so) + 3));
	    		(objects+i)->name = strdup ("");
	    	}
    	}

    	free(mem);
	fclose (fp);

	return err_OK;
}


void unload_objects ()
{
	int i;

	if (objects != NULL) {
		for (i = 0; i < num_objects; i++)
			free (objects[i].name);
		free (objects);
	}
}


int show_objects ()
{
	int i;

	printf(" ID   Objects\n");
	for (i=0; i<num_objects; i++)
		printf ("%3i - %s\n", (objects+i)->location, (objects+i)->name);

	printf ("\n%i objects\n", num_objects);

	return err_OK;
}

