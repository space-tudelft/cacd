
include ../Config.mk

SPICE_LIB = $(ICDPATH)/share/lib/spice3

install: make_64bit
	@mkdir -p $(BINPATH)
	cp ./src/bin/nutmeg   $(BINPATH)
	cp ./src/bin/sconvert $(BINPATH)
	cp ./src/bin/spice3   $(BINPATH)
	rm -rf   $(SPICE_LIB)
	mkdir -p $(SPICE_LIB)/helpdir
	mkdir -p $(SPICE_LIB)/scripts
	cp ./lib/news              $(SPICE_LIB)
	cp ./lib/helpdir/spice.txt $(SPICE_LIB)/helpdir
	./src/bin/makeidx          $(SPICE_LIB)/helpdir/spice.txt
	cp ./lib/scripts/setplot   $(SPICE_LIB)/scripts
	cp ./lib/scripts/spinit    $(SPICE_LIB)/scripts

make_64bit:
	./util/build linux64
make_32bit:
	./util/build linux32
clean:
	./util/build clean

#EOF
