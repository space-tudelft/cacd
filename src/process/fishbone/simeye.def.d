/* auto_set */
/* begin_disabled
set #* no_edit *# vdd = h*~
   end_disabled */
/* begin_disabled
set #* no_edit *# vss = l*~
   end_disabled */
/* begin_disabled
set #* no_edit *# CK = (l*50 h*50)*~
   end_disabled */
/* begin_disabled
set #* no_edit *# R = h*100 l*~
   end_disabled */
option sigunit = 1n
option outacc = 10p
option level = 3
option simperiod = 2000
option sta_file = on
/*
*%
tstep 0.1n
trise 0.5n
tfall 0.5n
*%
*%
.options cptime=500
*%
*/
/* auto_print */
/* auto_plot */
