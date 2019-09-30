.T= "update option -c and min_coup_cap"
.DS 2
.rs
.sp 1i
.B
.S 15 20
Space update
option -c
and min_coup_cap
.S
.sp 1
.I
S. de Graaf
.sp 1
.R
Circuits and Systems Group
Faculty of Electrical Engineering,
Mathematics and Computer Science
Delft University of Technology
The Netherlands
.DE
.sp 2c
.ce
Report EWI-ENS 11-07
.ce
October 12, 2011
.sp |9i
.DS
.in +5
.S 9
Copyright \s+3\(co\s-3 2011 by the author.

Last revision: October 14, 2011.
.S
.in -5
.DE
.SK
.S=
\s+2\fBIntroduction\fP\s-2
.P
Both updates have in common that
it concerns coupling capacitance nodes which are in the same node group.
.P
\s+2\fBOption -c\fP\s-2
.P
When this
.P= space
extraction option is used to extract capacitances
and not option
.B -C
and not option
.B -l
are used,
then all extracted capacitances are connected to the ground node.
Thus, when in the technology file surface and edge coupling capacitances
are specified, then these capacitances are folded to the ground (GND) node.
.F+
.PSPIC "an1107/fig1.ps" 4.5i
.F-
Not all coupling capacitances are folded to the ground node.
When the node group of the poly silicon conductor is connected with the node group
of the metal1 conductor, then the coupling capacitances are omitted.
.F+
.PSPIC "an1107/fig2.ps" 4.5i
.F-
However, capacitances to the SUBSTR node are not omitted,
these capacitances are always connected with the GND node:
.F+
.PSPIC "an1107/fig3.ps" 4.5i
.F-
.P
\s+2\fBParameter min_coup_cap\fP\s-2
.P
The min_coup_cap heuristic is changed.
