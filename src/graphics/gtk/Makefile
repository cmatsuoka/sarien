
XCFLAGS	= -I../../include `pkg-config --cflags gtk+-2.0` $(X11CFLAGS)
OBJS	= gtk.o
DFILES	= Makefile $(OBJS:.o=.c)
CFILES	=
DDIRS	=
XDFILES	=
LIB	= ../../../lib/libsarien.a

all: $(LIB)

include ../../../Rules
include ../../../Version

