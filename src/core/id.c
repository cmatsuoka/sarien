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
				t++;

				for (; *t == ' ' || *t == '\t'; t++);
			}

			report ("AGI game detected: %s\n\n", t);
			fclose (f);
			return ver;
		}
	}

	fclose (f);

	return 0;
}


/* FIXME: use registry read function from savegame.c
 * DF, Rosinha or Igor, please fix it accordingly
 */
static UINT32 match_version (UINT32 crc)
{

#ifdef WIN32
	char buf[256];
	int ver;
	char *q;

	strcpy(buf, "./");

	if(getenv("SARIEN")!=NULL)
	{
		sprintf(buf, "%s/%s", getenv("SARIEN"), "sarien.ini");
	}
	strcat(buf, "/sarien.ini");
	ver = match_crc (crc, buf);
#else
#ifdef _M_MSDOS
	char buf[256];
	int ver;
	char *q;

	if(getenv("SARIEN")!=NULL)
	{
		sprintf(buf, "%s/%s", getenv("SARIEN"), "sarien.ini");
	}
	else
	{
		strcpy(buf, exec_name);
		q=strchr(buf, 0x0);
		q--;
		while((*q!='\\' && *q!='/') && q>buf)
			q--;
		if(q!=buf)
			*q=0x0;
		strcat(buf, "/sarien.ini");
	}

	_D("sarien conf : (%s)", buf);
	ver = match_crc (crc, buf);
#else
	char buf[256];
	int ver;

	snprintf (buf, 256, "%s/.sarienrc", getenv ("HOME"));

	if (!(ver = match_crc (crc, buf)))
		ver = match_crc (crc, "/etc/sarien.conf");
#endif
#endif
	return ver;
}


int v2id_game ()
{
	int ec = err_OK, y, ver;
	UINT32 len, c, crc;
	UINT8 *buff;
	FILE *fp;
	char *fn[] = { "viewdir", "logdir", "picdir", "snddir",
		"words.tok", "object", "" };

	_D (_D_WARN "");
	buff = malloc (8192);

	for (crc = y = 0; fn[y][0]; y++) {
		char *path = fixpath (NO_GAMEDIR, fn[y]);
		if ((fp = fopen(path, "rb")) != NULL) {
			for (len = 1; len > 0; ) {
				memset (buff, 0, 8192);
				len = fread (buff, 1, 8000, fp);
				for (c = 0; c < len; c++)
					crc += *(buff + c);
			}
			fclose(fp);
		}
	}
	free(buff);

	ver = match_version (crc);
	agi_set_release (ver);
	ec = setup_v2_game(ver, crc);
	
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
	int ec = err_OK, y, ver;
	UINT32 len, c, crc;
	UINT8 *buff;
	FILE *fp;
	char *path, *fn[] = { "words.tok", "object", "" };

	buff = malloc (8192);

	for (crc = 0, y = 0; fn[y][0]!=0x0; y++) {
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

	/* no do the directory file */

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

	ver = match_version (crc);
	agi_set_release (ver);

	ec = setup_v3_game(ver, crc);

	
	return ec;
}

int v4id_game (UINT32 crc)
{
	int ec = err_OK, ver;	

	ver = match_version (crc);
	agi_set_release (ver);
	
	switch ((ver>>12)&0xFF) {
	case 2:
		ec = setup_v2_game (ver, crc);
		break;
	case 3:
		ec = setup_v3_game (ver, crc);
		break;
	}
			
	return ec;
}

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
	case 0x2272:
		logic_names_cmd[0x97].num_args = 3;	/* print.at: 3 args */
		logic_names_cmd[0x98].num_args = 3;	/* print.at.v: 3 args */
		break;
	case 0x2440:
		break;
	case 0x2917:
		break;
	case 0x2936:
		break;
	default:
		printf("** Cannot setup for unknown version\n");
		ec = err_UnknownAGIVersion;
		break;
	}

	return ec;
}

int setup_v3_game(int ver, UINT32 crc)
{
	int ec=err_OK;
	
	if (ver == 0) {
		printf("Unknown v3 Sierra game: %08x\n\n", crc);
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
		printf("Error: cannot setup for unknown version\n");
		ec = err_UnknownAGIVersion;
		break;
	}
	
	return ec;
}
