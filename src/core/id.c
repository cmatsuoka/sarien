/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999 Dark Fiber, (C) 1999,2001 Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#include "sarien.h"
#include "agi.h"
#include "opcodes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern struct agi_loader *loader;
extern struct sarien_options opt;

/*
 * Determine what AGI v2 system to emulate, these are the "majour" version
 * to emulate, thus 2.915 comes under 2.917, 2.4xxx is 2.440, etc.
 *
 * 0x2089
 * 0x2272
 * 0x2440
 * 0x2917
 * 0x2936
 */


void list_games ()
{
}

static UINT32 match_crc (UINT32 crc, char *path)
{
	FILE *f;
	char *c, *t, buf[256];
	UINT32 id, ver;

	if ((f = fopen (path, "r")) == NULL)
		return 0;

	while (!feof(f)) {
		fgets (buf, 256, f);
		if ((c = strchr (buf, '#')))
			*c = 0;

		/* Remove spaces at end of line */
		if (strlen (buf)) {
			for (c = buf + strlen (buf) - 1;
				*c == ' ' || *c == '\t'; *c-- = 0);
		}
		

		if (!(t = strtok (buf, " \t\n")))
			continue;
		id = strtoul (t, NULL, 0);
		if (!(t = strtok (NULL, " \t\n")))
			continue;
		ver = strtoul (t, NULL, 0);
		t = strtok (NULL, "\n");

		if (id == crc) {
			/* Now we must check options enclosed in brackets
		 	 * like [A] for Amiga
			 */
		
			if (*t == '[') {
				while (*t != ']') {
					switch (*t++) {
					case 'A':
						opt.amiga = TRUE;
						break;
					case 'a':
						opt.agds = TRUE;
						break;
					}
				}
				for (; *t == ' ' || *t == '\t'; t++);
			}

			printf ("AGI game detected: %s\n", t);
			fclose (f);
			return ver;
		}
	}

	fclose (f);

	return 0;
}


/* This is UNIX-specific code and will break DOS/windows implementations
 * DF, Rosinha or Igor, please fix it accordingly
 */
static UINT32 match_version (UINT32 crc)
{
	char buf[256];
	int ver;

	snprintf (buf, 256, "%s/.sarienrc", getenv ("HOME"));

	if (!(ver = match_crc (crc, buf)))
		ver = match_crc (crc, "/etc/sarien.conf");

	return ver;
}


int v2id_game ()
{
	UINT8 *fn[] = {
		(UINT8*)"viewdir",
		(UINT8*)"logdir",
		(UINT8*)"picdir",
		(UINT8*)"snddir",
		(UINT8*)"words.tok",
		(UINT8*)"object",
		(UINT8*)""
	};
	UINT16	ec=err_OK, y;
	UINT32	len, c, crc;
	UINT8	*buff;
	FILE	*fp;

	buff=(UINT8*)malloc(8192);

	for (crc = y = 0; fn[y][0]; y++) {
		fixpath(NO_GAMEDIR, fn[y]);
		if ((fp = fopen((char*)path, "rb")) != NULL) {
			for (len = 1; len > 0; ) {
				memset (buff, 0x0, 8192);
				len = fread (buff, 1, 8000, fp);
				for (c = 0; c < len; c++)
					crc += *(buff + c);
			}
			fclose(fp);
		}
	}
	free(buff);

	loader->int_version = match_version (crc);

	if (loader->int_version == 0) {
		printf("Unknown Sierra Game Version: %08lx\n", crc);
			loader->int_version=0x2917;
	}

	/* setup the differences in the opcodes and other bits in the
	 * AGI v2 specs
	 */
	if (opt.emuversion)
		loader->int_version = opt.emuversion;

	if (opt.agds)
		loader->int_version=0x2440;/* ALL AGDS games built for 2.440 */

	switch(loader->int_version) {
	case 0x2089:
		logic_names_cmd[0x86].num_args=0;	/* quit: 0 args */
	case 0x2272:
		logic_names_cmd[0x97].num_args=3;	/* print.at: 3 args */
		logic_names_cmd[0x98].num_args=3;	/* print.at.v: 3 args */
		break;
	case 0x2440:
		break;
	case 0x2917:
		break;
	case 0x2936:
		break;
	default:
		printf("** Cannot setup for unknown version\n");
		ec=err_UnknownAGIVersion;
		break;
	}

	return ec;
}

/*
 * Currently, there is no known difference between v3.002.098 -> v3.002.149
 * So version emulated;
 *
 * 0x0086,
 * 0x0149
 */

int v3id_game ()
{
	UINT8 *fn[] = {
		(UINT8*)"words.tok",
		(UINT8*)"object",
		(UINT8*)""
	};
	UINT16	ec=err_OK, y;
	UINT32	len, c, crc;
	UINT8	*buff;
	FILE	*fp;

	buff = malloc (8192);

	for (crc = 0, y=0; fn[y][0]!=0x0; y++) {
		fixpath (NO_GAMEDIR, fn[y]);
		if ((fp=fopen((char*)path, "rb")) != NULL) {
			len = 1;
			while (len > 0) {
				memset (buff, 0, 8192);
				len = fread (buff, 1, 8000, fp);
				for (c = 0; c < len; c++)
					crc += *(buff + c);
			}
			fclose(fp);
		}
	}

	/* no do the directory file */

	fixpath (GAMEDIR, (UINT8*)DIR);

	if ((fp=fopen((char*)path, "rb")) != NULL) {
		for (len = 1; len > 0; ) {
			memset (buff, 0, 8192);
			len = fread (buff, 1, 8000, fp);
			for (c = 0; c < len; c++)
				crc += *(buff + c);
		}
		fclose (fp);
	}

	free (buff);

	loader->int_version = match_version (crc);

	if (loader->int_version == 0) {
		printf("Unknown Sierra game version: %08lx\n", crc);
		loader->int_version = 0x3149;
	}


	if (opt.emuversion)
		loader->int_version = opt.emuversion;

	switch(loader->int_version) {
	case 0x3086:
		logic_names_cmd[0xAD].num_args = 1;	/* 173 : 1 args */
		break;
	case 0x3149:
		logic_names_cmd[0xAD].num_args = 0;	/* 173 : 0 args */
		break;
	default:
		printf("Error: cannot setup for unknown version\n");
		ec = err_UnknownAGIVersion;
		break;
	}

	return ec;
}

