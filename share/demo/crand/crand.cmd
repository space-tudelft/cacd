/* directory: demo/crand
/*
/* Command file for simulating the crand circuit using simeye.
*/

set inpar_0     = h*~
set inpar_1     = h*~
set inpar_2     = h*~
set inpar_3     = h*~
set inpar_4     = h*~
set inpar_5     = h*~
set inpar_6     = h*~
set inpar_7     = h*~
set sc_l        = h*2 l*~
set nsc_l       = l*2 h*~
set phi1_l      = (h*1 l*1)*~
set nphi2_l     = (h*1 l*1)*~
set nphi1_l     = (l*1 h*1)*~
set phi2_l      = (l*1 h*1)*~
set vdd_lb      = h*~
set vdd_lo      = h*~
set vss_lb      = l*~

option level = 3
option sigunit = 10n
option outunit = 1n
option outacc = 10p
option simperiod = 32
print phi1_l phi2_l nsc_l sc_l,
print out_0 out_1 out_2 out_3 out_4 out_5 out_6 out_7,serial
plot phi1_l phi2_l nsc_l sc_l,
plot out_0 out_1 out_2 out_3 out_4 out_5 out_6 out_7,serial

/*  spice commands:

*%
trise 4n
tfall 4n
tstep 0.25n
*%
.options nomod
.options limpts=601
.options cptime=30
*%

*/
