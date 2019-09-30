.T= "Hierarchical Extraction and Terminals"
.DS 2
.rs
.sp 1i
.B
.S 15 20
Hierarchical Extraction
and Terminals
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
Report EWI-ENS 11-02
.ce
April 5, 2011
.sp |9i
.DS
.in +5
.S 9
Copyright \s+3\(co\s-3 2011 by the author.

Last revision: April 5, 2011.
.S
.in -5
.DE
.SK
.S=
.H 1 "INTRODUCTION"
When a hierarchical extraction is performed with the
.P= space
program, then the layout of the sub cells is not taken into account.
However, because the layout of the top cell must connect with the terminals of the sub cells (sub terminals or sub cell instance pins).
Therefor the layout of the sub terminals is lifted and added to the layout of the top cell.
This is done for the mask or layer where in the sub terminals are specified.
Therefor the sub terminal mask (conductor) can easy connect with the same mask (conductor) on the top level,
when they touch or overlap each other.
However, because the sub terminal layout elements are lifted to the top level, the dimension of these terminals
is taken into account for parasitics extraction of the top level cell.
.br
See the figure below.
.P
.F+
.PSPIC "an1102/fig1.ps" 5i
.F-
.P
When no sub terminal layout must be added to the top cell layout, the designer has the possibility to use
sub terminals which has no dimension.
For example, when the sub terminal element has no height and/or width the terminal has no dimension.
In that case a connection to the top level layout is only made, when the sub terminal position touches or
lays inside the top level mask element.
.P
For example, when sub terminals A and Q have no height and only a width, then a connection is only made when the
left x position of sub terminal A is used for its position, but sub terminal Q does not connect.
Default, when no resistances are extracted, always the left x and center y position of sub terminals is used.
However, when resistances are extracted by the
.P= space
program,
then the center x,y position of the sub terminals is used.
In that case, in the example above, sub terminal A does not connect, but sub terminal Q does connect.
Thus you see, that the used overlap of the layout element of the top level is important.
You can use
.P= space
parameter "term_use_center" to chose for another default for the sub terminal positions.
.H 1 "USING XCONTROL"
When using the
.P= xcontrol
program,
the designer can give sub cells a special cell status.
.P
For example the "library" status can be given to sub cells.
Even when the layout is flat extracted, these cells are not flattend.
For these "library" cells you can specify "free" or "freemasks".
These mask elements are added to the extracted layout and thus the library cell pins can easy connect.
.P
For parasitics extraction of only the top level, you must exclude the mask elements of the library cells.
Thus, you need to use a boundery box mask for these library cells, to mask out these masks for
parasitics extraction.
Thus, you can give these elements a zero conductance value in the space technology file.
And you can specify that no capacitances must be extracted for these conductance elements.
.P
Note that, when the "macro" status is given to sub cells, then these sub cells are always flattend.
This is useful when doing a hierchical extraction
and is normally used for via cells.
