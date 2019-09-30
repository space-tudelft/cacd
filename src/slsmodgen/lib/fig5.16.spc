cat << xxx
inverter
.subckt inv 1 2 3 4 5
m1 5 4 3 1 penh l=$lp w=$wp
m2 0 0 5 1 penh l=$lp w=$wp
.ends
x1 1 2 1 5 6 inv
vnbulk 2 0 0v
rnbulk 2 0 1meg
vdd 1 0 5v
vi 5 0 0v
xxx

cat ../models

cat << xxx
.options cptime=400 nomod
.op
.print op v(6)
.end
xxx
