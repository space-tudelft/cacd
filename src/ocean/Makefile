
TARGETS=libocean libseadif nelsea blif2sls celllibs colaps esea gnarp \
	layflat madonna makebus scripts seadali seedif trout

install: testicd
	@for dir in $(TARGETS); do \
		echo "### Building targets in" $$dir; \
		make -C $$dir; \
	done

testicd:
	@if [ "$(ICDPATH)" == "" ]; then echo "ICDPATH not set"; exit 1; fi
	@echo ICDPATH=$(ICDPATH)

clean:
	@for dir in $(LIBS) $(TARGETS); do \
		echo "### Cleaning targets in" $$dir; \
		make -C $$dir clean; \
	done

#EOF
