/* directory: demo/invert
/* 
/* Command file for simulating the inverter circuit using simeye/nspice.
*/

option sigunit = 1.000000e-11
option simperiod = 60
set b_in = l*10 h*30 l*20
set l_vdd = h*20 
set l_vss = l*20 
option level = 3 
plot b_in t_out

/*  spice commands:

*%
trise 0.05n
tfall 0.05n
tstep 0.005n
vlow  0
vhigh 5
*%
.options nomod
.options limpts=601
.options cptime=30
*%

*/
