
#ifndef __AGI2PDB_H
#define __AGI2PDB_H

#ifdef __cplusplus
extern "C"{
#endif

#include "sarien.h"
#include "agi.h"
#include "palmdb.h"


void write_word(UINT8 *mem, UINT16 w);
void write_dword(UINT8 *mem, UINT32 w);
void DieWithError(char *errstr, Err err);

int convert2pdb (struct agi_loader *loader, char *pdbname);
int convertDir(struct agi_dir *agid, UINT16 dir[], UINT16 *index);
Err convertObjects(PalmDBRecord rec, UINT16 num_objects);
UINT32 match_crc_name (UINT32 crc, char *path, char *game_name);


#define sndTypeAGI 1
#define sndTypeSMF0 2
#define sndTypeSMF1 3
struct agi_v4_sndInfo {
			  UINT8 sndType;
			  UINT8 sndChannels; /* Valid range: 0..4 */
};

struct agi_v4_header {
	UINT16  format;  /* Type of the database. Currently 0 */
	UINT16  revision; /* Revision number of the database format, currently 0 */
	char    id[8]; /* Game ID in order to easily clean up (savegames) when deleting a game. */
	UINT16  version; /* Interpreter version used by the game */
	UINT16  emulation; /* Interpreter version to emulate */
	UINT32  options; /* For Amiga/AGDS, etc (it seems that the current agi_v4 is independent of those)*/
	struct agi_v4_sndInfo sndInfo;
}
#ifdef __GNUC__
__attribute__((packed))
#endif
;


#ifdef __cplusplus
};
#endif
#endif
