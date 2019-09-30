.H 1 "The Boundary-Element Method"
.sY %SE_BEM%  \n(H1
.H 2 "Introduction"
The boundary-element method (BEM) allows to compute accurate
substrate resistance values for a (not too large) number of
substrate terminals on top of a substrate.
The substrate is described by specifying
the conductivity of the different substrate
layers (see Section %SE_TECHBEM%).
Currently, at most two different substrate layers can be described.
The thickness of the substrate is considered infinite.
By default, the substrate is considered to have
infinite dimensions in the horizontal direction.
Optionally, also substrate edges (saw-lanes) can be
taken into account.
.P
Since there are several degrees of freedom with the
boundary-element method to compute substrate resistances
such as size of elements, type of shape function, etc.,
there are also several
parameters that can be set with
.P= space
when using the boundary-element method.
A brief description of these parameters is given 
below.
For more background information on the boundary-element method
to compute substrate resistances, see.
.[
A Boundary-Element Method for Substrate Cross-talk Analysis
Smedes
.]
.[
Extraction of Circuit Models for Substrate Cross-talk
Smedes
.]
.P
Besides the substrate conductivity that is specified in the
space element definition file, all parameters for 
the boundary-element method
are specified in the space parameter file
(see also the Space User's Manual).
.P
In the parameter file,
lengths and distances are specified in microns
and areas are specified in square microns.
.P
All parameters that have a name starting with "sub3d."
may be used without this prefix if they are included between the
lines "BEGIN sub3d" and "END sub3d".
E.g. 
.fS I
BEGIN sub3d
saw_dist   5
edge_dist 14
END sub3d
.fE
is equivalent to
.fS I
sub3d.saw_dist   5
sub3d.edge_dist 14
.fE
.P
Note that the BEM parameters for substrate capacitance calculation
are default equal to the "sub3d.\fIxxx\fP" parameters.
To specify different values, use the prefix "\fBsubcap3d\fP.".
However, this is not allowed for parameter "sub3d.be_window".
.H 2 "Substrate Edge Effects"
Note that edge effects can only taken into account when using the collocation method
(see parameter "sub3d.be_mode").
And only when parameter "use_multipoles" is turned "off".
To use edge effects,
the value of parameter "sub3d.edge_dist" must be larger than
the value of parameter "sub3d.saw_dist".
.DS I
sub3d.saw_dist  \fIdistance\fP    (default: infinity)
.DE
This parameter specifies the edge of the substrate (saw-lane).
When a bounding box is assumed around the layout of
the set of substrate terminals,
the edge of the substrate is defined as the rectangle that
extends the bounding box
sub3d.saw_dist microns.
.DS I
sub3d.edge_dist  \fIdistance\fP    (default: 0)
.DE
Specifies the maximum distance to the saw-lane
for an element in the layout to be influenced
by the saw-lane.
(Non-negative real value in microns.)
For elements that are more than sub3d.edge_dist micron away from
the saw-line, no edge effects are taken into account.
Edge effects can generally be neglected for 
elements that are further away from the saw-lane 
than 2 times the epi-thickness.
.H 2 "Mesh Construction"
.sY %SE_MESHCON%  \n(H1.\n(H2
.DS I
sub3d.max_be_area  \fIarea\fP    (no default)
.DE
This parameter specifies (in square microns)
the maximum area of boundary elements
that are interior elements (i.e. elements that are not along the edges
of the substrate terminals).
This parameter has no default
and must therefore always be specified when performing 3D substrate
resistance extraction.
.DS I
sub3d.edge_be_ratio  \fIfloat\fP    (default: 1)
.DE
This parameter specifies the ratio between the maximum size
of edge elements
and the maximum size of interior elements
(edge elements are elements that are along the edges of the substrate terminals,
interior elements are the other elements,
see also the parameter sub3d.max_be_area).
To efficiently compute accurate substrate resistances it is recommended
to use smaller elements near the edges of the substrate terminals.
This is achieved by using for sub3d.edge_be_ratio a value
smaller than 1.
Because the mesh refinement is done incrementally,
the size of the elements will gradually decrease towards
the edges of the substrate terminals.
This is also influenced by the parameter sub3d.edge_be_split.
.DS I
sub3d.edge_be_split  \fIfloat\fP    (default: 0.5)
.DE
If, during mesh refinement, a quadrilateral edge element is split 
into two elements
(see also the description of the parameter sub3d.edge_be_ratio),
this parameter specifies the ratio between the size
of the element that becomes an edge element and the size
of the element that becomes an interior element.
.DS I
sub3d.edge_be_split_lw  \fIfloat\fP    (default: 4)
.DE
During mesh refinement, this parameter is used to determine
the split direction of a quadrilateral element.
Interior elements are always split perpendicular to their longest side.
If the ratio between the longest side and the shortest
side of an edge element does not becomes larger than 
sub3d.edge_be_split_lw,
an edge element is split in a direction parallel to the edge direction.
Otherwise, the edge element is split perpendicular to its longest side.
The minimum value for sub3d.edge_be_split_lw is 2.
.DS I
sub3d.be_shape  \fInumber\fP    (default: 1)
.DE
Enforces a particular shape of the boundary element faces.
Value 1 means no enforcement.
Value 3 means triangular faces (is always used in "piecewise linear" mode).
Value 4 means quadrilateral faces (is the default for constant shape functions; see below).
.H 2 "Shape and Weight Functions"
.DS I
sub3d.be_mode  \fImode\fP    (default: 0c)
.DE
Specifies the type of shape functions and the type
of weight functions that are used.
.TS
center;
c c c.
_
mode	shape function	weight method
_
0c	piecewise constant	collocation
0g	piecewise constant	Galerkin
1c	piecewise linear	collocation
1g	piecewise linear	Galerkin
_
.TE
In general, it is recommended not to use mode 1c due to its poor
numerical behavior.
Further, given a certain accuracy,
the Galerkin method, as compared to the collocation method, 
allows to use larger elements.
Note that the Galerkin method is more accurate than the collocation method,
but it also requires more computation time.
.DS I
sub3d.mp_min_dist  \fIdistance_ratio\fP    (default: 2.0)
.DE
When the current- and observation-elements are not too close together,
the influence matrix element linking them can be calculated much
faster (2 to 20 times) using a multipole expansion than by
numerical integration.
This parameter specifies a threshold value of the ratio between the
current-observation distance and the convergence radius of
the multipole expansion: for larger distances the multipole expansion is
used, for smaller distances numerical integration.
Usually, a ratio of 1.5 is satisfactory.
When setting the parameter to infinity,
\fIall\fP influence matrix elements are calculated by numerical integration.
.DS I
sub3d.mp_max_order  \fI0 ... 3\fP    (default: 2)
.DE
Specifies the highest multipole to be included in the multipole-expansion.
For 0 only the monopole is included, for 1 also the dipole, and so forth.
The highest implemented value is 3 (octopole), because on the one hand
this typically suffices for a precision of one per mil, while on the
other hand the required CPU time increases drastically with the number
of multipoles.
.DS I
use_multipoles  \fIboolean\fP    (default: on)
.DE
With this parameter, you can turn "off" the multipole expansion.
.H 2 "Accuracy of Elastance Matrix" 
.sY %SE_ELAST%  \n(H1.\n(H2
.DS I
sub3d.green_eps  \fIerror\fP    (default: 0.001)
.DE
Positive real value specifying the relative accuracy
for evaluating the entries in the elastance matrix.
.DS I
sub3d.max_green_terms  \fInumber\fP    (default: 500)
.DE
For substrates consisting of more than one layer, more than
one term (iteration) will in general be necessary to
find an approximation of the Green's function such that the error
in the entries in the elastance matrix is within sub3d.green_eps
(see above).
This parameter specifies the value for the maximum number of
terms that may be used.
The upper bound of this parameter is 500.
.H 2 "Window Size"
.DS I
sub3d.be_window \fI w \fP [\fIwy\fP]
.DE
Specifies the size (in micron) of the influence window.
All influences between elements that are, in the horizontal
direction, within a distance $w$ will be taken into account,
and all influences between elements that are more than
a distance $2w$ apart will not be taken into account.
If only one value is given, this value specifies the size of
the window in the layout x direction and the layout y direction.
If two values are given, the first value specifies the size of the window
in the x direction and the second value specifies the size of the window
in the y direction.
.P
The extraction time is proportional to $O ( N w sup 4 )$,
where $N$ is the number of elements.
The memory usage of the program is $O ( w sup 4 )$.
No default.
.N(
It is recommended not to
use small window sizes for configurations
consisting of a thin good conducting top substrate layer
and a poorly conducting bottom substrate layer.
This is because of the relatively large error that may occur.
.N)
.SK
.H 2 "Example Parameter File"
.sY %SE_PARAMFILE% \n(H1.\n(H2
An example of parameter settings for 3D substrate resistance
extraction is as follows:
.fS I
BEGIN sub3d
be_mode           0g   # piecewise constant galerkin
max_be_area       1.0  # microns^2
edge_be_ratio     0.05 
edge_be_split     0.2  
be_window         inf  # infinity
END sub3d
.fE
.H 2 "Example of 3 Substrate Terminals"
The following example consists of three substrate terminals
on top of a two layered substrate.
To run the example, first create a project, e.g. with name "sub3term",
for a "scmos_n" process with lambda equal to 0.1 micron.
.fS I
% mkpr -l 0.1 sub3term
available processes:
process id       process name
         1       nmos
         3       scmos_n
         ...
select process id (1 - 60): 3
mkpr: -- project created --
%
.fE
Next, go to the project directory
and copy the example source files from the
directory /usr/cacd/share/demo/sub3term (it
is supposed that demo directory has been installed under \fB/usr/cacd\fP).
.fS I
% cd sub3term
% cp /usr/cacd/share/demo/sub3term/* .
.fE
The layout description is put into the database using the program
.P= cgi.
.fS I
% cgi sub3term.gds
.fE
.SK
A top-view of the configuration is shown below (use e.g.
the layout editor
.P= dali
to inspect the layout).
Coordinates are in lambda.
.F+
.PSPIC "../spacesubman/exam1.eps" 3.7i
.F-
.P
An appropriate element definition file (with name "elem.s") is
as follows:
.fS I
% cat elem.s
#
# space element definition file for metal substrate terminals
#

colors :
    cmf   blue
    @sub  pink
.fE
.fS I
conductors :
  # name     : condition : mask : resistivity : type
    cond_cmf : cmf       : cmf  : 0.0         : m   
.fE
.fS I
contacts :
  # name     : condition : lay1 lay2 : resistivity
    cont_cmf : cmf       : cmf  @sub : 0.0    
.fE
.fS I
sublayers :
  # name       conductivity  top
    epi        6.7           0.0
    substrate  2.0e3        -7.0

#EOF
%
.fE
To use this file with
.P= space3d ,
it has to be compiled into a file "elem.t" using
.P= tecc.
.fS I
% tecc elem.s
.fE
.SK
The following parameter file is used for this example:
.fS I
% cat param.p
BEGIN sub3d
be_mode         0c   # piecewise constant collocation
max_be_area     1.0  # max. size of interior elements in sq. microns
edge_be_ratio   0.05 # max. size edge elem. / max size inter. elem.
edge_be_split   0.2  # split fraction for edge elements
be_window     inf    # infinite window, all resistances
END sub3d

disp.save_prepass_image  on
%
.fE
Then
.P= space3d
is used in combination with the option
.B -B
for 3D substrate resistance extraction, as follows:
.fS I
% space3d -v -E elem.t -P param.p -B sub3term
.fE
The circuit that has been extracted is retrieved
in SPICE format as follows:
.fS I
% xspice -a sub3term

sub3term

* Generated by: xspice 2.39 25-Jan-2006
* Date: 22-Jun-06 10:02:05 GMT
* Path: /users/space/sub3term
* Language: SPICE

* circuit sub3term c b a
r1 c a 1.265872meg
r2 c b 679.1891k
r3 c SUBSTR 75.86724k
r4 a b 679.1891k
r5 a SUBSTR 75.86724k
r6 b SUBSTR 49.44378k
* end sub3term

%
.fE
Alternatively
.P= Xspace
can be used for extraction.
.fS I
% Xspace -E elem.t -P param.p
.fE
Click button "sub3term" in the menu "Database",
click button "3D sub. res." in the menu "Options",
click button "DrawBEMesh" and "DrawGreen"
in the menu "Display",
and click "extract" in the menu "Extract" or use hotkey "e".
Note, to leave the
.P= Xspace
program, use hotkey "q".
This will yield the following picture:
.F+
.PSPIC "../spacesubman/X3dterm.eps" 5.0i
.F-
.H 2 "Run-time Versus Accuracy"
.sY %SE_TIMEVSACC%  \n(H1.\n(H2
The runtime of the program is largely dependent
on the values of the parameters that are used.
For example,
if sub3d.max_be_area is decreased (smaller elements are used),
the accuracy will increase but also
the number of elements will increase and the computation time will
become larger.
The larger the size of the window, the more accurate results are obtained
but also longer extraction times will occur.
The Galerkin method is more accurate than the collocation method,
but it also requires more computation time.
.P
Also, substrate resistance computation for configurations
consisting of 2 substrate layers
may require much more computation time 
than the same computation for configurations
consisting of 1 substrate layer.
This is because the computation
of the Green's functions requires much more time.
In this case, the computation time can be decreased
(on the penalty of some loss in accuracy)
by increasing the value for the maximum error for the evaluation
of the entries in the elastance matrix (green_eps).
.H 2 "Solving Problems"
.sY %SE_SOLV_PROB%  \n(H1.\n(H2
When the ratio between the conductivities of two different substrate
layers is large, it is possible that the following warning may occur:
.fS 
<prg>: Computation of Greens function truncated after 6 green_terms,
   error specified by green_eps not reached (layers are 'epi' and 'epi').
<prg>: Warning: maximum error not reached for 3.0% of the Greens functions.
.fE
Note that <prg> can be "space3d", "Xspace" or "makesubres".
.br
In addition, some direct coupling resistances between substrate contacts
may have a negative value.
This problem occurs because the computation of the Green's function 
for the layered substrate with a large difference between the
conductivities requires a lot of terms (iterations),
and a sufficient accuracy is not obtained after the maximum
number of terms has been reached
(see Section %SE_ELAST%).
In the above case "sub3d.max_green_terms" (default 500) is not reached,
because the Greens function computation is truncated after 6 green_terms.
This happens when "sub3d.green_eps" divergence occurs.
Set parameter "debug.print_green_terms" for more details.
If you set parameter "min_divergence_term" (note: without leading "sub3d.") to
a higher value (i.e. 20),
then the program stops not more so often.
The last warning is changed into:
.fS 
<prg>: Warning: maximum error not reached for 0.1% of the Greens functions.
.fE 
.P
There are two cases that should be considered when trying to find a solution
for this problem:
.P
First the case when the conductivity of the top layer is much larger
than the conductivity of the bottom layer.
In this case, when the top layer is thin, a solution may be to model 
only the top layer as a resistive conductor and use interconnect
resistance extraction only.
When the top layer is thick, reasonably accurate results may
be obtained by modeling the substrate by only the top layer
(with infinite thickness) and omitting the bottom layer.
.P
Second the case when the conductivity of the top layer is much smaller
than the conductivity of the bottom layer.
In this case a solution is often given by the fact that
the resistances between the substrate contacts and the substrate node
SUBSTR are accurately computed though and the (negative) coupling resistances
can simply be removed from the output.
Whether or not this is valid can be verified by changing the conductivity
of the bottom layer.
When the resistances to the SUBSTR node do not change very much, and when
the absolute values of the coupling resistances remain much larger
than those of the resistances to the SUBSTR node, the method is valid.
.P
Other possible useful "debug" parameters are:
.DS I
debug.print_cap3d_init       (default: off)
debug.print_green_init        (default: off)
debug.print_green_gterms   (default: off)
.DE
.SK
