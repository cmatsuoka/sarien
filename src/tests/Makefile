# $Id$

XCFLAGS	= -I../include
XLDFLAGS=
OBJS	= format.o arith.o
DFILES	= Makefile $(TESTS:.=.c)
CFILES	=
DDIRS	=
XLIBS	= -L../../lib -lagi -lsarien
LIB	=

all: test

test: test.o $(OBJS)
	$(LD) $(LDFLAGS) $(XLDFLAGS) $+ $(XLIBS) $(X11LIBS) $(LIBS)

include ../../Version
include ../../Rules

include depend

