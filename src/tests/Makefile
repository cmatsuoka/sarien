# $Id$

XCFLAGS	= -I../include
XLDFLAGS=
OBJS	= format.o arith.o flag.o picture.o random.o
DFILES	= Makefile $(TESTS:.=.c) orat.c orat.h test.c test.h
CFILES	=
DDIRS	= data
XLIBS	= -L../../lib -lagi -lsarien

all: test

test: test.o orat.o $(OBJS)
	$(LD) $(LDFLAGS) $(XLDFLAGS) $+ $(XLIBS) $(X11LIBS) $(LIBS)

include ../../Version
include ../../Rules

include depend

