
#include <stdio.h>
#include <string.h>

#include "agi2pdb.h"

struct game_id_list {
	struct game_id_list *next;
	UINT32 version;
	UINT32 crc;
	char *gName;
	char *switches;
};


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

			printf ("AGI game detected: %s\n", t);
			strcpy(game_name, t);
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
#ifndef _M_MSDOS
	char buf[256];
	int ver;

	snprintf (buf, 256, "%s/.sarienrc", getenv ("HOME"));

	if (!(ver = match_crc (crc, buf)))
		ver = match_crc (crc, "/etc/sarien.conf");
#endif

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

	ver = match_crc (crc, buf);
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

	if (ver == 0) {
		printf("Unknown Sierra Game Version: %08lx\n", crc);
		ver=0x2917;
	}
	write_word((UINT8*)&palmagi.version, (UINT16)ver);
	write_dword((UINT8*)&palmagi.crc, (UINT32)crc);
	

	return ec;
}

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

	if (ver == 0) {
		printf("Unknown Sierra game version: %08lx\n", crc);
		ver=0x3149;
	}
	write_word((UINT8*)&palmagi.version, (UINT16)ver);
	write_dword((UINT8*)&palmagi.crc, (UINT32)crc);

	return ec;
}

