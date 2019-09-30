
INCL = n3.pic n4.pic n5.pic
SRCS = title.mm man.mm

install: sls.pdf
	mv sls.pdf $(ICDPATH)/share/doc

sls.pdf: $(SRCS) $(INCL)
	cat ../lib/style/setup.mm $(SRCS) > tmp1.mm
	refer -e -n -P -p ../lib/bib tmp1.mm > tmp3.mm
	groff -ept -rN2 -rf1 ../lib/tmac.m tmp3.mm > tmp6.ps
	psresize -PA4 -pletter tmp6.ps tmp7.ps
	ps2pdf tmp7.ps sls.pdf
	@rm -f tmp*

clean:
	rm -f sls.pdf tmp*

#EOF
