# $Id$

XCFLAGS	= -I../../include
BIN	= ../../../bin/agi2pdb
LIBS	=
OBJS	= agi2pdb.o global.o id.o
DFILES	= README Makefile $(OBJS:.o=.c)
DDIRS	= 

all: $(BIN)

$(BIN): $(OBJS)
	$(LD) $(LDFLAGS) $+

include ../../../Rules

include depend
