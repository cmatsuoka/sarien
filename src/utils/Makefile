# $Id$

XCFLAGS =
OBJS	=
DFILES	= Makefile README
CFILES	=
DDIRS	= agiplay showlog xv3 volx agi2pdb agic agiglue
XDFILES =
LIB	=

all:
	for i in $(DDIRS); do           \
		(cd $$i; $(MAKE) all);  \
	done

include ../../Rules

