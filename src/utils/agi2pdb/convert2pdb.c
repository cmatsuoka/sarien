#include <stdio.h>

#include "agi2pdb.h"

extern struct agi_object *objects;
extern UINT8 *words;			/* words in the game */
extern UINT32 words_flen;		/* length of word memory */

int convert2pdb (struct agi_loader *loader, char *pdbname) {

	struct agi_v4_header header;
	PalmDBRef db;
	PalmDBRecord rec;
	UINT16 diridx, idx = 0;
	UINT32 dirsize;
	Err err = 0;
	UINT16 *dirbuf = NULL;
	struct agi_dir *agidirs[] = {game.dir_logic, game.dir_pic, game.dir_view, game.dir_sound};
	char *agiRscTypes[] = {"LOGIC" ,"PICTURE", "VIEW", "SOUND"};
	int numRsc[4];
	int i;


	/* Create database */
	printf("Creating database...\n");
	db = CreatePalmDB (pdbname, MAKE_TYPE('a','g','i','4'), &err);
	if (db == NULL)
		DieWithError("Error creating database.", err);

	/* Fill information into header */
	write_word((UINT8*)&header.format, 0);
	write_word((UINT8*)&header.revision, 0);
	/* Oops, it turns out that getting the game id wasn't easy.
	   Have to start the logic... Is there a better way? */
	memset(&header.id, 0x0, 8);
	write_word((UINT8*)&header.version, game.ver);
	write_word((UINT8*)&header.emulation, loader->int_version);
	write_dword((UINT8*)&header.options, 0); /* The agi_v4 format seems to be independent
	                                    of the AGDS and AMIGA options. */
	header.sndInfo.sndType = sndTypeAGI;
	header.sndInfo.sndChannels = 4;

	/* Write header record */
	printf("Writing header record...\n");
	err = AddPalmDBRecord (db, &idx, (UINT8*)&header, sizeof(struct agi_v4_header));
	if (err)
		DieWithError("Error writing header record.", err);

	/* Write directory records */
	if ((dirbuf = calloc(2, MAX_DIRS)) == NULL)
		DieWithError("Error allocating buffer.", 0);

	diridx = 7;
	for (i=0; i<4; i++) {
		int previdx = diridx;
		dirsize = convertDir(agidirs[i], dirbuf, &diridx);
		printf("dirsize: %d diridx: %d\n", dirsize, diridx);
		numRsc[i] = diridx - previdx;
		idx = i + 1;
		err = AddPalmDBRecord (db, &idx, (UINT8*)dirbuf, 2*dirsize);
		if (err)
			DieWithError("Error writing directory record.", err);
		printf("Written %s directory for %d resources.\n", agiRscTypes[i], numRsc[i]);
	}

	free(dirbuf);

	/* Write objects record */
	idx = 5;
	rec = CreatePalmDBRecord (db, &idx, 1024);
	if (rec == NULL)
		DieWithError("Error creting objects record.", 0);
	err = convertObjects(rec, game.num_objects);
	if (err)
		DieWithError("Error writing objects record.", err);
	err = FinalizePalmDBRecord (rec) ;
	if (err)
		DieWithError("Error finalizing objects record.", err);

	/* Write words record */
	idx = 6;
	err = AddPalmDBRecord (db, &idx, words, words_flen);
	if (err)
		DieWithError("Error writing word list record.", err);


	/* Write resources */

	/* Not yet completed... */



	/* Closing database */
	printf("Closing database.\n");
	err = ClosePalmDB (db);
	if (err)
		DieWithError("Error closing database.", err);


	return 0;
}

int convertDir(struct agi_dir *agid, UINT16 dir[], UINT16 *index) {

	int i, maxID = 0;

	for (i = 0; i<MAX_DIRS; i++) {
		/* printf("%x " , agid[i].offset); */
		if (agid[i].offset != _EMPTY) {
			write_word((UINT8*)&dir[i], (*index)++);
			maxID = i;
		} else {
			write_word((UINT8*)&dir[i], 0xffff);
		}
	}
	return maxID+1;

}


Err convertObjects(PalmDBRecord rec, UINT16 num_objects) {

	Err err = 0;
	UINT8 buf[num_objects+2];
	UINT32 offset = 0;
	char *objstr;
	int i;

	/* Create list of object locations */
	write_word(&buf[0], num_objects);
	for (i = 0; i<num_objects; i++) {
		buf[i+2] = object_get_location(i);
	}
	err = WritePalmDBRecord (rec, &offset, (UINT8*)buf, num_objects+2);
	if (err)
		return err;

	/* Create block of packed object strings */
	for (i = 0; i<num_objects; i++) {
		objstr = object_name(i);
		err = WritePalmDBRecord (rec, &offset, (UINT8*)objstr, strlen(objstr)+1);
		if (err)
			return err;
	}
	return err;
}