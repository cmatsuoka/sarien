
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "agi2pdb.h"

#define VERSION "0.0.1"

int test_v3_game(char *dirfile, char *gpath);
int test_v2_game(char *gpath);
void copy_file(FILE *ifp, FILE *ofp);
void do_pdbheader(void);
int convert_v2(char *outfile, char *gpath);
int convert_v3(char *outfile, char *dirfile, char *gpath);

struct PalmOSAGI palmagi;
struct pdb_recheader rh;
struct pdb_pdbheader PDBHeader;
struct sarien_options opt;

int main(int argc, char *argv[])
{
	printf("agi2pdb v" VERSION " :: Converting Sierra AGI files to PalmOS Databases\n"
		"(C) Copyright 2001 Stuart George\n"
		"\n"
		"This program is free software; you can redistribute it and/or modify it\n"
		"under the terms of the GNU General Public License, version 2 or later,\n"
		"as published by the the Free Software Foundation.\n");

	if(argc<3)
	{
		printf("syntax;\n"
			"For AGI v2 games;\n"
			"  agi2pdb game.pdb {directory}\n"
			"For AGI v3 games;\n"
			"  agi2pdb game.pdb DIRFILE {directory}\n"
			"\n"
			"eg:\n"
			"agi2pdb KingsQuestII.pdb /games/kq2\n"
			"agi2pdb KingsQuestIII.pdb\n"
			"agi2pdb GoldRush.pdb GRDIR /games/gr\n"
			"agi2pdb BlackCauldron.pdb BCDIR /games/bc/2.10\n");
	}
	else
	{
		strcpy(exec_name, argv[0]);

    	if(test_v3_game(argv[2], argv[3])==err_OK)
    	{
    		convert_v3(argv[1], argv[2], argv[3]);
    	}
    	else
    	{
    		if(test_v2_game(argv[2])==err_OK)
    		{
    			convert_v2(argv[1], argv[2]);
    		}
    	}
	}

	return 0;
}

int test_v3_game(char *dirfile, char *gpath)
{
	FILE *fp;
	int rc=err_Unk;
	char *p;

	if(gpath!=NULL)
		strcpy(dir, gpath);
	else
		strcpy(dir, "./");

	fixpath(NO_GAMEDIR, dirfile);
	if((fp=fopen(path, "rb"))!=NULL)
	{
		fclose(fp);

		/* valid dir file */

		p=strchr(dirfile, 0x0);
		p--;

		if(p>dirfile && tolower(*p--)=='r')
		{
		    if(p>dirfile && tolower(*p--)=='i')
		    {
		        if(p>dirfile && tolower(*p--)=='d')
		        {
		        	rc=err_OK;
		        }
		    }
		}
	}

	return rc;
}

int test_v2_game(char *gpath)
{
	char *fnames[]={"logdir", "picdir"};
	int i;
	int rc=err_OK;
	FILE *fp;

	if(gpath!=NULL)
		strcpy(dir, gpath);
	else
	    strcpy(dir, "./");

	for(i=0; rc==err_OK && i<2; i++)
	{
		fixpath(NO_GAMEDIR, fnames[i]);
		if((fp=fopen(path, "rb"))!=NULL)
			fclose(fp);
		else
			rc=err_Unk;
	}

	return rc;
}

void copy_file(FILE *ifp, FILE *ofp)
{
	char *buff;
	int len;

	buff=(char*)malloc(8192);

	len=1;

	do
	{
		len=fread(buff, 1, 8192, ifp);
		if(len>0)
			fwrite(buff, 1, len, ofp);
	}while(len>0);

	free(buff);
}

void do_pdbheader(void)
{
	memset(&PDBHeader, 0x0, sizeof(struct pdb_pdbheader));

	/* db name in palm */
	strncpy(PDBHeader.name, game_name, 32);

	/* can install over existing pdb */
	write_word((UINT8*)&PDBHeader.fileAttributes, 0x10);
	write_word((UINT8*)&PDBHeader.version, 0x0001);
	write_dword((UINT8*)&PDBHeader.creationDate, 1);
	write_dword((UINT8*)&PDBHeader.modificationDate, 1);

	/* AGI v4 PalmOS DB File */
	PDBHeader.databaseType[0]='a';
	PDBHeader.databaseType[1]='g';
	PDBHeader.databaseType[2]='i';
	PDBHeader.databaseType[3]='4';

	/* Our registered Palm Creator ID */
	PDBHeader.creatorID[0]='F';
	PDBHeader.creatorID[1]='A';
	PDBHeader.creatorID[2]='G';
	PDBHeader.creatorID[3]='I';
}

int convert_v2(char *outfile, char *gpath)
{
	FILE *ofp;
	FILE *fp;
	char	buff[256];
	int		len;
	int		i;
	int		total_len;

	printf("\nConverting a v2 game file to PalmOSDB v4\n");
	ofp=fopen("$$temp$$.tmp", "wb+");

	memset(&palmagi, 0x0, sizeof(struct	PalmOSAGI));

	/* does version + crc */
	v2id_game();
	strncpy(palmagi.name, game_name, 32);

	if(opt.agds)
		palmagi.options|=OPT_AGDS;
	if(opt.amiga)
		palmagi.options|=OPT_AMIGA;

	memset(buff, 0x0, 256);

	/* 256 byte header */
	fwrite(buff, 1, 256, ofp);

	fixpath(NO_GAMEDIR,	"logdir");
	if((fp=fopen(path, "rb"))!=NULL)
	{
		printf("Inserting %s\n", path);
		palmagi.v4.v2.logdir[0]=ftell(ofp);
		copy_file(fp, ofp);
		palmagi.v4.v2.logdir[1]=ftell(fp);
		fclose(fp);
	}

	fixpath(NO_GAMEDIR,	"picdir");
	if((fp=fopen(path, "rb"))!=NULL)
	{
		printf("Inserting %s\n", path);
		palmagi.v4.v2.picdir[0]=ftell(ofp);
		copy_file(fp, ofp);
		palmagi.v4.v2.picdir[1]=ftell(fp);
		fclose(fp);
	}

	fixpath(NO_GAMEDIR,	"viewdir");
	if((fp=fopen(path, "rb"))!=NULL)
	{
		printf("Inserting %s\n", path);
		palmagi.v4.v2.viewdir[0]=ftell(ofp);
		copy_file(fp, ofp);
		palmagi.v4.v2.viewdir[1]=ftell(fp);
		fclose(fp);
	}

	fixpath(NO_GAMEDIR,	"snddir");
	if((fp=fopen(path, "rb"))!=NULL)
	{
		printf("Inserting %s\n", path);
		palmagi.v4.v2.snddir[0]=ftell(ofp);
		copy_file(fp, ofp);
		palmagi.v4.v2.snddir[1]=ftell(fp);
		fclose(fp);
	}

	fixpath(NO_GAMEDIR,	"object");
	if((fp=fopen(path, "rb"))!=NULL)
	{
		printf("Inserting %s\n", path);
		palmagi.v4.v2.object[0]=ftell(ofp);
		copy_file(fp, ofp);
		palmagi.v4.v2.object[1]=ftell(fp);
		fclose(fp);
	}

	fixpath(NO_GAMEDIR,	"words.tok");
	if((fp=fopen(path, "rb"))!=NULL)
	{
		printf("Inserting %s\n", path);
		palmagi.v4.v2.words[0]=ftell(ofp);
		copy_file(fp, ofp);
		palmagi.v4.v2.words[1]=ftell(fp);
		fclose(fp);
	}

	for(i=0; i<16; i++)
	{
		sprintf(buff, "vol.%i", i);
		fixpath(NO_GAMEDIR,	buff);
		if((fp=fopen(path, "rb"))!=NULL)
		{
			printf("Inserting %s\n", path);
			palmagi.v4.v2.vol[i*2]=ftell(ofp);
			copy_file(fp, ofp);
			palmagi.v4.v2.vol[1+(i*2)]=ftell(fp);
			fclose(fp);
		}
	}

	fseek(ofp, 0x0, SEEK_END);
	total_len=ftell(ofp);

	/* write good header */
	fseek(ofp, 0x0, SEEK_SET);
	fwrite(&palmagi, 1, sizeof(struct PalmOSAGI), ofp);

	/* now build the real palm PDB file */
	fp=fopen(outfile, "wb+");
	if(fp!=NULL)
	{
		/* fill in Palm PDB Header */
	 	do_pdbheader();

		/* skip pdbheader size */
		fseek(fp, sizeof(struct pdb_pdbheader), SEEK_SET);

		total_len=total_len + 4095 & 0xfffff000;
		write_word((UINT8*)&PDBHeader.numberOfRecords, total_len/4096);

		/* create record headers */
		for(i=0; i<total_len/4096; i++)
		{
			memset(&rh, 0x0, sizeof(struct pdb_recheader));
			write_dword((UINT8*)&rh.recordDataOffset, (sizeof(struct pdb_pdbheader)) + (total_len/4096 * sizeof(struct pdb_recheader)) + i*4096);
			rh.recordAttributes=0x60;
			fwrite(&rh, 1, sizeof(struct pdb_recheader), fp);
		}

		/* copy created pdb */
		fseek(ofp, 0x0, SEEK_SET);
		copy_file(ofp, fp);

		/* write good PDB Header */
		fseek(fp, 0x0, SEEK_SET);
		fwrite(&PDBHeader, 1, sizeof(struct pdb_pdbheader), fp);

		fclose(fp);
	}

	fclose(ofp);

	remove("$$temp$$.tmp");

	return 0;
}

int convert_v3(char *outfile, char *dirfile, char *gpath)
{
	FILE *ofp;
	FILE *fp;
	char	buff[256];
	int		len;
	int		i;
	int		total_len;

	printf("\nConverting a v3 game file to PalmOSDB v4\n");
	ofp=fopen("$$temp$$.tmp", "wb+");

	memset(&palmagi, 0x0, sizeof(struct	PalmOSAGI));

	strncpy(name, dirfile, strlen(dirfile)-3);

	/* does version + crc */
	v3id_game();
	strncpy(palmagi.name, game_name, 32);

	if(opt.agds)
		palmagi.options|=OPT_AGDS;
	if(opt.amiga)
		palmagi.options|=OPT_AMIGA;

	memset(buff, 0x0, 256);

	/* 256 byte header */
	fwrite(buff, 1, 256, ofp);

	fixpath(NO_GAMEDIR,	dirfile);
	if((fp=fopen(path, "rb"))!=NULL)
	{
		printf("Inserting %s\n", path);
		palmagi.v4.v3.dir[0]=ftell(ofp);
		copy_file(fp, ofp);
		palmagi.v4.v3.dir[1]=ftell(fp);
		fclose(fp);
	}

	fixpath(NO_GAMEDIR, "object");
	if((fp=fopen(path, "rb"))!=NULL)
	{
		printf("Inserting %s\n", path);
		palmagi.v4.v3.object[0]=ftell(ofp);
		copy_file(fp, ofp);
		palmagi.v4.v3.object[1]=ftell(fp);
		fclose(fp);
	}

	fixpath(NO_GAMEDIR, "words.tok");
	if((fp=fopen(path, "rb"))!=NULL)
	{
		printf("Inserting %s\n", path);
		palmagi.v4.v3.words[0]=ftell(ofp);
		copy_file(fp, ofp);
		palmagi.v4.v3.words[1]=ftell(fp);
		fclose(fp);
	}

	for(i=0; i<16; i++)
	{
		sprintf(buff, "vol.%i", i);
		fixpath(GAMEDIR, buff);
		if((fp=fopen(path, "rb"))!=NULL)
		{
			printf("Inserting %s\n", path);
			palmagi.v4.v3.vol[i*2]=ftell(ofp);
			copy_file(fp, ofp);
			palmagi.v4.v3.vol[1+(i*2)]=ftell(fp);
			fclose(fp);
		}
	}

	fseek(ofp, 0x0, SEEK_END);
	total_len=ftell(ofp);

	/* write good header */
	fseek(ofp, 0x0, SEEK_SET);
	fwrite(&palmagi, 1, sizeof(struct PalmOSAGI), ofp);

	/* now build the real palm PDB file */
	fp=fopen(outfile, "wb+");
	if(fp!=NULL)
	{
		/* fill in Palm PDB Header */
	 	do_pdbheader();

		/* skip pdbheader size */
		fseek(fp, sizeof(struct pdb_pdbheader), SEEK_SET);

		total_len=total_len + 4095 & 0xfffff000;
		write_word((UINT8*)&PDBHeader.numberOfRecords, total_len/4096);

		/* create record headers */
		for(i=0; i<total_len/4096; i++)
		{
			memset(&rh, 0x0, sizeof(struct pdb_recheader));
			write_dword((UINT8*)&rh.recordDataOffset, (sizeof(struct pdb_pdbheader)) + (total_len/4096 * sizeof(struct pdb_recheader)) + i*4096);
			rh.recordAttributes=0x60;
			fwrite(&rh, 1, sizeof(struct pdb_recheader), fp);
		}

		/* copy created pdb */
		fseek(ofp, 0x0, SEEK_SET);
		copy_file(ofp, fp);

		/* write good PDB Header */
		fseek(fp, 0x0, SEEK_SET);
		fwrite(&PDBHeader, 1, sizeof(struct pdb_pdbheader), fp);

		fclose(fp);
	}

	fclose(ofp);

	remove("$$temp$$.tmp");

	return 0;

}
