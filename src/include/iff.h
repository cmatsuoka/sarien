/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#ifndef __IFF_H
#define __IFF_H

#define IFF_NOBUFFER 0x0001

#define IFF_LITTLE_ENDIAN	0x01
#define IFF_FULL_CHUNK_SIZE	0x02

struct iff_header {
    UINT8 form[4];		/* FORM */
    UINT8 len[4];		/* File length */
    UINT8 id[4];		/* IFF type identifier */
};

struct iff_info {
    char id[5];
    void (*loader) ();
    struct iff_info *next;
    struct iff_info *prev;
};

void iff_chunk (FILE *);
void iff_register (char *, void (*)(int, UINT8*));
void iff_idsize (int);
void iff_setflag (int);
void iff_release (void);
int iff_process (char *, long, FILE *);

#endif /* __IFF_H */
