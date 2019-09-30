.T= "Path Preserving Resistance Extraction"
.DS 2
.rs
.sp 1i
.B
.S 15 20
Path Preserving
Resistance Extraction
.S
.sp 2
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
Report EWI-ENS 14-05
.ce
December 15, 2014
.sp |9i
.DS
.in +5
.S 9
Copyright \s+3\(co\s-3 2014 by the author.

Last revision: December 16, 2014.
.S
.in -5
.DE
.SK
.S=
.H 1 "INTRODUCTION"
Path preserving can be enabled when a resistance extraction is done.
To preserve corner nodes, set parameter "use_corner_nodes" to "on".
Thus, around the corners of high resistive conductor paths, the
.P= space
extractor shall retain special line nodes.
At this way the topology of paths in the layout can be found back in
the resulting extracted resistance network.
.P
For making these special line nodes, special equi_line rectangles
are layed done around the corners in the path.
Note that for a horizontal path the equi_line rectangle has a delta-x
of use_corner_ratio * delta-y (the thickness of the path).
And for a vertical path the equi_line rectangle has a delta-y
of use_corner_ratio * delta-x.
Where parameter "use_corner_ratio" has the value 0.5 as default.
This makes it possible that these special line nodes are layed done closely to the corners of the path.
You may experiment with this "use_corner_ratio" value, to see what happens with the
resistivity value of the path.
.P
Note that a negative or zero value disables the layment of these special nodes on the path.
Thus it is possible to use the special mesh for other resistance extractions.
.P
Note that the resistance mesh corrections has as goal to make maximum use of line nodes.
To do this, currently two extra prepass steps are needed.
Therefor no other mesh refinement is possible (like option -z).
Because the prepass and extract pass must look around to the tiles at the scanline position
a new bandwidth is added to the code.
By this the enumTile procedure is delayed against the enumPair procedure.
Thus, when a tile becomes ready and is done by the enumTile procedure, we can look forward
to other tiles with the new introduced bandwidth.
And also we can look backward with the same bandwidth, because the tiles are not directly cleared
by the clearTile procedure.
.P
Thus, parameter "equi_line_width" must be set to a large enough value to work with.
.P
Note that parameter "equi_line_area" must not be set to "off" (default "on").
Because in that case the special nodes are not flagged as special area nodes
and are not preserved (but eliminated).
However you can use it, if you want to know the total resistivity value of the path.
.P
The following pages demonstrate the path preserving technique.

.F+
.PSPIC "an1405/fig1.ps" 6.5i
.F-
.P
The above pictures demonstrate path preserving.
The first picture gives the normally used tiles and boundaries.
The first tile on the left side can not have line nodes because it has extra points on its right side.
On the right side of the layout you see a tile which is split, because there are terminals defined.
The second picture demonstrates what the prepasses have done.
The tiles have get extra splits and two stopping boundaries are layed done to stop the terminal splits.
.P
The third picture shows the resulting resistivity mesh by using path preserving.
.br
The fourth picture shows the resulting outputted resistivity path and nodes.
Because there are only four terminals defined, some parts of the path are dangling.
Therefor some special nodes are generated but not preserved.

.F+
.PSPIC "an1405/fig2.ps" 6i
.F-
.P
The above pictures demonstrate path preserving.
The first picture shows the resulting resistivity mesh and
the second picture shows the resulting outputted resistivity path and nodes.

.F+
.PSPIC "an1405/fig3.ps" 6i
.F-
.P
Another example of path preserving.
Depending on the distance not always two special nodes can be used.
By the terminal node on left top a node is preserved very close to it.
On the right side you see that a different path delta-x gives also
a different height of the special equi-line rectangle.

.F+
.PSPIC "an1405/fig4.ps" 6i
.F-
.P
This picture is made of the demo/suboscil project
and shows the resistivity mesh.
.br
You see that only the poly interconnect/gate area's and the source/drain active area's
have high resistivity.

.F+
.PSPIC "an1405/fig5.ps" 6i
.F-
.P
This picture of the demo/suboscil project
shows the outputted resistor network.
.br
You see that each mos-transistor has three fixed terminal pins.
Note that always the upper right node of the gate area is chosen as gate terminal.
The vss rail is low res and has one node with coordinates on the lower left.
A drain/source pin of each nenh-transistor is connected with a resistor to the vss rail.
And also
a drain/source pin of each penh-transistor is connected with a resistor to the vdd rail.
.P
You can also see that the inputs of all invertors have an area=2 node with a degree=3,
which nodes are retained in the network.
.P
Using the min_res heuristic shall try to remove the small resistors from the network.
.br
I made some arrangements to retain the resistors connected to the special area=1 nodes.
