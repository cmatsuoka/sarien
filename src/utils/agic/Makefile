# $Id$

XCFLAGS =
OBJS	= agiavis.o agicore.o agierr.o agifunc.o agigoto.o agiobj.o \
	  agistr.o agitok.o boolean.o
DFILES	= Makefile $(OBJS:.o=.c) agic.txt agic.h agicodes.h agierr.h \
	  boolean.h agi.y
CFILES	=
DDIRS	=
XDFILES =
LIB	=

all: agicy.c agic

agic: agicy.o $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) $(LIBS)

include ../../../Rules

