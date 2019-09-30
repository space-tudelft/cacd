
OUT = outx

test:	test1.$(OUT) test2.$(OUT) test3.$(OUT) test4.$(OUT) test5.$(OUT) \
	test6.$(OUT) test7.$(OUT) test8.$(OUT) test9.$(OUT) test10.$(OUT) \
	test11.$(OUT) test12.$(OUT) test13.$(OUT) test14.$(OUT)

%.$(OUT): %.v3
	spice3 -b $*.v3 > $*.$(OUT)
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
