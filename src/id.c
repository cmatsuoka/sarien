/* Sarien - A Sierra AGI resource interpreter engine
 * Copyright (C) 1999-2003 Stuart George and Claudio Matsuoka
 *
 * $Id$
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; see docs/COPYING for further details.
 */

#include <stdio.h>
#include <string.h>
#include "sarien.h"
#include "agi.h"
#include "opcodes.h"

/*
 * Determine what AGI v2 system to emulate, these are the major version
 * to emulate, thus 2.915 comes under 2.917, 2.4xxx is 2.440, etc.
 *
 * 0x2089
 * 0x2272
 * 0x2440
 * 0x2917
 * 0x2936
 */

int setup_v2_game(int ver, UINT32 crc);
int setup_v3_game(int ver, UINT32 crc);
int v4id_game (UINT32 crc);

#ifdef USE_COMMAND_LINE

void list_games ()
{
	FILE *f;
	char *c, *t, buf[256];
	UINT32 id, ver;
	int min, maj, i = 0;

	if ((f = fopen (get_config_file(), "r")) == NULL) {
		printf ("Configuration file not found.\n");
		return;
	}

	printf (
"Game#  AGI ver.   Title                                    CRC\n"
"------ ---------- ---------------------------------------- -------\n"
	);

	while (!feof (f)) {
		fgets (buf, 256, f);
		c = strchr (buf, '#');
		if (c) *c = 0;

		/* Remove spaces at end of line */
		if (strlen (buf)) {
			for (c = buf + strlen (buf) - 1;
				*c == ' ' || *c == '\t'; *c-- = 0) {}
		}


		t = strtok (buf, " \t\r\n");
		if (t == NULL)
			continue;
		id = strtoul (t, NULL, 0);

		t = strtok (NULL, " \t\r\n");
		if (t == NULL)
			continue;
		ver = strtoul (t, NULL, 0);
		maj = (ver >> 12) & 0xf;
		min = ver & 0xfff;

		t = strtok (NULL, "\n\r");

		if (maj == 2) {
			printf ("[%3d]  %x.%03x      %-40.40s 0x%05x\n",
				++i, maj, min, t, id);
		} else {
			printf ("[%3d]  %x.002.%03x  %-40.40s 0x%05x\n",
				++i, maj, min, t, id);
		}
	}

	fclose (f);
}

#endif

UINT32 match_crc (UINT32 crc, char *path, char *name, int len)
{
	FILE *f;
	char *c, *t, buf[256];
	UINT32 id, ver;

	if ((f = fopen (path, "r")) == NULL)
		return 0;

	while (!feof (f)) {
		fgets (buf, 256, f);
		c = strchr (buf, '#');
		if (c) *c = 0;

		/* Remove spaces at end of line */
		if (strlen (buf)) {
			for (c = buf + strlen (buf) - 1;
				*c == ' ' || *c == '\t'; *c-- = 0) {}
		}

		t = strtok (buf, " \t\r\n");
		if (t == NULL)
			continue;
		id = strtoul (t, NULL, 0);

		t = strtok (NULL, " \t\r\n");
		if (t == NULL)
			continue;
		ver = strtoul (t, NULL, 0);

		t = strtok (NULL, "\n\r");
		for (; *t == ' ' || *t == '\t'; t++);

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
#ifdef USE_MOUSE
					case 'm':
						opt.agimouse = TRUE;
						break;
#endif
					}
				}
				t++;

				for (; *t == ' ' || *t == '\t'; t++) {}
			}

			strncpy (name, t, len);
			fclose (f);

			return ver;
		}
	}

	fclose (f);

	return 0;
}


static UINT32 match_version (UINT32 crc)
{
	int ver;
	char name[80];

	if ((ver = match_crc(crc, get_config_file(), name, 80)) > 0)
		report ("AGI game detected: %s\n\n", name);

	return ver;
}


int v2id_game ()
{
	int y, ver;
	UINT32 len, c, crc;
	UINT8 *buff;
	FILE *fp;
	char *fn[] = { "viewdir", "logdir", "picdir", "snddir",
		"words.tok", "object", "" };

	buff = malloc (8192);

	for (crc = y = 0; fn[y][0]; y++) {
		char *path = fixpath (NO_GAMEDIR, fn[y]);
		if ((fp = fopen (path, "rb")) != NULL) {
			for (len = 1; len > 0; ) {
				memset (buff, 0, 8192);
				len = fread (buff, 1, 8000, fp);
				for (c = 0; c < len; c++)
					crc += *(buff + c);
			}
			fclose (fp);
		}
	}
	free (buff);

	report ("Computed CRC: 0x%05x\n", crc);
	ver = match_version (crc);
	game.crc = crc;
	game.ver = ver;
	_D (_D_WARN "game.ver = 0x%x", game.ver);
	agi_set_release (ver);

	return setup_v2_game(ver, crc);
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
	int ec = err_OK, y, ver;
	UINT32 len, c, crc;
	UINT8 *buff;
	FILE *fp;
	char *path, *fn[] = { "words.tok", "object", "" };

	buff = malloc (8192);

	for (crc = 0, y = 0; fn[y][0] != 0x0; y++) {
		path = fixpath (NO_GAMEDIR, fn[y]);
		if ((fp = fopen (path, "rb")) != NULL) {
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

	/* now do the directory file */

	path = fixpath (GAMEDIR, DIR_);

	if ((fp = fopen(path, "rb")) != NULL) {
		for (len = 1; len > 0; ) {
			memset (buff, 0, 8192);
			len = fread (buff, 1, 8000, fp);
			for (c = 0; c < len; c++)
				crc += *(buff + c);
		}
		fclose (fp);
	}

	free (buff);

	report ("Computed CRC: 0x%05x\n", crc);
	ver = match_version (crc);
	game.crc = crc;
	game.ver = ver;
	agi_set_release (ver);

	ec = setup_v3_game(ver, crc);

	return ec;
}

/**
 *
 */
int setup_v2_game (int ver, UINT32 crc)
{
	int ec=err_OK;

	if (ver == 0) {
		report ("Unknown v2 Sierra game: %08x\n\n", crc);
		agi_set_release (0x2917);
	}

	/* setup the differences in the opcodes and other bits in the
	 * AGI v2 specs
	 */
	if (opt.emuversion)
		agi_set_release (opt.emuversion);

	if (opt.agds)
		agi_set_release (0x2440);/* ALL AGDS games built for 2.440 */

	switch(agi_get_release ()) {
	case 0x2089:
		logic_names_cmd[0x86].num_args = 0;	/* quit: 0 args */
		logic_names_cmd[0x97].num_args = 3;	/* print.at: 3 args */
		logic_names_cmd[0x98].num_args = 3;	/* print.at.v: 3 args*/
		break;
	case 0x2272:
		/* KQ3 0x88673 (2.272) requires print.at with 4 arguments */
		break;
	case 0x2440:
		break;
	case 0x2917:
		break;
	case 0x2936:
		break;
	default:
		report ("** Cannot setup for unknown version\n");
		ec = err_UnknownAGIVersion;
		break;
	}

	return ec;
}

/**
 *
 */
int setup_v3_game (int ver, UINT32 crc)
{
	int ec = err_OK;

	if (ver == 0) {
		report ("Unknown v3 Sierra game: %08x\n\n", crc);
		agi_set_release (ver = 0x3149);
	}

	if (opt.emuversion)
		agi_set_release (ver = opt.emuversion);

	switch(ver) {
	case 0x3086:
		logic_names_cmd[0xad].num_args = 1;	/* 173 : 1 args */
		break;
	case 0x3149:
		logic_names_cmd[0xad].num_args = 0;	/* 173 : 0 args */
		break;
	default:
		report ("Error: cannot setup for unknown version\n");
		ec = err_UnknownAGIVersion;
		break;
	}

	return ec;
}
