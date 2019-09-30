
INCL = dali_fig2.ps um_coor.mm um_grid.mm um_wind.mm
SRCS = title.mm um_intro.mm um_main.mm um_dm.mm um_box.mm um_term.mm um_inst.mm um_chk.mm um_aux.mm um_vis.mm

install: dali.pdf
	mv dali.pdf $(ICDPATH)/share/doc

dali.pdf: $(SRCS) $(INCL)
	cat ../lib/style/setup.mm $(SRCS) > tmp1.mm
	refer -e -n -P -p ../lib/bib tmp1.mm > tmp3.mm
	groff -ept -rs2 -rN2 -rf1 ../lib/tmac.m tmp3.mm > tmp6.ps 2> tmp4.sed
	sed -f tmp4.sed tmp3.mm > tmp5.mm
	groff -ept -rs2 -rN2      ../lib/tmac.m tmp5.mm > tmp6.ps
	psresize -PA4 -pletter tmp6.ps tmp7.ps
	ps2pdf tmp7.ps dali.pdf
	@rm -f tmp*

clean:
	rm -f dali.pdf tmp*

#EOF
