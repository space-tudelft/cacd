
CWD = $(shell pwd)

install: init
	@if [ "$(ICDPATH)" != "$(CWD)" ]; then \
		echo "ICDPATH != CWD";\
		if [ ! -d $(ICDPATH)/share ]; then \
			echo "cp -rp share $(ICDPATH)";\
			cp -rp share $(ICDPATH);\
		fi;\
	else \
		echo "ICDPATH == CWD";\
	fi
	mkdir -p $(ICDPATH)/share/lib
	@echo "### Install src"
	echo "setenv ICDPATH $(ICDPATH)" > $(ICDPATH)/sourceme.csh
	echo 'setenv PATH $$ICDPATH/bin:$$PATH' >> $(ICDPATH)/sourceme.csh
	echo "export ICDPATH=$(ICDPATH)" > $(ICDPATH)/sourceme.bash
	echo 'export PATH=$$ICDPATH/bin:$$PATH' >> $(ICDPATH)/sourceme.bash
	make -C src
	@echo "### Ready"
init:
	@echo "### Install init"
	@if [ "$(ICDPATH)" == "" ]; then echo "ICDPATH not set"; exit 1; fi
	@echo ICDPATH=$(ICDPATH)
	mkdir -p $(ICDPATH)/bin
	mkdir -p $(ICDPATH)/lib
uninstall:
	@if [ "$(ICDPATH)" == "" ]; then echo "ICDPATH not set"; exit 1; fi
	rm -rf $(ICDPATH)/bin
	rm -rf $(ICDPATH)/lib
	rm -rf $(ICDPATH)/opprog
clean:
	@echo "### Clean src"
	make -C src clean
	@echo "### Ready"

#EOF
