# Sarien toplevel Makefile
# $Id$

# DIST		distribution package name
# DFILES	standard distribution files 
# DDIRS		standard distribution directories

include Version

PKG	= sarien
XCFLAGS	= -Iloaders/include
DIST	= $(PKG)-$(VERSION)
DFILES	= configure configure.in Makefile Makefile.wat Makefile.dj \
	  Makefile.tc Makefile.dc Sarien.make Sarien.DICE Rules.in \
	  Rules.dj Rules.wat Version README
DDIRS	= VC++6.0 OSX-ProjectBuilder eVC3 doc scripts bin debian etc lib \
	  src rulesets
CFILES	=
DCFILES	= Rules config.log config.status config.cache

all:
	for i in $(DDIRS); do \
		(cd $$i; [ -f $(MAKEFILE) ] && $(MAKE) $@) \
	done; true
	
include Rules

clean::
	rm -f config.cache config.log

distclean::
	rm -f Rules

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

bz2:
	zcat $(DIST).tar.gz | bzip2 > $(DIST).tar.bz2

bindist:
	$(MAKE) _bindist1 CONFIGURE="./configure"

binpkg: bindist $(PORTS)

_bindist1:
	rm -Rf $(DIST)
	gzip -dc $(DIST).tar.gz | tar xvf -
	cd $(DIST); $(CONFIGURE); $(MAKE) _bindist2
	rm -Rf $(DIST)
	sync

chkoldver:
	@if [ "$(OLDVER)" = "" ]; then \
	    echo "parameter missing: OLDVER=<old_version>"; \
	    false; \
	fi

diff: chkoldver
	@if [ "$(OLDVER)" != "none" ]; then \
	    echo "Creating diff from $(OLDVER) to $(VERSION)"; \
	    rm -Rf $(PKG)-$(OLDVER) $(PKG)-$(VERSION); \
	    tar xzf $(PKG)-$(OLDVER).tar.gz; \
	    tar xzf $(PKG)-$(VERSION).tar.gz; \
	    diff -rud --new-file $(PKG)-$(OLDVER) $(PKG)-$(VERSION) | \
		gzip -9c > $(PKG)-$(VERSION).diff.gz; \
	    rm -Rf $(PKG)-$(OLDVER) $(PKG)-$(VERSION); \
	    sync; \
	fi

Rules: Rules.in
	./config.status

$(OBJS): Makefile

