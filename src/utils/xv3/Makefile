# $Id$

XCFLAGS	= -I../../include
BIN	= ../../../bin/xv3
LIBS	=
OBJS	= xv3.o
DFILES	= README Makefile agi3info.txt $(OBJS:.o=.c)
DDIRS	= 

all: $(BIN)

$(BIN): $(OBJS)
	$(LD) $(LDFLAGS) $+

include ../../../Rules

include depend
