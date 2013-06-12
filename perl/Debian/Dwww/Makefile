# vim ts=2
# Makefile for dwww.
#
ALL_TARGET    := all-local
SUBDIRS       :=
include ../../../common.mk

generated     :=  $(bdir)/Version.pm
SOURCE_FILES  :=  $(sort $(wildcard *.pm) $(generated)) 
INSTALL_DIR   :=  $(subst $(realpath $(TOPDIR)/perl),,$(realpath $(CURDIR)))

all-local: $(generated)

install-local:
	$(call msg,$@)
	$(call install,$(perllibdir)/$(INSTALL_DIR),$(SOURCE_FILES))
