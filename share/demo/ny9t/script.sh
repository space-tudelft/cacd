#!/bin/sh

unset CWD
if test -f .dmrc; then rmpr -fs .; fi

mkpr -p scmos_n -l 0.01 .
cgi ny9t.gds
tecc tech.s

space3d -vFC3r -E tech.t -P param.p -S name_extension=_nosub ny9t
xspice -ax ny9t_nosub -S stim.spc > ny9t_nosub.spc
spice3 < ny9t_nosub.spc > ny9t_nosub.dat

space3d -BvFC3r -E tech.t -P param.p -Sname_extension=_sub ny9t
xspice -ax ny9t_sub -S stim.spc > ny9t_sub.spc
spice3 < ny9t_sub.spc > ny9t_sub.dat

gnuplot < commands.gnp
