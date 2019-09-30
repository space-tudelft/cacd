
INCL = *.pic *.ps *.eps
SRCS = head.mm title.mm intro.mm usage.mm tech.mm capcomp.mm example.mm problems.mm app_cap.mm tail.mm

install: spacecap3dman.pdf
	mv spacecap3dman.pdf $(ICDPATH)/share/doc

spacecap3dman.pdf: $(SRCS) $(INCL)
	cat ../lib/style/setup.mm $(SRCS) > tmp1.mm
	refer -e -n -P -p ../lib/bib tmp1.mm > tmp3.mm
	groff -ept -rs2 -rB1 -rN2 -rf1 ../lib/tmac.m tmp3.mm > tmp6.ps 2> tmp4.sed
	sed -f tmp4.sed tmp3.mm > tmp5.mm
	groff -ept -rs2 -rB1 -rN2      ../lib/tmac.m tmp5.mm > tmp6.ps
	psresize -PA4 -pletter tmp6.ps tmp7.ps
	ps2pdf tmp7.ps spacecap3dman.pdf
	@rm -f tmp*

clean:
	rm -f spacecap3dman.pdf tmp*

#EOF
