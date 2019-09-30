
TARGETS = dimcheck drcscripts dubcheck nbool

install:
	for dir in $(TARGETS); do \
	  make -C $$dir; \
	done
clean:
	for dir in $(TARGETS); do \
	  make -C $$dir clean; \
	done

#EOF
