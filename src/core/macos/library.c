/* $Id$ */

#include <Memory.h>
#include <string.h>
#include <stdio.h>

char *strdup (char *s)
{
	char *r = malloc (strlen (s) + 1);
	strcpy (r, s);
	return r;
}


void *my_malloc (int n)
{
	int i = MaxBlock ();
	void *x;

	if (n >= i) {
		printf ("Error: malloc = %d, maxblock = %d\n", n, i);
		exit (-1);
	}

	if ((x = (void *)NewPtr(n)) == NULL) {
		printf ("Error: %d\n", MemError());
		exit (-1);
	}

	return x;
}


void *my_calloc (int n, int s)
{
	int i = MaxBlock ();
	void *x;

	if (n * s >= i) {
		printf ("Error: calloc = %d,%d, maxblock = %d\n", n, s, i);
		exit (-1);
	}

	if ((x = (void *)NewPtrClear(n * s)) == NULL) {
		printf ("Error: %d\n", MemError());
		exit (-1);
	}
	
	return x;
}


void *my_realloc (void *x, int n)
{
	SetPtrSize ((Ptr)x, n);
	return x;
}


void my_free (void *x)
{
	DisposePtr ((Ptr)x);
}


