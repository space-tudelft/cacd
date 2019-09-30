
install: examples.dvi
	@touch groff.enc
	dvips -t a4 -o examples.ps examples.dvi
	ps2pdf examples.ps
	mv examples.pdf $(ICDPATH)/share/doc
	@rm -f examples.ps groff.enc

examples.dvi: *.tex */*.tex *.sty
	./atp ny9t/demo.tex.atp > ny9t/demo.tex
	latex examples.tex
	latex examples.tex
	@rm -f *.aux */*.aux ny9t/demo.tex
	@rm -f examples.log examples.out examples.toc

clean:
	rm -f examples.dvi examples.pdf examples.ps
	rm -f examples.log examples.out examples.toc
	rm -f *.aux */*.aux ny9t/demo.tex dvi.log

#EOF
