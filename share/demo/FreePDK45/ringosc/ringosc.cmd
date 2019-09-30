/* directory: demo/FreePDK45/ringosc */

option simperiod = 10 
option sigunit = 1e-10 
option outacc  = 1p

option level = 3
plot in out out5 sens
/* print in out out5 sens */
set VDD = h
set VSS = l

/*  spice commands:

*%
trise 50p
tfall 50p
tstep 5p
vhigh 3.5
vlow  0.0
*%
.ic v("in") 3.5
.options nomod
.options cptime=30
*%

*/
