# vim:ts=2
# Makefile for dwww.
# "@(#)dwww:$Id: Makefile 521 2009-01-16 23:34:26Z robert $"
#
ALL_TARGET  :=
SUBDIRS     :=
include ../../common.mk

templatefiles  := $(sort $(wildcard *.*))
dwwwendlinks   := dwww-convert.dir.end dwww-convert.end dwww-find.end



install-local:
	$(call msg,$@)
	$(call install,$(pkgsharedir)/templates,$(templatefiles))
	$(call install_links,dwww.end,$(patsubst %,$(pkgsharedir)/templates/%,$(dwwwendlinks)))

