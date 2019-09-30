
INCL = mfig1.pic mfig2.pic mfig3.pic mfig4.pic mfig6.pic mfig8.pic mfig13.pic mfig16.pic
SRCS = title.mm chap1.mm chap2.mm chap3a.mm chap3b.mm chap3c.mm chap3d.mm chap4.mm tail.mm

install: funman.pdf
	mv funman.pdf $(ICDPATH)/share/doc

funman.pdf: $(SRCS) $(INCL)
	cat ../lib/style/setup.mm $(SRCS) > tmp1.mm
	refer -e -n -P -p ../lib/bib tmp1.mm > tmp3.mm
	groff -ept -rN2 -rf1 ../lib/tmac.m tmp3.mm > tmp6.ps
	psresize -PA4 -pletter tmp6.ps tmp7.ps
	ps2pdf tmp7.ps funman.pdf
	@rm -f tmp*

clean:
	rm -f funman.pdf tmp*

#EOF
