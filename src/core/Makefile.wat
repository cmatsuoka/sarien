#
# Makefile for Watcom C/C++ v10.6
#

#
# Dependancies for Watcom C/C++ 10.6
#

!include $(HOME)\rules.wat

OBJS =	main.obj global.obj agi_v2.obj agi_v3.obj agi.obj		&
		cli.obj words.obj objects.obj picture.obj id.obj	&
		view.obj logic.obj text.obj keyboard.obj rand.obj	&
		menu.obj lzw.obj getopt.obj getopt1.obj sound.obj	&
		font.obj op_test.obj op_dbg.obj				&
		op_cmd.obj savegame.obj silent.obj cycle.obj 		&
		console.obj inv.obj graphics.obj agi_v4.obj checks.obj	&
		motion.obj patches.obj sprite.obj picview.obj

all: start agi_core.lib

start: .symbolic
	@if exist agi_core.lib @del agi_core.lib

agi_core.lib: $(OBJS)
	@%create wlib.lnk
	@for %i in ($(OBJS)) do @%append wlib.lnk +%i
	wlib -n -b -q agi_core @wlib.lnk
	@copy agi_core.lib $(HOME)\lib

clean: .symbolic
	@if exist *.bak @del *.bak
	@if exist *.obj @del *.obj
	@if exist *.lib @del *.lib

.after
	@if exist wlib.lnk @del wlib.lnk
	@cd $(HOME)
