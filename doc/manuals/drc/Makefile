
INCL = fig1.pic fig2.pic fig3.pic fig4.pic fig5.pic fig6.pic fig7.pic fig8.pic
SRCS = title.mm \
	chkart1.mm chkart2.mm chkart3.mm chkart4.mm chkart5.mm chkart6.mm \
	chkartA.mm chkartB.mm chkartC.mm chkartD.mm tail.mm

install: drc.pdf
	mv drc.pdf $(ICDPATH)/share/doc

drc.pdf: $(SRCS) $(INCL)
	cat ../lib/style/setup.mm $(SRCS) > tmp1.mm
	refer -e -n -P -p ../lib/bib tmp1.mm > tmp3.mm
	groff -ept -rB1 -rN2 -rf1 ../lib/tmac.m tmp3.mm > tmp6.ps 2> tmp4.sed
	sed -f tmp4.sed tmp3.mm > tmp5.mm
	groff -ept -rB1 -rN2      ../lib/tmac.m tmp5.mm > tmp6.ps
	psresize -PA4 -pletter tmp6.ps tmp7.ps
	ps2pdf tmp7.ps drc.pdf
	@rm -f tmp*

clean:
	rm -f drc.pdf tmp*

#EOF
