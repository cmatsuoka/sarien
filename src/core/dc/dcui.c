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

#include <kos.h>
#include <mp3/sfxmgr.h>
#include <mp3/sndmp3.h>

#include "sarien.h"
#include "image.h"
#include "gslogo.h"
#include "pvrutils.h"
#include "gsi.h"
#include "util.h"

struct game_list_t
{
	char name[255];
	char dir[255];
} game_list_t;

#define BAR_H			(16)
#define BAR_W			(448)
#define BOX_LEFT		(96.0f)
#define BOX_TOP			(127.0f) + (20.0f)
#define MAX_GAMES		(100)
#define GAMES_PER_PAGE	(15)

struct game_list_t gamelist[MAX_GAMES];

extern char g_gamename[255];
static int numgames;
static int selected = -1;
static int page = 0;
int scroll_idx, select_idx;
IMAGE imgMenu, imgFont, imgWait, imgInsert;

void sort_list()
{
	int i, j;
	struct game_list_t gltemp;

	for(i = 0; i < numgames; i++)
	{
		for(j = 0; j < numgames; j++)
		{
			if(strcmp(gamelist[i].name, gamelist[j].name) < 0)
			{
				strcpy(gltemp.name, gamelist[i].name);
				strcpy(gltemp.dir, gamelist[i].dir);
				strcpy(gamelist[i].name, gamelist[j].name);
				strcpy(gamelist[i].dir, gamelist[j].dir);
				strcpy(gamelist[j].name, gltemp.name);
				strcpy(gamelist[j].dir, gltemp.dir);
			}
		}
	}
}

int draw_select_bar(float x, float y)
{
	vertex_oc_t v;
	poly_hdr_t	hdr;

	ta_poly_hdr_col(&hdr, TA_TRANSLUCENT);
	ta_commit_poly_hdr(&hdr);

	v.a = 0.5f; v.r = 1.0f; v.g = v.b = 0.0f;

	v.x = x;
	v.y = y + BAR_H;
	v.z = 21.0f;
	v.flags = TA_VERTEX_NORMAL;
	ta_commit_vertex(&v, sizeof(v));

	v.x = x;
	v.y = y;
	ta_commit_vertex(&v, sizeof(v));

	v.x = x + BAR_W;
	v.y = y + BAR_H;
	ta_commit_vertex(&v, sizeof(v));

	v.x = x + BAR_W;
	v.y = y;
	v.flags = TA_VERTEX_EOL;
	ta_commit_vertex(&v, sizeof(v));
}

int poll_keyboard()
{
	static uint8 mkb = 0;
	int key;

	if(!mkb)
	{
		mkb = maple_first_kb();
		if (!mkb)
		{
			printf("No keyboard attached\n");
			return 0;
		}
	}

	if (kbd_poll(mkb))
	{
		printf("Error checking keyboard status\r\n");
		return 0;
	}

	key = kbd_get_key();

	/* ENTER */
	if(key == 0x0d)
	{
		snd_sfx_play(select_idx, 255, 0x80);
		return 1;
	}

	if(key>>8 == KBD_KEY_UP)
	{
		if(selected > 0)
		{
			selected--;
			if(selected != 0)
			{
				if(selected % (GAMES_PER_PAGE-1) == 0)
					page--;
			}
			snd_sfx_play(scroll_idx, 255, 0x80);
		}
	}

	if(key>>8 == KBD_KEY_DOWN)
	{
		if(selected < numgames-1)
		{
			selected++;
			if(selected % GAMES_PER_PAGE == 0)
				page++;
			snd_sfx_play(scroll_idx, 255, 0x80);
		}
	}

	if(key>>8 == KBD_KEY_PGDOWN)
	{
		if(selected < numgames-1 && selected < GAMES_PER_PAGE)
		{
			page++;
			selected = page * GAMES_PER_PAGE;
			snd_sfx_play(scroll_idx, 255, 0x80);
		}
	}

	if(key>>8 == KBD_KEY_PGUP)
	{
		if(selected >= GAMES_PER_PAGE)
		{
			page--;
			selected = page * GAMES_PER_PAGE;
			snd_sfx_play(scroll_idx, 255, 0x80);
		}
	}

	return 0;
}

int dc_logos()
{
	init_logo(DC_GFX_PATH"/ganksoft.gsi");
	gs_logo(120);
	shutdown_logo();
	pvr_clear_texture_ram();

	init_logo(DC_GFX_PATH"/title.gsi");
	gs_logo(240);
	shutdown_logo();
	pvr_clear_texture_ram();
	return 0;
}

int dc_init_ui()
{
	if(load_gsi(&imgMenu, DC_GFX_PATH"/menu.gsi") == -1)
	{
		printf("Couldn't open menu.gsi");
		return -1;
	}

	if(load_gsi(&imgFont, DC_GFX_PATH"/font.gsi") == -1)
	{
		printf("Couldn't open font.gsi");
		return -1;
	}

	if(load_gsi(&imgWait, DC_GFX_PATH"/wait.gsi") == -1)
	{
		printf("Couldn't open wait.gsi");
		return -1;
	}

	if(load_gsi(&imgInsert, DC_GFX_PATH"/insert.gsi") == -1)
	{
		printf("Couldn't open insert.gsi");
		return -1;
	}
	mp3_init();
	return 0;
}

int dc_ui(char *game)
{
	uint32 size;
	uint32 tex_addr, font_addr, wait_addr, insert_addr;
	float f;
	int i, j;
	dirent_t *de;
	file_t fd;
	char filename[255];

	ta_txr_release_all();
	pvr_clear_texture_ram();

	vid_clear(0,0,0);
	vid_set_mode(DM_640x480, PM_RGB565);
	ta_init_defaults();

	mp3_start("/rd/adventure.mp3", 1);
	scroll_idx = snd_sfx_load("/rd/scroll.wav");
	select_idx = snd_sfx_load("/rd/select.wav");

	size = imgMenu.w * imgMenu.h * 2;
	tex_addr = ta_txr_allocate(size);
	ta_txr_load(tex_addr, imgMenu.data, size);

	size = imgFont.w * imgFont.h * 2;
	font_addr = ta_txr_allocate(size);
	ta_txr_load(font_addr, imgFont.data, size);

	size = imgWait.w * imgWait.h * 2;
	wait_addr = ta_txr_allocate(size);
	ta_txr_load(wait_addr, imgWait.data, size);

	size = imgInsert.w * imgInsert.h * 2;
	insert_addr = ta_txr_allocate(size);
	ta_txr_load(insert_addr, imgInsert.data, size);

	for (f=0.0f; f<1.0f; f+=0.01f)
	{
		ta_begin_render();
		gs_draw_bkg(f, tex_addr);
		ta_commit_eol();
		pvr_dummy_poly(TA_TRANSLUCENT);
		ta_commit_eol();
		ta_finish_frame();
	}

	if(selected == -1)
	{
		selected = 0;
		printf("reading dir\n");

		fd = fs_open(DC_BASE_PATH"/games",  O_RDONLY | O_DIR);

		while(!fd)
		{
			iso_ioctl(0, 0, 0);
			for(i = 0; i < 2; i++)
			{
				ta_begin_render();
				gs_draw_bkg(1.0f, tex_addr);
				ta_commit_eol();
				draw_bm_scale(insert_addr, 1.0f, 1.0f, 512, 128, 0.0f, 0.0f, 1);
				ta_commit_eol();
				ta_finish_frame();
			}
			fd = fs_open(DC_BASE_PATH"/games",  O_RDONLY | O_DIR);
		}

		while((de = fs_readdir(fd)))
		{
			ta_begin_render();
			gs_draw_bkg(1.0f, tex_addr);
			ta_commit_eol();
			draw_bm_scale(wait_addr, 1.0f, 1.0f, 512, 128, 0.0f, 0.0f, 1);
			ta_commit_eol();
			ta_finish_frame();

			printf("de->name=%s, de->attr=%d\n", de->name, de->attr);
			sprintf(filename, DC_BASE_PATH"/games/%s/", de->name);
			agi_detect_game(filename);
			printf("g_gamename:%s\n", g_gamename);
			if(strcmp(g_gamename, "Unknown") == 0)
				sprintf(g_gamename, "%s (%s)", g_gamename, de->name);
			strcpy(gamelist[numgames].name, g_gamename);
			strcpy(gamelist[numgames].dir, de->name);
			numgames = numgames + 1;
		}
		fs_close(fd);
		sort_list();
		printf("done\n");
	}

	while(1)
	{
		ta_begin_render();
		gs_draw_bkg(1.0f, tex_addr);
		for(j = 0; j < (numgames > GAMES_PER_PAGE ? GAMES_PER_PAGE : numgames); j++)
		{
			strcpy(filename, gamelist[j + (page * GAMES_PER_PAGE)].name);
			filename[28] = '\0';
			draw_string(font_addr, BOX_LEFT, BOX_TOP + (j * 16.0f), filename);
		}
		ta_commit_eol();
		draw_select_bar(BOX_LEFT, BOX_TOP + ((selected % GAMES_PER_PAGE) * BAR_H));
		ta_commit_eol();
		ta_finish_frame();
		i = poll_keyboard();
		if(i)
			break;
	}
	strcpy(game, gamelist[selected].dir);
	pvr_clear_texture_ram();
	ta_txr_release_all();
	snd_sfx_unload_all();
	mp3_stop();
	return i;
}

