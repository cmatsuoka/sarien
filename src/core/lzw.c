/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

/***************************************************************************
** decomp.c
**
** Routines that deal with AGI version 3 specific features.
** The original LZW code is from DJJ, October 1989, p.86.
** It has been modified to handle AGI compression.
**
** (c) 1997  Lance Ewing
***************************************************************************/

#include <stdio.h>
#include <string.h>

#include "sarien.h"
#include "lzw.h"

#define MAXBITS		12
#define TABLE_SIZE	18041		/* strange number */
#define START_BITS	9

static SINT32	BITS, MAX_VALUE, MAX_CODE;
static UINT32	*prefix_code;
static UINT8	*append_character;
static UINT8	*decode_stack;
static SINT32 input_bit_count=0;	/* Number of bits in input bit buffer */
static UINT32 input_bit_buffer=0L;

static void initLZW ()
{
	decode_stack = calloc (1, 8192);
	prefix_code= malloc (TABLE_SIZE * sizeof(UINT32));
	append_character= malloc (TABLE_SIZE * sizeof(UINT8));
	input_bit_count = 0;	/* Number of bits in input bit buffer */
	input_bit_buffer = 0L;
}

static void closeLZW ()
{
	free (decode_stack);
	free (prefix_code);
	free (append_character);
}

/***************************************************************************
** setBITS
**
** Purpose: To adjust the number of bits used to store codes to the value
** passed in.
***************************************************************************/
int setBITS (SINT32 value)
{
	if (value == MAXBITS)
		return TRUE;

	BITS = value;
	MAX_VALUE = (1 << BITS) - 1;
	MAX_CODE = MAX_VALUE - 1;

	return FALSE;
}

/***************************************************************************
** decode_string
**
** Purpose: To return the string that the code taken from the input buffer
** represents. The string is returned as a stack, i.e. the characters are
** in reverse order.
***************************************************************************/
static UINT8 *decode_string(UINT8 *buffer, UINT32 code)
{
	UINT32 i;

	for (i = 0; code > 255; ) {
		*buffer++ = append_character[code];
		code=prefix_code[code];
		if (i++ >= 4000) {
			fprintf (stderr, "lzw: error in code expansion.\n");
			abort ();
		}
	}
	*buffer = code;

	return buffer;
}

/***************************************************************************
** input_code
**
** Purpose: To return the next code from the input buffer.
***************************************************************************/
static UINT32 input_code (UINT8 **input)
{
	UINT32 r;

	while (input_bit_count <= 24) {
		input_bit_buffer |= (UINT32) *(*input)++ << input_bit_count;
		input_bit_count += 8;
	}
	r = (input_bit_buffer & 0x7FFF) % (1 << BITS);
	input_bit_buffer >>= BITS;
	input_bit_count -= BITS;

	return r;
}

/***************************************************************************
** expand
**
** Purpose: To uncompress the data contained in the input buffer and store
** the result in the output buffer. The fileLength parameter says how
** many bytes to uncompress. The compression itself is a form of LZW that
** adjusts the number of bits that it represents its codes in as it fills
** up the available codes. Two codes have special meaning:
**
**  code 256 = start over
**  code 257 = end of data
***************************************************************************/
void LZW_expand(UINT8 *in, UINT8 *out, SINT32 len)
{
	SINT32 c, lzwnext, lzwnew, lzwold;
	UINT8 *s, *end;

	initLZW();

	setBITS(START_BITS);		/* Starts at 9-bits */
	lzwnext = 257;			/* Next available code to define */

	end = (unsigned char *)((long)out + (long)len);

	lzwold = input_code(&in);	/* Read in the first code */
	c = lzwold;
	lzwnew = input_code(&in);

	while ((out < end) && (lzwnew != 0x101)) {
		if (lzwnew == 0x100) {
			/* Code to "start over" */
			lzwnext = 258;
			setBITS(START_BITS);
			lzwold = input_code(&in);
			c = lzwold;
			*out++ = (char)c;
			lzwnew = input_code(&in);
		} else {
			if (lzwnew >= lzwnext) {
				/* Handles special LZW scenario */
				*decode_stack = c;
				s = decode_string(decode_stack+1, lzwold);
			}
			else
				s = decode_string(decode_stack, lzwnew);

			/* Reverse order of decoded string and
			 * store in out buffer
			 */
			c = *s;
			while (s >= decode_stack)
				*out++ = *s--;

			if (lzwnext > MAX_CODE)
				setBITS(BITS + 1);

			prefix_code[lzwnext] = lzwold;
			append_character[lzwnext] = c;
			lzwnext++;
			lzwold = lzwnew;

			lzwnew = input_code(&in);
		}
	}
	closeLZW ();
}

