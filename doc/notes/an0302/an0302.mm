.T= "Space Capacitance Extraction Application Note"
.DS 2
.rs
.sp 1i
.B
.S 15 20
SPACE CAPACITANCE EXTRACTION
APPLICATION NOTE
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
Report EWI-ENS 03-02
.ce
January 23, 2003
.sp |9i
.DS
.in +5
.S 9
Copyright \s+3\(co\s-3 2003 by the author.

Last revision: August 26, 2004.
.S
.in -5
.DE
.SK
.S=
.H 1 "INTRODUCTION"
To understand how capacitance extraction works
i have written this note.
There are two capacitance extraction methodes,
the standard (2D) capacitance extraction methode
and the 3D capacitance extraction methode.
For capacitance extraction with the \*(sP system
see also the "Space User's Manual"
.[
Gender Meijs Beeftink Elias
.]
and the
"Space Tutorial".
.[
Space Tutorial
.]
For 3D capacitance extraction with the \*(sP system
see also the
"Space 3D Capacitance Extraction User's Manual".
.[
Space Capacitance Extraction
.]
.P
Use the following options for 2D capacitance extraction:
.fS I
-c   All capacitors (excl. lateral) to ground node.
-C   All capacitors (excl. lateral).
-l   All capacitors (incl. lateral).
.fE
Use the following options for 3D capacitance extraction:
.fS I
-3c  All capacitors to ground node.
-3C  All capacitors.
.fE

.H 1 "THE 2D CAPACITANCE EXTRACTION METHOD"
This method does not know anything about the thickness of masks
and the height positions (z-direction) of the masks.
The method is based on the mask surface area (x,y dimensions)
and the distances between mask edges and the length of mask edges.
For the 2D method,
the capacitance values for capacitance elements are specified in the technology file
in the "capacitance list".
For the syntax and description see also the "Space User's Manual".
.[
Gender Meijs Beeftink Elias
.]
The capacitances are divided in different kind of coupling capacitors.
.AL
.LI
Surface capacitors.
.LI
Edge capacitors.
.LE

Note that all capacitors are between two conducting masks (layers).
The surface capacitors can be between the following conducting layers:
.AL
.LI
Between two different layout masks (cmf cms).
.LI
Between one layout mask and the ground plane (cmf @gnd).
.LI
Between one layout mask and the substrate plane (cmf @sub).
.LE

The edge capacitors can be between the following conducting layers:
.AL
.LI
Between edges of two the same layout masks (-cmf =cmf).
.LI
Between edges of two different layout masks (-cmf =cms).
.LI
Between edge/surface of two different layout masks (-cmf cms).
.LI
Between one layout mask edge and the ground or substrate plane.
.LE

Note that the preceding '=' character is used to denote the opposite edge.
These edge capacitors are called lateral capacitors.
The opposite edge mask may only be used in combination with an edge mask.
.br
Note that the substrate plane @sub may also be written as %(mcl).
Where 'mcl' is a mask condition list.
.SK
.H 2 "The Possible Pin Layer Combinations"
The following pin (node) mask combinations are possible (see also
.P= tecc ):
.TS
box;
l l l.
mask	mask#	occ_type
_
msk	>= 0	SURFACE
@gnd	-1	SURFACE
@sub	-2	SURFACE
%(mcl)	-4	SURFACE
-msk	>= 0	EDGE
=msk	>= 0	OTHEREDGE
.TE
.TS
box;
l l l l.
occ1	occ2	cap_type
_
SURFACE	SURFACE	SURFCAP
SURFACE	EDGE	EDGECAP
SURFACE	OTHEREDGE	not possible
EDGE	SURFACE	EDGECAP
EDGE	EDGE	EDGECAP
EDGE	OTHEREDGE	LATCAP
OTHEREDGE	SURFACE	not possible
OTHEREDGE	EDGE	LATCAP
OTHEREDGE	OTHEREDGE	not possible
.TE
SURFCAP/EDGECAP notes:
.TS
box;
l l l.
mask	mask	note
_
msk	msk	not useful
-msk	-msk	not useful
@...	@...	not possible
@...	%(mcl)	not possible
%(mcl)	%(mcl)	not possible
.TE

.P= Tecc
gives the message "\fBIncorrect pin layer specification\fP" when the
node mask combination is not possible.
.SK
.H 2 "SURFCAP Elements Examples"
For surface cap element matches is only looked to the color of one tile.
Function enumTile(tile) does a RecogS(tile) to find the SURFCAP elements in the tile.
The dimension of the tile surface area is used to calculate the cap value.
The mask condition list in the element specification must only contain SURFACE masks.
The SURFCAP elements are only matched for the not EDGE mask positions.
Thus, when you specify an EDGE mask condition the SURFCAP element can not be found.
.F+
.PSPIC an0302/surfcaps.ps 5i
.F-
Example of mask conditions for surface caps:
.fS I
surfcap_cmfgnd : cmf      : cmf @gnd : 30.5
surfcap_cmfcms : cmf cms  : cmf  cms : 60.2
surfcap_cmsgnd : cms !cmf : cms @gnd : 25.5
.fE
Note that the ground (substrate) plane is always present and
is the default node (GND) when a second pin mask is not specified.
.P
Normally, no edge mask conditions are specified.
Thus, the SURFCAP elements are placed on both the not EDGE masks and EDGE masks
positions in the element table (only for existing EDGE masks).
This can be improved as follows:
.fS I
surfcap_cmfgnd : cmf      !-cmf !-cms : cmf @gnd : 30.5
surfcap_cmfcms : cmf cms  !-cmf !-cms : cmf  cms : 60.2
surfcap_cmsgnd : cms !cmf !-cmf !-cms : cms @gnd : 25.5
.fE
.H 2 "EDGECAP Elements Examples"
For edge cap element matches is looked to the color of one tile
and the (edge) color of another new tile.
Function enumPair(tile,newtile) does a RecogE(tile,newtile) and RecogE(newtile,tile)
to find the EDGECAP elements for the edge between the tile and newtile.
Only the edge length is used to calculate the edge cap value.
Look out not to extract edge caps for internal mask edges.
.F+
.PSPIC an0302/edgecaps1.ps 5i
.F-
The following conditions are needed to match not the false -cmf of Fig.C:
.fS I
edgesurf_cmfcms : !cmf -cmf cms : -cmf  cms : 5.8   # C1
edgesurf_cmscmf : cmf !cms -cms :  cmf -cms : 5.8   # C2
.fE
The following conditions are needed to match the edge/edge caps of Fig.D:
.fS I
edgeedge_cmfcms : !cmf -cmf !cms -cms : -cmf -cms : 2.5  # C3
.fE
The following conditions are needed to match the lateral/edge cap of Fig.A:
.fS I
edgelate_cmfcms : !cmf -cmf !cms =cms : -cmf =cms : 1.0  # C4
.fE
.SK
The following examples discuss the shielding problem:
.F+
.PSPIC an0302/edgecaps2.ps 5i
.F-

Note that in Fig.B mask cmf shields caps between cms and GND.
Maybe in this case the surface cap C2 and edge-surf caps C1 and C3 must
not be calculated.
.br
Specify the following mask conditions:
.fS I
surfgnd_cms :  cms      !cmf :  cms @gnd : 3.2
edgegnd_cms : !cms -cms !cmf : -cms @gnd : 1.0
.fE
Possible the edge cap C1 is already shielded in Fig.A, specify:
.fS I
edgegnd_cms : !cms -cms !cmf !-cmf : -cms @gnd : 1.0
.fE
.SK
As explained before,
.P= space
looks for each edge in both directions.
Thus you must not add an extra condition for the other direction.
.F+
.PSPIC an0302/edgecaps3.ps 5i 1.7i
.F-
Only the following specification is needed for edge caps C1 and C2,
because
.P= space
does also RecogE(newtile2,tile2):
.fS I
edgesurf_cmfcms : !cmf -cmf  cms : -cmf cms : 5.8
.fE
Don't add the following specification for edge cap C2:
.fS I
edgesurf_cmfcms2: cmf !-cmf -cms : -cmf cms : 5.8
.fE
.SK
.H 2 "LATCAP Elements Examples"
.F+
.PSPIC an0302/latcaps.ps 5i 1.7i
.F-
For the above lateral edge caps between cmf/cms you specify the
condition for tile/newtile at position 1, as follows:
.fS I
latcap_cmfcms: !cmf -cmf !cms =cms : -cmf =cms : 5.8
.fE
Note that the
.P= tecc
program adds the condition for position 2
to the technology file (don't forget to specify =cms for this purpose
in the condition list).
This condition has the same element id as the previous condition.
Thus, don't add the following condition:
.fS I
latcap_cmscmf: !cmf =cmf !cms -cms : -cms =cmf : 5.8
.fE
Note that
.P= space
only looks in the backward direction to find the other tile mask.
When both edge masks -cmf and -cms are present at position 1 or 2,
then
.P= space
looks back for both lateral elements.
Thus, both elements (with same id) can be at the same element table position.
Note that
.P= space
needs a "lat_cap_window" for lateral cap extraction.
Parameter "lat_cap_window" needs to be set > 0.
.H 1 "THE 3D CAPACITANCE EXTRACTION METHOD"
By 3D capacitance extraction method
.[
Space Capacitance Extraction
.]
are the capacitances calculated with the dielectric materials constants
and vertical dimensions with Green's functions.
The 2D capacitance element specifications in the technology file are
in that case normally not used.
See the "Non-3D Capacitances" section (3.9 on page 10)
in the "Space 3D Capacitance Extraction User's Manual".
.[
Space Capacitance Extraction
.]
Only an exception is made for diffusion capacitances.
However,
with parameter "cap3d.all_non3d_cap" set you can take all 2D capacitances
into account.
Note that by 3D extraction default the 2D diffusion capacitances are used.
But with parameter "cap3d.omit_diff_cap" set to "off" (default "on"),
you can use the 3D diffusion capacitances (extractDiffusionCap3d = TRUE).
Thus, for extractDiffusionCap3d is TRUE may not the 2D diffusion cap elements
be used!
.H 2 "Diffusion Capacitances"
See for diffusion capacitors also the "Diffused Conductors" section (3.7 on page 8)
in the "Space 3D Capacitance Extraction User's Manual".
.[
Space Capacitance Extraction
.]
.H 1 "EXTRACTION WITH CAPACITANCES TO SUBSTRATE"
See for substrate extraction also the
"Space Substrate Resistance Extraction User's Manual".
.[
Space Substrate Resistance
.]
When you specify capacitors to substrate in the technology file:
.fS I
acap_cmfsub :  cmf      :  cmf @sub : 5.8
ecap_cmfsub : !cmf -cmf : -cmf @sub : 1.2
.fE
or:
.fS I
acap_cmfsub :  cmf      :  cmf %(cmf) : 5.8
ecap_cmfsub : !cmf -cmf : -cmf %(cmf) : 1.2
.fE
Then are these capacitors connected with the substrate plane (SUBSTR node)
in place of the ground plane (GND node).
By substrate resistance extraction (options -b and -B) is the substrate plane
divided in several substrate terminal nodes and the SUBSTR node.
Between these nodes is a network of substrate resistors.
The capacitors to substrate generate by substrate res extraction
a substrate terminal position and they are connected to this substrate terminal node.
With the "%(...)" notation it is possible to specify more exactly
the area of the substrate terminal.
.P
Note that by 3D capacitance extraction the above substrate terminal positions
are not generated by the 2D capacitance specification.
Only for specified diffusion capacitors they are generated.
.H 1 "NODE ASSIGNMENT OF 2D CAPACITANCES"
.H 2 "Edge Capacitances"
Normally, when one subnode is used for a conductor in a tile, the edge cap
is assigned between two subnodes.
However, by interconnect res extraction, the tile edges are represented with nodepoints.
And high res conductor edges have two subnodes.
This edge begin and end points get each a part of the edge cap value assigned.
See function updateResEdgeCap in "extract/enumpair.c".
.F+
.PSPIC an0302/fig1.ps 5i
.F-
Thus, parameter "cap_assign_type" can be used to change the default node assignment.
.H 2 "Surface Capacitances"
Normally, when one subnode is used for a conductor in a tile, the surface cap
is assigned between two subnodes.
However, by interconnect res extraction, the tile edges are represented with nodepoints.
Note that one nodepoint is used for the tile, when it contains only low res conductors.
When a tile is not a rectangular shape or the tile has extra nodepoints (besides the corner points),
then the tile is split in triangular parts.
Functions doRectangle and doTriangle are called by resEnumTile (see "extract/enumtile.c").
Both functions call function parPlateCap to assign surface cap value parts to the nodepoint subnodes.
.P
See the following figures for rectangular and triangular node assignment.
Parameter "cap_assign_type" can be used to change the default node assignment.
.F+
.PSPIC an0302/fig2.ps 5i
.F-
Thus, function doRectangle does 4 parPlateCap calls for cap_assign_type=0 and
16 calls for cap_assign_type=1.
.F+
.PSPIC an0302/fig3.ps 5i
.F-
Thus, function doTriangle does 3 parPlateCap calls for cap_assign_type=0 and
9 calls for cap_assign_type=1.
.H 2 "3D Surface Capacitances"
Note that not rectangular tiles or tiles with extra nodepoints (besides the corner points) are
not always refined in "cap3d" mode.
Thus, the resistor mesh can use the extra nodepoints, while cap3d assignment is done only
to the corner points (using cap3d.cap_assign_type > 0).
This depends also on parameter "cap3d.max_be_area".
Because rectangular tiles are also refined, when they are too big.
However, when using piecewise linear mode (parameter "cap3d.be_mode"),
all tiles are refined into triangular faces.
.F+
.PSPIC an0302/fig4.ps 5i 1.5i
.F-
The 3D surface cap of tile1 is assigned to A,B,C,D
and of tile2(a) to C,D,E,F
and of tile2(b) to C,G,E,F.
However, the 2D surface cap of tile2(a) is assigned to C,D,F and C,E,F
and of tile1(b) to A,B,G and A,C,G and B,D,G.
Note that for option
.B -z
another tile split is used.
.SK
.[
$LIST$
.]
.TC
