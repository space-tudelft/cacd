/*
* NOTE: only for pspice simulations
*
*%
*%

iref "Vcc" "iref" 1u
v1   "ground" 0 0v
v2   "asssymm" "outpos" 0v
v3   "vreg" "rv" 0v
rcshunt "rcshunt" "rshunt" 200k
rcomp1 "comp1" 500 8k
ccomp1 500 0 15p
rcomp2 "comp2" 501 250k
ccomp2 501 0 10p
rcomp3 "comp3" 502 5k
ccomp3 502 0 15p
rcomp4 "comp4" 503 8k
ccomp4 503 0 15p
rcomp5 "comp5" 504 250k
ccomp5 504 0 10p
rcomp6 "comp6" 505 5k
ccomp6 505 0 15p
ccompv "compv" "vreg" 10p
routneg "outneg" 507 159k
vk 507 0 0.5
rv "rv" 0 5k
rcompc "comcomp" 506 25k
ccompc 506 0 100p
ireg "Vcc" "ireg" 0
rreg "ireg" 0 75k
rschaal1 "Vcc" "rschaal1" 30k
rschaal2 "Vcc" "rschaal2" 30k
vcc "Vcc" 0 1v
ibron   "comp3" 0 ac 1

.step ireg 0 1.92u 320n
.ac dec 10 10 1meg
.op
.probe
*.plot ac vdb("outneg")
*.print ac vdb("outneg")

*%
*/
