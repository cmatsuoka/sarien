
#ifndef __AGI2PDB_H
#define __AGI2PDB_H

#include <stdlib.h>

#ifdef __cplusplus
extern "C"{
#endif

typedef unsigned char	UINT8;
typedef unsigned short	UINT16;
#ifndef WIN32
typedef unsigned long	UINT32;
#endif
typedef signed char	SINT8;
typedef signed short	SINT16;
typedef signed long	SINT32;

#ifndef FALSE
#define FALSE		0
#define TRUE		(!FALSE)
#endif

#ifndef MAX_PATH
#define MAX_PATH	256
#endif

extern	char	path[];
extern	char	dir[];
extern	char	name[];
extern	char	game_name[];
extern	char	exec_name[];
extern	char	*out_file;
extern	struct PalmOSAGI palmagi;

#define	OPT_AGDS		0x0001
#define OPT_AMIGA		0x0002

struct sarien_options {
	int forceload;		/* force loading of all resources */
	int cache;		/* cache loaded resources */
	int gamerun;		/* status for game */
	int pcjrgfx;		/* PCJr graphics */
	int showscreendraw;	/* show screen drawing */
	int showkeypress;
	int emuversion;
#ifndef NO_DEBUG
	int debug;
#endif
	int scale;		/* window scale factor */
	int agds;		/* enable AGDS mode */
	int amiga;		/* enable Amiga mode */
	int fullscreen;		/* use full screen mode if available */
	int nosound;		/* disable sound */
#ifdef MITSHM
	int mitshm;		/* use shared memory extension */
#endif
#ifdef XF86DGA
	int dga;		/* use XFree86 DGA extension */
#endif
	int soundemu;		/* sound emulation mode */
};

extern struct sarien_options opt;
extern char *fixpath (int flag, char *fname);

enum error {
	err_OK = 0,
	err_DoNothing,
	err_BadCLISwitch,
	err_InvalidAGIFile,
	err_BadFileOpen,
	err_NotEnoughMemory,
	err_BadResource,
	err_UnknownAGIVersion,
	err_RestartGame,
	err_NoLoopsInView,
	err_ViewDataError,
	err_NoGameList,

	err_Unk = 127
};

enum {
	NO_GAMEDIR = 0,
	GAMEDIR
};

#define DIR_		"dir."

#define LOGDIR		"logdir"
#define PICDIR		"picdir"
#define VIEWDIR		"viewdir"
#define	SNDDIR		"snddir"
#define OBJECTS		"object"
#define WORDS		"words.tok"

#ifdef _TRACE
#include <stdio.h>
#define _D(args...) do { \
        printf("\x1b[33m" __PRETTY_FUNCTION__ " \x1b[37m[" __FILE__ \
        ":%d] " _D_INFO, __LINE__); printf (args); printf ("\x1b[0m\n"); \
        } while (0)
#else
#ifdef _D
#undef _D
#endif

void _D(char *, ...);
#endif

extern void write_word(UINT8 *mem, UINT16 w);
extern void write_dword(UINT8 *mem, UINT32 w);


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
	UINT32 nextRecordListID;// Set this to zero. (This does not appear
				//  to be used, but that has not been
				//  verified by a third party.)
	UINT16 numberOfRecords;	// This contains the number of records
}
#ifdef __GNUC__
__attribute__((packed))
#endif
;

struct pdb_recheader{
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


struct PalmOSAGI2
{
	UINT32	logdir[2];
	UINT32	picdir[2];
	UINT32	viewdir[2];
	UINT32	snddir[2];

	UINT32	object[2];
	UINT32	words[2];

	UINT32	vol[16*2];		// 0=offs, 1=len
};

struct PalmOSAGI3
{
	UINT32	dir[2];
	UINT32	object[2];
	UINT32	words[2];
	UINT32	vol[16*2];		// 0=offs, 1=len
};

union PalmOSAGI4
{
	struct PalmOSAGI2 v2;
	struct PalmOSAGI3 v3;
};

struct PalmOSAGI
{
	char	name[32];
	UINT16	version;
	UINT16	emulation;
	UINT32	options;
	UINT32	crc;

	union PalmOSAGI4 v4;
};

int v2id_game (void); 
int v3id_game (void);

#ifdef __cplusplus
};
#endif
#endif
