# vim:ts=2
# Makefile for dwww.
# "@(#)dwww:$Id: Makefile 555 2011-02-08 22:31:39Z robert $"
#
ALL_TARGET  :=
SUBDIRS     := data man perl scripts src
include ./common.mk

docfiles    := README TODO

install-local:
	$(call msg,$@)
	$(call install,$(docdir),$(docfiles))
	$(call install,$(libdir))
	$(call install,$(webdocrootdir))
	$(call install,$(cachedir))
