# $Id$

XCFLAGS =
OBJS	=
DFILES	= Makefile AUTHORS BUGS Changelog COPYING README.sdl \
	  README.svgalib README.unix README.win32 README.ports README.macos \
	  README.turboc README.pocketpc README.djgpp README.dc TODO \
	  sarien.doxygen roadmap.txt sarien.1 manpage.sgml
CFILES	=
DDIRS	=
XDFILES =
LIB	=

all: sarien.1

include ../Rules

dox:
	[ -d html ] || mkdir html
	doxygen sarien.doxygen

flowchart.ps: flowchart.dot
	dot -Tps -o$@ $+

sarien.1: manpage.sgml
	docbook-to-man $+ > $@

specs: flowchart.ps
	@if [ -x /usr/bin/docbook2html ]; then		\
		[ -d agispecs ] || mkdir agispecs;	\
		(cd agispecs; docbook2html ../agispecs.sgml) \
	else						\
		sgmltools -b html agispecs.sgml;	\
	fi
	(cd agispecs;					\
	 convert ../flowchart.ps flowchart.png;		\
	 ln -sf book1.html index.html; 			\
	 cp ../images/specs/*png .)

