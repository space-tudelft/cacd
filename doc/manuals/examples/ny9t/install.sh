#!/bin/sh

# inkscape -E circuit_a.eps circuit_a.svg
# inkscape -E circuit_b.eps circuit_b.svg

atp demo.tex.atp > demo.tex
latex demo; latex demo
dvips demo
ps2pdf demo.ps
exit

I=ny9t

if ! test -d $I; then mkdir $I; fi

for f in tech.s param.p
do
    atp -s $f > $I/$f
done

for f in commands.gnp ny9t.pdf meas.dat ny9t.gds script.sh stim.spc README
do
    cp $f $I
done
