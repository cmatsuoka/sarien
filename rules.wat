.SILENT

!include $(HOME)\Version

lflags = option quiet debug all
sys=dos4g
stack_size = 1024k
debug = all

INC=$(HOME)\src\include
CP = wpp386
CC = wcc386
debug = all
cflags = -I$(INC) -fpi87 -fp5 -d2 -s -zq -zp8 -6r -oneatx -D__MSDOS__ -DVERSION="$(VERSION)"
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
