# vim:ts=2
# Makefile for dwww.
#
ALL_TARGET  :=
SUBDIRS     := 
include ../../common.mk

files  := dwww.src dwww.png
ifeq (Ubuntu,$(DISTRIBUTOR))
  browsers	:= firefox seamonkey
else
  browsers	= iceweasel iceape
endif

install-local: install-local-files install-local-links

install-local-files:
	$(call install,$(pkgsharedir)/mozilla,$(files))

install-local-links: override ATT := $(AT)
install-local-links: override AT  :=
install-local-links:
	$(ATT) $(foreach browser,$(browsers),$(foreach file,$(files),$(call install_links,../../$(PACKAGE)/mozilla/$(file),$(sharedir)/$(browser)/searchplugins/$(file))))

.PHONY: install-local-files  install-local-links
