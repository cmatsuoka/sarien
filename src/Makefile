
XCFLAGS =
OBJS	=
DFILES	= Makefile
CFILES	=
DDIRS	= include fileglob sound graphics core utils
XDFILES =
LIB	=

all:
	for i in $(DDIRS); do		\
		(cd $$i; $(MAKE) all);	\
	done

include ../Rules

