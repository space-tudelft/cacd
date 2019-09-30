
install: main.dvi
	@touch groff.enc
	dvips -t a4 -o main.ps main.dvi 2> dvi.log
	ps2pdf main.ps
	rm main.ps
	mv main.pdf $(ICDPATH)/share/doc/spock_thesis.pdf

main.dvi: *.tex
	latex  main
	bibtex main
	latex  main
	latex  main

clean:
	rm -f main.dvi main.pdf main.ps
	rm -f main.lof main.log main.lot
	rm -f main.toc main.blg main.bbl
	rm -f *.aux dvi.log groff.enc

#EOF
