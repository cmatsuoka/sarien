/* Sarien - A Sierra AGI resource interpreter engine
 * Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *
 * $Id$
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; see docs/COPYING for further details.
 */

#include <stdio.h>
#include <string.h>
#include "sarien.h"
#include "agi.h"

int get_app_dir (char *app_dir, unsigned int size)
{
	strncpy (app_dir, "agi:", size);
	return 0;
}

char* get_config_file (void)
{
	return "agi:sarien.conf";
}

