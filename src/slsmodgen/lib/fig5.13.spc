cat << xxx
inverter
.subckt inv 1 2 3 4 5
m1 3 4 5 1 penh l=$lp w=$wp
m2 5 4 0 2 nenh l=$ln w=$wn
.ends
.subckt inv2 1 2 3 4 5
m1 3 4 5 1 penh l=$lp w=$wp
m2 5 4 0 2 nenh l=$ln w=$wn
xxx

i=1
stop=`expr $nx + 1`
while [ $i != $stop ] ; do
    cat << xxx
m3$i 5 3 5 2 nenh l=$ln w=$wn
xxx
    i=`expr $i + 1`
done

cat << xxx
.ends
x1 1 2 1 5 6 inv
x2 1 2 1 6 7 inv2
x3 1 2 1 7 8 inv
x4 1 2 1 8 9 inv2
x5 1 2 1 9 10 inv
co 10 0 25ff
vnbulk 2 0 0v
vdd 1 0 5v
vi 5 0 pulse(0v 5v 0ns 0.5ns)
.tran $tstep $tend
.plot tran v(9) v(8) v(7) v(6)  (0,6)
.print tran v(7) v(9)
xxx

if [ $uic ]; then
    cat << xxx
.ic v(6)=5v v(8)=5v v(10)=5v
.ic v(7)=0v v(9)=0v
xxx
fi

cat ../models

cat << xxx
.options cptime=400 nomod
.end
xxx
