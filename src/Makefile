# $Id$

XCFLAGS =
OBJS	=
DFILES	= Makefile
CFILES	=
DDIRS	= include filesys sound graphics core
XDFILES =
LIB	=

all:
	for i in $(DDIRS); do		\
		(cd $$i; $(MAKE) all);	\
	done

include ../Rules

