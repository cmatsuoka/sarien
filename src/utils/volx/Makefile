# $Id$

XCFLAGS	= -I../../include
BIN	= ../../../bin/volx
LIBS	=
OBJS	= volx.o
DFILES	= README Makefile $(OBJS:.o=.c)
DDIRS	= 

all: $(BIN)

$(BIN): $(OBJS)
	$(LD) $(LDFLAGS) $+

include ../../../Rules

include depend
