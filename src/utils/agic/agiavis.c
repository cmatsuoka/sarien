
/*
 *  AGI Logic Compiler
 *  Copyright (C) 1997, Floating Hills Software.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Send comments, ideas, and criticism to greekcat@hotmail.com
 *
 */

/* "Avis Durgan" Encryption/Decryption Module */
/* Author: Jewel of Mars */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "agic.h"

void EncryptBlock(char *buffer, int len)
{
    int codedex = 0;
    int i;
    char szCode[12] = "Avis Durgan";

    for (i = 0; i < len; i++) {
	buffer[i] ^= szCode[codedex];

	codedex++;
	codedex %= 11;
    }

    return;
}
