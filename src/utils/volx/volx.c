/* VOLX by Lance Ewing */

/* Unix port and cosmetic tty output changes
	made by Claudio Matsuoka 99-03-15 */

/* Modifications for OBJECT file and ALL extraction
	made by Joakim Moller 96-08-20 */

/* Modifications for WORDS.TOK file
	made by Martin Tillenius 96-08-20 */

/* This file is under constant development, and still very unstable.. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if 0
#include <dir.h>		// not ANSI-C, but mkdir() is very common
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#if 0
#include "agi.h"
#endif

#define lhconv(a,b) (((a) & 0xff) | ((b) << 8))
	

const char dirNames[4][8] = { "logdir", "picdir", "viewdir", "snddir" };

// I changed this.. Please notice.
const char fileNames[4][9] = { "logic", "picture", "view", "sound" };

// have to declare functions in BC4.5..
void extractFile( int, int );
void extractAll( int );
void extractObjFile( void );
void extractWordsTok( void );
void extractComplete( void );
void decryptText( char *, long );
long getLength( FILE * );

void extractObjFile()
{
	FILE *objfile, *outfile;
	long objlength;
	char *buffer, ctxt[2][10] = { "encrypted", "decrypted" };
	int flag;

	if( NULL == ( objfile = fopen( "object", "rb" ) ) ) {
		printf( "Couldn't open file: object\n" );
		return;
	}

	objlength = getLength( objfile );

	if( NULL == ( outfile = fopen( "object.new", "wb" ) ) ) {
		printf( "Couldn't create outfile!\n" );
		fclose( objfile );
		return;
	}

	if( NULL == ( buffer = malloc( objlength ) ) ) {
		printf( "Couldn't allocate %ld bytes of memory!\n", objlength );
		fclose( objfile );
		fclose( outfile );
	}

	if( 0 == fread( buffer, objlength, 1, objfile ) ) {
		printf( "Read error!\n" );
		fclose( objfile );
		fclose( outfile );
		free( buffer );
	}

	flag = buffer[5] == 0 ? 0 : 1;

	printf( "Converting %s object file to object.new... ", ctxt[flag] );

	decryptText( buffer, objlength );

	if( 0 == fwrite( buffer, objlength, 1, outfile ) ) {
		printf( "Error!\nWrite error!\n" );
		fclose( objfile );
		fclose( outfile );
		free( buffer );
	}

	printf( "Done.\n" );

	fclose( objfile );
	fclose( outfile );
	free( buffer );
}

void decryptText( char *buffer, long length )
{
	char avis[] = "Avis Durgan";
	long i, cnt;

	for( i = 0, cnt = 0; i <= length; i++ ) {
		buffer[i] = buffer[i] ^ avis[cnt++];
		if( cnt > 10 ) cnt = 0;
	}
}

void extractFile(int fileNameNum, int fileNum)
{
	FILE *volFile, *dirFile, *dumpFile;
	long volFilePos;
	unsigned char firstByte, secondByte, thirdByte;
	char volFileName[80], dumpFileName[80];
	unsigned char *buffer;
	unsigned int fLen;

	if ((dirFile = fopen(dirNames[fileNameNum], "rb")) == NULL)
	{
		printf("Cannot open %s\n", dirNames[fileNameNum]);
		exit(1);
	}

	printf( "Extracting %s.%d... ", fileNames[fileNameNum], fileNum );

	fseek(dirFile, fileNum*3, SEEK_SET);
	firstByte = fgetc(dirFile);
	secondByte = fgetc(dirFile);
	thirdByte = fgetc(dirFile);

	fclose(dirFile);

	if ((firstByte == 0xFF) && (secondByte == 0xFF) && (thirdByte == 0xFF))
	{
		printf("Error!\nFile doesn't exist.\n");
		exit(1);
	}
	else
	{
		volFilePos = ((firstByte & 0x0F) * (unsigned long)0x10000)
	 + (secondByte * (unsigned long)0x100) + thirdByte;

		sprintf( volFileName, "vol.%d", (firstByte & 0xF0) >> 4 );

		if ((volFile = fopen(volFileName, "rb")) == NULL)
		{
	 printf("Error!\nCannot open %s.", volFileName);
	 exit(1);
		}

		fseek(volFile, volFilePos + 3, SEEK_SET);
		fLen = lhconv( fgetc(volFile), fgetc(volFile) );

		if( fLen <= 30000 ) {
			if( NULL == ( buffer = malloc( fLen ) ) ) {
				printf( "Error!\nNot enough memory!\n" );
				exit(1);
			}

			sprintf( dumpFileName, "%s.%d", fileNames[fileNameNum], fileNum );

			if ((dumpFile = fopen(dumpFileName, "wb")) == NULL) {
				printf("Error!\nCannot open %s.\n", dumpFileName);
				free( buffer );
				exit(1);
			}

			fread( buffer, fLen, 1, volFile );
			fwrite( buffer, fLen, 1, dumpFile );

			free( buffer );
			fclose(dumpFile);
		} else
			printf( "Error!\n %s.%d - file too big!\n", fileNames[fileNameNum], fileNum );

	}

	printf( "Done.\n" );

	fclose(volFile);
}

void extractAll(int fileNameNum)
{
	FILE *volFile, *dirFile, *dumpFile;
	long volFilePos, filelength;
	unsigned char byte[3];
	char volFileName[80], dumpFileName[80];
	unsigned char *buffer;
	int fLen;
	int fileNum;

	if ((dirFile = fopen(dirNames[fileNameNum], "rb")) == NULL) {
		printf("Cannot open %s.", dirNames[fileNameNum]);
		exit(1);
	}

	printf( "Extracting %s files... ", fileNames[fileNameNum] );

	mkdir( fileNames[fileNameNum], 0755 );

	filelength = getLength( dirFile );

	for( fileNum = 0; fileNum*3 < filelength; fileNum++ ) {
		fread( byte, sizeof( unsigned char ), 3, dirFile );

		if ( (byte[0] != 0xFF) && (byte[1] != 0xFF) && (byte[2] != 0xFF) ) {
			volFilePos = ((byte[0] & 0x0F) * (unsigned long)0x10000)
							+ (byte[1] * (unsigned long)0x100) + byte[2];

			sprintf( volFileName, "vol.%d", (byte[0] & 0xF0) >> 4 );

			if ((volFile = fopen(volFileName, "rb")) == NULL) {
				printf("Error!\nCannot open %s.", volFileName);
				exit(1);
			}

			fseek(volFile, volFilePos + 3, SEEK_SET);
			fLen = lhconv( fgetc(volFile), fgetc(volFile) );

			if( (fLen <= 30000) && (fLen > 0) ) {
				if( NULL == ( buffer = malloc( fLen ) ) ) {
					printf( "Error!\nNot enough memory!\n" );
					exit(1);
				}

				sprintf( dumpFileName, "%s/%s.%d", fileNames[fileNameNum], fileNames[fileNameNum], fileNum );

				if ((dumpFile = fopen(dumpFileName, "wb")) == NULL) {
					printf("Error!\nCannot open %s.\n", dumpFileName);
					free( buffer );
					exit(1);
				}

				fread( buffer, fLen, 1, volFile );
				fwrite( buffer, fLen, 1, dumpFile );

				free( buffer );
				fclose(dumpFile);
			} else
				printf( "\nError! %s.%d - extreme filesize (%d bytes)!\n", fileNames[fileNameNum], fileNum, fLen );

			fclose(volFile);
		}
	}

	printf( "Done.\n" );

	fclose(dirFile);
}

/* Routine written by Martin Tillenius.. Don't blame me for errors.. =) */
void extractWordsTok()
{
	int	i;
	int	same;
	char	letter;
	char	w_name[50];
	int	w_nr;
	int	index[26];
	FILE *in, *ut;

	in = fopen( "words.tok", "rb" );
	ut = fopen( "words.txt", "wb" );

	printf( "Converting words.tok to words.txt... " );

	for(i = 0; i < 26; index[i++] = (fgetc(in) << 8) + fgetc(in));

	for(i = 0; i < 26; i++) {
		fprintf(ut, "*** %c\n", i + 65);
		if(index[i] != 0) {
			fseek(in, index[i], SEEK_SET);
			same = fgetc(in);
			do {
				do {
					letter = fgetc(in);
					w_name[same++] = (letter ^ 0x7f) & 0x7f;
				} while( !feof(in) && ((letter & 0x80) == 0));

				w_name[same] = 0;
				w_nr = (fgetc(in) << 8) + fgetc(in);
				fprintf(ut, "%03d %s\n", w_nr, w_name);
				same = fgetc(in);
			} while( !feof(in) && (same != 0));
		}
	}

	fclose(in);
	fclose(ut);

	printf( "Done.\n" );
}

int main(int argc, char **argv)
{
	int fileNum;

	printf( "VOLX - AGI system extractor\n\n" );

	switch( argc ) {
		case 0:
		case 1:
			printf("Usage: volx -l logicnumber\n");
			printf("       volx -s soundnumber\n");
			printf("       volx -v viewnumber\n");
			printf("       volx -p picturenumber\n");
			printf("       volx -o\n");
			printf("       volx -w\n");
			printf("       volx -a\n\n");
			printf("You can use \"ALL\" instead of number to extract all files..\n");
			break;
		case 2:
			switch( argv[1][1] ) {
				case 'w':
					extractWordsTok();
					break;
				case 'o':
					extractObjFile();
					break;
				case 'a':
					extractComplete();
					break;
				default:
					printf( "Unknown option: %s", argv[1] );
			}
			break;
		case 3:
			if (argv[1][0] == '-')
			{
				fileNum = atoi(argv[2]);

				if ( (fileNum >= 0) && strcmp( argv[2], "ALL" ) )
				{
					switch (argv[1][1])
					{
						case 'l': extractFile(0, fileNum);
							break;
						case 's': extractFile(3, fileNum);
							break;
						case 'v': extractFile(2, fileNum);
							break;
						case 'p': extractFile(1, fileNum);
							break;
						default: printf("Invalid option : %s\n", argv[1]);
					}
				}
				else
				{
					if( !strcmp( argv[2], "ALL" ) )
						switch( argv[1][1] ) {
							case 'l': extractAll(0);
								break;
							case 's': extractAll(3);
								break;
							case 'v': extractAll(2);
								break;
							case 'p': extractAll(1);
								break;
							default: printf("Invalid option : %s\n", argv[1]);
						}
					else
						printf("File number must be a postive number\n");
				}
			}
			else
			{
				printf("Invalid option : %s\n", argv[1]);
			}
	}

	return 0;
}

void extractComplete()
{
	char i;

	printf( "Please wait while extracting...\n" );

	/* extract all LOGIC, VIEW, PIC and SOUND files */
	for( i = 0; i <= 3; i++ )
		extractAll(i);

	/* en-/decrypt OBJECT file */
	extractObjFile();

	/* convert WORDS.TOK to text */
	extractWordsTok();

	printf( "\nAll done!\n" );
}

long getLength( FILE *file )
{
	long tmp;

	fseek( file, 0L, SEEK_END );
	tmp = ftell( file );
	fseek( file, 0L, SEEK_SET );

	return( tmp );
}
