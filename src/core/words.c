/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

/*
 * New find_word algorithm by Thomas Akesson <tapilot@home.se>
 */

#include <stdio.h>
#include <string.h>
#include "sarien.h"
#include "agi.h"
#include "keyboard.h"		/* for clean_input() */

UINT8 *words;			/* words in the game */


char *strndup (char* src, int n) {

	char *tmp = strncpy (malloc(n+1), src, n);
	tmp[n] = 0;
	return tmp;
}

int load_words (char *fname)
{
#ifndef PALMOS
	FILE *fp = NULL;
	UINT32 flen;
	UINT8 *mem = NULL;
	char *path = NULL;

	words = NULL;

	path = fixpath (NO_GAMEDIR, fname);
	report ("Loading dictionary: %s\n", path);

	if ((fp = fopen(path, "rb")) == NULL)
		return err_BadFileOpen;

	fseek (fp, 0, SEEK_END);
	flen = ftell (fp);
	fseek (fp, 0, SEEK_SET);

	if ((mem = (UINT8*)calloc(1, flen+32)) == NULL) {
		fclose (fp);
		return err_NotEnoughMemory;
	}

	fread(mem, 1, flen, fp);
	fclose (fp);

	words = mem;

	return err_OK;
#endif
}


void unload_words ()
{
	if (words != NULL) {
		free (words);
		words = NULL;
	}
}


/**
 *
 * Uses an algorithm hopefully like the one Sierra used.
 * Returns the ID of the word and the length in flen. Returns -1 if not found.
 *
 * Thomas Åkesson, November 2001
 */
int find_word (char *word, int* flen)
{
	#define chra 0x61
	#define chrz 0x7A

	int fchr, matchedchars = 0, id = -1;
	int len = strlen(word);
	UINT8* p = words;
	*flen = 0;

	if (word[0] >= chra && word[0] <= chrz)
		fchr = word[0] - chra;
	else
		return -1;

	/* Get the offset to the first word beginning with the right character. */
	p += hilo_getword (p + 2*fchr);
	while (p[0] >= matchedchars) {
		if (p[0] == matchedchars){
			p++;
			/* Loop through all matching characters */
			while ((p[0] ^ word[matchedchars]) == 0x7F && matchedchars < len) {
				matchedchars++;
				p++;
			}
			/* Check if this is the last character of the word and if it matches */
			if ((p[0] ^ word[matchedchars]) == 0xFF && matchedchars < len) {
				matchedchars++;
				if (word[matchedchars] == 0 || word[matchedchars] == 0x20) {
					id = hilo_getword (p+1);
					*flen = matchedchars;
				}
			}
		}
		/* Step to the next word */
		while (p[0] < 0x80)
				p++;
		p += 3;
	}
	return id;
}


void dictionary_words (char *msg)
{
	char *p	= NULL;
	char *q = NULL;
	int wid, wlen;

	_D ("msg = \"%s\"", msg);

	clean_input ();

	for (p = msg; p && *p && getvar(V_word_not_found)==0; ) {
		if (*p == 0x20)
			p++;

		if(*p == 0)
			break;

 		wid = find_word(p, &wlen);
 		_D ("find_word(p) == %d", wid);

 		switch (wid) {
 			case -1:
 				_D (_D_WARN "unknown word");
 				game.ego_words[game.num_ego_words].word = strdup(p);
 				q = game.ego_words[game.num_ego_words].word;
 				game.ego_words[game.num_ego_words].id = 19999;
 				setvar(V_word_not_found, 1 + game.num_ego_words);
 				game.num_ego_words++;
 				p += strlen (p);
 				break;
 			case 0:
 				/* ignore this word */
 				_D (_D_WARN "ignore word");
 				p += wlen;
 				q = NULL;
 				break;
 			default:
 				/* an OK word */
 				/* _D (_D_WARN "ok word (%d)", wc1); */
 				game.ego_words[game.num_ego_words].id = wid;
 				game.ego_words[game.num_ego_words].word = strndup(p, wlen);
 				game.num_ego_words++;
 				p += wlen;
 				break;
 			}

 		if (p != NULL && *p) {
 			*p=0;
 			p++;
 		}

 		if (q != NULL) {
			for (; (*q!=0 && *q!=0x20); q++);
			if (*q) {
				*q=0;
 				q++;
 			}
 		}
	}

	_D (_D_WARN "num_ego_words = %d", game.num_ego_words);
	if (game.num_ego_words > 0) {
		setflag (F_entered_cli, TRUE);
		setflag (F_said_accepted_input, FALSE);
	}
}

#ifdef OPT_LIST_DICT
int show_words ()
{
	/* This does not work anymore since the word list is not expanded as before.
	   A similar function can easily be written based on the old decode_words(...).
	unsigned int i, uid, sid;
	int lid;

	uid = sid = 0;
	lid = 0xffff;

	printf("  ID   Word\n");
	for (i = 0; i < num_words; i++) {
		if (lid == words[i].id) {
			sid++;
		} else {
			lid = words[i].id;
			uid++;
		}
		printf ("%4i - %s\n", words[i].id, words[i].word);
	}

	printf ("\n%i words in dictionary\n", num_words);
	printf ("%5i unique words\n", uid);
	printf ("%5i synonyms\n", sid);
	printf (
"\nThis is dodgy, only synonyms are id'd if they follow each other\n"
"e.g. in Space Quest I, knife, army knife, xenon army knife are all synonyms\n"
"but are not treated as such as they do not alphabetically follow each other.\n");
*/
	return err_OK;
}
#endif
