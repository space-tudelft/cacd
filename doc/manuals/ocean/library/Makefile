
install: library.pdf
	mv library.pdf $(ICDPATH)/share/doc/ocean_library.pdf

library.pdf: *.tex
	latex library.tex
	latex library.tex
	dvips library.dvi
	ps2pdf library.ps
	rm -f *.aux *.dvi *.log *.ps *.toc

clean:
	rm -f *.aux *.dvi *.log *.ps *.toc *.pdf

#EOF
