#
# Makefile for Watcom C/C++ v10.6
#

#
# Dependancies for Watcom C/C++ 10.6
#

!include $(HOME)\rules.wat

OBJS = dummy.obj

all: sound.lib

sound.lib: $(OBJS)
	@%create wlib.lnk
	for %i in ($(OBJS)) do @%append wlib.lnk +%i
	wlib -n -q -q sound @wlib.lnk
	@copy sound.lib $(HOME)\lib

.error:
#	@cd $(HOME)

clean: .symbolic
	@if exist *.lib @del *.lib
	@if exist *.obj @del *.obj
	@if exist *.bak @del *.bak

.after
	@if exist wlib.lnk @del wlib.lnk
#	@cd $(HOME)
