.T= "Mesh Refinement Application Note"
.DS 2
.rs
.sp 1i
.B
.S 15 20
SPACE APPLICATION NOTE
ABOUT
MESH REFINEMENT
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
Report EWI-ENS 03-09
.ce
November 27, 2003
.sp |9i
.DS
.in +5
.S 9
Copyright \s+3\(co\s-3 2003 by the author.

Last revision: December 9, 2003.
.S
.in -5
.DE
.SK
.S=
.H 1 "INTRODUCTION"
The
.P= space
mesh refinement option (
.B -z )
can be used for interconnect resistance extraction.
Without this option,
.P= space
shall split the tiles in triangles, when there are extra node points.
For an explanation about node points, see the node points application note.
With refinement,
.P= space
shall split the tiles horizontally and vertically,
when there are extra node points.
See for example the following figure:
.P
.F+
.PSPIC an0309/fig1.ps 10c
.F-
.P
The splits are only done in high resistive conductor tiles
(tiles that contain one or more high res conductors).
The triangularization is done in source file "extract/enumtile.c" by
function triangular_core.
This function is called by enumTile -> resEnumTile -> triangular.
A disadvantage of the standard resistance calculation method is,
that the result is less accurate and that there can be negative resistance values.
Thus, mesh refinement is a good choice.
A disadvantage of the mesh refinement method is, that it uses a preprocessing step
to generate the horizontal interior mesh edges.
And, that it generates more input tiles, because the tile splits are handled by function scan.
Thus, mesh refinement is more accurate, but it cost more execution time.
.P
Note that it generates in general more tile splits than the ones you see above.
This happens, because the horizontal or vertical interior edge shall also create
an extra node point on the other side of the tile.
And this new extra node point can again give a tile split in the neighbor tile.
.P
The figure below shows the mesh preprocessing step.
.P
.F+
.PSPIC an0101/spmesh.ps 12c
.F-
.P
.H 1 "PROBLEM WITH TERMINALS"
A terminal is in most cases a rectangular layout element.
By resistance extraction is the rectangle center position chosen as
the position of the terminal point.
This terminal point shall split the tile in vertical direction and
shall possible also introduce a vertical split of the neighbor tiles, above and below the tile.
And, because the terminal point is laying on a vertical edge,
it shall also split a high res tile in horizontal direction (see figure).
.P
.F+
.PSPIC an0309/fig2.ps 10c
.F-
.P
A terminal can also be a terminal point (when it has no dimension).
Such a terminal can everywhere be positioned in the layout.
When the terminal is positioned on a horizontal tile edge,
it shall split both the tile above and below that edge.
It shall also split a tile, which does not contain the terminal conductor.
And it can also split a tile, which does not contain anything.
This can give the following vertical split results (see figure below).
.P
.F+
.PSPIC an0309/fig3.ps 10c
.F-
.P
