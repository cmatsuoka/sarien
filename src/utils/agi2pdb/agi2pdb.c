
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "agi2pdb.h"

#ifdef VERSION
#undef VERSION
#endif

#define VERSION "0.2"


#ifndef MACOSX_SDL
volatile UINT32 clock_ticks;
volatile UINT32 clock_count;
#endif

struct sarien_options opt;
struct agi_game game;

extern struct agi_loader *loader;

extern struct agi_loader agi_v2;
extern struct agi_loader agi_v3;



void DieWithError(char *errstr, Err err) {
	printf("%s", errstr);
	if (err)
		printf(" (%x )\n", err);
	else
		printf("\n");
	exit(1);
}

int main(int argc, char *argv[])
{
	char *dir = ".";
	char *pdb = NULL;
	int ec;

	printf("agi2pdb v" VERSION " - Converting Sierra AGI files to PalmOS Database\n"
		"(C) Copyright 2001-2002 Stuart George & Thomas Akesson\n"
		"\n"
		"This program is free software; you can redistribute it and/or modify it\n"
		"under the terms of the GNU General Public License, version 2 or later,\n"
		"as published by the the Free Software Foundation.\n"
		"\n");

	if(argc>3 || (argc==2 && argv[1][1]=='?')) {
		printf("syntax:\n"
			"  agi2pdb {directory} {game.pdb}\n");
		return 0;
	}
	else if (argc>1){
		dir = argv[1];
		if (argc>2)
			pdb = argv[2];
	}

	ec = agi_detect_game (dir);

	if (ec == err_OK) {
		if (loader == &agi_v2)
			printf("AGI version 2 game detected.\n");
		else
			printf("AGI version 3 game detected.\n");


		if (pdb == NULL) {
			pdb = (char*) malloc(32);
			match_crc_name (game.crc, get_config_file(), pdb);
		}
		if (game.ver) {
			agi_init();
			convert2pdb (loader, pdb);
		}
		else {
			printf("Game version could not be determined.\n"
			"Make sure Sarien configuration file is available!\n");
		}
	}
	else {
		printf("No AGI game detected.\n");
		return 1;
	}
	return 0;
}


/* Well, this is a hack to get the name from the conf file */
UINT32 match_crc_name (UINT32 crc, char *path, char *game_name)
{
	FILE *f;
	char *c, *t, buf[256];
	UINT32 id, ver;
	int len;

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

		t = strtok (NULL, "(\n\r");

		if (id == crc) {


			if (*t == '[') {
				while (*t != ']') {
					t++;
				}
				t++;

				for (; *t == ' ' || *t == '\t'; t++) {}
			}

			fclose (f);
			len = strlen(t);
			if (t[len-1] == ' ')
				len--;
			if (len > 31)
				len = 31;
			strncpy(game_name, t, len);
			game_name[len] = '\0';
			return ver;
		}
	}

	fclose (f);
	return 0;
}