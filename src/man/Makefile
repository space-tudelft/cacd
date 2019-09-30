
include ../Config.mk

TARGET_DOC=$(ICDPATH)/share/doc/manpages
TARGET_MAN=$(ICDPATH)/share/man

install: whatis
	mkdir -p $(TARGET_DOC)
	mkdir -p $(TARGET_MAN)
	@echo "### update $(TARGET_MAN)/whatis"
	@mv whatis $(TARGET_MAN)
	@for i in 1 3 4 5; do \
	  cd man$$i;\
	  mkdir -p $(TARGET_MAN)/cat$$i;\
	  echo "### update $(TARGET_MAN)/cat$$i/$$i.$$i";\
	  sh ../update $$i > $(TARGET_MAN)/cat$$i/$$i.$$i;\
	  for f in *.$$i; do \
	  if [ $$f -nt $(TARGET_MAN)/cat$$i/$$f ]; then \
		echo "### update $(TARGET_DOC)/$$f.pdf";\
		groff -t -Tps -c ../tmac.icd3.A4 $$f | ps2pdf - $(TARGET_DOC)/$$f.pdf;\
		echo "### update $(TARGET_MAN)/cat$$i/$$f";\
		groff -t -Tascii -P-c ../tmac.icd3.A4 $$f > $(TARGET_MAN)/cat$$i/$$f;\
	  fi;\
	  done;\
	  cd ..;\
	done
whatis:
	$(CC) getNAME.c -o getNAME
	@$(RM) whatis
	for i in 1 3 4 5; do cd man$$i; ../getNAME *.$$i >> ../whatis; cd ..; done
	@$(RM) getNAME
clean:
	$(RM) getNAME whatis

#EOF
