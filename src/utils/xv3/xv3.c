/***************************************************************************
** xv3.c
**
** Extractor for AGIv3 resources.
**
** The original LZW code is from DDJ, October 1989, p.86.
** It has been modified to handle AGI compression by Lance Ewing.
**
** Joakim Moller and Martin Tillenius' code from VOLX have been
** duplicated here for the OBJECT, WORDS.TOK and ALL options.
**
** Unix port by Claudio Matsuoka <claudio@helllabs.org>
** Mon Mar 15 11:17:20 EST 1999
**
** All the rest, (c) Lance Ewing, 1997.
***************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#if 0
#include <dir.h>
#else
#include <glob.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#define MAXBITS 12
#define TABLE_SIZE 18041
#define START_BITS 9

const char fileNames[4][9] = {
   "logic.", "picture.", "view.", "sound."
};

typedef struct {
   unsigned short sig;
   unsigned char compType;
   unsigned short decompSize;
   unsigned short fLen;
} resHeaderType;

#define TRUE 1
#define FALSE 0

/*************************** Function prototypes **************************/
void initLZW();
void closeLZW();
int setBITS(int value);
char *decode_string(unsigned char *buffer,unsigned int code);
unsigned int input_code(unsigned char **input);
void expand(unsigned char *input, unsigned char *output, int fileLength);
void loadGameSig(char *gameSig);
void extractFile(int agiFileType, int fileNum);
void extractObjFile();
void decryptText(char *buffer, long length);
void extractWordsTok();
void extractComplete();
void extractAll(int agiFileType);
long getLength(FILE *file);
void dumpAGIv2PIC(unsigned char *picBuf, int picLen, FILE *dumpFile);
void dumpAGIv2LOG(unsigned char *logBuf, int logLen, FILE *dumpFile);

/******************************* LZW variables ****************************/
int BITS, MAX_VALUE, MAX_CODE;
unsigned int *prefix_code;
unsigned char *append_character;
unsigned char decode_stack[4000];  /* Holds the decoded string */
static int input_bit_count=0;    /* Number of bits in input bit buffer */
static unsigned long input_bit_buffer=0L;

/************************ AGI resources buffers ***************************/
unsigned char sierraBuf[20000], outputBuf[20000];

/***************************** Misc variables *****************************/
int V2CONV = 0;

void initLZW()
{
   prefix_code = (unsigned int *)malloc(TABLE_SIZE*sizeof(unsigned int));
   append_character = (unsigned char *)malloc(TABLE_SIZE*sizeof(unsigned char));
}

void resetLZW()
{
   input_bit_count=0;
   input_bit_buffer=0L;
}

void closeLZW()
{
   free(prefix_code);
   free(append_character);
}

long getLength(FILE *file)
{
	long tmp;

	fseek(file, 0L, SEEK_END);
	tmp = ftell(file);
	fseek(file, 0L, SEEK_SET);

	return(tmp);
}

/***************************************************************************
** setBITS
**
** Purpose: To adjust the number of bits used to store codes to the value
** passed in.
***************************************************************************/
int setBITS(int value)
{
   if (value == MAXBITS) return TRUE;

   BITS = value;
   MAX_VALUE = (1 << BITS) - 1;
   MAX_CODE = MAX_VALUE - 1;
   return FALSE;
}

/***************************************************************************
** decode_string
**
** Purpose: To return the string that the code taken from the input buffer
** represents. The string is returned as a stack, i.e. the characters are
** in reverse order.
***************************************************************************/
char *decode_string(unsigned char *buffer,unsigned int code)
{
   int i;

   i=0;
   while (code > 255) {
      *buffer++ = append_character[code];
      code=prefix_code[code];
      if (i++>=4000) {
         printf("Fatal error during code expansion.\n");
	      exit(0);
      }
   }
   *buffer=code;
   return(buffer);
}
/***************************************************************************
** input_code
**
** Purpose: To return the next code from the input buffer.
***************************************************************************/
unsigned int input_code(unsigned char **input)
{
   unsigned int return_value;

   while (input_bit_count <= 24) {
      input_bit_buffer |= (unsigned long) *(*input)++ << input_bit_count;
      input_bit_count += 8;
   }

   return_value = (input_bit_buffer & 0x7FFF) % (1 << BITS);
   input_bit_buffer >>= BITS;
   input_bit_count -= BITS;
   return(return_value);
}

/***************************************************************************
** expand
**
** Purpose: To uncompress the data contained in the input buffer and store
** the result in the output buffer. The fileLength parameter says how
** many bytes to uncompress. The compression itself is a form of LZW that
** adjusts the number of bits that it represents its codes in as it fills
** up the available codes. Two codes have special meaning:
**
**  code 256 = start over
**  code 257 = end of data
***************************************************************************/
void expand(unsigned char *input, unsigned char *output, int fileLength)
{
   unsigned int next_code, new_code, old_code;
   int character, /* counter=0, index, */ BITSFull /*, i */;
   unsigned char *string, *endAddr;

   BITSFull = setBITS(START_BITS);  /* Starts at 9-bits */
   next_code = 257;                 /* Next available code to define */

   endAddr = (unsigned char *)((long)output + (long)fileLength);

   old_code = input_code(&input);    /* Read in the first code */
   character = old_code;
   new_code = input_code(&input);

   while ((output < endAddr) && (new_code != 0x101)) {

      if (new_code == 0x100) {      /* Code to "start over" */
	      next_code = 258;
	      BITSFull = setBITS(START_BITS);
	      old_code = input_code(&input);
	      character = old_code;
         *output++ = (char)character;
	      new_code = input_code(&input);
      }
      else {
	      if (new_code >= next_code) { /* Handles special LZW scenario */
	         *decode_stack = character;
	         string = decode_string(decode_stack+1, old_code);
	      }
	      else
	         string = decode_string(decode_stack, new_code);

         /* Reverse order of decoded string and store in output buffer. */
	      character = *string;
	      while (string >= decode_stack)
            *output++ = *string--;

	      if (next_code > MAX_CODE)
	         BITSFull = setBITS(BITS + 1);

         prefix_code[next_code] = old_code;
	      append_character[next_code] = character;
	      next_code++;
	      old_code = new_code;

	      new_code = input_code(&input);
      }
   }
}

/****************************************************************************
** loadGameSig
**
** Purpose: To determine the game ID signature by finding the DIR and VOL.0
** file and copying what the prefix for each is. If they agree, then it is
** a valid version 3 game.
****************************************************************************/
void loadGameSig(char *gameSig)
{
   glob_t pglob;
   char dirString[10]="", volString[10]="";

   if (glob ("*dir", GLOB_ERR | GLOB_NOSORT, NULL, &pglob)) {
	globfree (&pglob);
	gameSig = NULL;
	return;
   }

   strncpy (dirString, pglob.gl_pathv[0], strlen (pglob.gl_pathv[0]) - 3);
   globfree (&pglob);

   if (glob ("*vol.0", GLOB_ERR | GLOB_NOSORT, NULL, &pglob)) {
	globfree (&pglob);
	gameSig = NULL;
	return;
   }

   strncpy (volString, pglob.gl_pathv[0], strlen (pglob.gl_pathv[0]) - 5);
   globfree (&pglob);

   if ((strcmp(volString, dirString) == 0) && (volString != NULL))
      strcpy(gameSig, volString);
   else
      gameSig = NULL;
}

void extractFile(int agiFileType, int fileNum)
{
   char gameSig[32], dirFileName[32], volFileName[32], dumpFileName[32];
   FILE *dirFile, *volFile, *dumpFile;
   unsigned char dirOffsetLo, dirOffsetHi, firstByte, secondByte, thirdByte;
   /* unsigned short dirFilePos; */
   long volFilePos;
   resHeaderType resHeader;

   loadGameSig(gameSig);
   if (gameSig == NULL) {
      printf("Could not locate version 3 game files!\n");
      printf("Make sure you run XV3 in a directory\n");
      printf("containing a game which uses AGI v3.\n");
      exit(0);
   }

   sprintf(dirFileName, "%s%s", gameSig, "dir");
   if ((dirFile = fopen(dirFileName, "rb")) == NULL) {
      printf("Error opening file : %s\n", dirFileName);
      exit(0);
   }

   fseek(dirFile, (agiFileType * 2), SEEK_SET);
   dirOffsetLo = fgetc(dirFile);
   dirOffsetHi = fgetc(dirFile);
   fseek(dirFile, ((dirOffsetHi << 8) + dirOffsetLo) + (fileNum*3), SEEK_SET);
   firstByte = fgetc(dirFile);
   secondByte = fgetc(dirFile);
   thirdByte = fgetc(dirFile);
   fclose(dirFile);

   if ((firstByte == 0xFF) && (secondByte == 0xFF) && (thirdByte == 0xFF)) {
      printf("Resource file doesn't exist\n");
      exit(1);
   }

   volFilePos = ((firstByte & 0x0F) * (unsigned long)0x10000)
      + (secondByte * (unsigned long)0x100) + thirdByte;

   sprintf( volFileName, "%svol.%d", gameSig, (firstByte & 0xF0) >> 4 );

   if ((volFile = fopen(volFileName, "rb")) == NULL) {
	   printf("Error!\nCannot open %s.", volFileName);
	   exit(1);
   }

   fseek(volFile, volFilePos, SEEK_SET);
   fread(&resHeader.sig, sizeof(unsigned short), 1, volFile);
   fread(&resHeader.compType, sizeof(unsigned char), 1, volFile);
   fread(&resHeader.decompSize, sizeof(unsigned short), 2, volFile);

   if (resHeader.sig != 0x3412) {
      printf("Error: The DIR file appears to be corrupt.\n");
      printf("       Resource is not at the specified location.\n");
      exit(1);
   }

   fread(sierraBuf, sizeof(char), resHeader.fLen, volFile);
   fclose(volFile);

   /********************** DECOMPRESS TEMP FILE ******************/

   initLZW();
   resetLZW();

   sprintf(dumpFileName, "%s%d", fileNames[agiFileType], fileNum);
   printf("Extracting %s... ", dumpFileName);
   if ((dumpFile = fopen(dumpFileName, "wb")) == NULL) {
      printf("Error opening dump file : %s\n", dumpFileName);
      exit(1);
   }

   /* AGI interpreter knows that it is not compressed if either the
      compressed size is equal to the uncompressed size, or the resource is
      specifically marked as being a PICTURE resource */
   if ((resHeader.compType & 0x80) || (resHeader.decompSize == resHeader.fLen)) {
      if (V2CONV) dumpAGIv2PIC(sierraBuf, resHeader.fLen, dumpFile);
      else fwrite(sierraBuf, sizeof(char), resHeader.fLen, dumpFile);
   }
   else {
      expand(sierraBuf, outputBuf, resHeader.decompSize);
      if (V2CONV) dumpAGIv2LOG(outputBuf, resHeader.decompSize, dumpFile);
      else fwrite(outputBuf, sizeof(char), resHeader.decompSize, dumpFile);
   }

   fclose(dumpFile);
   closeLZW();

   printf("Done.\n");
}

void extractAll(int agiFileType)
{
   char gameSig[6], dirFileName[9], volFileName[9], dumpFileName[13];
   FILE *dirFile, *volFile, *dumpFile;
   unsigned char dirOffsetLo, dirOffsetHi, firstByte, secondByte, thirdByte;
   unsigned short /* dirFilePos, */ dirStart, dirEnd, dirLength;
   long volFilePos, filelength;
   resHeaderType resHeader;
   char dirName[13];
	int fileNum;

   loadGameSig(gameSig);
   if (gameSig == NULL) {
      printf("Could not locate version 3 game files!\n");
      printf("Make sure you run XV3 in a directory\n");
      printf("containing a game which uses AGI v3.\n");
      exit(0);
   }

   sprintf(dirFileName, "%s%s", gameSig, "dir");
   if ((dirFile = fopen(dirFileName, "rb")) == NULL) {
      printf("Error opening file : %s\n", dirFileName);
      exit(0);
   }

   strcpy(dirName, fileNames[agiFileType]);
   dirName[strlen(fileNames[agiFileType]) - 1] = 0;
	printf( "Extracting %s files... ", dirName);
	mkdir(dirName, 0755);
	filelength = getLength(dirFile);

   fseek(dirFile, (agiFileType * 2), SEEK_SET);
   dirOffsetLo = fgetc(dirFile);
   dirOffsetHi = fgetc(dirFile);
   dirStart = (dirOffsetHi << 8) + dirOffsetLo;
   if (agiFileType < 3) {
      fseek(dirFile, ((agiFileType + 1) * 2), SEEK_SET);
      dirOffsetLo = fgetc(dirFile);
      dirOffsetHi = fgetc(dirFile);
      dirEnd = (dirOffsetHi << 8) + dirOffsetLo;
   }
   else {
      dirEnd = filelength;
   }
   dirLength = (dirEnd - dirStart);

   initLZW();

	for( fileNum = 0; fileNum*3 < dirLength; fileNum++ ) {
      fseek(dirFile, (agiFileType * 2), SEEK_SET);
      dirOffsetLo = fgetc(dirFile);
      dirOffsetHi = fgetc(dirFile);
      fseek(dirFile, ((dirOffsetHi << 8) + dirOffsetLo) + (fileNum*3), SEEK_SET);
      firstByte = fgetc(dirFile);
      secondByte = fgetc(dirFile);
      thirdByte = fgetc(dirFile);

      if ((firstByte == 0xFF) && (secondByte == 0xFF) && (thirdByte == 0xFF))
         continue;  /* Only attempt to extract resources that are there */

      volFilePos = ((firstByte & 0x0F) * (unsigned long)0x10000)
         + (secondByte * (unsigned long)0x100) + thirdByte;

      sprintf( volFileName, "%svol.%d", gameSig, (firstByte & 0xF0) >> 4 );

      if ((volFile = fopen(volFileName, "rb")) == NULL) {
   	   printf("Error!\nCannot open %s.", volFileName);
   	   exit(1);
      }

      fseek(volFile, volFilePos, SEEK_SET);
      fread(&resHeader.sig, sizeof(unsigned short), 1, volFile);
      fread(&resHeader.compType, sizeof(unsigned char), 1, volFile);
      fread(&resHeader.decompSize, sizeof(unsigned short), 2, volFile);

      /* Skip resources that have corrupt DIR entries */
      if (resHeader.sig != 0x3412) {
         printf("\nWarning: %s.%d was not extracted because of an incorrect DIR entry.",
            dirName, fileNum);
         continue;
      }

      fread(sierraBuf, sizeof(char), resHeader.fLen, volFile);
      fclose(volFile);

      /********************** DECOMPRESS TEMP FILE ******************/

      resetLZW();

      sprintf(dumpFileName, "%s/%s%d", dirName, fileNames[agiFileType], fileNum);
      if ((dumpFile = fopen(dumpFileName, "wb")) == NULL) {
         printf("Error opening dump file : %s\n", dumpFileName);
         exit(1);
      }

      /* AGI interpreter knows that it is not compressed if either the
         compressed size is equal to the uncompressed size, or the resource is
         specifically marked as being a PICTURE resource */
      if ((resHeader.compType & 0x80) || (resHeader.decompSize == resHeader.fLen)) {
         if (V2CONV) dumpAGIv2PIC(sierraBuf, resHeader.fLen, dumpFile);
         else fwrite(sierraBuf, sizeof(char), resHeader.fLen, dumpFile);
      }
      else {
         expand(sierraBuf, outputBuf, resHeader.decompSize);
         if (V2CONV) dumpAGIv2LOG(outputBuf, resHeader.decompSize, dumpFile);
         else fwrite(outputBuf, sizeof(char), resHeader.decompSize, dumpFile);
      }

      fclose(dumpFile);
   }

   closeLZW();
   fclose(dirFile);
   printf("Done.\n");
}

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

void extractComplete()
{
	char i;

	printf( "Please wait while extracting...\n" );

	/* extract all LOGIC, VIEW, PIC and SOUND files */
	for( i = 0; i <= 3; i++ ) extractAll(i);

	/* en-/decrypt OBJECT file */
	extractObjFile();

	/* convert WORDS.TOK to text */
	extractWordsTok();

	printf( "\nAll done!\n" );
}

#define  NORMAL     0
#define  ALTERNATE  1

/***************************************************************************
** dumpAGIv2PIC
**
** Purpose: To take the picture data contained in picBuf, and convert it
** to AGIv2 while it is being dumped to the file dumpFile.
***************************************************************************/
void dumpAGIv2PIC(unsigned char *picBuf, int picLen, FILE *v2File)
{
   unsigned char data, oldData = 0, outData;
   int mode = NORMAL, bufPos = 0;

   while (bufPos < picLen) {

      data = picBuf[bufPos++];

      if (mode == ALTERNATE)
	      outData = ((data & 0xF0) >> 4) + ((oldData & 0x0F) << 4);
      else
	      outData = data;

      if ((outData == 0xF0) || (outData == 0xF2)) {
	      fputc(outData, v2File);
	      if (mode == NORMAL) {
	         data = picBuf[bufPos++];
	         fputc((data & 0xF0) >> 4, v2File);
	         mode = ALTERNATE;
	      }
	      else {
	         fputc((data & 0x0F), v2File);
	         mode = NORMAL;
	      }
      }
      else
	      fputc(outData, v2File);

      oldData = data;
   }
}

/***************************************************************************
** dumpAGIv2LOG
**
** Purpose: To take the logic data contained in logBuf, and convert it
** to AGIv2 while it is being dumped to the file dumpFile.
***************************************************************************/
void dumpAGIv2LOG(unsigned char *logBuf, int logLen, FILE *dumpFile)
{
   int startPos, endPos, i, avisPos=0, numMessages;
   unsigned char *fileData, AVIS_DURGAN[] = "Avis Durgan";

   /* Find the start and end of the message section */
   fileData = logBuf;
   startPos = *fileData + (*(fileData+1))*256 + 2;
   numMessages = fileData[startPos];
   endPos = fileData[startPos+1] + fileData[startPos+2]*256;
   fileData += (startPos + 3);
   startPos = (numMessages * 2) + 0;

   /* Encrypt the message section so that it compiles with AGIv2 */
   for (i=startPos; i<endPos; i++)
	   fileData[i] ^= AVIS_DURGAN[avisPos++ % 11];

   /* Dump the result to the given file */
   fwrite(logBuf, sizeof(char), logLen, dumpFile);
}

int main(int argc, char **argv)
{
	int fileNum;

	printf( "XV3 v1.1 - AGIv3 resource extractor\n\n" );

	switch( argc ) {
		case 0:
		case 1:
			printf("Usage: xv3 -l[2] logicnumber\n");
			printf("       xv3 -s soundnumber\n");
			printf("       xv3 -v viewnumber\n");
			printf("       xv3 -p[2] picturenumber\n");
			printf("       xv3 -o\n");
			printf("       xv3 -w\n");
			printf("       xv3 -a\n\n");
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
					printf( "Invalid option: %s\n", argv[1] );
			}
			break;
		case 3:
			if (argv[1][0] == '-') {
				fileNum = atoi(argv[2]);

            /* This recognises the request to convert a resource to AGIv2
            ** before dumping. */
            if (argv[1][2] != 0) {
               if (argv[1][2] == '2')
                  V2CONV = 1;
               else {
                  printf("Invalid option : %s\n", argv[1]);
                  break;
               }
            }

				if ((fileNum >= 0) && strcmp( argv[2], "ALL" )) {
					switch (argv[1][1]) {
						case 'l': extractFile(0, fileNum);
							break;
						case 's': V2CONV = 0; extractFile(3, fileNum);
							break;
						case 'v': V2CONV = 0; extractFile(2, fileNum);
							break;
						case 'p': extractFile(1, fileNum);
							break;
						default: printf("Invalid option : %s\n", argv[1]);
					}
				}
				else {
					if( !strcmp( argv[2], "ALL" ) )
						switch( argv[1][1] ) {
							case 'l': extractAll(0);
								break;
							case 's': V2CONV = 0; extractAll(3);
								break;
							case 'v': V2CONV = 0; extractAll(2);
								break;
							case 'p': extractAll(1);
								break;
							default: printf("Invalid option : %s\n", argv[1]);
						}
					else
						printf("File number must be a postive number\n");
				}
			}
			else {
				printf("Invalid option : %s\n", argv[1]);
			}
	}

	return 0;
}

