# $Id$

XCFLAGS	= -I../include
OBJS	= global.o agi_v2.o agi_v3.o agi_v4.o agi.o cli.o words.o objects.o \
	  picture.o view.o id.o logic.o op_test.o op_cmd.o op_dbg.o patches.o \
	  keyboard.o rand.o menu.o font.o lzw.o getopt.o getopt1.o savegame.o \
	  sound.o silent.o iff.o console.o cycle.o inv.o text.o graphics.o \
	  sprite.o checks.o motion.o picview.o
XDEPS	= $(MAIN_FILE).o
DFILES	= Makefile $(OBJS:.o=.c) main.c winmain.c id.c Makefile.wat Makefile.dj
CFILES	=
DDIRS	=
XLIBS	= -L../../lib -lagi

LIB	= ../../lib/libagi.a

all: fixcr $(LIB) ../../bin/sarien

include ../../Rules

../../bin/sarien: $(MAIN_FILE).o ../../lib/libagi.a
	$(LD) $(LDFLAGS) $(MAIN_FILE).o id.o $(XLIBS) $(X11LIBS) $(LIBS)

fixcr:
	@grep "\\\$$" picture.c>/dev/null; \
	if [ "$$?" = "0" ]; then \
		echo Fixing carriage returns in picture.c; \
		mv picture.c picture.c.orig; \
		cat picture.c.orig | sed "s/\\\$$/\\\/" > picture.c; \
	fi

include ../../Version

include depend

