/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  Dreamcast files Copyright (C) 2002 Brian Peek/Ganksoft Entertainment
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#include <stdio.h>
#include <string.h>
#include <kos.h>
#include "sarien.h"
#include "agi.h"
#include "text.h"
#include "graphics.h"
#include "dcui.h"
#include "crown.h"

volatile UINT32 clock_ticks;
volatile UINT32 clock_count;

extern int optind;

struct sarien_options opt;
struct game_id_list game_info;
struct agi_game game;

extern uint8 romdisk_boot[];

typedef struct
{
	char desc[16];
	char title[32];
	char app[16];
	uint16 num_icons;
	uint16 anim_speed;
	uint16 eyecatch_type;
	uint16 crc;
	uint32 size;
	uint8 dummy[20];
	uint16 palette[16];
} vms_file_header_t;

uint8 maple_create_port(uint8 addr, uint8 *port, uint8 *unit)
{
        *port = addr >> 6;
        *unit = 1 << ((addr & 0x1f) - 1);
        return 0;
}

int main(int argc, char *argv[])
{
	int ec				= err_OK;
	char filename[MAX_PATH];
	char gamename[MAX_PATH];

	kos_init_all(ALL_ENABLE, romdisk_boot);

	if(!maple_first_kb())
	{
		char *s = "Dreamcast Keyboard required.";
		ta_shutdown();
		bfont_draw_str(vram_s + (160 * 640) + 16*((40 - strlen(s))/2), 640, 0, s);
		printf("No keyboard attached!");
		while(1);
	}

	dc_logos();

	dc_init_ui();

	while(1)
	{
		if(dc_ui(filename) == -1)
			return -1;

		game.clock_enabled = FALSE;
		game.state = STATE_INIT;
	
		opt.scale = 2;

		init_machine (argc, argv);
	
		game.color_fg = 15;
		game.color_bg = 0;
	#ifdef USE_HIRES
		game.hires = malloc (_WIDTH * _HEIGHT * 2);
	#endif
	
		if (init_video () != err_OK)
		{
			printf("init_video() failed\n");
			goto bail_out;
		}
	
		report ("Enabling interpreter console\n");
		console_init ();
		report ("--- Starting console ---\n\n");
		if (!opt.gfxhacks)
			report ("Graphics driver hacks disabled (if any)\n");
	
		sprintf(gamename, "%s/games/%s/", DC_BASE_PATH, filename);
		printf("gamename=%s\n", gamename);

		_D ("Detect game");
	
		printf("agi_detect\n");
		if (agi_detect_game (gamename) == err_OK || agi_detect_game (get_current_directory ()) == err_OK)
		{
			printf("loaded\n");
			game.state = STATE_LOADED;
		}
		report("Init sound");
	
		game.ver = -1;	/* Don't display the conf file warning */
	
		_D ("Init sound");
		init_sound ();
	
		printf("done\n");
	
		report(" \nSarienDC " VERSION " is ready.\n");

		if (game.state < STATE_LOADED)
		{
       		console_prompt ();
			do
			{
				main_cycle ();
			} while (game.state < STATE_RUNNING);
			if (game.ver < 0)
				game.ver = 0;	/* Enable conf file warning */
		}
	
		ec = run_game ();
	
		deinit_sound ();
		deinit_video ();
#ifdef USE_HIRES
		free (game.hires);
#endif
	}

bail_out:
	deinit_machine ();

	return ec;
}

void write_vmu_header(FILE *f, char *desc)
{
	vms_file_header_t vfh;

	printf("write header");
	memset(&vfh, 0, sizeof(vfh));

	if(f)
	{
		strncpy(vfh.desc, desc, 16);
		strcpy(vfh.title, "SarienDC Save Game");
		strcpy(vfh.app, "SarienDC");
		vfh.num_icons = 1;
		vfh.anim_speed = 0;
		vfh.eyecatch_type = 0;
		vfh.crc = 0;
		vfh.size = 0;
		memcpy(&vfh.palette, &crown_palette, 16);

		fwrite(&vfh, sizeof(vfh), 1, f);
		fwrite(&crown_data, 512, 1, f);
	}
	printf("done\n");
}

void read_vmu_header(FILE *f)
{
	vms_file_header_t vfh;
	unsigned char icon[512];

	if(f)
	{
		fread(&vfh, sizeof(vfh), 1, f);
		fread(&icon, 512, 1, f);
	}
}
