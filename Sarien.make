#   File:       Sarien.make
#   Target:     Sarien
#   Created:    Saturday, July 7, 2001 09:50:50 PM
#
#   $Id$


MAKEFILE        = Sarien.make
�MondoBuild�    = {MAKEFILE}  # Make blank to avoid rebuilds when makefile is modified

ObjDir          = :
Includes        =  �
				  -i :src:core:macos: �
				  -i :src:include:

Sym-68K         = -sym off

COptions        = {Includes} {Sym-68K} -model far -typecheck relaxed -d VERSION='"0.8.0"' -d __MPW__ -w 35,7,2


### Source Files ###

SrcFiles        =  �
				  :src:core:agi.c �
				  :src:core:agi_v2.c �
				  :src:core:agi_v3.c �
				  :src:core:checks.c �
				  :src:core:console.c �
				  :src:core:cycle.c �
				  :src:core:font.c �
				  :src:core:global.c �
				  :src:core:graphics.c �
				  :src:core:id.c �
				  :src:core:inv.c �
				  :src:core:keyboard.c �
				  :src:core:logic.c �
				  :src:core:lzw.c �
				  :src:core:main.c �
				  :src:core:menu.c �
				  :src:core:motion.c �
				  :src:core:objects.c �
				  :src:core:op_cmd.c �
				  :src:core:op_dbg.c �
				  :src:core:op_test.c �
				  :src:core:patches.c �
				  :src:core:picture.c �
				  :src:core:rand.c �
				  :src:core:savegame.c �
				  :src:core:silent.c �
				  :src:core:sound.c �
				  :src:sound:dummy:dummy.c �
				  :src:core:macos:library.c �
				  :src:graphics:macos:macos.c �
				  :src:filesys:macos:fileglob.c �
				  :src:filesys:macos:path.c �
				  :src:core:sprite.c �
				  :src:core:text.c �
				  :src:core:words.c �
				  :src:core:view.c


### Object Files ###

ObjFiles-68K    =  �
				  "{ObjDir}agi.c.o" �
				  "{ObjDir}agi_v2.c.o" �
				  "{ObjDir}agi_v3.c.o" �
				  "{ObjDir}checks.c.o" �
				  "{ObjDir}console.c.o" �
				  "{ObjDir}cycle.c.o" �
				  "{ObjDir}font.c.o" �
				  "{ObjDir}global.c.o" �
				  "{ObjDir}graphics.c.o" �
				  "{ObjDir}id.c.o" �
				  "{ObjDir}inv.c.o" �
				  "{ObjDir}keyboard.c.o" �
				  "{ObjDir}logic.c.o" �
				  "{ObjDir}lzw.c.o" �
				  "{ObjDir}main.c.o" �
				  "{ObjDir}menu.c.o" �
				  "{ObjDir}motion.c.o" �
				  "{ObjDir}objects.c.o" �
				  "{ObjDir}op_cmd.c.o" �
				  "{ObjDir}op_dbg.c.o" �
				  "{ObjDir}op_test.c.o" �
				  "{ObjDir}patches.c.o" �
				  "{ObjDir}picture.c.o" �
				  "{ObjDir}rand.c.o" �
				  "{ObjDir}savegame.c.o" �
				  "{ObjDir}macos.c.o" �
				  "{ObjDir}library.c.o" �
				  "{ObjDir}fileglob.c.o" �
				  "{ObjDir}path.c.o" �
				  "{ObjDir}dummy.c.o" �
				  "{ObjDir}silent.c.o" �
				  "{ObjDir}sound.c.o" �
				  "{ObjDir}sprite.c.o" �
				  "{ObjDir}text.c.o" �
				  "{ObjDir}words.c.o" �
				  "{ObjDir}view.c.o"


### Libraries ###

LibFiles-68K    =  �
				  "{Libraries}MathLib.o" �
				  "{CLibraries}StdCLib.o" �
				  "{Libraries}MacRuntime.o" �
				  "{Libraries}IntEnv.o" �
				  "{Libraries}ToolLibs.o" �
				  "{Libraries}Interface.o"


### Default Rules ###

.c.o  �  .c  {�MondoBuild�}
	{C} {depDir}{default}.c -o {targDir}{default}.c.o {COptions}


### Build Rules ###

Sarien  ��  {ObjFiles-68K} {LibFiles-68K} {�MondoBuild�}
	ILink �
		-o {Targ} �
		{ObjFiles-68K} �
		{LibFiles-68K} �
		{Sym-68K} �
		-mf -d �
		-t 'APPL' �
		-c 'FAGI' �
		-model far �
		-state rewrite �
		-compact -pad 0
	If "{Sym-68K}" =~ /-sym �[nNuU]�/
		ILinkToSYM {Targ}.NJ -mf -sym 3.2 -c 'sade'
	End



### Required Dependencies ###

"{ObjDir}agi.c.o"  �  :src:core:agi.c
"{ObjDir}agi_v2.c.o"  �  :src:core:agi_v2.c
"{ObjDir}agi_v3.c.o"  �  :src:core:agi_v3.c
"{ObjDir}checks.c.o"  �  :src:core:checks.c
"{ObjDir}cli.c.o"  �  :src:core:cli.c
"{ObjDir}console.c.o"  �  :src:core:console.c
"{ObjDir}cycle.c.o"  �  :src:core:cycle.c
"{ObjDir}font.c.o"  �  :src:core:font.c
"{ObjDir}global.c.o"  �  :src:core:global.c
"{ObjDir}graphics.c.o"  �  :src:core:graphics.c
"{ObjDir}id.c.o"  �  :src:core:id.c
"{ObjDir}inv.c.o"  �  :src:core:inv.c
"{ObjDir}keyboard.c.o"  �  :src:core:keyboard.c
"{ObjDir}logic.c.o"  �  :src:core:logic.c
"{ObjDir}lzw.c.o"  �  :src:core:lzw.c
"{ObjDir}main.c.o"  �  :src:core:main.c
"{ObjDir}menu.c.o"  �  :src:core:menu.c
"{ObjDir}motion.c.o"  �  :src:core:motion.c
"{ObjDir}objects.c.o"  �  :src:core:objects.c
"{ObjDir}op_cmd.c.o"  �  :src:core:op_cmd.c
"{ObjDir}op_dbg.c.o"  �  :src:core:op_dbg.c
"{ObjDir}op_test.c.o"  �  :src:core:op_test.c
"{ObjDir}patches.c.o"  �  :src:core:patches.c
"{ObjDir}picture.c.o"  �  :src:core:picture.c
"{ObjDir}rand.c.o"  �  :src:core:rand.c
"{ObjDir}savegame.c.o"  �  :src:core:savegame.c
"{ObjDir}library.c.o"  �  :src:core:macos:library.c
"{ObjDir}macos.c.o"  �  :src:graphics:macos:macos.c
"{ObjDir}fileglob.c.o"  �  :src:filesys:macos:fileglob.c
"{ObjDir}path.c.o"  �  :src:filesys:macos:path.c
"{ObjDir}silent.c.o"  �  :src:core:silent.c
"{ObjDir}sound.c.o"  �  :src:core:sound.c
"{ObjDir}dummy.c.o"  �  :src:sound:dummy:dummy.c
"{ObjDir}sprite.c.o"  �  :src:core:sprite.c
"{ObjDir}text.c.o"  �  :src:core:text.c
"{ObjDir}words.c.o"  �  :src:core:words.c
"{ObjDir}view.c.o"  �  :src:core:view.c


### Optional Dependencies ###
### Build this target to generate "include file" dependencies. ###

Dependencies  �  $OutOfDate
	MakeDepend �
		-append {MAKEFILE} �
		-ignore "{CIncludes}" �
		-objdir "{ObjDir}" �
		-objext .o �
		{Includes} �
		{SrcFiles}


