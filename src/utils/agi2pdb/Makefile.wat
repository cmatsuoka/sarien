#
# Makefile for Watcom C/C++ v10.6
#
# $Id$
#

.silent

lflags = option quiet debug all
sys=dos4g
stack_size = 1024k
debug = all

CP = wpp386
CC = wcc386
debug = all
cflags = -zp2 -fpi87 -fp5 -d2 -s -zq -5r -oneatx -D__MSDOS__
O = obj

.error:
	@cd $(HOME)

.cpp.obj : .AUTODEPEND
		@echo	Compiling $^&
        $(cp) $(cflags) $^&

.cc.obj : .AUTODEPEND
		@echo	Compiling $^&
        $(cp) $(cflags) $^&

.c.obj : .AUTODEPEND
		@echo	Compiling $^&
        $(cc) $(cflags) $^&

.asm.obj : .AUTODEPEND
		@echo	Compiling $^&
        $(asm) $(aflags) $^&



OBJS=agi2pdb.obj global.obj id.obj

all: agi2pdb.exe

agi2pdb.exe: $(OBJS)
	@%create wlib.lnk
	@%append wlib.lnk system $(sys)
	@%append wlib.lnk $(lflags)
	@%append wlib.lnk name agi2pdb.exe
	@for %i in ($(OBJS)) do @%append wlib.lnk file %i
	@wlink @wlib.lnk

clean: .symbolic
	@if exist *.err @del *.err
	@if exist *.lib @del *.lib
	@if exist *.obj @del *.obj
	@if exist *.bak @del *.bak

.after
	@if exist wlib.lnk @del wlib.lnk

.error:
