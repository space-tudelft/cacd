.H 1 "Program Usage"
.sY %SE_USAGE% \n(H1
.H 2 "General"
Substrate resistance extraction, using a boundary-element
or an interpolation method,
can be performed using one of the
following versions of
.P= space:
.P= space3d
(for batch mode extraction)
and
.P= Xspace
(for interactive extraction, including mesh visualization).
Substrate resistance extraction using an interpolation method
can also be performed using the standard version of
.P= space
(for batch mode extraction).
All of these programs can also be invoked from the GUI
.P= helios .
.P
When extracting substrate resistances,
.P= space
will always perform a flat extraction,
unless the parameter
.I allow_hierarchical_subres
is turned on in the parameter file.
.H 2 "Batch Mode Extraction"
In order to use 
the boundary-element method of
.P= space3d
to compute substrate
resistances,
use the option
.O= "-B" "-3dsub" "" 
with 
.P= space3d.
This method uses default the
.P= makesubres
program to make the mesh and to calculate the "subres" values.
When no separate
.P= makesubres
user program exists, then a link to
.P= space3d
is used.
When you don't want to call the external
.P= makesubres
program, you can use parameter "sub3d.internal".
The above method uses a more consistent mesh,
which is independent of other masks that cross the substrate terminal areas.
.br
Additional, substrate capacitances can be extracted.
This additional step uses the
.P= makesubcap
program to make the mesh and to calculate the "subcap" values.
It cannot be used with "sub3d.internal".
To add 3D substrate capacitances,
use parameter "add_sub_caps=2".
To add fast substrate capacitances,
use parameter "add_sub_caps=1" and set the rc-ratio parameter "sub_rc_const" to a suitable value.
For example, a ratio of epsilon0 (8.855e-12) can be used.
The rc-method can also be used with the interpolation method.
.P
In order to use the interpolation method of
.P= space3d
or
.P= space
to compute substrate
resistances,
use the option
.O= "-b" "-intersub" "" .
This method uses the
.P= makedela
program to make the the delaunay triangulation.
.H 2 "Interactive Extraction"
For substrate resistance extraction it may be helpful to use the
.P= Xspace
visualization program.
This program is not more an interactive version of the
.P= space3d
program.
But it runs
.P= space3d
in batch mode for each new extraction.
It is using a "display.out" file to show the results.
The
.P= Xspace
program runs under X Windows and 
uses a graphical window to,
among other things,
show the boundary-element mesh that is generated by the program.
Interactively, the user can select
the cell that is extracted, the options that are used,
and the items that are displayed.
.P
For both methods, to display the substrate terminals,
click on "DrawSubTerm" in the "Display" menu,
and to display the substrate resistances that are computed,
click on "DrawSubResistor"
in the "Display" menu.
Default, substrate terminals are drawn in their mask color.
To draw only the border of substrate terminals, set the
parameter "disp.fill_sub_term" to "off".
.P
In order to use the boundary-element method of
.P= Xspace
to compute substrate resistances,
turn on "3D sub. res."
in the menu "Options".
To display also the 3D mesh,
turn on "DrawBEMesh" (and turn off "DrawSubTerm")
in the menu "Display".
Optionally,
to view the Greens computation, turn on "DrawGreen".
Then, after selecting the name of the cell in 
the menu "Database",
the extraction can be started by clicking on "extract" in the
menu "Extract".
.P
To preview only the mesh for substrate resistance computation
using a boundary-element method, use
.P= Xspace
as described above and
also turn on "BE mesh only".
.P
In order to use the interpolation method of
.P= Xspace 
to compute substrate
resistances,
turn on "inter. sub. res."
in the menu "Options".
To display the delaunay triangulation that is used to
determine which substrate resistances are computed,
turn on "DrawDelaunay" in the menu "Display".