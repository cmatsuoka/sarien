# $Id$

XCFLAGS	= -I../../include
BIN	= ../../../bin/showlog
LIBS	=
OBJS	= showlog.o
DFILES	= README Makefile agicodes.h $(OBJS:.o=.c)
DDIRS	= 

all: $(BIN)

$(BIN): $(OBJS)
	$(LD) $(LDFLAGS) $+

include ../../../Rules

include depend
