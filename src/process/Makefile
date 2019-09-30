
PROCESS=$(ICDPATH)/share/lib/process

DIRS =	ami-c5n c3tu dimes01 dimes03 fishbone FreePDK45 \
	gatearray nmos octagon scmos_n scmos-orb-2 tsmc025

install:
	mkdir -p $(PROCESS)
	@if [ processlist -nt $(PROCESS)/processlist ]; then \
		echo "cp -f processlist $(PROCESS)";\
		cp -f processlist $(PROCESS);\
	fi
	@for dir in $(DIRS); do \
	  if [ ! -d $(PROCESS)/$$dir ]; then \
		echo "cp -r $$dir $(PROCESS)";\
		cp -r $$dir $(PROCESS);\
	  fi;\
	done
	@for dir in $(DIRS); do \
	    for sf in $$dir/*; do \
		if [ $$sf -nt $(PROCESS)/$$sf ]; then \
			echo "cp -f $$sf $(PROCESS)/$$dir";\
			cp -f $$sf $(PROCESS)/$$dir;\
		fi;\
	    done;\
	done
	@for dir in $(DIRS); do \
	    cd $(PROCESS)/$$dir;\
	    for sf in *.s; do \
		if [ $$sf -nt `basename $$sf .s`.t ]; then \
			echo "cd $$dir; tecc -m maskdata $$sf";\
			$(ICDPATH)/bin/tecc -m maskdata $$sf;\
		fi;\
	    done;\
	done
clean:

#EOF
