#
# gpart Makefile
#
include inst.defs
include make.defs

all: gpart

gpart:
	$(MAKE) -C src
	$(MAKE) -C man

install: install-object install-man

install-object:
	$(MAKE) -C src install

install-man:
	$(MAKE) -C man install

uninstall:
	$(MAKE) -C src uninstall
	$(MAKE) -C man uninstall

clean:
	$(MAKE) -C src clean
	$(MAKE) -C man clean

.PHONY: all gpart install install-object install-man uninstall clean
