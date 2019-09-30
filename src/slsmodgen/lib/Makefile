
include ../../Config.mk

MODGENDIR = $(ICDPATH)/share/lib/slsmodgen

install:
	if [ ! -d $(MODGENDIR) ]; then mkdir $(MODGENDIR); fi
	if [   -d $(MODGENDIR)/lib ]; then rm -fr $(MODGENDIR)/lib; fi
	mkdir     $(MODGENDIR)/lib
	cp *.spc  $(MODGENDIR)/lib

#EOF
