.T= "cap3d couple cap reduction note"
.DS 2
.rs
.sp 1i
.B
.S 15 20
Space cap3d
couple capacitance
reduction note
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
Report EWI-ENS 11-06
.ce
October 5, 2011
.sp |9i
.DS
.in +5
.S 9
Copyright \s+3\(co\s-3 2011 by the author.

Last revision: October 13, 2011.
.S
.in -5
.DE
.SK
.S=
Given the following example layout of a high resistive conductor:
.F+
.PSPIC "an1106/fig1.ps" 6.0i
.F-
.P
Given a Rsheet, the resistor network can be calculated as follows:
.F+
.PSPIC "an1106/fig2.ps" 3.8i
.F-
.P
For a Rsheet of 0.077 Ohm/square (using parameter low_sheet_res=0.05) the following resistor network is the result:
.F+
.PSPIC "an1106/fig3.ps" 3.8i
.F-
.P
I shall explain what happens with the couple capacitances when reducing the
network when doing a cap3d extraction of the above layout.
.br
The following network can be the result of the -rC3 extraction:
.F+
.PSPIC "an1106/fig4.ps" 5.4i
.F-
.SK
After reduction, the following network is the result of the -rC3 extraction:
.F+
.PSPIC "an1106/fig5.ps" 5.4i
.F-
.P
You see that the 600 aF couple capacitances have a small effect (+ 0.69 aF) on
the resulting couple capacitance between nodes A and B.
.P
When we extract the above example with options -rc3, we get:
.F+
.PSPIC "an1106/fig6.ps" 5.4i
.F-
After reduction:
.F+
.PSPIC "an1106/fig7.ps" 5.4i
.F-
.P
You see that the 600 aF couple capacitances have a big effect on the ground capacitances
when we do an extraction with options -rc3.


\s+2\fBNote\fP\s-2
.P
See next page, note about change in option \fB-c\fP extraction mode.
.SK
\s+2\fBNote about update of option -c mode\fP\s-2
.P
The
.P= space
option \fB-c\fP mode, extract capacitances to ground,
is now changed.
The coupling capacitances in the same conductor (node group) are now not more
folded to the ground node, but skipped.
In cap3d mode, most nodes of the same node group are already in one group,
because cap3d mode is processed after a number of be_windows are ready.
.br
In case groups are later on connected to each other, then the coupling capacitances
are removed when the group becomes ready.
.P
Thus, for the new option \fB-c\fP mode (see previous example)
the network before the reduction (with skipped couple caps) shall be:
.F+
.PSPIC "an1106/fig8.ps" 5.4i
.F-
.P
And after reduction:
.F+
.PSPIC "an1106/fig9.ps" 5.4i
.F-


\s+2\fBNote\fP\s-2
.P
See also next application note (report).
