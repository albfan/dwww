# vim:ts=2:et
# common includes for dwww
#

getCurrentMakefileName := $(CURDIR)/$(lastword $(MAKEFILE_LIST))
override TOPDIR   := $(dir $(call getCurrentMakefileName))

override PACKAGE  := dwww

PATH            := /usr/bin:/usr/sbin:/bin:/sbin:$(PATH)

# build abstraction
install_file    := install -p -o root -g root -m 644
install_script  := install -p -o root -g root -m 755
install_dir     := install -d -o root -g root -m 755
install_link    := ln -sf
compress        := gzip -9f

prefix          := /usr
etcdir          := /etc
bindir          := $(prefix)/bin
sbindir         := $(prefix)/sbin
mandir          := $(prefix)/share/man
sharedir        := $(prefix)/share
pkgsharedir     := $(prefix)/share/$(PACKAGE)
perllibdir      := $(prefix)/share/perl5
docdir          := $(prefix)/share/doc/$(PACKAGE)
libdir          := /var/lib/$(PACKAGE)
nlsdir          := $(prefix)/share/locale
cachedir 	      := /var/cache/$(PACKAGE)
webdocrootdir 	:= /srv/http
webcgidir	      := $(prefix)/lib/cgi-bin
applicationsdir := $(prefix)/share/applications

PERL            = /usr/bin/perl
ifndef COMPILER_FLAGS_ALREADY_SET
export CC       ?= gcc
export CPPFLAGS += -DVERSION='"$(VERSION)"'
export CFLAGS   += -Wall -Wextra -Wstrict-prototypes -Wmissing-prototypes -Werror -g $(shell getconf LFS_CFLAGS) -L../publib/lib -I../publib/include
export LDFLAGS  +=
export LIBS     += -lpub -L../publib/lib -I../publib/include

export COMPILER_FLAGS_ALREADY_SET := 1
endif


# determine our version number
ifndef VERSION
  CHANGELOGFILE     := $(TOPDIR)/debian/changelog
  VERSION_COMMAND = cat debian/changelog | grep Version: | head -n 1 | sed -ne 's/.*Version: \(.*\) .*/\1/p'
  VERSION        = $(shell $(VERSION_COMMAND))
  export VERSION

  ifndef DISTIBUTOR
    ifneq (,$(findstring ubuntu,$(VERSION)))
      DISTRIBUTOR     := Ubuntu
    else
      DISTRIBUTOR     := `lsb_release -is`
    endif
  endif
  ifneq (Ubuntu,$(DISTRIBUTOR))
    DISTRIBUTOR   := Debian
  endif
  distributor     := $(shell echo $(DISTRIBUTOR) | tr "[:upper:]" "[:lower:]")

  export DISTRIBUTOR distributor
  unexport CDPATH

  ifdef DESTDIR
    ifneq ($(DESTDIR),$(abspath $(DESTDIR)))
      $(error DESTDIR "$(DESTDIR)" is not an absolute path)
    endif
    override ddirshort  :=  [DESTDIR]
    export ddirshort
  endif
endif

sdir            := $(CURDIR)
ifndef bdir
  ifneq (,$(ALL_TARGET))
    bdir        := _build
  else
    bdir        :=
  endif
endif

ifndef DIR
  DIR           := .
endif

XGETTEXT_COMMON_OPTIONS   := --msgid-bugs-address $(PACKAGE)@packages.debian.org  \
                            --package-name $(PACKAGE)                             \
                            --package-version $(VERSION)                          \
                            --copyright-holder='Robert Luberda <robert@debian.org>'


ifndef MAKE_VERBOSE
  MAKEFLAGS  += --no-print-directory
  AT         := @
else
  AT         :=
endif


#SHELL:=/bin/echo
# install(dir,files,mode=compress|script|notdir)
define install
  $(AT) set -e;                                                     \
  tgt="$1"; dir="$1"; files="$2";  prg="$(install_file)";           \
  doCompress=0;  bfile=""; what="file  ";                           \
  set -- $3;                                                        \
  while [ "$$1" ] ; do                                              \
    if [ "$$1" = "notdir" ] ; then                                  \
      dir="`dirname "$$dir"`";                                      \
      bfile="`basename "$$tgt"`";                                   \
    elif [ "$$1" = "compress" ] ; then                              \
      doCompress=1;                                                 \
    elif [ "$$1" = "script" ] ; then                                \
      prg="$(install_script)";                                      \
      what="script";                                                \
    else                                                            \
     echo "Unknown parameter for install $$files: $$1"  >&2;        \
     exit 1;                                                        \
    fi;                                                             \
    shift;                                                          \
  done;                                                             \
  [ -n "$$files" ] ||                                               \
    echo "installing dir    $(ddirshort)$$dir";                     \
  $(install_dir) "$(DESTDIR)/$$dir";                                \
  for file in $$files; do                                           \
      [ -n "$$bfile" ] && tgt="$$dir/$$bfile"  ||                   \
        tgt="$$dir/`basename "$$file"`";                            \
      echo "installing $$what $(ddirshort)$$tgt";                   \
      $$prg "$$file" "$(DESTDIR)/$$tgt";                            \
      if [ "$$doCompress" -eq 1 ] ; then                            \
        echo "compressing file  $(ddirshort)$$tgt";                 \
        $(compress) "$(DESTDIR)/$$tgt";                             \
      fi;                                                           \
  done;
endef

# install(link_target,files)
define install_links
  $(AT) set -e;                                                    \
  for file in $2; do                                               \
    echo "installing link   $(ddirshort)$$file";                   \
    $(install_dir) $(DESTDIR)/`dirname "$$file"`;                  \
    rm -f "$(DESTDIR)/$$file";                                     \
    $(install_link) "$1" "$(DESTDIR)/$$file";                      \
  done;
endef


define pochanged
  $(AT) set -e;                                                                 \
  [ ! -e $(1) ] && rename=1 || rename=0 ;                                       \
  if [ $$rename = 0 ] ; then                                                    \
    diff -q  -I'POT-Creation-Date:' -I'PO-Revision-Date:' $(1) $(2) >/dev/null  \
      ||  rename=1;                                                             \
  fi;                                                                           \
  [ $$rename = 1 ] && mv -f $(2) $(1) || rm -f $(2);                            \
  touch $(1)
endef

define recurse
  $(AT) set -e;                                                 \
  for dir in $(SUBDIRS); do                                     \
    $(MAKE) -C "$$dir" DIR="$(DIR)/$$dir" $(1);                 \
  done
endef

define podtoman
    $(AT) set -e;                                               \
    find $(1) -type f -name '*.pod' -path '*/man*'              \
    | while read file; do                                       \
      sed -ne '1i=encoding utf8\n' -e '/^=head1/,$$p'  < $$file \
      | pod2man --utf8 --section=8 --center="Debian"            \
        --release="$(PACKAGE) v$(VERSION)"                      \
        --date="$(DATE_EN)"                                     \
        --name="INSTALL-DOCS"                                   \
      > `dirname $$file`/`basename $$file .pod`;                \
  done
endef


all: $(ALL_TARGET) | $(bdir)
	$(call recurse,$@)

clean-local:

install-local: $(ALL_TARGET)

clean: clean-local
	@ test -z "$(bdir)" || echo "removing $(CURDIR)/$(bdir)"
	$(AT) test -z "$(bdir)" || rm -rf $(bdir)
	$(call recurse,$@)

install: install-local
	$(call recurse,$@)
	$(AFTER_INSTALL)

$(bdir):
	$(AT) test -z "$(bdir)" || mkdir -p $(bdir) 

$(bdir)/%: %.in | $(bdir) $(MAKEFILE_LIST)
# try to be compatible with the both sarge and sid versions of make
	$(AT) echo "creating $(CURDIR)/$@"
	$(AT) PERL5LIB="$(TOPDIR)/perl" $(PERL)  -e  \
	'exec ("'$(PERL)'", "-e", join("",@ARGV)) if $$#ARGV >-1; '\
	' $$|=1;                                    '\
	' use Debian::Dwww::ConfigFile;             '\
	' $$d=ReadConfigFile("/dev/null");          '\
	' $$v="";                                   '\
	' foreach $$k (sort keys %{$$d}) {          '\
	'   $$v.="\t$$k=\"$$d->{$$k}->{defval}\"\n" '\
	'     if $$k !~ /(TITLE)$$/;                '\
	' }                                         '\
	' while (<>) {                              '\
	'   s/#VERSION#/$(VERSION)/g;               '\
	'   s/#DISTRIBUTOR#/$(DISTRIBUTOR)/g;       '\
	'   s/#distributor#/$(distributor)/g;       '\
	'   s/^.*#DWWWVARS#.*$$/$$v/g;              '\
	'   print;                                  '\
	' }                                         '\
	  < $< > $@
	$(AT) touch -r $< $@

# debug
print-%:
	 $(AT) echo "$* is >>$($*)<<"

ifdef SHOW_DEPS
OLD_SHELL := $(SHELL)
SHELL = $(warning [$@ ($^)  \
 ($?)])$(OLD_SHELL)
endif

.PHONY: all clean install $(ALL_TARGET) clean-local install-local
.DEFAULT: all
