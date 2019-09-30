
DIRS = bin comp_files do_files quartus ref_files sim_libs src_libs synth_libs vhdl
SRCS = sourceme install_op README.CellsLib README.install

# OPPROGPATH = $(ICDPATH)/opprog

install:
	@if [ "$(OPPROGPATH)" = "" ]; then echo "OPPROGPATH not set!"; exit 1; fi
	@if [ ! -d $(OPPROGPATH) ]; then echo mkdir -p $(OPPROGPATH); mkdir -p $(OPPROGPATH); fi
	cp $(SRCS) $(OPPROGPATH)
	@for dir in $(DIRS); do \
		echo "### Installing dir" $$dir; \
		cp -r $$dir $(OPPROGPATH); \
	done
	cd $(OPPROGPATH); ./install_op
clean:

#EOF
