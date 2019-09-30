' \" @(#)comb_ntw.cmd	 3.1	 03/22/88
set phi = (l*1 h*1)*16

option level = 1
option outunit = 1n
option outacc = 10p
option sigunit = 100n

print  us_in[1..4],,oinv,oand,onand,oor,onor,oexor,oexnor,osp[1..2]
