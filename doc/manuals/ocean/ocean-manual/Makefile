
install: manual.pdf
	mv manual.pdf $(ICDPATH)/share/doc/ocean_manual.pdf

manual.pdf: *.tex
	@rm -f index.tex
	@touch index.tex
	latex manual.tex > manual.grep1
	cat manual.grep1 | grep -v -e '^$$' -e bounding -e earching -e ncluding -e rfull
	makeindex -o index.tex manual.idx
	latex manual.tex > manual.grep2
	cat manual.grep2 | grep -v -e '^$$' -e bounding -e earching -e ncluding -e rfull
	latex manual.tex > manual.grep3
	grep Warning manual.grep3
	@touch groff.enc
	dvips manual.dvi
	ps2pdf manual.ps
	@rm -f manual.ps manual.[adil]* manual.toc index.tex

clean:
	rm -f manual.[adgilp]* manual.toc index.tex groff.enc

#EOF
