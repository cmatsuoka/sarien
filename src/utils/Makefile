# $Id$

XCFLAGS =
OBJS	=
DFILES	= Makefile README
CFILES	=
DDIRS	= showlog xv3 volx
XDFILES =
LIB	=

all:
	for i in $(DDIRS); do           \
		(cd $$i; $(MAKE) all);  \
	done

include ../../Rules

