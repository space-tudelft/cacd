/* space online helios tutorial */

set b_data_0 = (l*10 h*10)*~
set b_data_1 = (l*20 h*40)*~
set b_data_2 = (l*20 h*10 l*10 h*10 l*20)*~
set b_data_3 = (h*10 l*40)*~
set sel_0_0 = (h*70 l*70)*~
set sel_1_0 = (h*70 l*70)*~
set sel_0_1 = (l*90 h*200)*~
set sel_1_1 = (h*90 l*80 h*40)*~
set sel_0_2 = l
set sel_1_2 = l
set sel_0_3 = h
set sel_1_3 = h 
set b_gnd = l
set b_vdd = h
option simperiod = 400
option sigunit = 1n
option outacc = 10p
option level = 2

print b_data_0, b_data_1, b_data_2, b_data_3
plot b_data_0, b_data_1, b_data_2, b_data_3
print sel_0_0, sel_1_0, out_0, sel_0_1, sel_1_1, out_1, 
plot sel_0_0, sel_1_0, out_0, sel_0_1, sel_1_1, out_1, 
print sel_0_2, sel_1_2, out_2, sel_0_3, sel_1_3, out_3, 
plot sel_0_2, sel_1_2, out_2, sel_0_3, sel_1_3, out_3, 
print b_gnd, b_vdd
plot b_gnd, b_vdd

