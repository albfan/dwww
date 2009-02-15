# vim:ts=2
# Makefile for dwww.
# "@(#)dwww:$Id: Makefile 522 2009-01-19 20:30:47Z robert $"
#
ALL_TARGET  := all-local
SUBDIRS     := img mozilla templates
include ../common.mk

generated			 :=	$(bdir)/dwww.css
templatefiles  := $(sort $(wildcard mozilla-dwww.*) swish++.conf $(generated))

all-local: $(generated) | $(bdir)
	$(call msg,$@)

install-local:
	$(call msg,$@)
	$(call install,$(pkgsharedir),$(templatefiles)) 
	$(call install,$(etcdir),apache.conf) 
	$(call install,$(applicationsdir),dwww.desktop) 

