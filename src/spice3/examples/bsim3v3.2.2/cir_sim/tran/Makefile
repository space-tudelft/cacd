
OUT = outx

test:	comparator.$(OUT) one-shot.$(OUT)

%.$(OUT): %.sp
	spice3 -b $*.sp > $*.$(OUT)
	@cat b3v3check.log_ > tmp1
	@cat b3v3check.log  > tmp2
	@sed -e /^$$/d -e /^Total/d -e /^Curr/d -e /^Dat/d $*.out >> tmp1
	@sed -e /^$$/d -e /CPU/d -e /memory/d -e /^Dat/d $*.$(OUT) >> tmp2
	@echo ==== diff $*.out $*.$(OUT) ====
	@-diff tmp1 tmp2
	@rm -f tmp1 tmp2 *.$(OUT)

clean:
	rm -f *.$(OUT)

#EOF
