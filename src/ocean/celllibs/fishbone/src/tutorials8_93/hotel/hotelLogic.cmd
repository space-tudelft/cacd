option level = 3
option sigunit = 1.250000e-09
option outunit = 10p
plot  s,freeze,q0,q1,q0new,q1new
print s,freeze,q0,q1,q0new,q1new
/*
*%
tstep 0.2n
trise 0.3n
tfall 0.3n
*%
.options nomod
*%
*/
option simperiod = 47
set vss = l*47
set vdd = h*47
set s = l*3 h*3 l*3 h*3 l*3 h*3 l*3 h*3 l*3 h*3 l*3 h*3 l*3 h*3 l*3 h*2
set freeze = l*47
set q0 = h*12 l*24 h*11
set q1 = l*24 h*23
