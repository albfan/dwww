# vim:ts=2
# Makefile for dwww.
# "@(#)dwww:$Id: Makefile 516 2009-01-15 19:51:36Z robert $"
#
ALL_TARGET  := all-local
SUBDIRS     :=
include ../common.mk



generated	= $(bdir)/dwww-cache \
		  $(bdir)/dwww-quickfind \
		  $(bdir)/dwww-txt2html 

perlprogs       = dwww-find dwww-build-menu dwww-index++
testprogs       := $(patsubst %,%.test,$(perlprogs))

all-local: $(generated)



$(bdir)/%: $(bdir)/%.o $(bdir)/utils.o | $(bdir)
	$(call msg,$@,Linking)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

$(bdir)/%.o: %.c utils.h | $(bdir)
	$(call msg,$@,Compiling)
	$(CC) -c $(CFLAGS) -o $@ $<
 

install-local:
	$(call msg,$@)
	$(call install,$(sbindir),$(generated),script) 

.SECONDARY: $(patsubst %.c,$(bdir)/%.o,$(wildcard %.c)) 
