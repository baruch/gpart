#
# gpart Makefile
#
include inst.defs
include make.defs

all: gpart

gpart:
	$(MAKE) -C src
	$(MAKE) -C man

install:
	$(MAKE) -C src install
	$(MAKE) -C man install

uninstall:
	$(MAKE) -C src uninstall
	$(MAKE) -C man uninstall

clean:
	$(MAKE) -C src clean
	$(MAKE) -C man clean
