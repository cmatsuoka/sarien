/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "sarien.h"
#include "agi.h"
#include "console.h"

static struct agi_word *words;			/* words in the game */

static int num_words, num_syns;


int load_words (char *fname)
{
	int sc, wc, woff, wid;
	FILE *fp;
	UINT32 flen;
	UINT8 *mem, x[128], c;
	char *path;

	num_words = 0;
	num_syns = 0;
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

	/* scan for first entry with words */
	for (wc = woff = 0; woff == 0 && woff < flen; wc += 2)
		woff = hilo_getword (mem + wc);

#ifdef AGDS_SUPPORT
	/* AGDS kludge for bad word file :( */
	if (woff > flen)
		return err_OK;
#endif

	/* count all the words in the list */
	for (sc = 0, wc = 0; woff < flen; ) {
		c = hilo_getbyte (mem + woff);
		woff++;

		if (c > 0x80) {
			wc++;
			wid = hilo_getword (mem+woff);
			woff += 2;
			if (wid > sc && wid != 9999)
				sc = wid;
		}
	}
	num_words = wc;
	num_syns = sc;

	/* scan for first words entry */
	for(wc = woff = 0; woff == 0; wc += 2)
		woff = hilo_getword(mem+wc);

	/* alloc word memory */

	if ((words = calloc (num_words, sizeof(struct agi_word))) == NULL) {
		free(mem);
		return err_NotEnoughMemory;
	}

	/* build word list */
	for (wc = 0; wc < num_words; wc++) {
		c = hilo_getbyte (mem + woff); woff++;
		wid = c;
		while (c < 0x80) {
			c = hilo_getbyte (mem + woff); woff++;
			x[wid] = ((c ^ 0x7F) & 0x7F); wid++;
		}
		x[wid] = 0;

		(words + wc)->id = hilo_getword (mem + woff); woff+=2;
		(words + wc)->word = strdup (x);
	}

	return err_OK;
}


void unload_words ()
{
	int i;

	if (words != NULL) {
		for (i = 0; i < num_words; i++)
			free (words[i].word);
		free (words);
	}
}


/*
 * Scan dictionary for word, returning its ID.
 * Uses a fast "Divide and Conquer" routine to get the word your looking for.
 */
int find_word (char *word)
{
	int i, offs = 0, id = 0, val, lid = 0, llen = 0;

	for (; offs < num_words && words[offs].word[0] != word[0]; offs++);
	for (; offs < num_words && words[offs].word[0] == word[0]; offs++) {
		i = strlen ((char*)words[offs].word);
		val = 1;

		if (strlen((char*)word) >= i) {
			val = strncmp ((char*)words[offs].word,
				(char*)word, i);
		}

		if (val == 0 && (word[i] == 0 || word[i] == 0x20)) {
			id = strlen((char*)words[offs].word);
			if(id > llen) {
				llen=id;
				lid=offs;
			}
		}
	}

	return llen ? lid : -1;
}


void dictionary_words (char *msg)
{
	static UINT8 bad_word[256];	/* FIXME: dynamic allocation? */
	char *p, *q = NULL;
	int wc1;

	_D ("(\"%s\")", msg);

	/* FR
	 * This shouldn't be here!
	 *
	 *	memmove((char*)x, (char*)msg, 127);
	 *
	 * for (p=(UINT8*)x; *msg!=0; msg++) {
	 *		if(*msg==0x20 || isalnum(*msg)) {
	 *			if(*msg!=0x20 || last!=0x20) {
	 *				*p=tolower(*msg);
	 *				last=*p;
	 *				p++;
	 *			}
	 *		}
	 * }
	 */
	clean_input();

	_D (_D_WARN "p = msg = \"%s\"", msg);

	for (p = msg; p && *p && getvar(V_word_not_found)==0; ) {
		if (*p == 0x20)
			p++;

		if(*p == 0)
			break;

 		wc1 = find_word(p);
 		_D ("find_word(p) == %d", wc1);

 		if (wc1 != -1) {
 			switch (words[wc1].id) {
 			case -1:
 				_D (_D_WARN "bad word");
 				game.ego_words[game.num_ego_words].word = strdup(p);
 				q = game.ego_words[game.num_ego_words].word;
 				game.ego_words[game.num_ego_words].id = 19999;
 				setvar(V_word_not_found, 1 + game.num_ego_words);
 				game.num_ego_words++;
 				p += strlen (words[wc1].word);
 				break;
 			case 0:
 				/* ignore this word */
 				_D (_D_WARN "ignore word");
 				p += strlen((char*)words[wc1].word);
 				q = NULL;
 				break;
 			default:
 				/* an OK word */
 				_D (_D_WARN "ok word (%d)", wc1);
 				game.ego_words[game.num_ego_words].id = words[wc1].id;
 				game.ego_words[game.num_ego_words].word = words[wc1].word;
 				game.num_ego_words++;
 				p += strlen((char*)words[wc1].word);
 				break;
 			}
 		} else {
 			/* unknown word */
 			_D (_D_WARN "unknown word");
			strcpy ((char*)bad_word, (char*)p);
 			game.ego_words[game.num_ego_words].word = bad_word;
			q = game.ego_words[game.num_ego_words].word;
 			game.ego_words[game.num_ego_words].id = 19999;
 			setvar (V_word_not_found, 1 + game.num_ego_words);
 			game.num_ego_words++;
 			p=(UINT8*)strchr((char*)p, 0x20);
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

	if (game.num_ego_words > 0) {
		setflag (F_entered_cli, TRUE);
		setflag (F_said_accepted_input, FALSE);
	}
}


int show_words ()
{
	int i, uid, sid, lid;

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

	return err_OK;
}

