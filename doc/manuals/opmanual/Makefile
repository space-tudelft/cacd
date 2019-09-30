
OPDIR = $(ICDPATH)/share/doc/opman

MANUALS = handleidingOP.pdf tutorial.pdf restricties.pdf testbaar.pdf howto_vhdl.pdf testingsog.pdf
MAN2017 = manualEE2CL11_2017.pdf
MANPAGES= gowiththeflow.pdf man_madonna.pdf man_schentry.pdf man_seadali.pdf man_simeye.pdf \
	man_sls.pdf man_trout.pdf

install: all
	rm -rf $(OPDIR)
	mkdir -p $(OPDIR)
	mv $(MANUALS) $(MANPAGES) $(OPDIR)
	cp -p $(MAN2017) $(OPDIR)
clean:
	rm -f $(MANUALS) $(MANPAGES)
	rm -f handleidingOP.[abdilp]* handleidingOP.toc groff.enc

all: manualstuff manpages

manualstuff: $(MANUALS)

manpages: $(MANPAGES)

%.pdf: %.tex
	latex $*
	latex $*
	@touch groff.enc
	dvips -t a4 $*
	ps2pdf $*.ps
	rm $*.aux $*.dvi $*.log $*.ps

handleidingOP.pdf:
	latex  handleidingOP
	bibtex handleidingOP
	latex  handleidingOP
	latex  handleidingOP
	@touch groff.enc
	dvips -t a4 handleidingOP
	ps2pdf handleidingOP.ps
	rm -f handleidingOP.[abdil]* handleidingOP.toc handleidingOP.ps

handleidingOP.pdf: handleidingOP.tex M_inleidin.tex M_doelstel.tex M_regeling.tex \
	M_opzeteno.tex M_teamwerk.tex M_ontwerpt.tex M_inwerkop.tex M_groepsopdr.tex \
	m_linux.tex m_seaofgat.tex m_celbibli.tex m_seqmachi.tex m_howto_vhdl.tex \
	M_analoged.tex m_testingsog.tex
M_inwerkop.tex   : M_tutorial.tex M_restricties.tex
M_groepsopdr.tex : M_testbaar.tex
tutorial.pdf     : tutorial.tex      man_init.tex M_tutorial.tex
restricties.pdf  : restricties.tex   man_init.tex M_restricties.tex
testbaar.pdf     : testbaar.tex      man_init.tex M_testbaar.tex
howto_vhdl.pdf   : howto_vhdl.tex    man_init.tex m_howto_vhdl.tex
testingsog.pdf   : testingsog.tex    man_init.tex m_testingsog.tex
gowiththeflow.pdf: gowiththeflow.tex man_init.tex m_gowiththeflow.tex
man_madonna.pdf	 : man_madonna.tex   man_init.tex m_madonna.tex
man_schentry.pdf : man_schentry.tex  man_init.tex m_schentry.tex
man_seadali.pdf  : man_seadali.tex   man_init.tex m_seadali.tex
man_simeye.pdf   : man_simeye.tex    man_init.tex m_simeye.tex
man_sls.pdf      : man_sls.tex       man_init.tex m_slsmnual.tex
man_trout.pdf    : man_trout.tex     man_init.tex m_trout.tex

#EOF
