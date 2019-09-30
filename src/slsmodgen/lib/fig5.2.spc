cat << xxx
inverter
.subckt inv 1 2 3 4 5
m1 3 4 5 1 penh l=$lp w=$wp
m2 5 4 0 2 nenh l=$ln w=$wn
.ends
m1 1 50 10 1 penh l=$lp w=$wp
m2 10 0 0 2 nenh l=$ln w=$wn
m3 1 1 20 1 penh l=$lp w=$wp
m4 20 5 0 2 nenh l=$ln w=$wn
m5 10 60 20 1 penh l=$lp w=$wp
m6 10 6 20 2 nenh l=$ln w=$wn
cx 10 0 $cx
xxx

i=1
stop=`expr $nx + 1`
while [ $i != $stop ] ; do
    cat << xxx
x$i 1 2 1 20 100 inv
xxx
    i=`expr $i + 1`
done

cat << xxx
co 100 0 25ff
vnbulk 2 0 0v
vdd 1 0 5v
v5 5 0 pulse(5v 0v 0ns 0.045ns)
v50 50 0 pulse(0v 5v 0ns 0.045ns)
v6 6 0 pulse(0v 5v 1ns 0.045ns)
v60 60 0 pulse(5v 0v 1ns 0.045ns)
.tran $tstep $tend
.plot tran v(10) v(20) (-1,6)
xxx

cat ../models

cat << xxx
.options cptime=400 nomod
.end
xxx
