.H 1 "3D Capacitance Computation"
.sY %SE_CAPCOMP%  \n(H1
.H 2 "Introduction"
.P= Space3d
uses a boundary-element method to compute 3D capacitances
(see Appendix %AX_3DCAPMODEL%).
Since there are several degrees of freedom with this method,
there are also several parameters that can be set with
.P= space3d
during 3D capacitance extraction.
A brief description of these parameters is given below.
For more background information on the parameters,
the reader is referred to
Appendix %AX_3DCAPMODEL%.
.P
The parameters are set in the space parameter file
(see also the Space User's Manual).
All lengths and distances are specified in micron
and all areas are specified in square micron.
.P
All parameters that have a name starting with "cap3d."
may be used without this prefix if they are included between the
lines "BEGIN cap3d" and "END cap3d".
E.g.
.fS I
BEGIN cap3d
max_be_area  1
be_window    3
END cap3d
.fE
is equivalent to
.fS I
cap3d.max_be_area  1
cap3d.be_window    3
.fE
.H 2 "Mesh Construction"
.sY %SE_MESHCON%  \n(H1.\n(H2
.DS I
cap3d.default_step_slope  \fIslope\fP    (default: 2.0)
.DE
Specifies the tangent of the slope of conductors
(i.e. the tangent of $alpha$ in the figure below)
at steps in height above the substrate
(e.g. the transition of metal above polysilicon
to metal not above polysilicon).
It must hold that cap3d.default_step_slope > 0.
.PS 2.2i
copy "../space3dman/step_slope.pic"
.PE
.sp -0.3
.N(
To prevent the overlap of different transition areas
of one conductor (which currently results in incorrect element meshes),
the value of parameter
cap3d.default_step_slope
may not be too small.
.N)
.DS I
cap3d.max_be_area  \fIarea\fP    (no default)
.DE
This parameter specifies (in square microns)
the maximum area of boundary elements
that are interior elements (i.e. elements that are not along the edges/corners
of the conductors).
This parameter has no default
and must therefore always be specified when performing 3D capacitance extraction.
.DS I
cap3d.min_be_area  \fIarea\fP    (default: 1 square unit)
.DE
This parameter specifies (in square microns)
the minimum area of boundary elements,
which may be used (smaller areas are skipped, a warning message is given).
.DS I
cap3d.edge_be_ratio  \fIfloat\fP    (default: 1)
.DE
This parameter specifies the ratio between the maximum size
of interior elements
and the maximum size of edge elements
(edge elements are elements that are adjacent to the edges/corners
of the conductors, interior elements are the other elements,
see also the parameter cap3d.max_be_area).
To efficiently compute accurate 3D capacitances it is advantageous
to use smaller elements near the edges/corners of the conductors.
This is achieved by using for cap3d.edge_be_ratio a value
smaller than 1.
Because the mesh refinement is done incrementally,
the size of the elements will gradually decrease towards
the edges/corners of the conductors.
This is also influenced by the parameter cap3d.edge_be_split.
.DS I
cap3d.edge_be_split  \fIfloat\fP    (default: 0.5)
.DE
If, during mesh refinement, a quadrilateral edge element is split
into two elements
(see also the description of the parameter cap3d.edge_be_ratio),
this parameter specifies the ratio between the size
of the element that becomes an edge element and the size
of the element that becomes an interior element.
.DS I
cap3d.edge_be_split_lw  \fIfloat\fP    (default: 4)
.DE
During mesh refinement, this parameter is used to determine
the split direction of a quadrilateral element.
Interior elements are always split perpendicular to their longest side.
If the ratio between the longest side and the shortest
side of an edge element does not becomes larger than
cap3d.edge_be_split_lw,
an edge element is split in a direction parallel to the edge direction.
Otherwise, the edge element is split perpendicular to its longest side.
The minimum value for cap3d.edge_be_split_lw is 2.
.DS I
cap3d.max_coarse_be_area  \fIfloat\fP    (default: cap3d.max_be_area)
.DE
For conductors that are sheet conductors (thickness is zero)
this parameter specifies (in square microns)
the maximum area of the boundary elements.
When this parameter is specified, edge elements of
sheet conductors are not further refined compared interior elements.
This parameter can for example be used to model large conductor planes
with a much coarser element mesh.
.DS I
cap3d.be_shape  \fInumber\fP    (default: 1)
.DE
Enforces a particular shape of the boundary element faces.
Value 1 means no enforcement.
Value 3 means triangular faces (is always used in "piecewise linear" mode).
Value 4 means quadrilateral faces (is the default for constant shape functions).
.H 2 "Shape and Weight Functions"
.sY %SE_SHAPEFUNC% \n(H1.\n(H2
.DS I
cap3d.be_mode  \fImode\fP    (default: 0c)
.DE
Specifies the type of shape functions and the type
of weight functions that are used (see Section %SE_BOUNDARY%).
.TS
center;
c c c.
_
mode	shape function	weight method
_
0c	piecewise constant	collocation
0g	piecewise constant	Galerkin
1g	piecewise linear	Galerkin
_
.TE
An example of a piecewise constant shape functions is given
in Figure %FG_SHAPEF%.b.
An example of a linear shape functions is given in Figure %FG_SHAPEF%.c.
Given a certain accuracy,
the Galerkin method, as compared to the collocation method,
allows to use larger elements.
.DS I
cap3d.mp_min_dist  \fIdistance_ratio\fP    (default: 2.0)
.DE
When the charge- and observation-elements are not too close together,
the influence matrix element linking them can be calculated much
faster (2 to 20 times) using a multipole expansion than by
numerical integration.
This parameter specifies a threshold value of the ratio between the
charge-observation distance and the convergence radius of
the multipole expansion: for larger distances the multipole expansion is
used, for smaller distances numerical integration.
Usually, a ratio of 1.5 is satisfactory.
When setting the parameter to infinity,
\fIall\fP influence matrix elements are calculated by numerical integration.
.DS I
cap3d.mp_max_order  \fI0 ... 3\fP    (default: 2)
.DE
Specifies the highest multipole to be included in the multipole-expansion.
For 0 only the monopole is included, for 1 also the dipole, and so forth.
The highest implemented value is 3 (octopole), because on the one hand
this typically suffices for a precision of one per mil, while on the
other hand the required CPU time increases drastically with the number
of multipoles.
.H 2 "Accuracy of Elastance Matrix"
.DS I
cap3d.green_eps  \fIerror\fP    (default: 0.001)
.DE
Positive real value specifying the relative accuracy
for evaluating the entries in the elastance matrix.
.DS I
cap3d.max_green_terms  \fInumber\fP    (default: 500)
.DE
For dielectrics consisting of more than one layer, more than
one term (iteration) will in general be necessary to
find an approximation of the Green's function such that the error
in the entries in the elastance matrix is within cap3d.green_eps
(see above).
This parameter specifies the value for the maximum number of
terms that may be used.
The upper bound of this parameter is 500.
.br
.ne 6
.H 2 "Window Size"
.DS I
cap3d.be_window  \fIw\fP
cap3d.be_window  \fIwx  wy\fP
.DE
Specifies the size (in micron) of the influence window.
All influences between elements that are within
a distance $w$ will be taken into account,
and all influences between elements that are more than
a distance $2w$ apart will not be taken into account
(see Section %SE_APPINV%).
If only one value is given, this value specifies the size of
the window in the x direction and the y direction.
If two values are given, the first value specifies the size of the window
in the x direction and the second value specifies the size of the window
in the y direction.
.P
The extraction time is proportional to $O ( N w sup 4 )$,
where $N$ is the number of elements.
The memory usage of the program is $O ( w sup 4 )$.
A reasonable value for
.I be_window
is 1-3 times the maximum height of the circuit.
No default.
.H 2 "Discarding 3D Capacitances"
.DS I
cap3d.omit_gate_ds_cap  \fIboolean\fP    (default: off)
.DE
Do not extract 3D capacitances between gates and diffused
conductors (drain/source areas), see Section %SE_GATECAP%.
.H 2 "Non 3D Capacitances"
.DS I
cap3d.also_non3d_cap  \fIboolean\fP    (default: off)
.DE
.sY %SE_NON3DCAP% \n(H1.\n(H2
Extract also all non-3D capacitances,
see Section %SE_NON3DCAP%.
.H 2 "New Cap3D Parameters"
.DS I
cap3d.connect_ground  \fIstring\fP    (default: @gnd)
.DE
The extraction is default using the @gnd node for ground plane connections.
But you may choice "@sub" or "distributed" instead (the @-sign may be omitted).
Distributed tries to use the substrate nodes directly below the cap3d nodes.
.DS I
cap3d.spider_hash  \fIboolean\fP    (default: off)
.DE
The new \fIspace3d\fP version does not use spider hashing to find existing spiders.
Now, by problem geometries, there can be two spiders at the same position.
However, this is not allowed for a piecewise linear be_mode (1c/1g).
.DS I
cap3d.new_via_mode  \fIboolean\fP    (default: on)
.DE
Now by vias, in new mode,
all calculated cap values are assigned to the top conductor via nodes
(and not more to bottom conductor nodes).
This is better, because in case the bottom conductor is
a diffused conductor the cap values can be lost.
.DS I
cap3d.contacts_sub  \fIboolean\fP    (default: on)
.DE
Now, the contacts to substrate are also modeled.
Thus, cap values for the surfaces of these contacts are now calculated.
.DS I
cap3d.new_refine  \fIboolean\fP    (default: on)
.DE
This is an important new feature,
because these mesh reductions results in a very fast cap3d extraction.
However, these refinements work only for be_mode "0c" and "0g".
.DS I
cap3d.new_convex  \fIboolean\fP    (default: on)
.DE
Now, concave polygons are tried to be repaired (if possible) by shifting the spiders.
Concave polygons can be created when using e/c-shape definitions.
.SK
.H 2 "Example Parameter File"
.sY %SE_PARAMFILE% \n(H1.\n(H2
An example of parameter settings for 3D capacitance
extraction is as follows:
.fS
BEGIN cap3d
max_be_area             1.0   # square micron
be_window               5.0   # micron
be_mode                 0g    # pwc galerkin
connect_ground          distr
contacts_sub            off
default_step_slope      1.5
omit_gate_ds_cap        on
green_eps               0.002
END cap3d
.fE
.H 2 "Run-time Versus Accuracy"
.sY %SE_TIMEVSACC%  \n(H1.\n(H2
The runtime of the program is largely dependent
on the values of the parameters that are used.
For example,
if max_be_area is decreased (smaller elements are used),
the accuracy will increase but also
the number of elements will increase and the computation time will
become larger.
The larger the size of the window, the more accurate results are obtained
but also longer extraction times will occur.
The Galerkin method is more accurate than the collocation method,
but it also requires more computation time.
.P
Also, 3D capacitance computation for configurations
consisting of 2 or 3 dielectric layers
may require much more computation time
than the same computation for configurations
consisting of 1 dielectric layer.
This is because the computation
of the Green's functions requires much more time.
In this case, the computation time can be decreased
(on the penalty of some loss in accuracy)
by increasing the value for the maximum error for the evaluation
of the entries in the elastance matrix (green_eps).
