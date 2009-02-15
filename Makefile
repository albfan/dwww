# vim:ts=2
# Makefile for dwww.
# "@(#)dwww:$Id: Makefile 516 2009-01-15 19:51:36Z robert $"
#
ALL_TARGET  :=
SUBDIRS     := data man perl scripts src
include ./common.mk

docfiles    := README TODO

install-local:
	$(call msg,$@)
	$(call install,$(docdir),$(docfiles))
	$(call install,$(libdir)/html)
	$(call install,$(webdocrootdir))
	$(call install,$(cachedir))
