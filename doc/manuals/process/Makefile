
install: process.pdf
	mv process.pdf $(ICDPATH)/share/doc

process.pdf: *.tex
	latex main.tex
	latex main.tex
	dvips -t a4 -o main.ps main.dvi
	ps2pdf main.ps process.pdf
	@rm -f main.ps main.dvi main.log main.toc *.aux

clean:
	rm -f  main.ps main.dvi main.log main.toc *.aux process.pdf

#EOF
