VERSION		= 0.9.0

CC		= gcc
CFLAGS		= -c -g -O2 -DHAVE_POPEN=1 -DHAVE_MKSTEMP=1 -DHAVE_FNMATCH=1 -DHAVE_LIBSDL=1 -DVERSION='\\\"$(VERSION)\\\"'
LD		= gcc
LDFLAGS		= -o $@ 
LIBS		= -lSDL 
RANLIB		= ranlib
INSTALL		= /usr/bin/install -c
DESTDIR		=
prefix		= /usr/local
exec_prefix 	= /usr/local
datarootdir	= ${prefix}/share
BINDIR		= ${exec_prefix}/bin
LIBDIR		= ${exec_prefix}/lib
MANDIR		= ${datarootdir}/man/man1
SHELL		= /bin/sh

DIST		= sarien-$(VERSION)
DFILES		= configure configure.in Makefile Makefile.wat Makefile.dj \
		  Rules.in \
		  README
DDIRS		= doc scripts bin etc lib src
V		= 0
LIB		= libagi.a
BIN		= sarien

all: $(LIB) $(BIN)

include src/Makefile

GCOBJS = $(OBJS:.o=.gco)

.SUFFIXES: .c .o .a .in .gco .gcda .gcno

.c.o:
	@CMD='$(CC) $(CPPFLAGS) $(CFLAGS) -o $*.o $<'; \
	if [ "$(V)" -gt 0 ]; then echo $$CMD; else echo CC $*.o ; fi; \
	eval $$CMD

dummy:

depend:       
	@echo Building dependencies...
	@$(CC) $(CFLAGS) $(XCFLAGS) -M $(OBJS:.o=.c) $(XDEPS:.o=.c) >$@

$(OBJS): $(MAKEFILE)

$(LIB): src/$(LIB)

$(BIN): src/$(BIN)

src/$(LIB): $(OBJS)
	@CMD='$(AR) r $@ $(OBJS)'; \
	if [ "$(V)" -gt 0 ]; then echo $$CMD; else echo AR $@ ; fi; \
	eval $$CMD
	$(RANLIB) $@

src/$(BIN): $(CLI_OBJS) $(LIB)
	@CMD='$(LD) $(LDFLAGS) -o $@ $(CLI_OBJS) $(LIBS) -Lsrc -lagi'; \
	if [ "$(V)" -gt 0 ]; then echo $$CMD; else echo LD $@ ; fi; \
	eval $$CMD

clean:
	@rm -f $(OBJS) src/$(BIN) src/$(LIB)
	@rm -f $(GCOBJS) $(OBJS:.o=.gcno) $(OBJS:.o=.gcda)

dist: all-docs dist-prepare dist-subdirs

dist-prepare:
	./config.status
	rm -Rf $(DIST) $(DIST).tar.gz
	mkdir -p $(DIST)
	cp -RPp $(DFILES) $(DIST)/

dist-subdirs: $(addprefix dist-,$(DDIRS))
	chmod -R u+w $(DIST)/*
	tar cvf - $(DIST) | gzip -9c > $(DIST).tar.gz
	rm -Rf $(DIST)
	ls -l $(DIST).tar.gz

distcheck:
	rm -Rf $(DIST)
	tar xf $(DIST).tar.gz
	(cd $(DIST); ./configure --enable-static --prefix=`pwd`/test-install; make; make check; make install; find test-install)

configure: configure.in
	autoconf

check: test

dust:
	@echo "<cough, cough>"

# Extra targets:
# 'dist' prepares a distribution package
# 'mark' marks the last RCS revision with the package version number
# 'whatsout' lists the locked files
# 'diff' creates a diff file
# 'rpm' generates an RPM package
# 'nsis' generates a Win32 NSIS installer

dist:
	rm -Rf $(DIST) $(DIST).tar.gz
	mkdir $(DIST)
	$(MAKE) DISTDIR=$(DIST) subdist
	chmod -R u+w $(DIST)/*
	tar cvf - $(DIST) | gzip -9c > $(DIST).tar.gz
	rm -Rf $(DIST)
	./config.status
	sync
	ls -l $(DIST).tar.gz

$(OBJS): Makefile

