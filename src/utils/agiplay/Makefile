# $Id$

XCFLAGS	= -I../../include
BIN	= ../../../bin/agiplay
OBJS	= agiplay.o
DFILES	= README Makefile $(OBJS:.o=.c)
DDIRS	= 

all: $(BIN)

$(BIN): $(OBJS)
	$(LD) $(LDFLAGS) $+ -L../../../lib -lsarien -lagi $(LIBS)

include ../../../Rules

include depend
