
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include "agi2pdb.h"

char 	path[MAX_PATH+1];
char	dir[MAX_PATH+1];
char	name[16];
char	game_name[64];
char	exec_name[MAX_PATH+1];

char	*out_file;

char *fixpath (int flag, char *fname)
{
	char *p;

   	strcpy (path, dir);

	if (*path && (path[strlen(path)-1]!='\\' && path[strlen(path)-1] != '/'))
	{
		if(path[strlen(path)-1]==':')
			strcat(path, "./");
		else
			strcat(path, "/");
	}

	if (flag==1)
		strcat (path, name);

	strcat (path, fname);

	p = path;

	while(*p) {
		if (*p=='\\')
		    *p='/';
		p++;
	}

	return path;
}

void write_word(UINT8 *mem, UINT16 w)
{
	mem[1]=(w&0xFF);
	mem[0]=(w>>8)&0xFF;
}

void write_dword(UINT8 *mem, UINT32 w)
{
	mem[3]=(w&0xFF);
	mem[2]=(w>>8)&0xFF;
	mem[1]=(w>>16)&0xFF;
	mem[0]=(w>>24)&0xFF;
}
