# $Id$

XCFLAGS	= -I../include
OBJS	= global.o agi_v2.o agi_v3.o agi.o cli.o words.o objects.o \
	  picture.o id.o view.o logic.o op_test.o op_misc.o op_cmd.o \
	  op_dbg.o keyboard.o rand.o menu.o font.o lzw.o getopt.o getopt1.o \
	  sound.o savegame.o silent.o iff.o console.o cycle.o inv.o text.o \
	  gfx_base.o gfx_agi.o
DFILES	= Makefile $(OBJS:.o=.c) main.c Makefile.wat Makefile.dj
CFILES	=
DDIRS	=
XLIBS	= -L../../lib -lagi

LIB	= ../../lib/libagi.a

all: fixcr $(LIB) ../../bin/sarien

../../bin/sarien: main.o ../../lib/libagi.a
	$(LD) $(LDFLAGS) main.o $(XLIBS) $(X11LIBS) $(LIBS)


fixcr:
	@grep "\\\$$" picture.c>/dev/null; \
	if [ "$$?" = "0" ]; then \
		echo Fixing carriage returns in picture.c; \
		mv picture.c picture.c.orig; \
		cat picture.c.orig | sed "s/\\\$$/\\\/" > picture.c; \
	fi

include ../../Version
include ../../Rules

include depend

