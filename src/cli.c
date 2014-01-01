/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#include "sarien.h"

#ifdef USE_COMMAND_LINE

#include <stdio.h>
#include <string.h>
#include <getopt.h>

#include "sound.h"

extern char *optarg;


static void help (int argc, char **argv)
{
	/* printf() breaks GCC 3.0 build */
	fprintf (stdout,
"Syntax is -:- %s [switches] [gamedir]\n"
"\n"
"Where [switches] are optionally:\n"
#ifdef AGDS_SUPPORT
"  -a --agds          Enables sarien to play AGDS created games.\n"
#endif
"  -A --amiga         Forces the game to be seen as an Amiga word padded game.\n"
"  -C --crc           CRC and identify the game files and stop.\n"
#ifndef PCCGA
"  -c --cga-palette   Use PC CGA video mode emulation.\n"
#endif
#ifdef OPT_LIST_DICT
"  -d --list-dictionary\n"
"                     List dictionary words.\n"
#endif
#ifndef PCCGA
"  -e --ega-palette   Use PC EGA palette instead of amiga-ish palette\n"
#endif
"  -L --list-games    List all the games in the ID file\n"
"  -F --full-screen   Run in full-screen mode if allowed by the graphics device\n"
"  -g --no-gfx-optimizations\n"
"                     Disable optimized graphic driver hacks (if available).\n"
#ifdef USE_HIRES
"  -H --hires {0|1}   Enable/disable hi-res mode.\n"
#endif
"  -h --help          Display this help screen.\n"
#ifdef USE_MOUSE
#ifndef __MSDOS__
"  -m --agimouse      AGI Mouse 1.0 compatibility mode.\n"
#endif
#endif
"  -n --no-sound      Disable sound output.\n"
#ifdef OPT_PICTURE_VIEWER
"  -p --picture-viewer\n"
"                     Interactive picture viewer.\n"
#endif
#ifdef OPT_LIST_OBJECTS
"  -o --list-objects  List objects.\n"
#endif
#ifndef __MSDOS__
"  -S --scale {num}   Window size scale (only for windowed graphics).\n"
#ifndef __MSDOS__
"  -s --emulate-sound {type}\n"
"                     Emulate the sound of Sierra AGI running in different\n"
"                     computers. Valid emulations are pc, mac and amiga\n"
#endif
"  -r --aspect-ratio {0|1}\n"
"                     Adjust aspect ratio to match the PC EGA 320x200 screen.\n"
#endif
"  -v --emulate-version {version}\n"
"                     Force version to emulate. Valid v2 game versions are:\n"
"                     2.089, 2.272, 2.440, 2.917, 2.936. Valid v3 games are:\n"
"                     3.002.086, 3.002.149.\n"
"  -V --version       Display version information.\n"
#ifndef __MSDOS__
"  -x --no-x-shm      Disable X shared memory extension (if available).\n"
#endif
#if 0
"  -xd --dga          Use XFree86 DGA extension (if available).\n"
#endif
"\n"
"[gamedir] is optionally the directory the game is in, if no directory is\n"
"specified, the current directory is assumed.\n"
"\n"
"Example: %s -v2.272 /games/sierra/sq1\n", argv[0], argv[0]);
}


int parse_cli (int argc, char **argv)
{
	int i;
	int ec = err_OK;

	struct {
		UINT16 vers;
		char *string;
	} cmp_versions[]= {
		{ 0x2089, "2.089" },
		{ 0x2272, "2.272" },
		{ 0x2440, "2.440" },
		{ 0x2917, "2.917" },
		{ 0x2936, "2.936" },
		{ 0x3086, "3.002.086" },
		{ 0x3149, "3.002.149" },
		{ 0x0, "" }
	};
	int o, optidx = 0;
#define OPTIONS "AaCcDdE:eFgH:hLlmnopr:skS:Vv:x"
	static struct option lopt[] = {
		{ "version",		0, 0, 'V' },
		{ "help",		0, 0, 'h' },
#ifdef AGDS_SUPPORT
		{ "agds",		0, 0, 'a' },
#endif
		{ "amiga",		0, 0, 'A'},
		{ "emulate-version",	1, 0, 'v' },
		{ "crc",		0, 0, 'C'},
#ifndef PCCGA
		{ "cga-palette",	0, 0, 'c'},
#endif
		{ "list-games",		0, 0, 'L'},
#ifdef OPT_LIST_DICT
		{ "list-dictionary",	0, 0, 'd' },
#endif
#ifndef PCCGA
		{ "ega-palette",	0, 0, 'e' },
#endif
		{ "full-screen",	0, 0, 'F' },
#ifdef USE_HIRES
		{ "hires",		1, 0, 'H' },
#endif
		{ "no-sound",		0, 0, 'n' },
#ifdef OPT_LIST_OBJECTS
		{ "list-objects",	0, 0, 'o' },
#endif
#ifdef OPT_PICTURE_VIEWER
		{ "picture-viewer",     0, 0, 'p' },                            
#endif
		{ "wait-key",		0, 0, 'k' },
		{ "no-x-shm",		0, 0, 'x' },
		{ "aspect-ratio",       1, 0, 'r' },
#ifdef USE_MOUSE
		{ "agimouse",		0, 0, 'm' },
#endif
		{ "scale",		1, 0, 'S' },
#ifndef __MSDOS__
		{ "emulate-sound",	1, 0, 's' },
#endif
		{ "no-gfx-optimizations",0,0, 'g' }
	};

	/* FIXME: Add support for a rc file for UNIX */

	/* Set defaults */
	memset (&opt, 0, sizeof (struct sarien_options));
	opt.gamerun = GAMERUN_RUNGAME;
	opt.scale = 2;
	opt.fixratio = TRUE;
	opt.gfxhacks = TRUE;
#ifdef USE_HIRES
	opt.hires = TRUE;
#endif
#ifdef MITSHM
	opt.mitshm = TRUE;
#endif
	opt.soundemu = SOUND_EMU_NONE;

	while ((o = getopt_long (argc, argv, OPTIONS, lopt, &optidx)) != -1) {
		switch (o) {
		case 'V':
			printf ("Version : " VERSION " Compiled on " __DATE__ "; " __TIME__ "\n");
			printf ("Logic patching is ");
#ifdef PATCH_LOGIC
			printf ("enabled\n");
#else
			printf ("disabled\n");
#endif
			printf("AGDS games and Russian/Cyrillic font is ");
#ifdef AGDS_SUPPORT
			printf("supported\n");
#else
			printf("not supported\n");
#endif
			printf("Listing Objects is ");
#ifdef OPT_LIST_OBJECTS
			printf("supported\n");
#else
			printf("not supported\n");
#endif
			printf("Dictionary Listing is ");
#ifdef OPT_LIST_DICT
			printf("enabled\n");
#else
			printf("disabled\n");
#endif
			exit (0);
		case 'A':
			opt.amiga = TRUE;
			break;
		case 'a':
			opt.agds = TRUE;
			break;
		case 'C':
			opt.gamerun = GAMERUN_CRC;
			break;
#ifndef PCCGA
		case 'c':
			opt.cgaemu = TRUE;
			break;
#endif
#ifdef OPT_LIST_DICT
		case 'd':
			opt.gamerun = GAMERUN_WORDS;
			break;
#endif
#ifndef PCCGA
		case 'e':
			opt.egapal = TRUE;
			break;
#endif
		case 'F':
			opt.fullscreen = TRUE;
			break;
		case 'L':
			opt.gamerun = GAMERUN_GAMES;
			break;
		case 'g':
			opt.gfxhacks = FALSE;
			break;
#ifdef USE_HIRES
		case 'H':
			opt.hires = strtoul (optarg, NULL, 0);
			break;
#endif
#ifdef USE_MOUSE
		case 'm':
			opt.agimouse = TRUE;
			break;
#endif
		case 'n':
			opt.nosound = TRUE;
			break;
#ifdef OPT_LIST_OBJECTS
		case 'o':
			opt.gamerun = GAMERUN_OBJECTS;
			break;
#endif
#ifdef OPT_PICTURE_VIEWER
		case 'p':
			opt.gamerun = GAMERUN_PICVIEW;
			break;
#endif
#ifndef __MSDOS__
		case 'r':
			opt.fixratio = strtoul (optarg, NULL, 0);
			break;
		case 'S':
			opt.scale = strtoul (optarg, NULL, 0);
			if (opt.scale < 1)
				opt.scale = 1;
			if (opt.scale > 4)
				opt.scale = 4;
			break;
		case 's':
			if (!strcmp (optarg, "pc"))
				opt.soundemu = SOUND_EMU_PC;
			else if (!strcmp (optarg, "mac"))
				opt.soundemu = SOUND_EMU_MAC;
			else if (!strcmp (optarg, "amiga"))
				opt.soundemu = SOUND_EMU_AMIGA;
			else {
				fprintf (stderr, "Sound emulation \"%s\" is "
					"unknown\n", optarg);
				exit (0);
			}
			break;
#endif
		case 'v':
			for (i = 0; cmp_versions[i].vers; i++) {
				if (!strcmp (optarg, cmp_versions[i].string)) {
					opt.emuversion = cmp_versions[i].vers;
					break;
				}
			}
			if (!opt.emuversion) {
				printf ("Error: bad AGI version number \"%s\"\n",
					optarg);
				exit (-1);
			}
			break;
#ifdef MITSHM
		case 'x':
			opt.mitshm = FALSE;
			break;
#endif
		case 'h':
			help (argc, argv);
		default:
			exit (-1);
		}
	}

	return ec;
}

#endif /* USE_COMMAND_LINE */

