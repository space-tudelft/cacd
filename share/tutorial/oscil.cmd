/* space online helios tutorial */

option simperiod = 10 
option sigunit = 1.000000e-09 
option outacc  = 10p

option level = 3
plot out sens
print out sens
set vdd = h*~
set vss = l*~

/*  spice commands:

*%
trise 0.1n
tfall 0.1n
tstep 0.02n
*%
.ic v("in") 5v
.options nomod
.options limpts=601
.options cptime=30
*%

*/
