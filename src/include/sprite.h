/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#ifndef __AGI_SPRITE_H
#define __AGI_SPRITE_H

void	erase_upd_sprites	(void);
void	erase_nonupd_sprites	(void);
void	erase_both		(void);
void	blit_upd_sprites	(void);
void	blit_nonupd_sprites	(void);
void	blit_both		(void);
void	checkmove_both		(void);
void	update_graf		(void);
void	adjustpos_putblock	(struct vt_entry *v);
void	add_to_pic		(int, int, int, int, int, int, int);

#endif /* __AGI_SPRITE_H */
