# $Id$

XCFLAGS =
OBJS	= agiglue.o
DFILES	= Makefile $(OBJS:.o=.c) agiglue.txt
CFILES	=
DDIRS	=
XDFILES =
LIB	=

all: agiglue

agic: $(OBJS)
	$(LD) $(LDFLAGS) $+

lex.yy.c: agic.lex
	flex $+

y.tab.c: agi.y
	yacc -d $+

include ../../../Rules

