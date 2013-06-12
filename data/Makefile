# vim:ts=2
# Makefile for dwww.
#
ALL_TARGET  := all-local
SUBDIRS     := img mozilla templates
include ../common.mk

generated			 :=	$(bdir)/dwww.css
templatefiles  := $(sort $(wildcard mozilla-dwww.*) swish++.conf $(generated))

all-local: $(generated) | $(bdir)

install-local:
	$(call install,$(pkgsharedir),$(templatefiles)) 
	$(call install,$(etcdir)/apache2/conf-available/dwww.conf,apache.conf,notdir)
	$(call install,$(applicationsdir),dwww.desktop) 
