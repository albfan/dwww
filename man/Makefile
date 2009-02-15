# vim:ts=2
# Makefile for dwww.
# "@(#)dwww:$Id: Makefile 531 2009-02-15 12:54:08Z robert $"
#
ALL_TARGET  :=
SUBDIRS     := 
include ../common.mk

man1mans	:= $(sort $(wildcard *.1))
man7mans	:= $(sort $(wildcard *.7))
man8mans	:= $(sort $(wildcard *.8))

install-local:
	$(call msg,$@)
	$(call install,$(mandir)/man1,$(man1mans),compress)
	$(call install,$(mandir)/man7,$(man7mans),compress)
	$(call install,$(mandir)/man8,$(man8mans),compress)
	$(call install_links,../man7/dwww.7.gz,$(mandir)/man5/dwww.conf.5.gz)
