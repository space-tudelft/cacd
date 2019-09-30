
INCL = *.ps *.pic
SRC1 = head.mm title1.mm body1.mm tail.mm
SRC2 = head.mm title2.mm body2.mm tail.mm

install:   spacetutor.pdf spacetutor_helios.pdf
	mv spacetutor.pdf spacetutor_helios.pdf $(ICDPATH)/share/doc

spacetutor.pdf: $(SRC1) $(INCL)
	cat ../lib/style/setup.mm $(SRC1) > tmp1.mm
	refer -e -n -P -p ../lib/bib tmp1.mm > tmp3.mm
	groff -ept -rB1 -rN2 -rf1 ../lib/tmac.m tmp3.mm > tmp6.ps 2> tmp4.sed
	sed -f tmp4.sed tmp3.mm > tmp5.mm
	groff -ept -rB1 -rN2      ../lib/tmac.m tmp5.mm > tmp6.ps
	psresize -PA4 -pletter tmp6.ps tmp7.ps
	ps2pdf tmp7.ps $@
	@rm -f tmp*

spacetutor_helios.pdf: $(SRC2) $(INCL)
	cat ../lib/style/setup.mm $(SRC2) > tmp1.mm
	refer -e -n -P -p ../lib/bib tmp1.mm > tmp3.mm
	groff -ept -rB1 -rN2 -rf1 ../lib/tmac.m tmp3.mm > tmp6.ps 2> tmp4.sed
	sed -f tmp4.sed tmp3.mm > tmp5.mm
	groff -ept -rB1 -rN2      ../lib/tmac.m tmp5.mm > tmp6.ps
	psresize -PA4 -pletter tmp6.ps tmp7.ps
	ps2pdf tmp7.ps $@
	@rm -f tmp*

clean:
	rm -f *.pdf tmp*

#EOF
