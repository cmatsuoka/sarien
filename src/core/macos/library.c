/* $Id$ */

#include <Memory.h>
#include <string.h>

char *strdup (char *s)
{
	char *r = malloc (strlen (s) + 1);
	strcpy (r, s);
	return r;
}


void *my_malloc (int n)
{
	int i = MaxBlock ();
	printf ("Malloc = %d, MaxBlock = %d\n", n, i);
	if (n >= i)
		exit (-1);
	return (void *)NewPtr(n);
}


void *my_calloc (int n, int s)
{
	int i = MaxBlock ();
	printf ("Calloc = %d*%d, MaxBlock = %d\n", n, s, i);
	if (n * s >= i)
		exit (-1);
	return (void *)NewPtrClear(n * s);
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


