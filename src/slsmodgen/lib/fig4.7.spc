cat << xxx
inverter
.subckt inv 1 2 3 4 5
m1 3 5 5 1 deps l=$lp w=$wp
m2 5 4 0 2 nenhs l=$ln w=$wn
.ends
.subckt inv2 1 2 3 4 5 6
m1 3 5 5 1 deps l=$lp w=$wp
m2 5 4 0 2 nenhs l=$ln w=$wn
xxx

i=1
stop=`expr $nx + 1`
while [ $i != $stop ] ; do
    cat << xxx
x$i 1 2 1 5 6 inv
xxx
    i=`expr $i + 1`
done

cat << xxx
co 6 0 100f
.ends
x1 2 2 1 5 6 21 inv2
x2 2 2 1 6 7 inv
x3 2 2 1 7 8 22 inv2
x4 2 2 1 8 9 inv
x5 2 2 1 9 10 23 inv2
co 10 0 25ff
vnbulk 2 0 -2.5v
vdd 1 0 5v
vi 5 0 pulse(0v 5v 0ns 0.5ns)
.tran $tstep $tend
.plot tran v(9) v(8) v(7) v(6)  (0,6)
.print tran v(7) v(9)
xxx

if [ $uic ]; then
    cat << xxx
.ic v(6)=5v v(8)=5v v(10)=5v v(21)=0v v(22)=0v v(23)=0v
.ic v(7)=0v v(9)=0v
xxx
fi

cat ../models

cat << xxx
.options cptime=400 nomod
.end
xxx
