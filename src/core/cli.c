/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#if !defined PALMOS && !defined WIN32

#include <stdio.h>
#include <string.h>
#include <getopt.h>

#include "sarien.h"
#include "sound.h"

extern char *optarg;


static void help (int argc, char **argv)
{
	printf (
"Syntax is -:- %s [switches] [gamedir]\n"
"\n"
"Where [switches] are optionally:\n"
#ifdef AGDS_SUPPORT
"  -a --agds          Enables sarien to play AGDS created games.\n"
#endif
"  -A --amiga         Forces the game to be seen as an Amiga word padded game.\n"
"  -C --crc           CRC and identify the game files and stop.\n"
#if 0
"  -c --cache-loading Cache loading, static resources remain cached in memory\n"
"                     but are only loaded as needed.\n"
#endif
#ifdef OPT_LIST_DICT
"  -d --list-dictionary\n"
"                     List dictionary words.\n"
#endif
"  -E --emulate-sound {type}\n"
"                     Emulate the sound of Sierra AGI running in different\n"
"                     computers. Valid emulations are pc and mac\n"
#if 0
"  -l -force-loading  Force loading of all volume information at start of\n"
"                     game which will give you faster play, this also implies\n"
"                     the caching.\n"
#endif
#if 0
"  -L --list-games    List all the games in the ID file\n"
#endif
"  -F --full-screen   Run in full-screen mode if allowed by the graphics device\n"
"  -g --no-gfx-optimizations\n"
"                     Disable optimized graphic driver hacks (if available).\n"
"  -h --help          Display this help screen.\n"
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
"  -r --fix-aspect-ratio\n"
"                     Adjust aspect ratio to match the PC EGA 320x200 screen.\n"
#endif
"  -v --emulate-version {version}\n"
"                     Force version to emulate. Valid v2 game versions are:\n"
"                     2.089, 2.272, 2.440, 2.917, 2.936. Valid v3 games are:\n"
"                     3.002.086, 3.002.149.\n"
"  -V --version       Display version information.\n"
"  -x --no-x-shm      Disable X shared memory extension (if available).\n"
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
	struct vstrings {
		UINT16	vers;
		UINT8	*string;
	} cmp_versions[]= {
		{0x2089, (UINT8*)"2.089"},
		{0x2272, (UINT8*)"2.272"},
		{0x2440, (UINT8*)"2.440"},
		{0x2917, (UINT8*)"2.917"},
		{0x2936, (UINT8*)"2.936"},

		{0x3086, (UINT8*)"3.002.086"},
		{0x3149, (UINT8*)"3.002.149"},
		{0x0, (UINT8*)""}
	};
	int o, optidx = 0;
#define OPTIONS "AaCDdE:FghLlnoprskS:Vv:x"
	static struct option lopt[] = {
		{ "version",		0, 0, 'V' },
		{ "help",		0, 0, 'h' },
#ifdef AGDS_SUPPORT
		{ "agds",		0, 0, 'a' },
#endif
		{ "amiga",		0, 0, 'A'},
		{ "emulate-version",	1, 0, 'v' },
#if 0
		{ "force-loading",	0, 0, 'l' },
		{ "cache-loading",	0, 0, 'c' },
#endif
		{ "crc",		0, 0, 'C'},
		{ "list-games",		0, 0, 'L'},
#ifdef OPT_LIST_DICT
		{ "list-dictionary",	0, 0, 'd' },
#endif
		{ "debug",		0, 0, 'D' },
		{ "full-screen",	0, 0, 'F' },
		{ "no-sound",		0, 0, 'n' },
#ifdef OPT_LIST_OBJECTS
		{ "list-objects",	0, 0, 'o' },
#endif
#ifdef OPT_PICTURE_VIEWER
		{ "picture-viewer",     0, 0, 'p' },                            
#endif
		{ "emulate-sound",	1, 0, 'E' },
		{ "wait-key",		0, 0, 'k' },
		{ "no-x-shm",		0, 0, 'x' },
		{ "fix-aspect-ratio",   0, 0, 'r' },
		{ "scale",		1, 0, 'S' },
		{ "no-gfx-optimizations",0,0, 'g' }
	};

	UINT16 xc;
	UINT16 ec=err_OK;

	/* FIXME: Add support for a rc file for UNIX */

	/* Set defaults */
	memset (&opt, 0, sizeof (struct sarien_options));
	opt.gamerun = gRUN_GAME;
	opt.scale = 2;
	opt.gfxhacks = TRUE;
#ifdef MITSHM
	opt.mitshm = TRUE;
#endif
	opt.soundemu = SOUND_EMU_NONE;

	while ((o = getopt_long (argc, argv, OPTIONS, lopt, &optidx)) != -1) {
		switch (o) {
		case 'V':
			printf ("Compiled on " __DATE__ "; " __TIME__ "\n");
			printf ("Disable Copyprotection is ");
#ifdef DISABLE_COPYPROTECTION
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
#ifdef USE_EGA_PALETTE
			printf("Using EGA colour palette\n");
#else
			printf("Using AMIGA colour palette\n");
#endif
			exit (0);
		case 'A':
			opt.amiga = TRUE;
			break;
		case 'C':
			opt.gamerun=gCRC;
			break;
		case 'E':
			if (!strcmp (optarg, "pc"))
				opt.soundemu = SOUND_EMU_PC;
			else if (!strcmp (optarg, "mac"))
				opt.soundemu = SOUND_EMU_MAC;
			else {
				fprintf (stderr, "Sound emulation \"%s\" is "
					"unknown\n", optarg);
				exit (0);
			}
			break;
		case 'F':
			opt.fullscreen = TRUE;
			break;
		case 'L':
			opt.gamerun = gLIST_GAMES;
			break;
		case 'g':
			opt.gfxhacks = FALSE;
			break;
#ifdef OPT_LIST_DICT
		case 'd':
			opt.gamerun = gSHOW_WORDS;
			break;
#endif
#ifdef OPT_LIST_OBJECTS
		case 'o':
			opt.gamerun = gSHOW_OBJECTS;
			break;
#endif
#ifdef OPT_PICTURE_VIEWER
		case 'p':
			opt.picview = TRUE;
			break;
#endif
		case 'n':
			opt.nosound = TRUE;
			break;
		case 'v':
			for (xc = 0; xc != 0xfff0 && cmp_versions[xc].vers; xc++) {
				if (!strcmp ((char*)optarg, cmp_versions[xc].string)) {
					opt.emuversion = cmp_versions[xc].vers;
					xc = 0xfff0 - 1;
				}
			}
			if (!opt.emuversion) {
				printf ("Error: bad AGI version number \"%s\"\n",
					optarg);
				exit (-1);
			}
			break;
		case 'a':
			opt.agds = TRUE;
			break;
		case 'c':
			opt.cache = TRUE;
			break;
		case 'f':
			opt.forceload = TRUE;
			opt.cache = TRUE;
			break;
#ifndef __MSDOS__
		case 'r':
			opt.fixratio = TRUE;
			break;
		case 'S':
			opt.scale = strtoul (optarg, NULL, 0);
			if (opt.scale < 1)
				opt.scale = 1;
			if (opt.scale > 4)
				opt.scale = 4;
			break;
#endif
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

#endif /* !PALMOS && !WIN32 */
