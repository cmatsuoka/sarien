/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999,2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#ifndef __AGI_H
#define __AGI_H

#ifdef __cplusplus
extern "C"{
#endif

struct agi_dir {
	UINT8  volume;
	UINT32 offset;
	UINT32 len;
	UINT32 clen;
	UINT8  flags;
	/* 0 = not in mem, can be freed
	 * 1 = in mem, can be released
	 * 2 = not in mem, cant be released
	 * 3 = in mem, cant be released
	 * 0x40 = was compressed
	 */
};

struct agi_loader {
	int version;
	int int_version;
	int (*init)(void);
	int (*deinit)(void);
	int (*detect_game)(UINT8 *gname);
	int (*load_resource)(int restype, int resnum);
	int (*unload_resource)(int restype, int resnum);
};


extern	UINT8	*gdir;
extern	UINT8	*gname;
extern	UINT8	*gid;
extern	UINT8	path[];
extern	UINT8	horizon;

extern	struct agi_dir	dir_logic[];
extern	struct agi_dir	dir_pic[];
extern	struct agi_dir	dir_view[];
extern	struct agi_dir	dir_sound[];

extern	UINT8		flags[];
extern	UINT8		vars[];

extern	UINT16		ego_in_new_room;
extern	UINT8		control_mode;
extern	UINT8		quit_prog_now;
extern	UINT8		status_line;
extern	UINT8		line_status;
extern	UINT8		line_user_input;
extern	UINT8		line_min_print;
extern	UINT8		allow_kyb_input;
extern	UINT8		clock_enabled;
extern	UINT8		timed_message_box;
extern	UINT16		message_box_key;
extern	UINT32		game_flags;

int	agi_init	(void);
int	agi_deinit	(void);
char	*agi_printf	(char *, int);

#ifdef __cplusplus
};
#endif
#endif
