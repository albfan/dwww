#
# Makefile for dwww.
# "@(#)dwww:$Id: Makefile,v 1.16 2002/04/25 06:41:54 robert Exp $"
#

VERSION=$(shell dpkg-parsechangelog | sed -ne 's/^Version: *//p')

CC=gcc
CFLAGS=-Wall -O2 -DVERSION='"$(VERSION)"'
LDFLAGS=
LIBS=-lpub


ifneq (,$(findstring debug,$(DEB_BUILD_OPTIONS)))
  CFLAGS += -g
endif
ifeq (,$(findstring nostrip,$(DEB_BUILD_OPTIONS)))
  LDFLAGS += -s
endif


prefix=debian/dwww

bindir=$(prefix)/usr/bin
sbindir=$(prefix)/usr/sbin
libdir=$(prefix)/usr/share/dwww
varlibdir=$(prefix)/var/lib/dwww
htmldir=$(prefix)/var/lib/dwww/html
docdir=$(prefix)/usr/share/doc/dwww
etcdir=$(prefix)/etc
etcdwwwdir=$(prefix)/etc/dwww
man1dir=$(prefix)/usr/share/man/man1
man8dir=$(prefix)/usr/share/man/man8
crondir=$(prefix)/etc/cron.daily
#cgidir=$(prefix)/cgi-bin
cachedir=$(prefix)/var/cache/dwww
menudir=$(prefix)/etc/menu-methods
webdocrootdir=$(prefix)/var/www
webcgidir=$(prefix)/usr/lib/cgi-bin


links_end=dwww-convert.dir.end dwww-convert.end dwww-find.end man-begins-with.end \
	  man-in-section.end packagedoc.end

lib=lib/[!Ceio]* lib/img/[!C]*
editorial=lib/editorial/*.html
varlib=
bin=realpath dwww
cgi=dwww.cgi
sbin=dwww-convert dwww-build dwww-cache dwww-find dwww-quickfind \
	dwww-txt2html dwww-format-man 
doc=README TODO
man1=	realpath.1 dwww.1
man8=	dwww-build.8 dwww-cache.8 dwww-convert.8 dwww-find.8 \
	dwww-format-man.8 dwww-quickfind.8 dwww-txt2html.8 dwww.8
cron=dwww-daily
menumethods=menu-methods

all: realpath dwww-cache dwww-quickfind dwww-txt2html functions.sh

functions.sh:functions.sh.in
	sed -e 's/#VERSION#/$(VERSION)/g' < $< > $@
	touch -r $< $@

dwww-txt2html: dwww-txt2html.o utils.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

realpath: realpath.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

dwww-cache: dwww-cache.o utils.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)
	
dwww-quickfind: dwww-quickfind.o utils.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)
	
debug:
	DEB_BUILD_OPTIONS=debug,nostrip $(MAKE) all

clean:
	rm -f core realpath dwww-cache dwww-quickfind dwww-txt2html functions.sh *.o


install:
	for i in $(prefix) $(prefix)/usr $(prefix)/var $(prefix)/var/lib \
	  $(etcdir) $(etcdwwwdir) $(libdir) $(varlibdir) $(bindir) $(sbindir) \
	  $(docdir) $(man1dir) $(man8dir) $(crondir) $(cachedir) \
	  $(webdocrootdir) $(webcgidir) \
	  $(htmldir) $(menudir); \
	do  \
		echo "$$i"; \
		test -d $$i || install -p -d $$i; \
	done

	install -p -m 0644 $(lib) $(libdir)
	for i in $(links_end) ; do \
		ln -sv dwww.end $(libdir)/$$i; \
	done

	install -p -m 0644 functions.sh $(libdir)
#	chmod a+x $(libdir)/dwww.cgi
#	rm -f $(cgidir)/dwww
#	ln -s $(libdir)/dwww.cgi $(cgidir)/dwww
#	ln -s /usr/lib/dwww/dwww.cgi $(webcgidir)/dwww
	install -p -m 0755 $(cgi) $(webcgidir)/dwww
	ln -s /var/lib/dwww/html $(webdocrootdir)/dwww
	install -p $(bin) $(bindir)
	install -p $(sbin) $(sbindir)
	install -p -m 0644 $(doc) $(docdir)
	install -p -m 0644 $(man1) $(man1dir)
	install -p -m 0644 $(man8) $(man8dir)
	install -p $(cron) $(crondir)/dwww

.PHONY: all debug clean install
