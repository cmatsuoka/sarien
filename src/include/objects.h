/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#ifndef __AGI_OBJECTS_H
#define __AGI_OBJECTS_H

#ifdef __cplusplus
extern "C"{
#endif

struct agi_object {
	int location;
	char *name;
};

int	show_objects	(void);
int	load_objects	(char *fname);
void	unload_objects	(void);


#ifdef __cplusplus
};
#endif

#endif /* __AGI_OBJECTS_H */
