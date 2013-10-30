#
# gpart man Makefile
#
include ../inst.defs
include ../make.defs


all: $(prg).$(manext)

$(prg).$(manext): $(prg).man
	@sed -e 's/%MANEXT%/$(manext)/g' \
	-e 's/%VERSION%/$(version)/g' < $< > $@

$(prg).$(manext).ps: $(prg).$(manext)
	@groff -man $< > $@

clean:
	@$(RM) $(prg).$(manext) $(prg).$(manext).ps

install: $(prg).$(manext)
	$(INSTALL) -d $(mandir)
	$(INSTALL) -d $(mandir)/man$(manext)
	$(INSTALL) -m 0444 $(prg).$(manext) $(mandir)/man$(manext)

uninstall:
	$(RM) $(mandir)/man$(manext)/$(prg).$(manext)
