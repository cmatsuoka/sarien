
XCFLAGS	= -I../../include $(X11CFLAGS)
OBJS	= x11.o
DFILES	= Makefile $(OBJS:.o=.c)
CFILES	=
DDIRS	=
XDFILES	= dga.c
LIB	= ../../../lib/libsarien.a

all: $(LIB)

x11.o: x11.c dga.c

include ../../../Rules

