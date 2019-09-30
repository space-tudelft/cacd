
TEST_OUTPUTS =	bjtnoise.raw bsim1tst.raw bsim2tst.raw diffpair.raw \
		diodisto.raw ltra_1.raw ltra_2.raw ltra_3.raw \
		mixdisto.raw mos6inv.raw mosamp2.raw mosmem.raw \
		pz2.raw pzt.raw rca3040.raw rc.raw resnoise.raw \
		rtlinv.raw schmitt.raw simplepz.raw testweep.raw

test: $(TEST_OUTPUTS)

%.raw: %.cir
	@rm -f rawspice.raw
	$(ICDPATH)/bin/spice3 -r < $*.cir > $*.sum 2>&1
	mv -f rawspice.raw $*.raw

clean:
	rm -f *.raw *.sum

#EOF
