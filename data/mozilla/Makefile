# vim:ts=2
# Makefile for dwww.
# "@(#)dwww:$Id: Makefile 530 2009-02-15 12:05:47Z robert $"
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

install-local:
	$(call msg,$@)
	$(call install,$(pkgsharedir)/mozilla,$(files))
	$(foreach browser,$(browsers),$(foreach file,$(files),$(call install_links,../../$(PACKAGE)/mozilla/$(file),$(sharedir)/$(browser)/searchplugins/$(file))))
