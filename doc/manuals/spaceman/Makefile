
INCL = *.pic *.eps
SRCS = head.mm title.mm intro.mm usage.mm tecc.mm spice.mm appendix.mm tail.mm
SRCX = head.mm Xspace.mm

install: spaceman.pdf Xspaceman.pdf
	mv spaceman.pdf Xspaceman.pdf $(ICDPATH)/share/doc

spaceman.pdf: $(SRCS) $(INCL)
	cat ../lib/style/setup.mm $(SRCS) > tmp1.mm
	refer -e -n -P -p ../lib/bib tmp1.mm > tmp3.mm
	groff -ept -rs2 -rB1 -rN2 -rf1 ../lib/tmac.m tmp3.mm > tmp6.ps 2> tmp4.sed
	sed /tmp1/d tmp4.sed > tmp5.sed
	sed -f tmp5.sed tmp3.mm > tmp5.mm
	groff -ept -rs2 -rB1 -rN2      ../lib/tmac.m tmp5.mm > tmp6.ps
	psresize -PA4 -pletter tmp6.ps tmp7.ps
	ps2pdf tmp7.ps spaceman.pdf
	@rm -f tmp*

Xspaceman.pdf: $(SRCX)
	cat ../lib/style/setup.mm $(SRCX) > tmp1.mm
	refer -e -n -P -p ../lib/bib tmp1.mm > tmp3.mm
	groff -ept -rs2 -rB1 -rN2 -rf1 ../lib/tmac.m tmp3.mm > tmp6.ps 2> tmp4.sed
	sed -f tmp4.sed tmp3.mm > tmp5.mm
	groff -ept -rs2 -rB1 -rN2      ../lib/tmac.m tmp5.mm > tmp6.ps
	psresize -PA4 -pletter tmp6.ps tmp7.ps
	ps2pdf tmp7.ps Xspaceman.pdf
	@rm -f tmp*

clean:
	rm -f spaceman.pdf Xspaceman.pdf tmp*

#EOF
