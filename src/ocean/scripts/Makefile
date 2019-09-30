
include ../../Config.mk

FILES = dofunc getldm ICELLS kissis mksls playout rmsdflock seatail vspace
LINKS = mkopr mkvopr mkepr mkropr tutorial

install:
	for link in $(LINKS); do \
	  ln -s ICELLS $$link; \
	done
	cp $(FILES) $(BINPATH)
	mv $(LINKS) $(BINPATH)
	for file in $(FILES); do \
	  chmod 755 $(BINPATH)/$$file; \
	done
clean:

#EOF
