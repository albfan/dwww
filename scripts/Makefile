# vim:ts=2
# Makefile for dwww.
#
ALL_TARGET  := all-local
SUBDIRS     :=
include ../common.mk



generated     = $(bdir)/functions.sh

binfiles      := dwww
sbinfiles     := dwww-build   \
                 dwww-build-menu  \
                 dwww-convert   \
                 dwww-find    \
                 dwww-format-man  \
                 dwww-index++   \
                 dwww-refresh-cache
libfiles  := $(bdir)/functions.sh


all-local: $(generated)


install-local:
	$(call msg,$@)
	$(call install,$(sbindir),$(sbinfiles),script) 
	$(call install,$(bindir),$(binfiles),script)
	$(call install,$(webcgidir)/dwww,dwww.cgi,notdir script)
	$(call install,$(pkgsharedir),$(libfiles))

