
NOTES =	an9801 an9901 an9902 an9903 an9904 an9905 \
	an0001 an0002 an0003 an0005 an0006 an0007 \
	an0101 an0102 an0103 \
	an0201 an0201e1 an0201e2 an0201e3 an0201e4 an0202 an0203 an0204 \
	an0301 an0302 an0303 an0304 an0305 an0306 an0307 an0308 an0309 an0310 an0311 \
	an0401 an0402 an0403 an0404 an0405 an0406 an0407 an0408 an0409 an0410 an0411 an0412 an0413 \
	an0501 an0502 an0503 an0504 an0505 an0506 an0506x \
	an0601 an0603 an0701 an0702 \
	an0803 an0804 an0901 an0902 an0903 \
	an1001 an1002 an1003 \
	an1101 an1102 an1103 an1104 an1105 an1106 an1107 an1108 an1109 \
	an1201 an1202 an1401 an1403 an1404 an1405 an1501

install:
	@rm -f ../lib an0201e? an0506x
	@ln -s manuals/lib ../lib
	@ln -s an0201 an0201e1
	@ln -s an0201 an0201e2
	@ln -s an0201 an0201e3
	@ln -s an0201 an0201e4
	@ln -s an0506 an0506x
	@rm -rf $(ICDPATH)/share/doc/notes
	@mkdir -p $(ICDPATH)/share/doc/notes
	@for n in $(NOTES); do \
		echo "### Build $$n.pdf"; \
		cat ../lib/style/setup.mm $$n/head.mm $$n/$$n.mm > tmp1.mm; \
		refer -e -n -P -p ../lib/bib tmp1.mm > tmp3.mm; \
		groff -ept -rB1 -rN2 ../lib/tmac.m tmp3.mm > tmp3.ps; \
		psresize -PA4 -pletter tmp3.ps tmp4.ps; \
		ps2pdf tmp4.ps $$n.pdf; \
		mv $$n.pdf $(ICDPATH)/share/doc/notes; \
	done
	@rm -f ../lib an0201e? an0506x tmp*
clean:
	rm -f ../lib an0201e? an0506x tmp* *.pdf

#EOF
