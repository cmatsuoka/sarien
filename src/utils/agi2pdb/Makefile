# $Id$

XCFLAGS	= -I../../include
BIN	= ../../../bin/agi2pdb
LIBS	=
OBJS	= agi2pdb.o convert2pdb.o
DFILES	= README Makefile $(OBJS:.o=.c)
DDIRS	= 

all: $(BIN)

$(BIN): $(OBJS)
	$(LD) $(LDFLAGS) $+ -L../../../lib -lagi -lsarien $(LIBS)

include ../../../Rules

include depend
