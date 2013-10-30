#
# gpart src Makefile
#
include ../inst.defs
include ../make.defs

CFLAGS+=-DVERSION=\"$(version)\"
ifeq ($(shell uname),Linux)
LIBS=-ldl
endif

ifdef DEBUG
CFLAGS+=-g
LDFLAGS+=-g
endif

ifdef GPART_LANG
CFLAGS+=-DGPART_LANG=\'$(GPART_LANG)\'
endif


mod=ext2 lswap fat bsddl ntfs hpfs s86dl minix rfs hmlvm qnx4 beos xfs
modobj=$(foreach m,$(mod),gm_$(m).o)
obj=gpart.o gmodules.o disku.o l64seek.o $(modobj)
src=$(obj:.o=.c)
hdr=$(wildcard *.h)
allsrc=Makefile $(src) $(hdr)

all: $(prg)

$(prg): $(obj)
	$(CC) -o $@ $(LDFLAGS) $(obj) $(LIBS)

install: $(prg)
	$(INSTALL) -d $(bindir)
	$(INSTALL) -s $(prg) $(bindir)

uninstall:
	$(RM) $(bindir)/$(prg)

clean:
	@$(RM) $(obj) $(prg) .depend

static:
	@$(MAKE) LDFLAGS=-static

rcscheck:
	@for f in $(allsrc); do \
		rcsdiff $$f >/dev/null 2>&1 || echo "Please checkin $$f"; \
	done

checkin:
	@for f in $(allsrc); do \
		rcsdiff $$f >/dev/null 2>&1 || ci -m"v$(version)" -l $$f; \
	done


.depend: $(src)
	@$(MAKEDEP) $(src) > $@

-include .depend
