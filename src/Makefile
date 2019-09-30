
LIBS = eseLib libddm libfunc libX2ps

TARGETS= addproj cacdcmap cfun cldm cmsk csls \
	dali dbcat dbclean dblist device drc \
	exp flatten gds2 getlld getproc impcell man \
	macro makeboxh makeboxl makevln match mkpr \
	net2net nspice ocean GoWithTheFlow putdevmod \
	rmdb rmpr simeye simeye2 sls sls_exp slsmodgen \
	space helios process spice3 spock subresgen \
	utilities xcontrol xedif xnle xsls xspf xspice xvhdl

# Note that file Config.mk is used!
# Note that the order of the targets is important.
# Note that space/tecc must be done before "process".
# And that the ICDPATH must be set in the PATH.

install: testicd libs
	@for dir in $(TARGETS); do \
		echo "### Building targets in" $$dir; \
		make -C $$dir; \
	done

testicd:
	@if [ "$(ICDPATH)" == "" ]; then echo "ICDPATH not set"; exit 1; fi
	@echo ICDPATH=$(ICDPATH)

libs:
	@for dir in $(LIBS); do \
		echo "### Building targets in" $$dir; \
		make -C $$dir; \
	done

clean: testicd
	@for dir in $(LIBS) $(TARGETS); do \
		echo "### Cleaning targets in" $$dir; \
		make -C $$dir clean; \
	done

#EOF
