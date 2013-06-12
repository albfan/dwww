# vim:ts=2
# Makefile for dwww.
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
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

$(bdir)/%.o: %.c utils.h | $(bdir)
	$(CC) -c $(CPPFLAGS) $(CFLAGS) -o $@ $<
 

install-local:
	$(call install,$(sbindir),$(generated),script) 

.SECONDARY: $(patsubst %.c,$(bdir)/%.o,$(wildcard %.c)) 
