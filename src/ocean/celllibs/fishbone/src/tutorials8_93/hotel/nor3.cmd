print vss, vdd, A, B, C, F
plot vss, vdd, A, B, C, F
option level = 3
option simperiod = 16
option sigunit = 1.000000e-09
option outacc = 10p
option simperiod = 16
/*
*%
tstep 0.2n
trise 0.5n
tfall 0.5n
*%
*%
.options cptime=500
*%
*/
set vdd = h*16
set vss = l*16
set A = l*1 h*1 l*1 h*1 l*1 h*1 l*1 h*1 l*1 h*1 l*1 h*1 l*1 h*1 l*1 h*1
set B = l*2 h*2 l*2 h*2 l*2 h*2 l*2 h*2
set C = l*4 h*4 l*4 h*4
