
include ../../Config.mk

CFLAGS=-Wall -Wno-unused $(OCFLAGS) -I../../..

MODGENDIR = $(ICDPATH)/share/lib/slsmodgen

SF1 =	README
SF2 =	collect dospice resis \
	docol_ex header_ex models_ex offsets_ex values_ex

install: files delay solve

files:
	if [ ! -d $(MODGENDIR) ]; then mkdir $(MODGENDIR); fi
	if [ ! -d $(MODGENDIR)/bin ]; then mkdir $(MODGENDIR)/bin; fi
	rm -f $(MODGENDIR)/$(SF1)
	cp $(SF1) $(MODGENDIR)
	if [   -d $(MODGENDIR)/src ]; then rm -fr $(MODGENDIR)/src; fi
	mkdir     $(MODGENDIR)/src
	cp $(SF2) $(MODGENDIR)/src

delay: delay.c
	$(CC) $(CFLAGS) $^ -o $@
	mv $@ $(MODGENDIR)/bin

solve: solve.c
	$(CC) $(CFLAGS) $^ -o $@
	mv $@ $(MODGENDIR)/bin

#EOF
