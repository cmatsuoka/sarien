# Microsoft Developer Studio Project File - Name="Sarien" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=Sarien - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Sarien.mak".
!MESSAGE 
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

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Sarien - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O1 /I "..\src\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "NATIVE_WIN32" /FR /YX /FD /c
# ADD BASE RSC /l 0x416 /d "NDEBUG"
# ADD RSC /l 0x416 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib /nologo /subsystem:windows /debug /machine:I386 /FIXED:NO
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "Sarien - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /Zi /Od /I "..\SRC\INCLUDE" /D "DEBUG" /D "WIN32" /D "_WINDOWS" /D "NATIVE_WIN32" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x416 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winmm.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /incremental:no /debug /machine:I386 /pdbtype:sept /FIXED:NO
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "Sarien - Win32 Release"
# Name "Sarien - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "CORE_AGI"

# PROP Default_Filter "*.C"
# Begin Source File

SOURCE=..\src\core\agi.c
# End Source File
# Begin Source File

SOURCE=..\src\core\agi_v2.c
# End Source File
# Begin Source File

SOURCE=..\src\core\agi_v3.c
# End Source File
# Begin Source File

SOURCE=..\SRC\CORE\AGI_V4.C
# End Source File
# Begin Source File

SOURCE=..\src\core\checks.c
# End Source File
# Begin Source File

SOURCE=..\src\core\console.c
# End Source File
# Begin Source File

SOURCE=..\src\core\cycle.c
# End Source File
# Begin Source File

SOURCE=..\src\core\font.c
# End Source File
# Begin Source File

SOURCE=..\src\core\global.c
# End Source File
# Begin Source File

SOURCE=..\src\core\graphics.c
# End Source File
# Begin Source File

SOURCE=..\src\core\id.c
# End Source File
# Begin Source File

SOURCE=..\SRC\CORE\INV.C
# End Source File
# Begin Source File

SOURCE=..\src\core\keyboard.c
# End Source File
# Begin Source File

SOURCE=..\src\core\logic.c
# End Source File
# Begin Source File

SOURCE=..\src\core\lzw.c
# End Source File
# Begin Source File

SOURCE=..\src\core\menu.c
# End Source File
# Begin Source File

SOURCE=..\src\core\motion.c
# End Source File
# Begin Source File

SOURCE=..\src\core\objects.c
# End Source File
# Begin Source File

SOURCE=..\src\core\op_cmd.c
# End Source File
# Begin Source File

SOURCE=..\src\core\op_dbg.c
# End Source File
# Begin Source File

SOURCE=..\src\core\op_test.c
# End Source File
# Begin Source File

SOURCE=..\src\core\patches.c
# End Source File
# Begin Source File

SOURCE=..\src\core\picture.c
# End Source File
# Begin Source File

SOURCE=..\src\core\picview.c
# End Source File
# Begin Source File

SOURCE=..\src\core\rand.c
# End Source File
# Begin Source File

SOURCE=..\src\core\savegame.c
# End Source File
# Begin Source File

SOURCE=..\src\core\silent.c
# End Source File
# Begin Source File

SOURCE=..\src\core\sound.c
# End Source File
# Begin Source File

SOURCE=..\src\core\sprite.c
# End Source File
# Begin Source File

SOURCE=..\SRC\CORE\TEXT.C
# End Source File
# Begin Source File

SOURCE=..\src\core\view.c
# End Source File
# Begin Source File

SOURCE=..\src\core\win32\winmain.c
# End Source File
# Begin Source File

SOURCE=..\src\core\words.c
# End Source File
# End Group
# Begin Group "CONSOLE"

# PROP Default_Filter "*.c"
# Begin Source File

SOURCE=..\src\graphics\win32\win32.c
# End Source File
# End Group
# Begin Group "SOUND"

# PROP Default_Filter "*.C"
# Begin Source File

SOURCE=..\src\sound\win32\sound_win32.c
# End Source File
# End Group
# Begin Group "FILESYS"

# PROP Default_Filter "*.C"
# Begin Source File

SOURCE=..\src\filesys\win32\fileglob.c
# End Source File
# Begin Source File

SOURCE=..\src\filesys\win32\path.c
# End Source File
# End Group
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\src\include\agi.h
# End Source File
# Begin Source File

SOURCE=..\src\include\cli.h
# End Source File
# Begin Source File

SOURCE=..\src\include\config.h
# End Source File
# Begin Source File

SOURCE=..\src\include\console.h
# End Source File
# Begin Source File

SOURCE=..\src\include\defines.h
# End Source File
# Begin Source File

SOURCE=..\src\include\enums.h
# End Source File
# Begin Source File

SOURCE=..\src\include\font.h
# End Source File
# Begin Source File

SOURCE=..\src\include\getopt.h
# End Source File
# Begin Source File

SOURCE=..\src\include\graphics.h
# End Source File
# Begin Source File

SOURCE=..\src\include\id.h
# End Source File
# Begin Source File

SOURCE=..\src\include\includes.h
# End Source File
# Begin Source File

SOURCE=..\src\include\keyboard.h
# End Source File
# Begin Source File

SOURCE=..\src\include\logic.h
# End Source File
# Begin Source File

SOURCE=..\src\include\lzw.h
# End Source File
# Begin Source File

SOURCE=..\src\include\machine.h
# End Source File
# Begin Source File

SOURCE=..\src\include\menu.h
# End Source File
# Begin Source File

SOURCE=..\src\include\objects.h
# End Source File
# Begin Source File

SOURCE="..\src\include\op-cmd.h"
# End Source File
# Begin Source File

SOURCE="..\src\include\op-dbg.h"
# End Source File
# Begin Source File

SOURCE="..\src\include\op-misc.h"
# End Source File
# Begin Source File

SOURCE="..\src\include\op-test.h"
# End Source File
# Begin Source File

SOURCE=..\src\include\picture.h
# End Source File
# Begin Source File

SOURCE=..\src\include\rand.h
# End Source File
# Begin Source File

SOURCE=..\src\include\savegame.h
# End Source File
# Begin Source File

SOURCE=..\src\include\sound.h
# End Source File
# Begin Source File

SOURCE=..\src\include\typedef.h
# End Source File
# Begin Source File

SOURCE=..\src\include\view.h
# End Source File
# Begin Source File

SOURCE=..\src\include\win32.h
# End Source File
# Begin Source File

SOURCE=..\src\include\words.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\src\core\win32\winres.rc
# End Source File
# End Group
# End Target
# End Project
