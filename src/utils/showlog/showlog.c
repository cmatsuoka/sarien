/**************************************************************************
** showlog.c
**
** A complex program to display the text representation of AGI LOGIC files.
**
** Written by Lance Ewing
** Unix port by Claudio Matsuoka <claudio@helllabs.org> (19990315)
**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include  "agicodes.h"

#define  IF      0xFF
#define  ELSE    0xFE
#define  NOT     0xFD
#define  OR      0xFC

#define  ANDMODE 0
#define  ORMODE  1

#define  INDENT  "  "

#define  TRUE    1
#define  FALSE   0

#define  AVIS_DURGAN  "Avis Durgan"

typedef unsigned char byte;
typedef unsigned int word;
typedef char boolean;

/* There is no longer any difference between AGIv2 and AGIv3 LOGIC files, so
** there isn't really a need for the ENCRYPTED flag. */
boolean ENCRYPTED = TRUE;

boolean DEBUG = FALSE;
byte numMessages = 0;
char *messages[1000];

word logPos = 0;
char *logData[2000];

int numObjects = 0;
char *objects[1000];

int numWords = 0;
char *words[1000];

int numGotos = 0;
word gotos[1000];

int bracketCount = 30000;
int numBracketsOpen = 0;

int count=0;


/* BRACKET STACK DECLARATIONS */

#define MAX 1000
int stack[MAX];
int tos=0;		/* top of stack */

void addToLine(char *stringToAdd);
void showBracketInfo();

void push(int i)
{
   if (tos>=MAX) {
      printf("Bracket Stack Full\n");
      exit(0);
   }
   stack[tos] = i;
   tos++;
}

int pop()
{
   tos--;
   if (tos<0) {
      printf("Bracket Stack Underflow\n");
      addToLine("ERROR HERE->> ");
      tos++;
      numBracketsOpen++;
   }

   return stack[tos];
}

int topCount()
{
   int retVal, i=1, nb;


   nb = numBracketsOpen;

   if (nb == 0) return (stack[tos]);

   do {
      retVal = stack[tos-i];
      nb--;
      i++;
   } while ((retVal <= 0) && (nb > 0));


   return (retVal);
}

/**************************************************************************
** getLength
**
** Returns the length of the file given.
**************************************************************************/
long getLength(FILE *file)
{
   long tmp;

   fseek(file, 0L, SEEK_END);
   tmp = ftell(file);
   fseek(file, 0L, SEEK_SET);

   return(tmp);
}

/**************************************************************************
** freeObjects
**
** Deallocate all memory used for object name storage.
**************************************************************************/
void freeObjects()
{
   int objNum;

   for (objNum=0; objNum < numObjects; objNum++)
      free(objects[objNum]);
}

/**************************************************************************
** freeWords
**
** Deallocate all memory used for word storage.
**************************************************************************/
void freeWords()
{
   int wordNum;

   for (wordNum=0; wordNum < numWords; wordNum++)
      free(words[wordNum]);
}

/**************************************************************************
** freeMessages
**
** Deallocate all memory used for message storage.
**************************************************************************/
void freeMessages()
{
   int messNum;

   for (messNum=0; messNum < numMessages; messNum++)
      free(messages[messNum]);
}

/**************************************************************************
** loadWords
**
** Load words in from the WORDS.TOK file.
**************************************************************************/
void loadWords()
{
   FILE *wordFile;
   byte data, wordPos, newWord[80], temp[80];
   long startPos;
   word wordNum, i;

   if ((wordFile = fopen("words.tok","rb")) == NULL) {
      printf("Cannot find file : words.tok\n");
      exit(1);
   }

   startPos = (byte)fgetc(wordFile) * (long)256 + (byte)fgetc(wordFile);
   fseek(wordFile, startPos, SEEK_SET);

   for (i=0; i<=numWords; i++) words[i] = strdup("");

   while (!feof(wordFile)) {
      data = fgetc(wordFile);
      wordPos = data;
      do {
	data = fgetc(wordFile);
	newWord[wordPos++] = ((data ^ 0x7F) & 0x7F);
      } while (data < 0x80);
      newWord[wordPos] = 0;
      wordNum = (byte)fgetc(wordFile)*256 + (byte)fgetc(wordFile);
      /*if (wordNum == 9999) wordNum */
      if (wordNum >= 9999)
        break;
      strcpy(temp, "");
      if (strcmp(words[wordNum], "") != 0) {   /* if word is a synonym   */
	 /* strcat(temp, words[wordNum]); */   /* add it to the existing */
	 /* free(words[wordNum]); */           /* list.                  */
	 /* strcat(temp, "||");   */
      }
      else {
	 strcat(temp, newWord);
	 words[wordNum] = strdup(temp);
      }
   }

   fclose(wordFile);
}

/**************************************************************************
** calcNumWords
**
** Calculate the number of different words (not including synonyms) which
** are contained in the WORDS.TOK file.
**************************************************************************/
int calcNumWords()
{
   FILE *wordFile;
   byte data;
   long startPos;
   int max = 0, wordNum;

   if ((wordFile = fopen("words.tok","rb")) == NULL) {
      printf("Cannot find file : word.tok\n");
      exit(1);
   }

   startPos = (byte)fgetc(wordFile) * (long)256 + (byte)fgetc(wordFile);
   fseek(wordFile, startPos, SEEK_SET);

   while (!feof(wordFile)) {
      data = fgetc(wordFile);
      if (data > 0x80) {
	 wordNum = (byte)fgetc(wordFile) * 256 + (byte)fgetc(wordFile);
	 if ((wordNum > max) && (wordNum < 9999)) max = wordNum;
      }
   }

   fclose(wordFile);
   return (max);
}

/**************************************************************************
** isObjCrypt
**
** Checks whether the OBJECT file is encrypted with Avis Durgan or not.
**************************************************************************/
boolean isObjCrypt(long fileLen, byte *objData)
{
   int i, checkLen;

   checkLen = ((fileLen < 20)? 10 : 20);

   /* Special empty OBJECT file case */
   if (fileLen == 8) {
      if ((objData[0] == 0x42) && (objData[1] == 0x76)) return TRUE;
      return FALSE;
   }

   for (i=fileLen-1; i>(fileLen - checkLen); i--) {
      if (((objData[i] < 0x20) || (objData[i] > 0x7F)) && (objData[i] != 0))
	 return TRUE;
   }

   return FALSE;
}

/**************************************************************************
** loadObjectNames
**
** Load the names of the inventory items from the OBJECT file.
**************************************************************************/
void loadObjectNames()
{
   FILE *objFile;
   int avisPos=0, objNum, i, strPos = 0;
   long fileLen;
   byte *objData, *marker, tempString[80];
   word index;
   boolean OBJ_ENCRYPTED = TRUE;

   if ((objFile = fopen("object","rb")) == NULL) {
      printf("Cannot find file : object\n");
      exit(1);
   }

   fileLen = getLength(objFile);
   marker = (objData = (char *)malloc(fileLen)) + 3;
   fread(objData, 1, fileLen, objFile);
   fclose(objFile);

   OBJ_ENCRYPTED = isObjCrypt(fileLen, objData);

   if (OBJ_ENCRYPTED)
      for (i=0; i<fileLen; i++) objData[i] ^= AVIS_DURGAN[avisPos++ % 11];

   numObjects = (((objData[1] * 256) + objData[0]) / 3);

   for (objNum=0; objNum<numObjects; objNum++, strPos=0, marker+=3) {
      index = *(marker) + 256*(*(marker+1)) + 3;
      while ((tempString[strPos++] = objData[index++]) != 0) {}
      objects[objNum] = strdup(tempString);
   }

   free(objData);
}


/**************************************************************************
** loadMessages
**
** Loads up all the messages from the logic file.
**************************************************************************/
void loadMessages(byte *fileData)
{
   word startPos, endPos, i, messNum, avisPos=0, strPos=0;
   int index;
   byte *marker, tempString[500];

   startPos = *fileData + (*(fileData+1))*256 + 2;
   numMessages = fileData[startPos];
   endPos = fileData[startPos+1] + fileData[startPos+2]*256;
   fileData += (startPos + 3);

   startPos = (numMessages * 2) + 0;

   if (ENCRYPTED)
      for (i=startPos; i<endPos; i++)
	 fileData[i] ^= AVIS_DURGAN[avisPos++ % 11];

   marker = fileData;

   for (messNum=0; messNum<numMessages; messNum++, strPos=0, marker+=2) {
      index = *(marker) + 256*(*(marker+1)) - 2;
      if (index < 0) {
	tempString[0] = 0;
      }
      else {
	 while ((tempString[strPos++] = fileData[index++]) != 0) {}
      }
      messages[messNum] = strdup(tempString);
   }

}

/***********************************************
** Section to read the AGI interpreter version.
***********************************************/

#define   v2_089       0
#define   v2_272       1
#define   v2_411       2
#define   v2_425       3
#define   v2_426       4
#define   v2_435       5
#define   v2_439       6
#define   v2_440       7
#define   v2_915       8
#define   v2_917       9
#define   v2_936      10
#define   v3_000_086  11
#define   v3_000_098  12
#define   v3_000_102  13
#define   v3_000_107  14
#define   v3_000_148  15
#define   NOTKNOWN    16

typedef struct { char version[15]; int verNum; } verType;

const verType verData[16] = {
   { "2.089", v2_089 },
   { "2.272", v2_272 },
   { "2.411", v2_411 },
   { "2.425", v2_425 },
   { "2.426", v2_426 },
   { "2.435", v2_435 },
   { "2.439", v2_439 },
   { "2.440", v2_440 },
   { "2.915", v2_915 },
   { "2.917", v2_917 },
   { "2.936", v2_936 },
   { "3.000.086", v3_000_086 },
   { "3.000.098", v3_000_098 },
   { "3.000.102", v3_000_102 },
   { "3.000.107", v3_000_107 },
   { "3.000.148", v3_000_148 }
};

int loadVersion()
{
   FILE *dataFile;
   char searchString[] = "Adventure Game Interpreter\x0A      Version ";
   int strPos = 0, i, retVal = NOTKNOWN;
   unsigned char aChar;
   char ver[15], found = 0;

   if ((dataFile = fopen("agidata.ovl", "rb")) == NULL) {
      printf("Error opening agidata.ovl\n");
      exit(1);
   }

   while (!feof(dataFile)) {
      aChar = fgetc(dataFile);

      if (found) {
	 ver[strPos++] = aChar;
	 if (aChar == 0) break;
      }
      else {
	 if (aChar == searchString[strPos]) {
	    strPos++;
	    if (strPos == 41) {
	       found = 1;
	       strPos = 0;
	    }
	 }
	 else {
	    strPos = 0;
	    if (aChar == searchString[strPos]) strPos++;
	 }
      }
   }

   for (i=0; i<16; i++) {
      if ((strcmp(ver, verData[i].version)) == 0) retVal = verData[i].verNum;
   }

   fclose(dataFile);

   return (retVal);
}

/**************************************************************************
** adjustCommandData
**
** This function makes adjustments to the array contained in AGICODES.H
** depending on what the AGI Interpreter Version is.
**************************************************************************/
void adjustCommandData()
{
   int verID;

   verID = loadVersion();

   switch (verID) {
      case v2_089:

	 /* Adjust quit() */

	 agiCommands[134].numArgs = 0;

      case v2_272:
      case v2_411:
      case v2_425:
      case v2_426:
      case v2_435:
      case v2_439:
      case v2_440:

	 /* Adjust print.at and print.at.v */

	 agiCommands[151].numArgs = 3;
	 agiCommands[152].numArgs = 3;
	 break;

      case v3_000_086:

	 /* Adjust unknown command */

	 agiCommands[176].numArgs = 1;
	 break;
   }
}


/**************************************************************************
** initScreen
**************************************************************************/
void initScreen()
{
#if 0
   union REGS regs;

   regs.h.ah = 0;
   regs.h.al = 3;
   int86(16, &regs, &regs);   /* Change to 80x25 text mode */

   regs.h.ah = 0x11;	      /* Set to 80x50 */
   regs.h.al = 0x12;
   regs.h.bl = 0x00;
   int86(16, &regs, &regs);
#endif
}

/**************************************************************************
** closeScreen
**************************************************************************/
void closeScreen()
{
#if 0
   union REGS regs;

   regs.h.ah = 0;
   regs.h.al = 3;
   int86(16, &regs, &regs);   /* Change to 80x25 text mode */
#endif
}


word lineArray[6000];
int topLine=0;
char currentLine[500];
char debugLine[500], debugChar[15];
boolean DONTADD = FALSE;

void addToLine(char *stringToAdd)
{
   strcat(currentLine, stringToAdd);
}

FILE *dumpFile;

void newLine(int lineStartPos)
{
   //showBracketInfo();

   if (topLine == 0)
      lineArray[topLine] = 0;

   topLine++;
   lineArray[topLine] = lineStartPos+1;
   if (DEBUG) {
      if (!DONTADD) {
	 strcat(debugLine, debugChar);
	 strcpy(debugChar, "");
      }
      if (strspn (debugLine, " ") == strlen (debugLine))
         fprintf (dumpFile, "%s\n", currentLine);
      else
         fprintf(dumpFile, "%s\t\t/* %s */\n", currentLine, debugLine);
      strcpy(debugLine, "");
   }
   else {
      fprintf(dumpFile, "%s\n", currentLine);
   }
   strcpy(currentLine, "");
}

/**************************************************************************
** indent
**************************************************************************/
void indent()
{
   int i;

   for (i=0; i<=numBracketsOpen; i++) {
      addToLine(INDENT);
   }
}

/**************************************************************************
** getData
**
** Returns the next byte from the logic file. Also decrements the current
** bracket count and if it is decremented to zero, print a close bracket,
** pop the next value off the stack and set the bracket count to the new
** value.
**************************************************************************/
byte getData(byte **data)
{
   byte retVal;

   retVal = *(*data)++;
   if (DEBUG) {
      if (retVal == 0xFF) DONTADD = TRUE; else DONTADD = FALSE;
      strcat(debugLine, debugChar);
      sprintf(debugChar, "%02X ", retVal);
   }
   bracketCount--;
   if (bracketCount <= 0) {
      if (numBracketsOpen > 0) {
	 do {
	   numBracketsOpen--;
	   indent();
	   addToLine("}");
	   if (DEBUG) DONTADD = TRUE;
	   newLine(logPos-2);
	 } while (((bracketCount = pop()) <= 0) && (numBracketsOpen > 0));
      }
      else
	 bracketCount = bracketCount;

      if (retVal != 0xFF) newLine(logPos-2);
   }

   logPos++;
   return retVal;
}

/**************************************************************************
** showBracketInfo
**************************************************************************/
void showBracketInfo()
{
   int i;
   char tempString[80];

   sprintf(tempString, "[%d : ", numBracketsOpen);
   addToLine(tempString);
   for (i=0; i<numBracketsOpen; i++) {
      sprintf(tempString, "%d ", stack[i]);
      addToLine(tempString);
   }

   sprintf(tempString, "] %d", bracketCount);
   addToLine(tempString);
   //newLine(logPos-2);
}

int varCount[256];

/**************************************************************************
** printTestArg
**************************************************************************/
void printTestArg(byte cmd, byte arg, byte value)
{
   char tempString[80];

   if (testCommands[cmd].argTypeMask & (0x80 >> arg)) {
      sprintf(tempString, "v%d", value);
      varCount[value]++;
   }
   else {
      sprintf(tempString, "%d", value);
   }
   addToLine(tempString);
}

/**************************************************************************
** processIF
**************************************************************************/
void processIF(byte **data)
{
   int mode, param, numTests=0;
   byte cmd;
   boolean stillInTestMode = TRUE, notmode = FALSE;
   int oldCount, argNum, wordNum;
   /* char tempString[500]; */

   mode = ANDMODE;
   newLine(logPos-2);
   indent();
   addToLine("if (");

   while (stillInTestMode) {

      switch (cmd = getData(data)) {
	 case 0xFF:
	    addToLine(") { ");
	    oldCount = bracketCount - 3;
	    bracketCount = (getData(data) + (getData(data) << 8)) + 1;
	    push((oldCount - bracketCount) + 1);
	    numBracketsOpen++;
	    stillInTestMode = FALSE;
	    newLine(logPos-2);
	    break;

	 case 0xFC:
	    if (mode == ANDMODE) {
	       if (numTests > 0) {
		  addToLine(" && ");
		  newLine(logPos-2);
		  indent();
		  addToLine("    ");
	       }
	       mode = ORMODE;
	       addToLine("(");
	       numTests = 0;
	    }
	    else {
	       mode = ANDMODE;
	       addToLine(")");
	    }
	    break;

	 case 0xFD:
	    notmode = TRUE;
	    break;

	 default:
	    if (numTests++ > 0) {
	       switch (mode) {
		  case ANDMODE:
		     addToLine(" && ");
		     newLine(logPos-2);
		     indent();
		     addToLine("    ");
		     break;
		  case ORMODE:
		     addToLine(" || ");
		     newLine(logPos-2);
		     indent();
		     addToLine("     ");
		     break;
	       }
	    }

	    switch (cmd) {
	       case 1:
	       case 2:
		  printTestArg(cmd, 0, getData(data));
		  if (notmode) {
		     addToLine(" != ");
		     notmode = FALSE;
		  }
		  else
		     addToLine(" == ");
		  printTestArg(cmd, 1, getData(data));
		  break;

	       case 3:
	       case 4:
		  if (notmode) addToLine("!(");
		  printTestArg(cmd, 0, getData(data));
		  addToLine(" < ");
		  printTestArg(cmd, 1, getData(data));
		  if (notmode) {
		     addToLine(")");
		     notmode = FALSE;
		  }
		  break;

	       case 5:
	       case 6:
		  if (notmode) addToLine("!(");
		  printTestArg(cmd, 0, getData(data));
		  addToLine(" > ");
		  printTestArg(cmd, 1, getData(data));
		  if (notmode) {
		     addToLine(")");
		     notmode = FALSE;
		  }
		  break;

	       case 9:
		  if (notmode)  {
		     addToLine("!");
		     notmode = FALSE;
		  }
		  addToLine(testCommands[cmd].commandName);
		  addToLine("(");
		  addToLine(objects[getData(data)]);
		  addToLine(")");
		  break;

	       case 14:
		  if (notmode)  {
		     addToLine("!");
		     notmode = FALSE;
		  }
		  addToLine(testCommands[cmd].commandName);
		  addToLine("(");
		  argNum = getData(data);
		  for (param=0; param<argNum; param++) {
		     if (param != 0) addToLine(", ");
		     wordNum = getData(data) + (getData(data) << 8);
		     if (wordNum == 9999) {
			addToLine("ROL");
		     }
		     else {
			addToLine(words[wordNum]);
		     }
		  }
		  addToLine(")");
		  break;

	       default:
		  if (notmode)  {
		     addToLine("!");
		     notmode = FALSE;
		  }
		  addToLine(testCommands[cmd].commandName);
		  addToLine("(");
		  argNum = testCommands[cmd].numArgs;

		  if (cmd > 18) {
		     printf("Error decoding LOGIC file: Unknown test code %d\n", cmd);
		  }
		  else {
		     for (param=0; param<argNum; param++) {
			if (param != 0) addToLine(", ");
			printTestArg(cmd, param, getData(data));
		     }
		  }
		  addToLine(")");
		  break;
	    }
	    break;
      }

   }

}


void displayMessage(char *string)
{
   int i, counter=0;
   char tempString[500];

   sprintf(tempString, "\"");
   addToLine(tempString);
   for (i=0; i<strlen(string); i++) {
      if ((counter++ > (50 - numBracketsOpen*2)) && (string[i] == ' ')) {
	 addToLine("\"");
	 newLine(logPos-2);
	 indent();
	 addToLine("      \"");
	 counter = 0;
      }
      if (string[i] == 0x0A) {
	 addToLine("\\n");
	 counter++;
      }
      else {
	 if (string[i] == '\"') {
	    addToLine("\\\"");
	    counter++;
	 }
	 else if (string[i] == '\\') {
	    addToLine("\\\\");
	    counter++;
	 }
	 else {
	    sprintf(tempString, "%c", string[i]);
	    addToLine(tempString);
	 }
      }
   }
}


int getLabelNum(word linePos)
{
   int i;

   for (i=0; i<numGotos; i++) {
      if (gotos[i] == linePos) return i;
   }

   gotos[numGotos++] = linePos;
   return (numGotos - 1);
}


/**************************************************************************
** clearVarCounts
**************************************************************************/
void clearVarCounts()
{
   int varNum;

   for (varNum=0; varNum<256; varNum++) varCount[varNum] = 0;
}

/**************************************************************************
** displayVarStats
**
** Purpose: To display the variables that are used by the LOGIC script
** and the number of times each of these variables is used.
**************************************************************************/
void displayVarStats()
{
   int varNum, numOfVarsUsed=0;
   char tempString[80];

   addToLine("/******************** Variable Usage Statistics ****************");
   newLine(logPos-2);
   newLine(logPos-2);
   addToLine("  Variable          # Times Used");
   newLine(logPos-2);
   newLine(logPos-2);

   for (varNum=0; varNum<256; varNum++) {
      if (varCount[varNum]) {
	 sprintf(tempString, "     v%-3d               %3d", varNum, varCount[varNum]);
	 addToLine(tempString);
	 newLine(logPos-2);
	 numOfVarsUsed++;
      }
   }

   newLine(logPos-2);
   sprintf(tempString, "  Number of variables used = %d", numOfVarsUsed);
   addToLine(tempString);

   newLine(logPos-2);
   newLine(logPos-2);
   addToLine("***************************************************************/");
   newLine(logPos-2);
}

/**************************************************************************
** displayMessages
**
** Purpose: To add the message list to end of the decoding logic script.
**          This is important in some cases where there are hidden messages
**          and messages need to be assigned certain numbers when
**          recompiled by a program such as MATS, AGIC, or AGI Studio.
**************************************************************************/
void displayMessages()
{
   int messNum, i, strPos;
   char tempString[500], messString[256];

   addToLine("/************************** Message List ****************************/");
   newLine(logPos-2);
   newLine(logPos-2);

   for (messNum=0; messNum<numMessages; messNum++) {
      for (i=0, strPos=0; i<=strlen(messages[messNum]); i++) {
	 switch (messages[messNum][i]) {
	    case '\n':
	       messString[strPos++] = '\\';
	       messString[strPos++] = 'n';
	       break;
	    case '\"':
	       messString[strPos++] = '\\';
	       messString[strPos++] = '"';
	       break;
	    case '\\':
	       messString[strPos++] = '\\';
	       messString[strPos++] = '\\';
	       break;
	    default:
	       messString[strPos++] = messages[messNum][i];
	       break;
	 }
      }

      sprintf(tempString, "#message %d \"%s\"", messNum+1, messString);
      addToLine(tempString);
      newLine(logPos-2);
   }

   newLine(logPos-2);
}

/**************************************************************************
** printArg
**************************************************************************/
void printArg(byte cmd, byte arg, byte value)
{
   char tempString[80];

   if (agiCommands[cmd].argTypeMask & (0x80 >> arg)) {
      sprintf(tempString, "v%d", value);
      varCount[value]++;
   }
   else {
      sprintf(tempString, "%d", value);
   }
   addToLine(tempString);
}


/**************************************************************************
** main
**************************************************************************/
int main(int argc, char **argv)
{
   FILE *logicFile, /* *comFile, */ *dummyFile = NULL;
   byte *fileData, *data, *endOfData, cmd, param, value;
   long progLength;
   int counter=1, oldCount, lineNum, i, opt, /* result, */ disp;
   char tempString[500], ch = 0;
   boolean STOP = FALSE, HALF = FALSE, BROWSE = FALSE, VARSTATS = FALSE;
   boolean MESSAGES = FALSE;

   if (argc < 2) {
      printf("SHOWLOG 1.2c\n\n");
      printf("Usage:   showlog [options] filename\n\n");
      printf("Options:\n\n");
#if 0
      printf("  -s   stop every ten lines of text.\n");
      printf("  -h   half height characters (50 lines).\n");
      printf("  -b   browse output using LIST (text viewer).\n");
#endif
      printf("  -r   display raw codes with each line (debugging).\n");
      printf("  -v   tags variable usage stats to the end of the output.\n");
      printf("  -m   add message list to end.\n");
      exit(0);
   }
   else {
      for (opt=1; opt!=(argc-1); opt++) {
	 if (argv[opt][0] == '-') {
	    switch(argv[opt][1]) {
#if 0
	       case 's': STOP = TRUE; break;
	       case 'h': HALF = TRUE; break;
	       case 'b': BROWSE = TRUE; break;
#endif
	       case 'r': DEBUG = TRUE; break;
	       case 'v': VARSTATS = TRUE; break;
	       case 'm': MESSAGES = TRUE; break;
	       default: printf("Illegal option : %s\n", argv[opt]); exit(0);
	    }
	 }
	 else {
	    printf("Illegal option : %s\n", argv[opt]);
	    exit(0);
	 }
      }
   }

   if ((logicFile = fopen(argv[argc-1], "rb")) == NULL) {
      printf("Error opening logic source file : %s\n", argv[argc-1]);
      exit(0);
   }

   if ((dumpFile = fopen("TEMPDUMP", "wt")) == NULL) {
      printf("Error opening temporary file : TEMPDUMP\n");
      exit(1);
   }

   if (HALF) initScreen();

   progLength = getLength(logicFile);
   fileData = (byte*)malloc(progLength+10);
   if (fileData == NULL) {
      printf("Error allocating memory!\n");
      exit(0);
   }
   fread(fileData, 1, progLength, logicFile);
   fclose(logicFile);

   numWords = calcNumWords();
   loadObjectNames();
   loadWords();
   loadMessages(fileData);

   data = fileData + 2;
   endOfData = data + (fileData[0] + fileData[1]*256);

   adjustCommandData();
   /* clrscr(); */

   addToLine("/********************** START OF SCRIPT ***********************/");
   newLine(logPos-2);

   do {

      switch (cmd = getData(&data)) {
	 case IF:
	    processIF(&data);
	    break;

	 case ELSE:
	    if (DEBUG) {
	       strcat(debugLine, debugChar);
	       sprintf(debugChar, "%02X %02X ", (*data), (*(data+1)));
	    }
	    if ((*(data-4) == 0xFF) && (*(data-3) == 0x03) && (*(data-2) == 0x00)) {
	       indent();
	       bracketCount = (*data) + ((*(data+1)) << 8) + 1;
	       sprintf(tempString, "goto Label%d;", getLabelNum((data - (fileData + 2)) + bracketCount));
	       addToLine(tempString);
	       newLine(logPos);
	       numBracketsOpen--;
	       bracketCount = pop() + 1;   /* This first one is different */
	       indent();
	       addToLine("}");

	       if ((bracketCount <= 0) && (numBracketsOpen > 0)) {
		  do {
		     bracketCount = pop();
		     numBracketsOpen--;
		     indent();
		     addToLine("}");
		     newLine(logPos);
		  } while ((bracketCount <= 0) && (numBracketsOpen > 0));
	       }
	    }
	    /* Next one could be applied to values other than 6 for bracket
	       count. Havn't seen any other examples though. */
	    else if ((*(data-4) == 0xFF) && (*(data-3) == 0x06) && (*(data-2) == 0x00)) {
	       disp = (*data) + ((*(data+1)) << 8) + 1;
	       indent();
	       sprintf(tempString, "goto Label%d;", getLabelNum((data - (fileData + 2)) + disp));
	       addToLine(tempString);
	       newLine(logPos);
	       indent();
	    }
	    else if ((numBracketsOpen == 0)) {
	       disp = (*data) + ((*(data+1)) << 8) + 1;
	       indent();
	       sprintf(tempString, "goto Label%d;", getLabelNum((data - (fileData + 2)) + disp));
	       addToLine(tempString);
	       newLine(logPos);
	       indent();
	    }
	    /*
	    ** If more than one bracket closes immediatedly after the ELSE
	    ** sequence, then it can't be an ELSE.
	    **
	    ** Needs to check that current bracket count is 3 though.  <<<---
	    */
	    else if ((stack[tos-1] == 0) && (bracketCount == 3)) {
	       disp = (*data) + ((*(data+1)) << 8) + 1;
	       indent();
	       sprintf(tempString, "goto Label%d;", getLabelNum((data - (fileData + 2)) + disp));
	       addToLine(tempString);
	       newLine(logPos);

	       //bracketCount = oldCount;
	       do {
		  bracketCount = pop();
		  numBracketsOpen--;
		  indent();
		  addToLine("}");
		  newLine(logPos-2);
	       } while ((bracketCount <= 0) && (numBracketsOpen > 0));
	       indent();
	    }
	    else {
	       oldCount = bracketCount;
	       bracketCount = (*data) + ((*(data+1)) << 8) + 1;
	       if (bracketCount > topCount() + 2) {
		  indent();
		  sprintf(tempString, "goto Label%d;", getLabelNum((data - (fileData + 2)) + bracketCount));
		  addToLine(tempString);
		  newLine(logPos);
		  bracketCount = oldCount;
		  do {
		     bracketCount = pop();
		     numBracketsOpen--;
		     indent();
		     addToLine("}");
		     newLine(logPos-2);
		  } while ((bracketCount <= 0) && (numBracketsOpen > 0));
		  indent();
	       }
	       else if (bracketCount < 0) {
		  indent();
		  sprintf(tempString, "goto Label%d;", getLabelNum((data - (fileData + 2)) + bracketCount));
		  addToLine(tempString);
		  newLine(logPos);
		  bracketCount = oldCount - 2;
		  /*
		  bracketCount = oldCount;
		  do {
		     bracketCount = pop();
		     numBracketsOpen--;
		     indent();
		     addToLine("}");
		     newLine(logPos-2);
		  } while ((bracketCount <= 0) && (numBracketsOpen > 0));
		  */

		  indent();

	       }
	       else {
		  do {
		     bracketCount = pop();
		     numBracketsOpen--;
		     indent();
		     addToLine("}");
		     newLine(logPos-2);
		  } while ((bracketCount <= 0) && (numBracketsOpen > 0));
		  indent();
		  oldCount = bracketCount;
		  bracketCount = (*data) + ((*(data+1)) << 8) + 1;

		  addToLine("else { ");
		  push((oldCount - bracketCount) + 1);  /* Only open bracket if its */
		  numBracketsOpen++;
	       }

	    }
	    data+=2;
	    logPos+=2;
	    newLine(logPos-2);
	    break;

	 case 0x00:  /* return */
	    indent();
	    addToLine("return();");
	    newLine(logPos - 2);
	    break;

	 default:
	    indent();

	    switch (cmd) {
	       case 1:
		  printArg(cmd, 0, getData(&data));
		  addToLine("++");
		  addToLine(";");
		  newLine(logPos - 2);
		  break;

	       case 2:
		  printArg(cmd, 0, getData(&data));
		  addToLine("--");
		  addToLine(";");
		  newLine(logPos - 2);
		  break;

	       case 3:
	       case 4:
		  printArg(cmd, 0, getData(&data));
		  addToLine(" = ");
		  printArg(cmd, 1, getData(&data));
		  addToLine(";");
		  newLine(logPos - 2);
		  break;

	       case 5:
	       case 6:
		  value = getData(&data);
		  printArg(cmd, 0, value);
		  addToLine(" = ");
		  printArg(cmd, 0, value);
		  addToLine(" + ");
		  printArg(cmd, 1, getData(&data));
		  addToLine(";");
		  newLine(logPos - 2);
		  break;

	       case 7:
	       case 8:
		  value = getData(&data);
		  printArg(cmd, 0, value);
		  addToLine(" = ");
		  printArg(cmd, 0, value);
		  addToLine(" - ");
		  printArg(cmd, 1, getData(&data));
		  addToLine(";");
		  newLine(logPos - 2);
		  break;

	       case 9:
	       case 11:
		  addToLine("*");
		  printArg(cmd, 0, getData(&data));
		  addToLine(" = ");
		  printArg(cmd, 1, getData(&data));
		  addToLine(";");
		  newLine(logPos - 2);
		  break;

	       case 10:
		  printArg(cmd, 0, getData(&data));
		  addToLine(" = ");
		  addToLine("*");
		  printArg(cmd, 1, getData(&data));
		  addToLine(";");
		  newLine(logPos - 2);
		  break;

	       case 165:
	       case 166:
		  value = getData(&data);
		  printArg(cmd, 0, value);
		  addToLine(" = ");
		  printArg(cmd, 0, value);
		  addToLine(" * ");
		  printArg(cmd, 1, getData(&data));
		  addToLine(";");
		  newLine(logPos - 2);
		  break;

	       case 167:
	       case 168:
		  value = getData(&data);
		  printArg(cmd, 0, value);
		  addToLine(" = ");
		  printArg(cmd, 0, value);
		  addToLine(" / ");
		  printArg(cmd, 1, getData(&data));
		  addToLine(";");
		  newLine(logPos - 2);
		  break;

	       case 114:  /* set.string & get.string */
	       case 115:
		  addToLine(agiCommands[cmd].commandName);
		  addToLine("(");
		  printArg(cmd, 0, getData(&data));
		  addToLine(", ");
		  displayMessage(messages[getData(&data)-1]);
		  addToLine("\"");
		  for (param=2; param<agiCommands[cmd].numArgs; param++) {
		     addToLine(", ");
		     printArg(cmd, param, getData(&data));
		  }
		  addToLine(");");
		  newLine(logPos - 2);
		  break;

	       case 103:  /* display */
		  addToLine(agiCommands[cmd].commandName);
		  addToLine("(");
		  printArg(cmd, 0, getData(&data));
		  addToLine(", ");
		  printArg(cmd, 1, getData(&data));
		  addToLine(", ");
		  displayMessage(messages[getData(&data)-1]);
		  addToLine("\");");
		  newLine(logPos - 2);
		  break;

	       case 108:
	       case 156:
	       case 157:
	       case 118:
	       case 144:
	       case 143:
	       case 151:
	       case 0x65:
		  addToLine(agiCommands[cmd].commandName);
		  addToLine("(");
		  i = getData(&data) - 1;
		  if (i >= numMessages) {
		     addToLine("\"");
		  }
		  else {
		     displayMessage(messages[i]);
		  }
		  addToLine("\"");
		  for (param=1; param<agiCommands[cmd].numArgs; param++) {
		     addToLine(", ");
		     printArg(cmd, param, getData(&data));
		  }
		  addToLine(");");
		  newLine(logPos - 2);
		  break;

	       default:
		  addToLine(agiCommands[cmd].commandName);
		  addToLine("(");
		  if (cmd >= 181) {
		     addToLine("Error decoding LOGIC file: Unknown code\n");
		  }
		  else {
		     for (param=0; param<agiCommands[cmd].numArgs; param++) {
			if (param != 0) addToLine(", ");
			printArg(cmd, param, getData(&data));
		     }
		     addToLine(");");
		     newLine(logPos-2);
		  }
		  break;
	    }
      }

   } while (data < endOfData);

   newLine(logPos-2);
   if (MESSAGES) displayMessages();
   newLine(logPos-2);

   addToLine("/*********************** END OF SCRIPT ************************/");
   newLine(logPos-2);
   newLine(logPos-2);


   if (VARSTATS) displayVarStats();

   fclose(dumpFile);


/**************************************************************************
** OUTPUT SECTION
**************************************************************************/

#if 0
   if (BROWSE) {

      /* Output browse array to a temporay file TEMP.COM */
      comFile = fopen("TEMP.COM", "wb");
      for (i=0; i<26063; i++) fputc(list[i], comFile);
      fclose(comFile);

      /* Open DUMMY.FIL to hold output for browse */
      dummyFile = fopen("DUMMY.FIL", "wb");

   }
#endif

   dumpFile = fopen("TEMPDUMP", "rt");
   lineNum = 0;

   while (!feof(dumpFile)) {
      fgets(tempString, 200, dumpFile);
      if (!feof(dumpFile)) {
	 for (i=0; i<numGotos; i++) {
	    if ((lineArray[lineNum] == gotos[i]) &&
		(lineArray[lineNum+1] != gotos[i])) {
	       if (BROWSE)
		  fprintf(dummyFile, "\nLabel%d:\n\n", i);
	       else
		  printf("\nLabel%d:\n\n", i);
	    }
	 }
	 if (BROWSE)
	    fprintf(dummyFile, "%s", tempString);
	 else
	    printf("%s", tempString);
	 lineNum++;
	 if (STOP && (counter++ % 10) == 0) ch = getchar();
	 if ((ch == 'q') || (ch == 27)) break;
      }
   }

   if (STOP) getchar();

   fclose(dumpFile);
   remove("TEMPDUMP");

#if 0
   if (BROWSE) {
      fclose(dummyFile);

      result = spawnl(P_WAIT, "TEMP.COM", "TEMP.COM", "DUMMY.FIL", NULL);

      remove("DUMMY.FIL");
      remove("TEMP.COM");
   }
#endif

   freeObjects();
   freeWords();
   freeMessages();
   free(fileData);

   if (HALF) closeScreen();

   return 0;
}
