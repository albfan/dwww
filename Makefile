#
# Makefile for dwww.
# "@(#)dwww:$Id: Makefile,v 1.28 2006-01-14 11:06:21 robert Exp $"
#

VERSION	= $(shell dpkg-parsechangelog | sed -ne 's/^Version: *//p')

CC	= gcc
CFLAGS	= -Wall -Wstrict-prototypes -Wmissing-prototypes -DVERSION='"$(VERSION)"' 
LDFLAGS	=
LIBS	= -lpub
PERL	= /usr/bin/perl


ifeq (,$(findstring nodebug,$(DEB_BUILD_OPTIONS)))
  CFLAGS += -g
endif

ifneq (,$(findstring noopt,$(DEB_BUILD_OPTIONS)))
  CFLAGS += -O0
else
  CFLAGS += -O2
endif

ifeq (,$(findstring nostrip,$(DEB_BUILD_OPTIONS)))
  LDFLAGS += -s
endif


prefix		= debian/dwww

bindir		= $(prefix)/usr/bin
sbindir		= $(prefix)/usr/sbin
libdir		= $(prefix)/usr/share/dwww
htmldir		= $(prefix)/var/lib/dwww/html
docdir		= $(prefix)/usr/share/doc/dwww
etcdir		= $(prefix)/etc
etcdwwwdir 	= $(prefix)/etc/dwww
varlibdir  	= $(prefix)/var/lib/dwww
man1dir		= $(prefix)/usr/share/man/man1
man8dir		= $(prefix)/usr/share/man/man8
cachedir 	= $(prefix)/var/cache/dwww
webdocrootdir 	= $(prefix)/var/www
webcgidir	= $(prefix)/usr/lib/cgi-bin
perlmoddir=$(prefix)/usr/share/perl5/Debian/Dwww

perlmodules	= Dwww/*.pm
source_links	= Debian


links_end	= dwww-convert.dir.end dwww-convert.end dwww-find.end 

lib		= lib/[!Ceio]* lib/img/[!C]*
editorial	= lib/editorial/*.html
bin		= realpath dwww
cgi		= dwww.cgi
sbin		= dwww-convert dwww-build dwww-cache dwww-find \
		  dwww-quickfind  dwww-txt2html dwww-format-man \
		  dwww-build-menu dwww-index++ dwww-refresh-cache
doc		= README TODO
man1		= man/*.1
man8		= man/*.8

generated	= realpath dwww-cache dwww-quickfind dwww-txt2html \
		  Dwww/Version.pm functions.sh

perlprogs       = dwww-find dwww-build-menu dwww-index++
testprogs       := $(patsubst %,%.test,$(perlprogs))

all: $(source_links) $(generated)


%::%.in $(source_links)
	# try to be compatible with the both sarge and sid versions of make
	PERL5LIB="." $(PERL)  -e \
	'exec ("'$(PERL)'", "-e", join("",@ARGV)) if $$#ARGV >-1; '\
	'	$$|=1;					'\
	'	use Debian::Dwww::Initialize; 		'\
	'	$$d=&DwwwInitialize;			'\
	'	$$v="";					'\
	'	foreach $$k (sort keys %{$$d}) {	'\
	'		$$v.="\t$$k=\"$$d->{$$k}\"\n"	'\
	'			if $$k ne "DWWW_TITLE";	'\
	'	}					'\
	'	while (<>) {				'\
	'		s/#VERSION#/$(VERSION)/g;  	'\
	'		s/^.*#DWWWVARS#.*$$/$$v/g;	'\
	'		print;				'\
	'	}					'\
	  < $< > $@
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
	DEB_BUILD_OPTIONS=noopt,nostrip $(MAKE) all


Debian:
	ln -s . $@

clean:
	rm -f $(source_links)
	rm -f core *.o $(generated)
	rm -f $(testprogs)


installdirs:
	for i in $(prefix) $(prefix)/usr $(prefix)/var $(prefix)/var/lib \
	  $(etcdir) $(etcdwwwdir) $(libdir) $(varlibdir) $(bindir) $(sbindir) \
	  $(docdir) $(man1dir) $(man8dir) $(cachedir) \
	  $(webdocrootdir) $(webcgidir) $(perlmoddir) \
	  $(htmldir) ; \
	do  \
		echo "$$i"; \
		test -d $$i || install -p -d $$i; \
	done

install: installdirs
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
	install -p -m 0755 $(bin) $(bindir)
	install -p -m 0755 $(sbin) $(sbindir)
	install -p -m 0644 $(perlmodules) $(perlmoddir)
	install -p -m 0644 $(doc) $(docdir)
	install -p -m 0644 $(man1) $(man1dir)
	install -p -m 0644 $(man8) $(man8dir)

%.test::%
	rm -f $@
	echo "#!/usr/bin/perl" > $@
	echo "use lib \".\";"  >> $@
	sed -e 's/\/usr\/share\/dwww/lib/g'  $< >> $@
	chmod 555 $@

test: $(testprogs) $(source_links)

.PHONY: all debug clean install installdirs test
