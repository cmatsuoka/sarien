# $Id$

XCFLAGS	= -I../include
XLDFLAGS= $(WINLDFLAGS)
OBJS	= global.o agi_v2.o agi_v3.o agi_v4.o agi.o cli.o words.o objects.o \
	  picture.o view.o id.o logic.o op_test.o op_cmd.o op_dbg.o patches.o \
	  keyboard.o menu.o font.o lzw.o getopt.o getopt1.o savegame.o \
	  sound.o silent.o console.o cycle.o inv.o text.o graphics.o \
	  sprite.o checks.o motion.o picview.o rand.o
GFILES	= agi_v2.c agi_v3.c agi_v4.c agi.c cli.c words.c view.c id.c \
	  logic.c keyboard.c savegame.c sound.c silent.c console.c \
	  cycle.c graphics.c sprite.c checks.c motion.c menu.c view.c
XDEPS	= $(MAIN_FILE).o
DFILES	= Makefile $(OBJS:.o=.c) rand.c main.c id.c hirespic.c \
	  Makefile.wat Makefile.dj
CFILES	=
DDIRS	= dc palmos win32 macos
XLIBS	= -L../../lib -lagi -lsarien

LIB	= ../../lib/libagi.a

all: $(LIB) archfiles exefile

include ../../Version
include ../../Rules

exefile: ../../bin/sarien$(EXE_SUFFIX)

archfiles:
	@[ "$(CORE_ARCH)" ] && (cd $(CORE_ARCH); $(MAKE) all) ||:

# Must be directly linked (not in lib)
ifeq ($(CORE_ARCH),win32)
XTRA_OBJS = win32/winres.o
endif

../../bin/sarien$(EXE_SUFFIX): $(MAIN_FILE).o $(XTRA_OBJS) $(LIB) ../../lib/libsarien.a
	$(LD) $(LDFLAGS) $(XLDFLAGS) $(MAIN_FILE).o id.o $(XTRA_OBJS) $(XLIBS) $(X11LIBS) $(LIBS)

graph: callgraph.ps

callgraph.png: callgraph.ps
	convert $+ $@

callgraph.ps: callgraph.dot
	dot -Tps -Gsize="42,10" -Gratio=0.25 -Eweight=2 $+ -o $@

callgraph.dot: $(GFILES) Makefile ../../scripts/cflow2dot.pl
	@echo "digraph sarien {" > $@
	@cflow $(XCFLAGS) -gAP $(GFILES) 2>/dev/null | \
		../../scripts/cflow2dot.pl > $@

include depend

