# vim:ts=2
# Makefile for dwww.
#
ALL_TARGET  :=
SUBDIRS     := data man perl scripts src
include ./common.mk

docfiles    := README TODO

install-local:
	$(call install,$(docdir),$(docfiles))
	$(call install,$(libdir))
	$(call install,$(webdocrootdir))
	$(call install,$(cachedir))
