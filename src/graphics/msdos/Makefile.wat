#
# Makefile for Watcom C/C++ v10.6
#

!include $(HOME)\rules.wat

OBJS = pcvga.$(O)

all: start driver.lib

start: .symbolic
	@if exist driver.lib @del driver.lib

driver.lib: $(OBJS)
	wlib -q -n -b driver.lib $(OBJS)
	@copy driver.lib $(HOME)\lib

clean: .symbolic
	@if exist *.bak @del *.bak
	@if exist *.$(O) @del *.$(O)
	@if exist *.lib @del *.lib

.after
	@if exist test.lnk @del test.lnk
	@if exist ok @del ok
	@cd $(HOME)
