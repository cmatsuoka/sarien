/*
 *  AGIGlue - Sticks resources together into one BIG volume file
 *  Copyright (C) 1997, Floating Hills Software.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Send comments, ideas, and criticism to greekcat@hotmail.com
 *
 */

/*
 * NOTE TO READER:  Yes.  This is a stupid program which isn't very
 * intelligent.  It is a little crufty code designed to do what one 
 * needs so one can play around with AGI files...  When I wrote this,
 * I couldn't find any other program which did this (but I didn't
 * look hard enough).  I include this with the distribution hoping that
 * it might prove marginally useful to you.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PROGRAM_NAME "AGIGlue version 0.90\nCopyright 1997, Floating Hills Software.\n\n"
#define FREE_BANNER "This is free software, and you are welcome to redistribute\nit under certain conditions -- see COPYING for details.\n\n"

#define MAX_NUMBER              250
#define VOL_FILE		"VOL.0"
FILE* volfile;

typedef unsigned short int WORD;
typedef unsigned char BYTE;

long GetFileLength( FILE *file );
void WriteWord( FILE* file, WORD w );
void WriteByte( FILE* file, BYTE b );

void Glue( FILE* file, FILE* dirfile );

long GetFileLength( FILE *file )
{
	long tmp;

	fseek( file, 0L, SEEK_END );
	tmp = ftell( file );
	fseek( file, 0L, SEEK_SET );

	return( tmp );
}

void WriteWord( FILE* file, WORD w )
	{
	fputc( w&255, file );
	fputc( (w>>8)&255, file );
	return;
	}

void WriteByte( FILE* file, BYTE b )
	{
	fputc( b&255, file );
	return;
	}

void Glue( FILE* file, FILE* dirfile )
	{
	long pos;
	long len;
	char* buffer;

	pos = ftell( volfile );
	WriteByte( volfile, 0x12 );
	WriteByte( volfile, 0x34 );
	WriteByte( volfile, 0x00 );

	len = GetFileLength( file );
	WriteWord( volfile, (WORD)len );

	buffer = malloc( len );
	if (buffer == NULL)
		{
		printf( "Error.\nCould not allocate memory for transfer.\n" );
		exit(0);
		}

	if (fread( buffer, 1, len, file ) != len)
		{
		printf( "Error.\nCould not read data from file.\n" );
		exit(0);
		}

	if (fwrite( buffer, 1, len, volfile ) != len)
		{
		printf( "Error.\nCould not write data to %s.\n", VOL_FILE );
		exit(0);
		}

	free( buffer );

	WriteByte( dirfile, (BYTE)((pos>>16) & 0x0F) );
	WriteByte( dirfile, (BYTE)((pos>>8)  & 0xFF) );
	WriteByte( dirfile, (BYTE)((pos)     & 0xFF) );

	return;
	}

void GlueAll( char* filename, FILE* dirfile )
	{ 
	char szFilename[30];
	int i;
	FILE* file;

	for( i=0; i<=MAX_NUMBER; i++ )
		{
		sprintf( szFilename, "%s.%i", filename, i );
		file = fopen( szFilename, "rb" );
		if (file != NULL)
			{
			Glue( file, dirfile );
			fclose( file );
			}
		else
			{
			WriteByte( dirfile, 0xFF );
			WriteByte( dirfile, 0xFF );
			WriteByte( dirfile, 0xFF );
			}
		}

	return;
	}

void GlueDirectory( char* szFilename, char* szDirectory )
	{
	FILE* file;

	file = fopen( szDirectory, "wb" );
	if (file == NULL)
		{
		printf( "Error.\nCould not glue %s.\n", szDirectory );
		exit(0);
		}

	GlueAll( szFilename, file );
	fclose( file );

	return;
	}

void main( void )
	{
        printf( "AGIGlue Version 0.01\nCopyright 1997, Floating Hills Software.\n\n" );

	volfile = fopen( VOL_FILE, "wb" );
	if (volfile == NULL)
		{
		printf( "Could not open " VOL_FILE "\n" );
		return;
		}
	
	printf( "Glueing SNDDIR..." );
	GlueDirectory( "SOUND", "SNDDIR" );
	printf( "Done!\nGlueing PICDIR..." );
	GlueDirectory( "PICTURE", "PICDIR" );
	printf( "Done!\nGlueing VIEWDIR..." );
	GlueDirectory( "VIEW", "VIEWDIR" );
	printf( "Done!\nGlueing LOGDIR..." );
	GlueDirectory( "LOGIC", "LOGDIR" );
	printf( "Done!\n\nNew VOL.0 file glued.\n" );

	fclose( volfile );

	return;
	}
