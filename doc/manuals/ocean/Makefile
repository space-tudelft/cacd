
DIRS = library ocean-manual

install:
	for dir in $(DIRS); do \
		make -C $$dir; \
	done
	rm -f $(ICDPATH)/share/doc/SIS_paper.pdf
	cp -p SIS_paper.pdf $(ICDPATH)/share/doc

clean:
	for dir in $(DIRS); do \
		make -C $$dir clean; \
	done

#EOF
