# $Id$

XCFLAGS =
OBJS	=
DFILES	= Makefile AUTHORS BUGS Changelog COPYING README.agi README.sdl \
	  README.svgalib README.unix README.win32 README.ports \
	  TODO sarien.doxygen
CFILES	=
DDIRS	=
XDFILES =
LIB	=

all:

include ../Rules

dox:
	[ -d html ] || mkdir html
	doxygen sarien.doxygen

