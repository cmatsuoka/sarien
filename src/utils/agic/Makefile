# $Id$

XCFLAGS = -D_MAX_PATH=256
OBJS	= agiavis.o agicore.o agierr.o agifunc.o agigoto.o agiobj.o \
	  agistr.o agitok.o boolean.o
DFILES	= Makefile $(OBJS:.o=.c) agic.txt agic.h agicodes.h agierr.h \
	  boolean.h agi.y agi.lex
CFILES	=
DDIRS	=
XDFILES =
LIB	=

all: lex.yy.c y.tab.c agic

agic: y.tab.o lex.yy.o $(OBJS)
	$(LD) $(LDFLAGS) $+

lex.yy.c: agic.lex
	flex $+

y.tab.c: agi.y
	yacc -d $+

include ../../../Rules

