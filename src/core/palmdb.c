/*****************************************************************************
 *
 * Written by Thomas Åkesson <tapilot@home.se> in March 2002
 * Copyright (C) 2002 Thomas Åkesson
 * Portions Copyright (C) 2001 Stuart George
 *
 ****************************************************************************/

#include <string.h>

#include "sarien.h"
#include "PalmDB.h"


#ifdef PALMOS

PalmDBRef CreatePalmDB (const char *nameP, UINT32 type, Err *err) {

 /*	We make a few assumptions in order to keep the interface clean:
 	1) cardNo is set to 0. That is normally where the main memory is located.
 	2) resDB is set to false, we are NOT creating a resource db.
 */

 	*err = DmCreateDatabase (0, nameP, sarienCreatorID, type, false);
 	if (*err == 0)
 		return OpenPalmDB(nameP, "rw");
 	else
 		return NULL;
}



PalmDBRef OpenPalmDB (const char* nameP, char* modeStr) {

	LocalID dbID;
	PalmDBRef dbP;
	UInt16 	mode;

	/* Searching only cardNo 0 at present, might add support for other
	   cards (e.g. Springboards) later. See MemNumCards */
	dbID = DmFindDatabase (0, nameP);

	if (!dbID) {
		return NULL;
	}

	if (modeStr[0] == 'w')
		mode = dmModeWrite;
	else if (modeStr[0] == 'r'){
		if (modeStr[1] == 'w')
			mode = dmModeReadWrite;
		else
			mode = dmModeReadOnly;
	}
	dbP = DmOpenDatabase (0, dbID, mode);
	return dbP;
}

Err ClosePalmDB (PalmDBRef dbP) {

	return DmCloseDatabase (dbP);
}

Err RemovePalmDB (const char* nameP) {
/* Databases are normally handled based on creatorID and type in PalmOS.
   Removing a database based on name is dangerous in an environment
   with a flat directory structure. Consequently, this function checks
   that the database has the correct creatorID before removing it.
*/

	LocalID dbID;
	UInt32 creator = 0;

	dbID = DmFindDatabase (0, nameP);

	if (!dbID)
		return dmErrCantFind;

	DmDatabaseInfo (0, dbID, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, &creator);
	if (creator == sarienCreatorID)
		return DmDeleteDatabase (0, dbID);
	else
		return dmErrDatabaseProtected;
}

Err AddPalmDBRecord (PalmDBRef dbP, UINT16 *atP, const UINT8 *srcP,	UINT32 bytes)  {

	PalmDBRecord record;
	void* recP;
	Err err;

	record = DmNewRecord (dbP, atP, bytes);
	if (record == NULL)
		return DmGetLastErr();
	void* recP = MemHandleLock (record);
	err = DmWrite (recP, *offset, srcP, bytes);
	MemHandleUnlock (record);
	*offset += bytes;
	DmReleaseRecord (dbP, *atP, true);
	return err;
}

PalmDBRecord CreatePalmDBRecord (PalmDBRef dbP, UINT16 *atP, UINT32 size) {

/* Creates a new record in the specified database.
   Initialize atP to dmMaxRecordIndex to append record to the
   end of the database.
*/
	PalmDBRecord rec;

	rec = (PalmDBRecord) malloc(sizeof(PalmDBRecStruct));
	if (rec == NULL) {
		return NULL;
	}
	rec->dbP = dbP;
	rec->recHandle = DmNewRecord (dbP, atP, size);
	if (rec->recHandle == NULL) {
		free(rec);
		return NULL;
	}
	rec->index = *atP;
	return rec;
}


Err WritePalmDBRecord (PalmDBRecord record, UINT32 *offset, const UINT8 *srcP,
						UINT32 bytes) {

	void *newRec;
	Err err;
	UINT32 newSize = *offset + bytes;

	if (newSize > MemHandleSize (record->recHandle)) {
		newRec = DmResizeRecord (record->dbP, record->index, newSize);
		if (newRec == NULL) {
			return DmGetLastErr ();
		} else {
			record->recHandle = newRec;
		}
	}
	void* recP = MemHandleLock (record);
	err = DmWrite (recP, *offset, srcP, bytes);
	MemHandleUnlock (record);
	if (err == 0) {
		*offset += bytes;
	}
	return err;
}


Err FinalizePalmDBRecord (PalmDBRecord record) {

	Err err;

	err = DmReleaseRecord (record->dbP, record->index, true);
	/* For easier debugging */
	memset(record, 0x0, sizeof(PalmDBRecStruct));
	free(record);
	return err;
}

Err RemovePalmDBRecord (PalmDBRef dbP, UINT16 index) {

	return DmRemoveRecord (DmOpenRef dbP, UInt16 index);
}

void* QueryPalmDBRecord (PalmDBRef dbP, UINT16 index) {

	MemHandle record;

	if ((record = DmQueryRecord (dbP, index)) != NULL)
		return MemHandleLock (record);
	else
		return NULL;
}

Err ReleasePalmDBRecord (void* dataP) {

	return MemPtrUnlock (dataP)
}


#else /*** All other platforms ***/

/* Placing some PalmOS related data structures here because they are completely
   irrelevant for all external code. */

struct pdb_pdbheader
{
	UINT8 name[32];		// String - This is the name of the database
				//  on the PalmPilot device.
				//  It need not match the name of the PDB file
				//  in the environment in which it is created.
	UINT16 fileAttributes;	// Attributes of the pdb file.
				//  0x0002 Read-Only
				//  0x0004 Dirty AppInfoArea
				//  0x0008 Backup this database
				//         (i.e. no conduit exists)
				//  0x0010 Okay to install newer over existing
				//         copy, if present on PalmPilot
				//  0x0020 Force the PalmPilot to reset after
				//         this database is installed
	UINT16 version;		// Defined by the application.
	UINT32 creationDate;	// Expressed as the number of seconds since
				//  January 1, 1904. The database will not
				//  install if this value is zero. (PalmOS
				//  1.0.6)
	UINT32 modificationDate;// Expressed as the number of seconds since
				//  January 1, 1904.
				//  The database will not install if this
				//  value is zero. (PalmOS 1.0.6)
	UINT32 lastBackupDate;	// Expressed as the number of seconds since
				//  January 1, 1904. This can be left at zero
				//  and the database will install.
	UINT32 modificationNumber;// Set to zero.
	UINT32 appInfoArea;	// The byte number in the PDB file (counting
				//  from zero) at which the AppInfoArea is
				//  located. This must be the first entry
				//  in the Data portion of the PDB file.
				//  If this database does not have an
				//  AppInfoArea, set this value to zero.
	UINT32 sortInfoArea;	// The byte number in the PDB file (counting
				//  from zero) at which the SortInfoArea is
				//  located. This must be placed immediately
				//  after the AppInfoArea, if one exists,
				//  within the Data portion of the PDB file.
				//  If this database does not have a
				//  SortInfoArea, set this value to zero.
				//  Do not use this. See Note C below for
				//  further details.
	UINT8 databaseType[4];	// String - Set this to the desired value.
				//  Generally it should match the Database Type
				//  used by the corresponding application.
				//  This is 4 characters long and does not have
				//  a terminating null.
	UINT8 creatorID[4];	// String - Set this to the desired value.
				//  Generally it should match the Creator ID
				//  used by the corresponding application. In
				//  all cases, you should always register your
				//  Creator ID before using it. This is 4
				//  characters long and does not have a
				//  terminating null.
	UINT32 uniqueIDSeed;	// This is used to generate the Unique ID
				//  number of subsequent records. Generally,
				//  this should be set to zero.
}
#ifdef __GNUC__
__attribute__((packed))
#endif
;

struct pdb_RecordList {
	UINT32 nextRecordListID;
	UINT16 numRecords;
	UINT16 firstEntry;
}
#ifdef __GNUC__
__attribute__((packed))
#endif
;

struct pdb_RecordEntry {
	UINT32 recordDataOffset;// The byte number in the PDB file (counting
				//  from zero) at which the record is located.
	UINT8 recordAttributes;	// The records attributes.
 				//  0x10 Secret record bit.
				//  0x20 Record in use (busy bit).
				//  0x40 Dirty record bit.
				//  0x80 Delete record on next HotSync.
				//  The least significant four bits are used
				//   to represent the category values.
	UINT8	uniqueID[3];	// Set this to zero and do not try to
				//  second-guess what PalmOS will do with
				//  this value.
}
#ifdef __GNUC__
__attribute__((packed))
#endif
;

/* Excerpt from SDK 3.5, DataMgr.h */
#define	dmHdrAttrBackup				0x0008	//	Set if database should be backed up to PC if
															//	no app-specific synchronization conduit has
															//	been supplied.
#define	dmHdrAttrOKToInstallNewer 	0x0010	// This tells the backup conduit that it's OK
															//  for it to install a newer version of this database
															//  with a different name if the current database is
															//  open. This mechanism is used to update the
															//  Graffiti Shortcuts database, for example.
#define	dmHdrAttrLaunchableData		0x0200	// This data database (not applicable for executables)
															//  can be "launched" by passing it's name to it's owner
															//  app ('appl' database with same creator) using
															//  the sysAppLaunchCmdOpenNamedDB action code.


/* These should be in some core file!!! */
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


PalmDBRef CreatePalmDB (const char *nameP, UINT32 type, Err *err) {

	PalmDBRef dbP;
	struct pdb_pdbheader header;

	*err = -1;

	dbP = (PalmDBRef) malloc(sizeof(PalmDBStruct));
	if (dbP == NULL) {
		return NULL;
	}
	memset(dbP, 0x0, sizeof(PalmDBStruct));

	/* Open files */
	if (strlen(nameP) > 31){
		*err = -2;
		free(dbP);
		return NULL;
	}
	sprintf(dbP->tmpName, "%s.pdb", nameP);
	dbP->dbFile = fopen(dbP->tmpName, "wb");
	if(dbP->dbFile == NULL) {
		free(dbP);
		return NULL;
	}

	sprintf(dbP->tmpName, "$$%s.tmp", nameP);
	dbP->tmpFile = fopen(dbP->tmpName, "wb+");
	if(dbP->tmpFile == NULL) {
		fclose(dbP->dbFile);
		dbP->dbFile = NULL;
		free(dbP);
		return NULL;
	}

 	/* Fill header */
 	memset(&header, 0x0, sizeof(struct pdb_pdbheader));
 	strncpy(header.name, nameP, 31);

	/* can install over existing pdb */
	write_word((UINT8*)&header.fileAttributes, (dmHdrAttrOKToInstallNewer | dmHdrAttrLaunchableData));
	write_word((UINT8*)&header.version, 0x0001);
	write_dword((UINT8*)&header.creationDate, 1);
	write_dword((UINT8*)&header.modificationDate, 1);

	/* Set database type */
	write_dword((UINT8*)&header.databaseType, type);

	/* Our registered Palm Creator ID */
	header.creatorID[0]='F';
	header.creatorID[1]='A';
	header.creatorID[2]='G';
	header.creatorID[3]='I';


	/* Write PDB header */
	fwrite(&header, 1, sizeof(struct pdb_pdbheader), dbP->dbFile);

	*err = 0;

	return dbP;
}


Err ClosePalmDB (PalmDBRef dbP) {

	Err err = 0;
	struct pdb_RecordList recList;
	struct pdb_RecordEntry recEntry;
	int i, offset;


	memset(&recList, 0x0, sizeof(struct pdb_RecordList));

	/* if the db has been open for writing we need to reassemble it. */
	if (dbP->tmpFile != NULL) {
		write_word((UINT8*)&recList.numRecords, dbP->numRec);
		/* Write recordlist to file, only the first 6 bytes */
		fwrite(&recList, 1, 6, dbP->dbFile);

		/* Write each record header */
		offset = sizeof(struct pdb_pdbheader) + sizeof(struct pdb_RecordList)
					+ dbP->numRec * sizeof(struct pdb_RecordEntry);
		memset(&recEntry, 0x0, sizeof(struct pdb_RecordEntry));
		for (i=0; i<dbP->numRec; i++) {
			write_dword((UINT8*)&recEntry.recordDataOffset, offset);
			fwrite(&recEntry, 1, sizeof(struct pdb_RecordEntry), dbP->dbFile);
			printf("Record %d: %d bytes.\n", i, dbP->recSize[i]);
			offset += dbP->recSize[i];
		}
		/* Write last 2 byte of recordlist */
		fwrite(&recList.firstEntry, 1, 2, dbP->dbFile);

		/* Write record data */
		err = fseek(dbP->tmpFile, 0x0, SEEK_SET);
		copy_file(dbP->tmpFile, dbP->dbFile);

		/* Close and clean up tmp file */
		fclose(dbP->tmpFile);
		remove(dbP->tmpName);
	}
	err = fclose(dbP->dbFile);
	free(dbP);

	return err;
}

Err AddPalmDBRecord (PalmDBRef dbP, UINT16 *atP, const UINT8 *srcP,	UINT32 bytes) {

	PalmDBRecord rec;
	UINT32 offset = 0;
	Err err;

	rec = CreatePalmDBRecord (dbP, atP, bytes);
	err = WritePalmDBRecord (rec, &offset, srcP, bytes);
	FinalizePalmDBRecord (rec);
	return err;
}

PalmDBRecord CreatePalmDBRecord (PalmDBRef dbP, UINT16 *atP, UINT32 size) {

	/* See header for limitations! */

	if (dbP->numRec >= MAXREC)
		return NULL;
	/* printf("size: %d numRec: %d\n", size, (int)dbP->numRec); */
	(dbP->numRec)++;
	return dbP;
}

Err WritePalmDBRecord (PalmDBRecord record, UINT32 *offset, const UINT8 *srcP,
						UINT32 bytes) {

	Err err = 0;
	int wrote;

	//printf("Writing %d bytes at offset %d.", bytes, *offset);
	wrote = fwrite(srcP, 1, bytes, record->tmpFile);
	if (wrote != bytes) {
		err = -1;
	}
	*offset += wrote;
	record->recSize[record->numRec-1] += wrote;
	return err;
}


Err FinalizePalmDBRecord (PalmDBRecord record) {

	/* Nothing much to do considering the current limitations, see header file */
	Err err = 0;
	return err;
}



#endif