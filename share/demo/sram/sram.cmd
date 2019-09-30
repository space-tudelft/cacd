/* directory: demo/sram
/* 
/* Command file for simulating the sram circuit with simeye/nspice.
*/

print vdd t_vss b_vss word bit notbit c1 c2
plot  vdd t_vss b_vss word bit notbit c1 c2
option simperiod = 30
option sigunit = 0.75e-10
option outacc  = 10p
option level = 3 

/*  spice commands:

*%
trise 0.075n
tfall 0.075n
tstep 0.0125n
*%
.options nomod
.options limpts=601
.options cptime=30
*%

*/
set vdd = h*25 
set t_vss = l*25 
set b_vss = l*25 
set word = l*5 h*5 l*5 h*5 l*5 h
set notbit = l*10 h*10 l*5 
set bit = h*10 l*10 h*5 

