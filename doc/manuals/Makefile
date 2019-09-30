
DIRS =	dali drc examples funman maskdata ocean opmanual process \
	sls space3dman spaceman spacesubman spacetutor spock

install:
	for dir in $(DIRS); do \
		make -C $$dir; \
	done
clean:
	for dir in $(DIRS); do \
		make -C $$dir clean; \
	done

#EOF
