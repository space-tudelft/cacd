cat << xxx
inverter
.subckt inv 1 2 3 4 5
m1 3 3 5 2 nenhs l=$ln w=$wn
m2 5 4 0 2 nenhs l=$ln w=$wn
.ends
x1 1 2 1 5 6 inv
vnbulk 2 0 -2.5v
vdd 1 0 5v
vi 5 0 5v
xxx

cat ../models

cat << xxx
.options cptime=400 nomod
.end
xxx
