/****************************************************************************
 *
 * Written by Thomas Åkesson <tapilot@home.se> in March 2002
 * Copyright (C) 2002 Thomas Åkesson
 *
 * This is the start of an abstraction of the PalmOS database access.
 *
 * The purpose is:
 * 1) To be able to create Palm databases on a desktop computer.
 * 2) To separate the DB access in order to write code portable between
 *    PalmOS and other platforms.
 * 3) This abstraction should also help non-PalmOS programmers to more
 *    easily understand the code.
 *
 *
 * Implementation details
 *
 * PalmOS:
 * The implementation is not tested (not even compiled!) but should work
 * without limitations.
 *
 * Other platforms:
 * This implementation has several limitations but fulfills the primary
 * purpose of writing a .pdb file from start to end.
 * The following limitations apply (incomplete):
 * 1) The number of records can not exceed 1024 (can easily be extended).
 * 2) Records must be written sequentially.
 * 3) Many functions are not implemented yet (but it should be fairly easy).
 * 4) ...
 *
 *
 ****************************************************************************/

#ifndef __PALMDB_H
#define __PALMDB_H


#ifdef PALMOS
	#include <PalmOS.h>
#else
	#include <stdio.h>
#endif

#define sarienCreatorID 'FAGI'
#define MAKE_TYPE(a,b,c,d) ((UINT32)(a)<<24 |(UINT32)(b)<<16 |(UINT32)(c)<<8 |(UINT32)(d))

#ifdef PALMOS
	typedef DmOpenRef PalmDBRef;
	typedef struct _PalmDBRec {
			PalmDBRef dbP;
			UINT16 index;
			void* recHandle;
	} PalmDBRecStruct;
	typedef PalmDBRecStruct* PalmDBRecord;
#else
	/* A dynamic data structure should be used to get rid of this limitation. */
	#define MAXREC 1024
	#define	dmMaxRecordIndex		0xffff
	typedef struct _PalmDB {
		FILE* dbFile;
		FILE* tmpFile;
		char tmpName[40];
		int numRec;
		int recSize[MAXREC];
	} PalmDBStruct;
	typedef PalmDBStruct* PalmDBRef;
	/* This implementation can only handle one record at a time from a database
	so we only need	to reference the database itself. */
	typedef PalmDBRef PalmDBRecord;
#endif



/*** Database functions ***/

PalmDBRef CreatePalmDB (const char *nameP, UINT32 type, Err *err) ;
/* Creates a new database with the creator set to "FAGI".
Name of new database can be up to 32 ASCII bytes long,
including the null terminator.

Returns a reference to an open database.

On PalmOS:
The error codes are the same as DmCreateDatabase.

On other platforms (in the current implementation):
The error codes are !=0.
*/

PalmDBRef OpenPalmDB (const char* nameP, char* modeStr) ;
/* Opens a database. Set modeStr to "r" or "w".

Returns a reference to an open database.

On PalmOS (in the current implementation):
Searches only card no 0 (will not find db on for example a Springboard).

NOT implemented on non-PalmOS platforms!
*/

Err ClosePalmDB (PalmDBRef) ;
/* Closes a database.

On PalmOS:
The error codes are the same as DmCloseDatabase.

On other platforms (in the current implementation):
The error codes are !=0.
*/

Err RemovePalmDB (const char* nameP) ;
/* Deletes a database.

On PalmOS:
The error codes are the same as DmDeleteDatabase or dmErrDatabaseProtected.

NOT implemented on non-PalmOS platforms!
*/


/*** Record writing functions ***/

Err AddPalmDBRecord (PalmDBRef dbP, UINT16 *atP, const UINT8 *srcP,	UINT32 bytes) ;
/* Creates a new database record with the specified data.
This call is equivalent to calling CreatePalmDBRecord, WritePalmDBRecord and
FinalizePalmDBRecord. On some implementations/platforms this call provides
significantly better performance, on others it is a convenience function.

Initialize atP to dmMaxRecordIndex to append record to the end of the database.

On PalmOS:
Provides significantly better performance compared to separate functions below.
The error codes are the same as DmWrite. The record is marked dirty.
See notes for CreatePalmDBRecord.

On other platforms:
See notes for CreatePalmDBRecord, WritePalmDBRecord and FinalizePalmDBRecord.
*/


PalmDBRecord CreatePalmDBRecord (PalmDBRef dbP, UINT16 *atP, UINT32 size) ;
/* Creates a new database record.
Call FinalizePalmDBRecord when finished writing.

Initialize atP to dmMaxRecordIndex to append record to the end of the database.

Returns a handle to the record or NULL if an error occurs.

On PalmOS:
If *atP is greater than the number of records currently in the database,
the new record is appended to the end and its index is returned in *atP.

On other platforms (in the current implementation):
atP is ignored, the record is always appended to the end of the database.
size is ignored, the size of the record will be the sum of all bytes written.
The record must be finalized before creating a new one.
*/


Err WritePalmDBRecord (PalmDBRecord record, UINT32 *offset, const UINT8 *srcP,
					UINT32 bytes) ;
/* Writes a chunk to a database record. On entry offset contains the position
within the record to start writing, on exit offset contains the position
directly after the last written byte.

On PalmOS:
The error codes are the same as DmWrite.

On other platforms (in the current implementation):
The error codes are !=0.
offset is ignored, the data is always appended to the end of the record.
*/


Err FinalizePalmDBRecord (PalmDBRecord record) ;
/* Closes a newly created database record.

On PalmOS:
The error codes are the same as DmReleaseRecord.
The record is marked dirty.

On other platforms (in the current implementation):
The error codes are !=0.
*/


Err RemovePalmDBRecord (PalmDBRef dbP, UINT16 index) ;
/* Deletes a database record.

On PalmOS:
The error codes are the same as DmRemoveRecord.

On other platforms:
NOT implemented!
*/


/*** Record reading functions ***/

UINT8* QueryPalmDBRecord (PalmDBRef dbP, UINT16 index) ;
/* Reads a database record.
Call ReleasePalmDBRecord when finished.

Returns a memory pointer to the data or NULL if record not found.

NOT implemented on non-PalmOS platforms!
*/

Err ReleasePalmDBRecord (void* dataP) ;
/* Releases a database record after reading.

NOT implemented on non-PalmOS platforms!
*/


#endif /* _PALMDB_H */
