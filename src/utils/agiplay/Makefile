# $Id$

XCFLAGS	= -I../../include
BIN	= ../../../bin/agiplay
LIBS	=
OBJS	= agiplay.o
DFILES	= README Makefile $(OBJS:.o=.c)
DDIRS	= 

all: $(BIN)

$(BIN): $(OBJS)
	$(LD) $(LDFLAGS) $+

include ../../../Rules

include depend
