/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#ifndef __AGI_OPCODES_H
#define __AGI_OPCODES_H

#ifdef __cplusplus
extern "C"{
#endif

struct agi_logicnames {
#ifdef USE_CONSOLE /* ifndef NO_DEBUG */
	char	*name;
#endif
	UINT16	num_args;
	UINT16	arg_mask;
};

extern	struct agi_logicnames	logic_names_test[];
extern	struct agi_logicnames	logic_names_cmd[];
extern	struct agi_logicnames	logic_names_if[];

void	debug_console	(int, int, char *);
int	test_if_code	(int);
void	new_room	(int);
void	execute_agi_command	(UINT8, UINT8 *);

#ifdef PATCH_LOGIC
void	patch_logic	(int);
#endif

#ifdef __cplusplus
};
#endif

#endif /* __AGI_OPCODES_H */
