/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#if defined PALMOS

#include <stdio.h>
#include <string.h>
#include "sarien.h"
#include "agi.h"

static int agi_v4_init (void);
static int agi_v4_deinit (void);
static int agi_v4_detect_game (char *);
static int agi_v4_load_resource (int, int);
static int agi_v4_unload_resource (int, int);
static int agi_v4_load_objects(char *);
static int agi_v4_load_words(char *);

UINT32 pdb_read_dword(UINT8 *mem);
UINT16 pdb_read_word(UINT8 *mem);
int load_v2_dir(struct agi_dir *agid, int offset, UINT32 flen, FILE *fp);
int load_v3_dir(struct agi_dir *agid, int offs, UINT32 len, FILE *fp);

extern struct agi_picture pictures[];
extern struct agi_logic logics[];
extern struct agi_view views[];
extern struct agi_sound sounds[];

struct agi_loader agi_v4 = {
	4,
	0,
	agi_v4_init,
	agi_v4_deinit,
	agi_v4_detect_game,
	agi_v4_load_resource,
	agi_v4_unload_resource,
	agi_v4_load_objects,
	agi_v4_load_words
};


/* this must be kept synchronous with the AGI2PDB headers */
#define	PDB_OPT_AGDS		0x0001
#define PDB_OPT_AMIGA		0x0002

#ifdef __WATCOMC__
#pragma pack(1)
#endif

struct pdb_pdbheader
{
	UINT8 name[32];
				/*
				   String - This is the name of the database
				    on the PalmPilot device.
				    It need not match the name of the PDB file
				    in the environment in which it is created.
				*/
	UINT16 fileAttributes;
				/*
				   Attributes of the pdb file.
				    0x0002 Read-Only
				    0x0004 Dirty AppInfoArea
				    0x0008 Backup this database
				           (i.e. no conduit exists)
				    0x0010 Okay to install newer over existing
				           copy, if present on PalmPilot
				    0x0020 Force the PalmPilot to reset after
				           this database is installed
				*/
	UINT16 version;
				/*
				   Defined by the application.
				*/
	UINT32 creationDate;
				/*
				   Expressed as the number of seconds since
				    January 1, 1904. The database will not
				    install if this value is zero. (PalmOS
				    1.0.6)
				*/
	UINT32 modificationDate;
				/*
				   Expressed as the number of seconds since
				    January 1, 1904.
				    The database will not install if this
				    value is zero. (PalmOS 1.0.6)
				*/
	UINT32 lastBackupDate;
				/*
				   Expressed as the number of seconds since
				    January 1, 1904. This can be left at zero
				    and the database will install.
				*/
	UINT32 modificationNumber;
				/*
				   Set to zero.
				*/
	UINT32 appInfoArea;
				/*
				   The byte number in the PDB file (counting
				    from zero) at which the AppInfoArea is
				    located. This must be the first entry
				    in the Data portion of the PDB file.
				    If this database does not have an
				    AppInfoArea, set this value to zero.
				*/
	UINT32 sortInfoArea;
				/*
				   The byte number in the PDB file (counting
				    from zero) at which the SortInfoArea is
				    located. This must be placed immediately
				    after the AppInfoArea, if one exists,
				    within the Data portion of the PDB file.
				    If this database does not have a
				    SortInfoArea, set this value to zero.
				    Do not use this. See Note C below for
				    further details.
				*/
	UINT8 databaseType[4];
				/*
				   String - Set this to the desired value.
				    Generally it should match the Database Type
				    used by the corresponding application.
				    This is 4 characters long and does not have
				    a terminating null.
				*/
	UINT8 creatorID[4];
				/*
				   String - Set this to the desired value.
				    Generally it should match the Creator ID
				    used by the corresponding application. In
				    all cases, you should always register your
				    Creator ID before using it. This is 4
				    characters long and does not have a
				    terminating null.
				*/
	UINT32 uniqueIDSeed;
				/*
				   This is used to generate the Unique ID
				    number of subsequent records. Generally,
				    this should be set to zero.
				*/
	UINT32 nextRecordListID;
				/*
				   Set this to zero. (This does not appear
				    to be used, but that has not been
				    verified by a third party.)
				*/
	UINT16 numberOfRecords;
				/*
				   This contains the number of records
				*/
}
#ifdef __GNUC__
__attribute__((packed))
#endif
;

struct pdb_recheader{
	UINT32 recordDataOffset;
				/*
				   The byte number in the PDB file (counting
				    from zero) at which the record is located.
				*/
	UINT8 recordAttributes;
				/*
				   The records attributes.
 				    0x10 Secret record bit.
				    0x20 Record in use (busy bit).
				    0x40 Dirty record bit.
				    0x80 Delete record on next HotSync.
				    The least significant four bits are used
				     to represent the category values.
				*/
	UINT8	uniqueID[3];
				/*
				   Set this to zero and do not try to
				    second-guess what PalmOS will do with
				    this value.
				*/
}
#ifdef __GNUC__
__attribute__((packed))
#endif
;

struct agi3vol {
	UINT32 sddr;
	UINT32 len;
};

/* a agi v2.xxx game format */
struct PalmOSAGI2
{
	UINT32	logdir[2];
	UINT32	picdir[2];
	UINT32	viewdir[2];
	UINT32	snddir[2];

	UINT32	object[2];
	UINT32	words[2];

	UINT32	vol[16*2];		/* 0=offs, 1=len */
};

/* an agi v3.xx.yyy game format */
struct PalmOSAGI3
{
	UINT32	dir[2];
	UINT32	object[2];
	UINT32	words[2];
	UINT32	vol[16*2];		/* 0=offs, 1=len */
};

union PalmOSAGI4
{
	struct PalmOSAGI2 v2;
	struct PalmOSAGI3 v3;
};

/* PalmOS AGI Header */
struct PalmOSAGI
{
	char	name[32];
	UINT16	version;
	UINT16	emulation;
	UINT32	options;
	UINT32	crc;

	union PalmOSAGI4 v4;
};

#ifdef __WATCOMC__
#pragma pack()
#endif

struct PalmOSAGI palmagi;
struct pdb_pdbheader PDBHeader;

/* our PDB file that contains everything */
char	pdb_file[MAX_PATH];
UINT32	pdb_start;

static int agi_v4_detect_game (char *gn)
{
	FILE *fp;
	int ec= err_InvalidAGIFile;
	int offset;
	struct pdb_recheader rh;

	/* gn is filename for a .pdb! */

	memset(&PDBHeader, 0x0, sizeof(struct pdb_pdbheader));
	memset(&palmagi, 0x0, sizeof(struct PalmOSAGI));
	strcpy(pdb_file, gn);

	fp = fopen (pdb_file, "rb");
	if(fp!=NULL)
	{
		fread(&PDBHeader, 1, sizeof(struct pdb_pdbheader), fp);

		if(PDBHeader.creatorID[0]=='F' && PDBHeader.creatorID[1]=='A' &&
		   PDBHeader.creatorID[2]=='G' && PDBHeader.creatorID[3]=='I')
		{
			/* valid palm database with Sarien Creator ID */
			/* now skip record headers, read in v4 Header */

			fread(&rh, 1, sizeof(struct pdb_recheader), fp);

			offset=(int)pdb_read_dword((UINT8*)&rh.recordDataOffset);

			/* our data all starts from here. add this to all offsets */
			pdb_start=offset;

			fseek(fp, offset, SEEK_SET);
			fread(&palmagi, 1, sizeof(struct PalmOSAGI), fp);

			palmagi.crc=pdb_read_dword((UINT8*)&palmagi.crc);
			palmagi.options=pdb_read_word((UINT8*)&palmagi.options);
			palmagi.emulation=pdb_read_word((UINT8*)&palmagi.version);
			palmagi.version=pdb_read_word((UINT8*)&palmagi.version);

			/* setup our options before we call setup game! */
			if((palmagi.options&PDB_OPT_AGDS))
				opt.agds=1;

			if((palmagi.options&PDB_OPT_AMIGA))
				opt.amiga=1;

			v4id_game(palmagi.crc);

			/* if the game is a v2 */
			
			/* use agi_get_release instead of palmagi.emulation */			
			if((agi_get_release()>>12)==0x2)
				agi_v4.int_version = 0x2917;		/* setup for 2.917 */
			else
				agi_v4.int_version = 0x3149;		/* setup for 3.002.149 */

			ec=err_OK;
		}

		fclose(fp);
	}
	
	return ec;
}


static int agi_v4_load_dir (struct agi_dir *agid, int intDirType)
{
	int ec=err_OK;

#ifndef PALMOS
	FILE *fp;
	struct agi3vol agi_vol3[4];
	UINT32	offs, flen;
	UINT16 xd[4];
	int	i;

	fflush(stdout);
	if ((fp = fopen (pdb_file, "rb")) == NULL) {
		return err_BadFileOpen;
	}

	if((palmagi.emulation>>12)==2)
	{
		/* load singledir v2 style */
		switch(intDirType)
		{
			case rLOGIC: ec=load_v2_dir(agid, pdb_start + palmagi.v4.v2.logdir[0], palmagi.v4.v2.logdir[1], fp); break;
			case rPICTURE: ec=load_v2_dir(agid, pdb_start + palmagi.v4.v2.picdir[0], palmagi.v4.v2.picdir[1], fp); break;
			case rSOUND: ec=load_v2_dir(agid, pdb_start + palmagi.v4.v2.snddir[0], palmagi.v4.v2.snddir[1], fp); break;
			case rVIEW: ec=load_v2_dir(agid, pdb_start + palmagi.v4.v2.viewdir[0], palmagi.v4.v2.viewdir[1], fp); break;
		}
	}
	else
	{
		/* load v3 gamedir style */
		offs=pdb_start + palmagi.v4.v3.dir[0];
		flen=palmagi.v4.v3.dir[1];

		fseek(fp, offs, SEEK_SET);

		/* FIXME: Not Endian Aware */
		fread (&xd, 1, 8, fp);

		for(i = 0; i < 4; i++)
			agi_vol3[i].sddr = offs + xd[i];

		agi_vol3[0].len = agi_vol3[1].sddr - agi_vol3[0].sddr;
		agi_vol3[1].len = agi_vol3[2].sddr - agi_vol3[1].sddr;
		agi_vol3[2].len = agi_vol3[3].sddr - agi_vol3[2].sddr;
		agi_vol3[3].len = ftell(fp) - agi_vol3[3].sddr;

		if (agi_vol3[3].len > 256 * 3)
			agi_vol3[3].len = 256 * 3;

		/* read in all directory file info */
		ec=load_v3_dir(game.dir_logic, agi_vol3[0].sddr, agi_vol3[0].len, fp);
		if(ec==err_OK)
			ec=load_v3_dir(game.dir_pic, agi_vol3[1].sddr, agi_vol3[1].len, fp);
		if(ec==err_OK)
			ec=load_v3_dir(game.dir_view, agi_vol3[2].sddr, agi_vol3[2].len, fp);
		if(ec==err_OK)
			ec=load_v3_dir(game.dir_sound, agi_vol3[3].sddr, agi_vol3[3].len, fp);
	}

	fclose(fp);
#endif

	return ec;
}

int load_v2_dir(struct agi_dir *agid, int offset, UINT32 flen, FILE *fp)
{
	UINT8 *mem;
	int i;

	if ((mem = malloc (flen + 32)) == NULL) {
		fclose (fp);
		return err_NotEnoughMemory;
	}

	fseek (fp, offset, SEEK_SET);
	fread (mem, 1, flen, fp);

	/* set all directory resources to gone */
	for (i = 0; i < MAX_DIRS; i++) {
		agid[i].volume = 0xff;
		agid[i].offset = _EMPTY;
	}

	/* build directory entries */
	for (i = 0; i < flen; i+=3) {
		agid[i/3].volume = hilo_getbyte (mem+i) >> 4;
		agid[i/3].offset = hilo_getpword (mem+i) & _EMPTY;
	}

	free (mem);

	return err_OK;
}

int load_v3_dir(struct agi_dir *agid, int offs, UINT32 len, FILE *fp)
{
	int ec = err_OK;
	UINT8 *mem;
	int i;

	fseek (fp, offs, SEEK_SET);
	if ((mem = malloc (len + 32)) != NULL) {
		fread(mem, 1, len, fp);

		/* set all directory resources to gone */
		for(i = 0; i < MAX_DIRS; i++) {
			agid[i].volume = 0xFF;
			agid[i].offset = _EMPTY;
		}

		/* build directory entries */
		for(i = 0; i < len; i += 3) {
			agid[i / 3].volume = hilo_getbyte (mem + i) >> 4;
			agid[i / 3].offset = hilo_getpword (mem+i) & _EMPTY;
		}

		free(mem);
	} else {
		ec = err_NotEnoughMemory;
	}

	return ec;
}


static int agi_v4_init ()
{
	int ec = err_OK;

	/* load directory files */
	switch((palmagi.emulation>>12))
	{
		case 2:
			ec = agi_v4_load_dir (game.dir_logic, rLOGIC);
			if (ec == err_OK)
				ec = agi_v4_load_dir (game.dir_pic, rPICTURE);
			if (ec == err_OK)
				ec = agi_v4_load_dir (game.dir_view, rVIEW);
			if (ec == err_OK)
				ec = agi_v4_load_dir (game.dir_sound, rSOUND);
			break;
		case 3:
			/* a v3 game load will load all DIR files,
			   so below is just dummy code */
			ec=agi_v4_load_dir(game.dir_logic, rLOGIC);
			break;
	}

	return ec;
}


static int agi_v4_deinit ()
{
	int ec = err_OK;
	return ec;
}



static int agi_v4_unload_resource (int restype, int resnum)
{
	switch (restype) {
	case rLOGIC:
		unload_logic (resnum);
		break;
	case rPICTURE:
		unload_picture (resnum);
		break;
	case rVIEW:
		unload_view (resnum);
		break;
	case rSOUND:
		unload_sound (resnum);
		break;
	}

	return err_OK;
}


/*
 * This function does noting but load a raw resource into memory,
 * if further decoding is required, it must be done by another
 * routine. NULL is returned if unsucsessfull.
 */

UINT8* agi_v4v2_load_vol_res (struct agi_dir *agid)
{
#ifndef PALMOS
	UINT8 *data = NULL;
	FILE *fp;
	UINT32	offs;
	char	x[16];

	/* fill offs with volume offset*/

	offs = pdb_start + palmagi.v4.v2.vol[(2*agid->volume)];

	/* loading a bad resource */
	if(offs==pdb_start)
		agid->offset=_EMPTY;

	_D ("(agi_dir = [offset:%ld, len:%ld])", agid->offset, agid->len);

	if (agid->offset != _EMPTY && (fp = fopen (pdb_file, "rb")) != NULL) {
		_D ("loading resource");
		fseek (fp, offs + agid->offset, SEEK_SET);
		fread (&x, 1, 5, fp);
		if (hilo_getword (x) == 0x1234) {
			agid->len = lohi_getword (x + 3);
			data = calloc (1, agid->len + 32);
			if (data != NULL)
				fread (data, 1, agid->len, fp);
		} else {
			/* FIXME: call some panic handler instead of
			 *        deiniting directly
			 */
			deinit_video_mode ();
			fprintf (stderr, "ACK! BAD RESOURCE!!!\n");
			exit (0);
		}
		fclose (fp);
	} else {
		/* we have a bad volume resource */
		/* set that resource to NA */
		agid->offset = _EMPTY;
	}

	return data;
#endif
}

UINT8* agi_v4v3_load_vol_res (struct agi_dir *agid)
{
#ifndef PALMOS
	UINT8 x[MAX_PATH], *data = NULL, *comp_buffer;
	FILE *fp;
	UINT32 offs;

	_D ("(%p)", agid);

	offs = pdb_start + palmagi.v4.v3.vol[2*agid->volume];
	/* loading a bad resource */
	if(offs==pdb_start)
		agid->offset=_EMPTY;

	if (agid->offset != _EMPTY && (fp = fopen(pdb_file, "rb")) != NULL) {
		fseek (fp, offs + agid->offset, SEEK_SET);
		fread (&x, 1, 7, fp);

		if (hilo_getword(x) != 0x1234) {
			/* FIXME */
			deinit_video_mode();
			printf("ACK! BAD RESOURCE!!!\n");
			exit(0);
		}

		agid->len = lohi_getword (x + 3);	/* uncompressed size */
		agid->clen = lohi_getword (x + 5);	/* compressed len */

		comp_buffer = calloc (1, agid->clen + 32);
		fread (comp_buffer, 1, agid->clen, fp);

		if (x[2] & 0x80 || agid->len == agid->clen) {
			/* do not decompress */
			data = comp_buffer;
		} else {
			/* it is compressed */
			data = calloc (1, agid->len + 32);
			LZW_expand (comp_buffer, data, agid->len);
			free (comp_buffer);
			agid->flags |= RES_COMPRESSED;
		}

		fclose(fp);
	} else {
		/* we have a bad volume resource */
		/* set that resource to NA */
		agid->offset = _EMPTY;
	}

	return data;
#endif
}



/*
 * Loads a resource into memory, a raw resource is loaded in
 * with above routine, then further decoded here.
 */
int agi_v4_load_resource (int restype, int resnum)
{
	int ec = err_OK;
	UINT8 *data = NULL;


	_D (_D_WARN "(restype = %d, resnum = %d)", restype, resnum);
	if (resnum > MAX_DIRS)
		return err_BadResource;

	switch (restype) {
	case rLOGIC:
		if (~game.dir_logic[resnum].flags & RES_LOADED) {
			_D (_D_WARN "loading logic resource %d", resnum);
			agi_v4.unload_resource (rLOGIC, resnum);
			/* load raw resource into data */
			if((palmagi.emulation>>12)==2)
				data = agi_v4v2_load_vol_res (&game.dir_logic[resnum]);
			else
				data = agi_v4v3_load_vol_res(&game.dir_logic[resnum]);

			ec = (logics[resnum].data = data) ?
				decode_logic (resnum) : err_BadResource;

			logics[resnum].sIP = 2;
       	}

   		logics[resnum].cIP = logics[resnum].sIP;
		break;
	case rPICTURE:
		/* if picture is currently NOT loaded *OR* cacheing is off,
		 * unload the resource (caching == off) and reload it
		 */

		_D (_D_WARN "loading picture resource %d", resnum);
		if (game.dir_pic[resnum].flags & RES_LOADED)
			break;

		/* if loaded but not cached, unload it */
		/* if cached but not loaded, etc */
		agi_v4.unload_resource (rPICTURE, resnum);

		if((palmagi.emulation>>12)==2)
		{
			data = agi_v4v2_load_vol_res (&game.dir_pic[resnum]);
    		if (data != NULL) {
    			pictures[resnum].rdata = data;
    			game.dir_pic[resnum].flags |= RES_LOADED;
    		} else {
    			ec = err_BadResource;
    		}
		}
		else
		{
			data = agi_v4v3_load_vol_res(&game.dir_pic[resnum]);
			if (data != NULL) {
				data = convert_v3_pic (data,
					game.dir_pic[resnum].len);
				pictures[resnum].rdata = data;
				game.dir_pic[resnum].flags |= RES_LOADED;
			} else {
				ec=err_BadResource;
			}
		}

		break;
	case rSOUND:
		_D (_D_WARN "loading sound resource %d", resnum);
		if (game.dir_sound[resnum].flags & RES_LOADED)
			break;

		if((palmagi.emulation>>12)==2)
			data = agi_v4v2_load_vol_res (&game.dir_sound[resnum]);
		else
			data = agi_v4v3_load_vol_res(&game.dir_sound[resnum]);

		if (data != NULL) {
			sounds[resnum].rdata = data;
			game.dir_sound[resnum].flags |= RES_LOADED;
			decode_sound (resnum);
		} else {
			ec = err_BadResource;
		}
		break;
	case rVIEW:
		/* Load a VIEW resource into memory...
		 * Since VIEWS alter the view table ALL the time
		 * can we cache the view? or must we reload it all
		 * the time?
		 */
		if (game.dir_view[resnum].flags & RES_LOADED)
			break;

		_D (_D_WARN "loading view resource %d", resnum);
    		agi_v4.unload_resource (rVIEW, resnum);

    		if((palmagi.emulation>>12)==2)
    		{
    			if ((data = agi_v4v2_load_vol_res (&game.dir_view[resnum]))) {
	    			views[resnum].rdata = data;
    				game.dir_view[resnum].flags |= RES_LOADED;
    				ec = decode_view (resnum);
    			} else {
	    			ec=err_BadResource;
				}
	    	}
	    	else
	    	{
	    		if ((data = agi_v4v3_load_vol_res (&game.dir_view[resnum]))) {
    				views[resnum].rdata = data;
    				game.dir_view[resnum].flags |= RES_LOADED;
    				ec = decode_view (resnum);
    			} else {
	    			ec=err_BadResource;
				}
	    	}
		break;
	default:
		ec = err_BadResource;
		break;
	}

	return ec;
}


UINT16 pdb_read_word(UINT8 *mem)
{
	UINT16	w;

	w = (UINT16)mem[1];
	w += (UINT16)mem[0]<<8;

	return w;
}

UINT32 pdb_read_dword(UINT8 *mem)
{
	UINT32 w;

	w = (UINT32)mem[3];
	w += (UINT32)mem[2]<<8;
	w += (UINT32)mem[1]<<16;
	w += (UINT32)mem[0]<<24;
	
	return w;
}

static int agi_v4_load_objects(char *fname)
{
	int ec=err_OK;
#ifndef PALMOS
	FILE *fp;
	UINT8	*mem;
	UINT32	len;
	UINT32	offs;

	game.num_objects=0;
	fp=fopen(pdb_file, "rb");
	if(fp!=NULL)
	{
		if((palmagi.emulation>>12)==2)
		{
			offs=palmagi.v4.v2.object[0];
			len=palmagi.v4.v2.object[1];
		}
		else
		{
			offs=palmagi.v4.v3.object[0];
			len=palmagi.v4.v3.object[1];
		}

		fseek(fp, pdb_start + offs, SEEK_SET);
		mem=(UINT8*)malloc(32+len);
		fread(mem, 1, len, fp);
		ec=decode_objects(mem, len);

		free(mem);
		fclose(fp);
	}
#endif
	return ec;
}

static int agi_v4_load_words(char *fname)
{
	int ec=err_OK;
#ifndef PALMOS
	FILE *fp;
	UINT8	*mem;
	UINT32	len;
	UINT32	offs;

	fp=fopen(pdb_file, "rb");
	if(fp!=NULL)
	{
		if((palmagi.emulation>>12)==2)
		{
			offs=palmagi.v4.v2.words[0];
			len=palmagi.v4.v2.words[1];
		}
		else
		{
			offs=palmagi.v4.v3.words[0];
			len=palmagi.v4.v3.words[1];
		}

		fseek(fp, pdb_start + offs, SEEK_SET);
		mem=(UINT8*)malloc(32+len);
		fread(mem, 1, len, fp);
		ec=decode_words(mem, len);
		free(mem);
		fclose(fp);
	}
#endif
	return ec;
}

#endif /* PALMOS */
