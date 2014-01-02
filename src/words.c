/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2002 Stuart George and Claudio Matsuoka
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

static UINT8 *words;		/* words in the game */
static UINT32 words_flen;	/* length of word memory */


/*
 * Local implementation to avoid problems with strndup() used by
 * gcc 3.2 Cygwin (see #635984)
 */
static char *my_strndup (char* src, int n)
{
	char *tmp = strncpy (malloc(n + 1), src, n);
	tmp[n] = 0;
	return tmp;
}


int load_words (char *fname)
{
	FILE *fp = NULL;
	UINT32 flen;
	UINT8 *mem = NULL;
	char *path = NULL;

	words = NULL;

	path = fixpath (NO_GAMEDIR, fname);

	if ((fp = fopen(path, "rb")) == NULL) {
		report ("Warning: can't open %s\n", path);
		return err_OK /*err_BadFileOpen*/;
	}
	report ("Loading dictionary: %s\n", path);

	fseek (fp, 0, SEEK_END);
	flen = ftell (fp);
	words_flen = flen;
	fseek (fp, 0, SEEK_SET);

	if ((mem = (UINT8*)calloc(1, flen + 32)) == NULL) {
		fclose (fp);
		return err_NotEnoughMemory;
	}

	fread (mem, 1, flen, fp);
	fclose (fp);

	words = mem;

	return err_OK;
}


void unload_words ()
{
	if (words != NULL) {
		free (words);
		words = NULL;
	}
}


/**
 * Find a word in the dictionary
 * Uses an algorithm hopefully like the one Sierra used. Returns the ID
 * of the word and the length in flen. Returns -1 if not found.
 *
 * Thomas Åkesson, November 2001
 */
int find_word (char *word, int *flen)
{
	int mchr = 0;	/* matched chars */
	int len, fchr, id = -1;
	UINT8* p = words;
	UINT8* q = words + words_flen;
	*flen = 0;

	_D ("%s", word);
	if (word[0] >= 'a' && word[0] <= 'z')
		fchr = word[0] - 'a';
	else
		return -1;

	len = strlen (word);

	/* Get the offset to the first word beginning with the
	 * right character
	 */
	p += hilo_getword (p + 2 * fchr);

	while (p[0] >= mchr) {
		if (p[0] == mchr) {
			p++;
			/* Loop through all matching characters */
			while ((p[0] ^ word[mchr]) == 0x7F && mchr < len) {
				mchr++;
				p++;
			}
			/* Check if this is the last character of the word
			 * and if it matches
			 */
			if ((p[0] ^ word[mchr]) == 0xFF && mchr < len) {
				mchr++;
				if (word[mchr] == 0 || word[mchr] == 0x20) {
					id = hilo_getword (p+1);
					*flen = mchr;
				}
			}
		}
	 	if (p >= q)
			return -1;	

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

	for (p = msg; p && *p && getvar (V_word_not_found) == 0; ) {
		if (*p == 0x20)
			p++;

		if (*p == 0)
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
 			game.ego_words[game.num_ego_words].word = my_strndup(p, wlen);
 			game.num_ego_words++;
 			p += wlen;
 			break;
 		}

 		if (p != NULL && *p) {
			_D ("p = %s", p);
 			*p = 0;
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
/*
	decode *words into words!
*/

	int	sc, wc, woff, wid;
	unsigned char x[128];
	unsigned char c;
	int	num_words;

	num_words=0;

	/* scan for first entry with words */
	for (wc = woff = 0; woff == 0 && woff < words_flen; wc += 2)
		woff = hilo_getword(words+wc);

	/* AGDS cludge for bad word file :( */
	if(woff > words_flen)
		return err_OK;

	/* count all the words in the list */
	for(sc=0, wc=0; woff<words_flen; ) {
		c = hilo_getbyte (words+woff);
		woff++;

		if(c > 0x80) {
			wc++;
			wid = hilo_getword (words+woff);
			woff += 2;
			if(wid > sc && wid != 9999)
				sc = wid;
		}
	}
	num_words = wc;

	/* scan for frist words entry */
	for(wc=0, woff=0; woff==0; wc+=2)
		woff=hilo_getword(words+wc);

	/* build word list */
	for(wc=0; wc<num_words; wc++)
	{
		c=hilo_getbyte(words+woff); woff++;
		wid=c;
		while(c<0x80)
		{
			c=hilo_getbyte(words+woff); woff++;
			x[wid]=((c^0x7F)&0x7F); wid++;
		}
		x[wid]=0x0;

		printf("id=%-4i %s\n", hilo_getword(words+woff), x);
		woff+=2;
	}

	return err_OK;
}
#endif
