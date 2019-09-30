option level = 3
option sigunit = 1.666666e-09
option outunit = 10p
plot  s1,s2,s3,freeze,light,ck
print s1,s2,s3,freeze,light,ck
/*
*%
tstep 0.2n
trise 0.3n
tfall 0.3n
*%
.options nomod
*%
*/
option simperiod = 264
set vss = l*264
set vdd = h*264
set s1 = l*45 h*15 l*204
set s2 = l*102 h*19 l*143
set s3 = l*220 h*29 l*15
set freeze = l*264
set ck = l*20 h*15 l*15 h*15 l*15 h*15 l*15 h*15 l*15 h*15 l*15 h*15 l*15  \
     h*15 l*15 h*14 l*14 h*6
set reset = h*40 l*224
