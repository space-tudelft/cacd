
DIRS = howto manuals memos notes

install:
	@if [ "$(ICDPATH)" == "" ]; then echo "ICDPATH not set"; exit 1; fi
	@if [ "$(TEXINPUTS)" == "" ]; then echo "TEXINPUTS not set"; exit 1; fi
	mkdir -p $(ICDPATH)/share/doc
	for dir in $(DIRS); do \
		if [ -d $$dir ]; then make -C $$dir; fi; \
	done
clean:
	for dir in $(DIRS); do \
		if [ -d $$dir ]; then make -C $$dir clean; fi; \
	done

#EOF
