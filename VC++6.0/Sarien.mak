# Microsoft Developer Studio Generated NMAKE File, Based on Sarien.dsp
!IF "$(CFG)" == ""
CFG=Sarien - Win32 Debug
!MESSAGE No configuration specified. Defaulting to Sarien - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Sarien - Win32 Release" && "$(CFG)" != "Sarien - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Sarien.mak" CFG="Sarien - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Sarien - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "Sarien - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "Sarien - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\Sarien.exe"


CLEAN :
	-@erase "$(INTDIR)\agi.obj"
	-@erase "$(INTDIR)\agi_v2.obj"
	-@erase "$(INTDIR)\agi_v3.obj"
	-@erase "$(INTDIR)\cli.obj"
	-@erase "$(INTDIR)\console.obj"
	-@erase "$(INTDIR)\cycle.obj"
	-@erase "$(INTDIR)\fileglob.obj"
	-@erase "$(INTDIR)\font.obj"
	-@erase "$(INTDIR)\getopt.obj"
	-@erase "$(INTDIR)\getopt1.obj"
	-@erase "$(INTDIR)\gfx.obj"
	-@erase "$(INTDIR)\global.obj"
	-@erase "$(INTDIR)\id.obj"
	-@erase "$(INTDIR)\keyboard.obj"
	-@erase "$(INTDIR)\logic.obj"
	-@erase "$(INTDIR)\lzw.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\menu.obj"
	-@erase "$(INTDIR)\objects.obj"
	-@erase "$(INTDIR)\op_cmd.obj"
	-@erase "$(INTDIR)\op_dbg.obj"
	-@erase "$(INTDIR)\op_misc.obj"
	-@erase "$(INTDIR)\op_test.obj"
	-@erase "$(INTDIR)\picture.obj"
	-@erase "$(INTDIR)\rand.obj"
	-@erase "$(INTDIR)\savegame.obj"
	-@erase "$(INTDIR)\silent.obj"
	-@erase "$(INTDIR)\sound.obj"
	-@erase "$(INTDIR)\sound_win32.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\view.obj"
	-@erase "$(INTDIR)\win32.obj"
	-@erase "$(INTDIR)\words.obj"
	-@erase "$(OUTDIR)\Sarien.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\Sarien.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Sarien.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\Sarien.pdb" /machine:I386 /out:"$(OUTDIR)\Sarien.exe" 
LINK32_OBJS= \
	"$(INTDIR)\agi.obj" \
	"$(INTDIR)\agi_v2.obj" \
	"$(INTDIR)\agi_v3.obj" \
	"$(INTDIR)\cli.obj" \
	"$(INTDIR)\console.obj" \
	"$(INTDIR)\cycle.obj" \
	"$(INTDIR)\font.obj" \
	"$(INTDIR)\getopt.obj" \
	"$(INTDIR)\getopt1.obj" \
	"$(INTDIR)\gfx.obj" \
	"$(INTDIR)\global.obj" \
	"$(INTDIR)\id.obj" \
	"$(INTDIR)\keyboard.obj" \
	"$(INTDIR)\logic.obj" \
	"$(INTDIR)\lzw.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\menu.obj" \
	"$(INTDIR)\objects.obj" \
	"$(INTDIR)\op_cmd.obj" \
	"$(INTDIR)\op_dbg.obj" \
	"$(INTDIR)\op_misc.obj" \
	"$(INTDIR)\op_test.obj" \
	"$(INTDIR)\picture.obj" \
	"$(INTDIR)\rand.obj" \
	"$(INTDIR)\savegame.obj" \
	"$(INTDIR)\silent.obj" \
	"$(INTDIR)\sound.obj" \
	"$(INTDIR)\view.obj" \
	"$(INTDIR)\words.obj" \
	"$(INTDIR)\win32.obj" \
	"$(INTDIR)\sound_win32.obj" \
	"$(INTDIR)\fileglob.obj"

"$(OUTDIR)\Sarien.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Sarien - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\Sarien.exe"


CLEAN :
	-@erase "$(INTDIR)\agi.obj"
	-@erase "$(INTDIR)\agi_v2.obj"
	-@erase "$(INTDIR)\agi_v3.obj"
	-@erase "$(INTDIR)\cli.obj"
	-@erase "$(INTDIR)\console.obj"
	-@erase "$(INTDIR)\cycle.obj"
	-@erase "$(INTDIR)\fileglob.obj"
	-@erase "$(INTDIR)\font.obj"
	-@erase "$(INTDIR)\getopt.obj"
	-@erase "$(INTDIR)\getopt1.obj"
	-@erase "$(INTDIR)\gfx.obj"
	-@erase "$(INTDIR)\global.obj"
	-@erase "$(INTDIR)\id.obj"
	-@erase "$(INTDIR)\keyboard.obj"
	-@erase "$(INTDIR)\logic.obj"
	-@erase "$(INTDIR)\lzw.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\menu.obj"
	-@erase "$(INTDIR)\objects.obj"
	-@erase "$(INTDIR)\op_cmd.obj"
	-@erase "$(INTDIR)\op_dbg.obj"
	-@erase "$(INTDIR)\op_misc.obj"
	-@erase "$(INTDIR)\op_test.obj"
	-@erase "$(INTDIR)\picture.obj"
	-@erase "$(INTDIR)\rand.obj"
	-@erase "$(INTDIR)\savegame.obj"
	-@erase "$(INTDIR)\silent.obj"
	-@erase "$(INTDIR)\sound.obj"
	-@erase "$(INTDIR)\sound_win32.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\view.obj"
	-@erase "$(INTDIR)\win32.obj"
	-@erase "$(INTDIR)\words.obj"
	-@erase "$(OUTDIR)\Sarien.exe"
	-@erase "$(OUTDIR)\Sarien.ilk"
	-@erase "$(OUTDIR)\Sarien.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /Gm /ZI /Od /I "..\SRC\INCLUDE" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "NATIVE_WIN32" /D "STACK_PUSHPOP" /D "NO_DEBUG" /Fp"$(INTDIR)\Sarien.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Sarien.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winmm.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\Sarien.pdb" /debug /machine:I386 /out:"$(OUTDIR)\Sarien.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\agi.obj" \
	"$(INTDIR)\agi_v2.obj" \
	"$(INTDIR)\agi_v3.obj" \
	"$(INTDIR)\cli.obj" \
	"$(INTDIR)\console.obj" \
	"$(INTDIR)\cycle.obj" \
	"$(INTDIR)\font.obj" \
	"$(INTDIR)\getopt.obj" \
	"$(INTDIR)\getopt1.obj" \
	"$(INTDIR)\gfx.obj" \
	"$(INTDIR)\global.obj" \
	"$(INTDIR)\id.obj" \
	"$(INTDIR)\keyboard.obj" \
	"$(INTDIR)\logic.obj" \
	"$(INTDIR)\lzw.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\menu.obj" \
	"$(INTDIR)\objects.obj" \
	"$(INTDIR)\op_cmd.obj" \
	"$(INTDIR)\op_dbg.obj" \
	"$(INTDIR)\op_misc.obj" \
	"$(INTDIR)\op_test.obj" \
	"$(INTDIR)\picture.obj" \
	"$(INTDIR)\rand.obj" \
	"$(INTDIR)\savegame.obj" \
	"$(INTDIR)\silent.obj" \
	"$(INTDIR)\sound.obj" \
	"$(INTDIR)\view.obj" \
	"$(INTDIR)\words.obj" \
	"$(INTDIR)\win32.obj" \
	"$(INTDIR)\sound_win32.obj" \
	"$(INTDIR)\fileglob.obj"

"$(OUTDIR)\Sarien.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("Sarien.dep")
!INCLUDE "Sarien.dep"
!ELSE 
!MESSAGE Warning: cannot find "Sarien.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "Sarien - Win32 Release" || "$(CFG)" == "Sarien - Win32 Debug"
SOURCE=..\src\core\agi.c

"$(INTDIR)\agi.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\core\agi_v2.c

"$(INTDIR)\agi_v2.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\core\agi_v3.c

"$(INTDIR)\agi_v3.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\core\cli.c

"$(INTDIR)\cli.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\core\console.c

"$(INTDIR)\console.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\core\cycle.c

"$(INTDIR)\cycle.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\core\font.c

"$(INTDIR)\font.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\core\getopt.c

"$(INTDIR)\getopt.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\core\getopt1.c

"$(INTDIR)\getopt1.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\core\gfx.c

"$(INTDIR)\gfx.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\core\global.c

"$(INTDIR)\global.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\core\id.c

"$(INTDIR)\id.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\core\keyboard.c

"$(INTDIR)\keyboard.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\core\logic.c

"$(INTDIR)\logic.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\core\lzw.c

"$(INTDIR)\lzw.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\core\main.c

"$(INTDIR)\main.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\core\menu.c

"$(INTDIR)\menu.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\core\objects.c

"$(INTDIR)\objects.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\core\op_cmd.c

"$(INTDIR)\op_cmd.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\core\op_dbg.c

"$(INTDIR)\op_dbg.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\core\op_misc.c

"$(INTDIR)\op_misc.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\core\op_test.c

"$(INTDIR)\op_test.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\core\picture.c

"$(INTDIR)\picture.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\core\rand.c

"$(INTDIR)\rand.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\core\savegame.c

"$(INTDIR)\savegame.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\core\silent.c

"$(INTDIR)\silent.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\core\sound.c

"$(INTDIR)\sound.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\core\view.c

"$(INTDIR)\view.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\core\words.c

"$(INTDIR)\words.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\graphics\win32\win32.c

"$(INTDIR)\win32.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\sound\win32\sound_win32.c

"$(INTDIR)\sound_win32.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\fileglob\win32\fileglob.c

"$(INTDIR)\fileglob.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

