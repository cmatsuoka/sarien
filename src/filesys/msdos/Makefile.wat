#
# Makefile for Watcom C/C++ v10.6
#

#
# Dependancies for Watcom C/C++ 10.6
#

!include $(HOME)\rules.wat

OBJS = fileglob.obj path.obj

all: fileglob.lib

fileglob.lib: $(OBJS)
	@%create wlib.lnk
	for %i in ($(OBJS)) do @%append wlib.lnk +%i
	wlib -n -q -q fileglob @wlib.lnk
	@copy fileglob.lib $(HOME)\lib

clean: .symbolic
	@if exist *.lib @del *.lib
	@if exist *.obj @del *.obj
	@if exist *.bak @del *.bak

.after
	@if exist wlib.lnk @del wlib.lnk
#	@cd $(HOME)

.error:
#	@cd $(HOME)
