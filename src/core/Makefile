# $Id$

XCFLAGS	= -I../include
OBJS	= global.o agi_v2.o agi_v3.o agi_v4.o agi.o cli.o words.o objects.o \
	  picture.o view.o id.o logic.o op_test.o op_cmd.o op_dbg.o patches.o \
	  keyboard.o rand.o menu.o font.o lzw.o getopt.o getopt1.o savegame.o \
	  sound.o silent.o iff.o console.o cycle.o inv.o text.o graphics.o \
	  sprite.o checks.o motion.o picview.o
GFILES	= agi_v2.c agi_v3.c agi_v4.c agi.c cli.c words.c view.c id.c \
	  logic.c op_test.c op_cmd.c op_dbg.c savegame.c sound.c silent.c \
	  console.c cycle.c inv.c graphics.c sprite.c checks.c motion.c
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

graph: callgraph.ps

callgraph.ps: callgraph.dot
	dot -Tps -Gsize="42,3" -Gratio=0.075 -Eweight=2 $+ -o $@

callgraph.dot: $(GFILES) Makefile
	@echo "digraph sarien {" > $@
	@cflow $(XCFLAGS) -d2 -gaAP $(GFILES) 2>/dev/null | \
	perl -ne '\
		s/^\d+\t//; \
		if (/^\t(_D|report|[sg]etflag|[sg]etvar|main)[^\w]/ || \
		    /^\t(memset|feof|strtok|atoi|atol|abs|tolower)[^\w]/ || \
		    /^\t(lohi_|hilo_|__|test_)/) { next; } \
		if (/^(\w+)/) { $$function = $$1; next; } \
		/^\t(\w+)/ && print "\t\"$$function\" -> \"$$1\";\n"' >> $@
	@echo "}" >> $@

fixcr:
	@grep "\\\$$" picture.c>/dev/null; \
	if [ "$$?" = "0" ]; then \
		echo Fixing carriage returns in picture.c; \
		mv picture.c picture.c.orig; \
		cat picture.c.orig | sed "s/\\\$$/\\\/" > picture.c; \
	fi

include ../../Version

include depend

