#   File:       Sarien.make
#   Target:     Sarien
#   Created:    Saturday, July 7, 2001 09:50:50 PM
#
#   $Id$


MAKEFILE        = Sarien.make
¥MondoBuild¥    = {MAKEFILE}  # Make blank to avoid rebuilds when makefile is modified

ObjDir          = :
Includes        =  ¶
				  -i :src:core:macos: ¶
				  -i :src:include:

Sym-PPC         = -sym off
Sym-68K         = -sym off

CommonOptions	= -typecheck relaxed -d VERSION='"0.8.0"' -d __MPW__ -w 35,7,2

PPCCOptions     = {Includes} {Sym-PPC} {CommonOptions}

COptions        = {Includes} {Sym-68K} -model far {CommonOptions}


### Source Files ###

SrcFiles        =  ¶
				  :src:core:agi.c ¶
				  :src:core:agi_v2.c ¶
				  :src:core:agi_v3.c ¶
				  :src:core:checks.c ¶
				  :src:core:console.c ¶
				  :src:core:cycle.c ¶
				  :src:core:font.c ¶
				  :src:core:global.c ¶
				  :src:core:graphics.c ¶
				  :src:core:id.c ¶
				  :src:core:inv.c ¶
				  :src:core:keyboard.c ¶
				  :src:core:logic.c ¶
				  :src:core:lzw.c ¶
				  :src:core:macos:main.c ¶
				  :src:core:menu.c ¶
				  :src:core:motion.c ¶
				  :src:core:objects.c ¶
				  :src:core:op_cmd.c ¶
				  :src:core:op_dbg.c ¶
				  :src:core:op_test.c ¶
				  :src:core:patches.c ¶
				  :src:core:picture.c ¶
				  :src:core:rand.c ¶
				  :src:core:savegame.c ¶
				  :src:core:silent.c ¶
				  :src:core:sound.c ¶
				  :src:sound:dummy:dummy.c ¶
				  :src:core:macos:library.c ¶
				  :src:graphics:macos:macos.c ¶
				  :src:filesys:macos:fileglob.c ¶
				  :src:filesys:macos:path.c ¶
				  :src:core:sprite.c ¶
				  :src:core:text.c ¶
				  :src:core:words.c ¶
				  :src:core:view.c


### Object Files ###

ObjFiles-PPC    =  ¶
				  "{ObjDir}agi.c.x" ¶
				  "{ObjDir}agi_v2.c.x" ¶
				  "{ObjDir}agi_v3.c.x" ¶
				  "{ObjDir}checks.c.x" ¶
				  "{ObjDir}console.c.x" ¶
				  "{ObjDir}cycle.c.x" ¶
				  "{ObjDir}font.c.x" ¶
				  "{ObjDir}global.c.x" ¶
				  "{ObjDir}graphics.c.x" ¶
				  "{ObjDir}id.c.x" ¶
				  "{ObjDir}inv.c.x" ¶
				  "{ObjDir}keyboard.c.x" ¶
				  "{ObjDir}logic.c.x" ¶
				  "{ObjDir}lzw.c.x" ¶
				  "{ObjDir}main.c.x" ¶
				  "{ObjDir}menu.c.x" ¶
				  "{ObjDir}motion.c.x" ¶
				  "{ObjDir}objects.c.x" ¶
				  "{ObjDir}op_cmd.c.x" ¶
				  "{ObjDir}op_dbg.c.x" ¶
				  "{ObjDir}op_test.c.x" ¶
				  "{ObjDir}patches.c.x" ¶
				  "{ObjDir}picture.c.x" ¶
				  "{ObjDir}rand.c.x" ¶
				  "{ObjDir}savegame.c.x" ¶
				  "{ObjDir}macos.c.x" ¶
				  "{ObjDir}library.c.x" ¶
				  "{ObjDir}fileglob.c.x" ¶
				  "{ObjDir}path.c.x" ¶
				  "{ObjDir}dummy.c.x" ¶
				  "{ObjDir}silent.c.x" ¶
				  "{ObjDir}sound.c.x" ¶
				  "{ObjDir}sprite.c.x" ¶
				  "{ObjDir}text.c.x" ¶
				  "{ObjDir}words.c.x" ¶
				  "{ObjDir}view.c.x"

ObjFiles-68K    =  ¶
				  "{ObjDir}agi.c.o" ¶
				  "{ObjDir}agi_v2.c.o" ¶
				  "{ObjDir}agi_v3.c.o" ¶
				  "{ObjDir}checks.c.o" ¶
				  "{ObjDir}console.c.o" ¶
				  "{ObjDir}cycle.c.o" ¶
				  "{ObjDir}font.c.o" ¶
				  "{ObjDir}global.c.o" ¶
				  "{ObjDir}graphics.c.o" ¶
				  "{ObjDir}id.c.o" ¶
				  "{ObjDir}inv.c.o" ¶
				  "{ObjDir}keyboard.c.o" ¶
				  "{ObjDir}logic.c.o" ¶
				  "{ObjDir}lzw.c.o" ¶
				  "{ObjDir}main.c.o" ¶
				  "{ObjDir}menu.c.o" ¶
				  "{ObjDir}motion.c.o" ¶
				  "{ObjDir}objects.c.o" ¶
				  "{ObjDir}op_cmd.c.o" ¶
				  "{ObjDir}op_dbg.c.o" ¶
				  "{ObjDir}op_test.c.o" ¶
				  "{ObjDir}patches.c.o" ¶
				  "{ObjDir}picture.c.o" ¶
				  "{ObjDir}rand.c.o" ¶
				  "{ObjDir}savegame.c.o" ¶
				  "{ObjDir}macos.c.o" ¶
				  "{ObjDir}library.c.o" ¶
				  "{ObjDir}fileglob.c.o" ¶
				  "{ObjDir}path.c.o" ¶
				  "{ObjDir}dummy.c.o" ¶
				  "{ObjDir}silent.c.o" ¶
				  "{ObjDir}sound.c.o" ¶
				  "{ObjDir}sprite.c.o" ¶
				  "{ObjDir}text.c.o" ¶
				  "{ObjDir}words.c.o" ¶
				  "{ObjDir}view.c.o"



### Libraries ###

LibFiles-PPC    =  ¶
				  "{SharedLibraries}InterfaceLib" ¶
				  "{SharedLibraries}StdCLib" ¶
				  "{SharedLibraries}MathLib" ¶
				  "{PPCLibraries}StdCRuntime.o" ¶
				  "{PPCLibraries}PPCCRuntime.o" ¶
				  "{PPCLibraries}PPCToolLibs.o"

LibFiles-68K    =  ¶
				  "{Libraries}MathLib.o" ¶
				  "{CLibraries}StdCLib.o" ¶
				  "{Libraries}MacRuntime.o" ¶
				  "{Libraries}IntEnv.o" ¶
				  "{Libraries}ToolLibs.o" ¶
				  "{Libraries}Interface.o"


### Resources ###

Resources		= :src:core:macos:sarien.r

RezOptions		= -i :src:core:macos


### Default Rules ###

.c.x  Ä  .c  {¥MondoBuild¥}
	{PPCC} {depDir}{default}.c -o {targDir}{default}.c.x {PPCCOptions}

.c.o  Ä  .c  {¥MondoBuild¥}
	{C} {depDir}{default}.c -o {targDir}{default}.c.o {COptions}


### Build Rules ###

Sarien  ÄÄ  {Resources} {¥MondoBuild¥}
	Rez {RezOptions} {Resources} -o Sarien

Sarien  ÄÄ  {ObjFiles-PPC} {LibFiles-PPC} {Resources} {¥MondoBuild¥}
	PPCLink ¶
		-o {Targ} ¶
		{ObjFiles-PPC} ¶
		{LibFiles-PPC} ¶
		{Sym-PPC} ¶
		-mf -d ¶
		-t 'APPL' ¶
		-c 'FAGI'

Sarien  ÄÄ  {ObjFiles-68K} {LibFiles-68K} {Resources} {¥MondoBuild¥}
	ILink ¶
		-o {Targ} ¶
		{ObjFiles-68K} ¶
		{LibFiles-68K} ¶
		{Sym-68K} ¶
		-mf -d ¶
		-t 'APPL' ¶
		-c 'FAGI' ¶
		-model far ¶
		-state rewrite ¶
		-compact -pad 0
	If "{Sym-68K}" =~ /-sym Å[nNuU]Å/
		ILinkToSYM {Targ}.NJ -mf -sym 3.2 -c 'sade'
	End



### Required Dependencies ###

"{ObjDir}agi.c.x" "{ObjDir}agi.c.o"  Ä  :src:core:agi.c
"{ObjDir}agi_v2.c.x" "{ObjDir}agi_v2.c.o"  Ä  :src:core:agi_v2.c
"{ObjDir}agi_v3.c.x" "{ObjDir}agi_v3.c.o"  Ä  :src:core:agi_v3.c
"{ObjDir}checks.c.x" "{ObjDir}checks.c.o"  Ä  :src:core:checks.c
"{ObjDir}cli.c.x" "{ObjDir}cli.c.o"  Ä  :src:core:cli.c
"{ObjDir}console.c.x" "{ObjDir}console.c.o"  Ä  :src:core:console.c
"{ObjDir}cycle.c.x" "{ObjDir}cycle.c.o"  Ä  :src:core:cycle.c
"{ObjDir}font.c.x" "{ObjDir}font.c.o"  Ä  :src:core:font.c
"{ObjDir}global.c.x" "{ObjDir}global.c.o"  Ä  :src:core:global.c
"{ObjDir}graphics.c.x" "{ObjDir}graphics.c.o"  Ä  :src:core:graphics.c
"{ObjDir}id.c.x" "{ObjDir}id.c.o"  Ä  :src:core:id.c
"{ObjDir}inv.c.x" "{ObjDir}inv.c.o"  Ä  :src:core:inv.c
"{ObjDir}keyboard.c.x" "{ObjDir}keyboard.c.o"  Ä  :src:core:keyboard.c
"{ObjDir}logic.c.x" "{ObjDir}logic.c.o"  Ä  :src:core:logic.c
"{ObjDir}lzw.c.x" "{ObjDir}lzw.c.o"  Ä  :src:core:lzw.c
"{ObjDir}main.c.x" "{ObjDir}main.c.o"  Ä  :src:core:macos:main.c
"{ObjDir}menu.c.x" "{ObjDir}menu.c.o"  Ä  :src:core:menu.c
"{ObjDir}motion.c.x" "{ObjDir}motion.c.o"  Ä  :src:core:motion.c
"{ObjDir}objects.c.x" "{ObjDir}objects.c.o"  Ä  :src:core:objects.c
"{ObjDir}op_cmd.c.x" "{ObjDir}op_cmd.c.o"  Ä  :src:core:op_cmd.c
"{ObjDir}op_dbg.c.x" "{ObjDir}op_dbg.c.o"  Ä  :src:core:op_dbg.c
"{ObjDir}op_test.c.x" "{ObjDir}op_test.c.o"  Ä  :src:core:op_test.c
"{ObjDir}patches.c.x" "{ObjDir}patches.c.o"  Ä  :src:core:patches.c
"{ObjDir}picture.c.x" "{ObjDir}picture.c.o"  Ä  :src:core:picture.c
"{ObjDir}rand.c.x" "{ObjDir}rand.c.o"  Ä  :src:core:rand.c
"{ObjDir}savegame.c.x" "{ObjDir}savegame.c.o"  Ä  :src:core:savegame.c
"{ObjDir}library.c.x" "{ObjDir}library.c.o"  Ä  :src:core:macos:library.c
"{ObjDir}macos.c.x" "{ObjDir}macos.c.o"  Ä  :src:graphics:macos:macos.c
"{ObjDir}fileglob.c.x" "{ObjDir}fileglob.c.o"  Ä  :src:filesys:macos:fileglob.c
"{ObjDir}path.c.x" "{ObjDir}path.c.o"  Ä  :src:filesys:macos:path.c
"{ObjDir}silent.c.x" "{ObjDir}silent.c.o"  Ä  :src:core:silent.c
"{ObjDir}sound.c.x" "{ObjDir}sound.c.o"  Ä  :src:core:sound.c
"{ObjDir}dummy.c.x" "{ObjDir}dummy.c.o"  Ä  :src:sound:dummy:dummy.c
"{ObjDir}sprite.c.x" "{ObjDir}sprite.c.o"  Ä  :src:core:sprite.c
"{ObjDir}text.c.x" "{ObjDir}text.c.o"  Ä  :src:core:text.c
"{ObjDir}words.c.x" "{ObjDir}words.c.o"  Ä  :src:core:words.c
"{ObjDir}view.c.x" "{ObjDir}view.c.o"  Ä  :src:core:view.c


### Optional Dependencies ###
### Build this target to generate "include file" dependencies. ###

Dependencies  Ä  $OutOfDate
	MakeDepend ¶
		-append {MAKEFILE} ¶
		-ignore "{CIncludes}" ¶
		-objdir "{ObjDir}" ¶
		-objext .o ¶
		{Includes} ¶
		{SrcFiles}


