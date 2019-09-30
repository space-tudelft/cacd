.H 1 "Technology Description"
.sY %SE_TECH% \n(H1
.H 2 "Introduction"
For 3D extraction, the space element definition file
is extended with a description of the vertical dimensions
of the conductors, optionally, a description
of the edge shapes of the conductors,
and a description of the dielectric structure of the circuit.
Information about these specifications is given below.
For basic information about the development of an element definition file (technology file),
see the Space User's Manual.
.H 2 "Unit Specification"
Optionally the unit for distances in the vertical dimension list and
the unit for distances in the shape lists are
specified in the unit specification of the element definition file.
A unit for the vertical dimension list is specified by means
of the keywords
.I unit
and 
.I vdimension,
followed by the value of the unit.
A unit for the edge/cross-over shape list is specified by means
of the keywords
.I unit
and 
.I shape,
followed by the value of the unit.
.E(
The following specifies a unit of 1 micron for distances that are given
in the vertical dimension list and for distances 
that are given in a shape list:
.fS
unit vdimension    1e-6   # micron
unit shape         1e-6   # micron
.fE
.E)
.H 2 "The Vertical Dimension List"
.sY %SE_VERLIST% \n(H1.\n(H2
Syntax:
.DS I
\fCvdimensions\fP :
.P
   \fIname : condition_list(s) : mask : bottom thickness\fP
     .
     .
   [ skip_cap3d \fI: name1 name2\fP [\fI: capacitivity\fP] ]
   [ keep_cap2d \fI: cap2d-name\fP ]
.DE
The vertical dimension list specifies for different conductors
under different conditions (e.g. metal2 above polysilicon or metal2 above
metal1)
(1)
\fIbottom\fP:
the distance between the substrate and the bottom of the
conductor 
(2)
\fIthickness\fP:
the thickness of the conductor.
The vertical dimension list is included in the element definition file
after the specification of the standard (non-3D) elements
(see the Space User's Manual).
.P
Optional "skip_cap3d" can be specified for partial 2D surface capacitance
extraction between the specified two vdimension names.
This is especial useful for large conductor plates and plates which lay very close to each other.
The to use 2D surface capacitivity value can be specified.
A zero value skips the surface capacitance completely.
Default the value is calculated from the dielectric structure.
.br
The "keep_cap2d" option can be specified for each 2D capacitance
you want to keep during 3D capacitance extraction.
.E(
An example of an almost minimal technology file (with corresponding
geometry) is given below. 
While minimal, this file can actually be complete
(except for a specification of the dielectric structure)
for 3D extraction for a double metal process in which only metal1 
and metal2 capacitances are extracted.
.fS
unit vdimension    1e-6  # micron

conductors :
    metal1 : in  : in  : 0
    metal2 : ins : ins : 0

vdimensions :
    metal1_shape : in  : in  : 1.6 1.0
    metal2_shape : ins : ins : 3.3 1.2
.fE
.E)
.PS 1.5i
copy "../space3dman/vdim.pic"
.PE
At a transition area, where a conductor goes from
one bottom and thickness specification
to another bottom and thickness specification,
the slope of the conductor is determined by the parameter
.I default_step_slope
(see Section %SE_MESHCON%).
.N(
To prevent the overlap of different transition areas
of one conductor (which currently results incorrect element meshes),
the differences in bottom and thickness specifications
of one conductor may not be too large (otherwise: increase the parameter
\fIdefault_step_slope\fP).
.N)
.P
See Section %SE_DIFFUSED% for a specification of diffused conductors.
.br
.ne 10
.H 2 "The Edge Shape List"
.sY %SE_ESHAPES% \n(H1.\n(H2
Syntax:
.DS I
\fCeshapes\fP :
.P
   \fIname : condition_list(s) : mask : db dt\fP
     .
     .
.DE
The edge shape list specifies for different conductors,
the extension of each conductor in the x (or y) direction
relative to the position of the original conductor edge in the layout.
The first value (\fIdb\fP) specifies the extension of the \fIbottom\fP
of the conductor and the second value (\fIdt\fP) specifies the extension of 
the \fItop\fP of the conductor.
Either extension may be negative.
The edge shape list should be present 
in the element definition file
after the vertical dimension list.
.E(
.fS
eshapes :
    metal1_eshape : !in -in : in : 0.2 0.1
.fE
.E)
.PS 1.5i
copy "../space3dman/eshape.pic"
.PE
.N(
In some cases, the use of eshapes may cause mesh generation problems 
because e.g. at corners the order of mesh nodes can get mixed up.
Because of that,
when \fIdb\fP and \fIdt\fP have (approximately) the same value, 
it is often better to use a
resize statement (see Space User's Manual) instead of an eshape
statement, since this option is less likely to cause mesh
generation problems.
.N)
.ne 10
.H 2 "The Cross-over Shape List"
.sY %SE_CSHAPES% \n(H1.\n(H2
Syntax:
.DS I
\fCcshapes\fP :
.P
   \fIname : condition_list(s) : mask : db1 dt1 db2 dt2\fP
     .
     .
.DE
The cross-over shape list specifies for different conductors,
the extensions of the \fIbottom\fP and \fItop\fP of each conductor in the x (or y) direction
relative to the position of a transition edge in the layout.
The transition edge, is an edge (caused by another mask),
where a conductor goes from
one bottom and thickness specification
to another bottom and/or thickness specification.
Thus, there must be two vdimension specifications for the same conductor.
The cross-over shape specification overrules the
.I default_step_slope
method (see Section %SE_MESHCON%).
.F+
.PSPIC "../space3dman/cshape.ps" 5.9i 2.6i
.F-
Note that the first and third value (\fIdb1\fP and \fIdb2\fP) specify the extension
of the \fIbottom\fP of the conductor (at left and right side of the transition,
\fIdb1\fP to \fIb1\fP the lowest \fIbottom\fP value) and
the second and fourth value (\fIdt1\fP and \fIdt2\fP) specify the extension
of the \fItop\fP of the conductor.
Each extension value can may be negative.
But be warned, some extension combinations can result in an incorrect mesh.
The cross-over shape list should be present 
in the element definition file after the edge shape list.
.E(
.fS
vdimensions :                     # bottom thickness
    ver_cpg_of_caa : cpg !caa  : cpg : 1.6 0.5
    ver_cpg_on_caa : cpg  caa  : cpg : 1.8 0.4

cshapes :
    cpg_cshape : cpg !caa -caa : cpg : 0.1 0.1 0.1 0.0
.fE
.E)
.H 2 "Dielectric Structure"
.sY %SE_DIELSTRUCT% \n(H1.\n(H2
Syntax:
.DS I
\fCdielectrics\fP :
.P
   \fIname\fP  \fIpermittivity\fP  \fIbottom\fP
     .
     .
.DE
Specifies the dielectric structure of the chip.
This specification is included in the element definition file
after the vertical dimension list
and the shape lists.
For each layer,
\fIname\fP is an arbitrary label that will be used for error messages etc,
\fIpermittivity\fP is a real number giving the relative dielectric constant,
and \fIbottom\fP specifies (in microns) the bottom of the dielectric layer.
The value of \fIbottom\fP must be $>= ~ 0$.
The first dielectric in the list specifies the lowest dielectric, 
the second dielectric the second
lowest, etc.
For the first dielectric layer \fIbottom\fP must be zero.
The top of a dielectric layer is at the bottom
of the next dielectric.
The top of the last dielectric
is at infinity.
No dielectric layers means vacuum.
If one or more dielectric layers are specified,
a ground plane at zero is present.
.E(
.fS
dielectrics :
   # Dielectric consists of 5 micron thick SiO2
   # (epsilon = 3.9) on a conducting plane.
   SiO2   3.9   0.0
   air    1.0   5.0
.fE
.E)
.N(
If more than 3 dielectric layers are specified, you should pass the \fB-u\fP
option to \fItecc\fP.
This enables the "unigreen" method, which is a different 
kind of computation that allows for more than 3 dielectric layers.
Note that the "unigreen" method is not available in the public version.
.N)
.H 2 "Additional Parameters in the Dielectrics Section"
This section discusses several additional parameters which may be specified
in the \fCdielectrics\fP section of the technology file.
\fBSpecifying these parameters should not be necessary under normal circumstances.\fP
They are present for advanced tuning of \fIspace3d\fP.
Note that these parameters will only be used when you specify
the \fB-u\fP option to \fItecc\fP (i.e., when the "unigreen" method is enabled).

Internally, \fIspace3d\fP needs to compute voltage values at different locations in the
dielectric layer structure, given a unit charge at some location. This computation
is known as the "Green's function" computation.
Let $z sub q$ be the elevation of the source point (i.e., of the unit charge), above the
$z=0$ plane; let $z sub p$ be the elevation of the observation point; and let $r$ be
the lateral distance between source and observation point. In vacuum, the
Green's value can be computed from
.DS
.EQ
g ~=~ { { 1 } over { 4 pi epsilon } } ~*~
{ { 1 } over { sqrt { { { ( { z sub p } - { z sub q } ) } sup 2 } ~+~ { r sup 2 } } } }.
.EN
.DE
When one or more dielectric layers are present, this formula quickly becomes more complicated
(this is due to the fact that a single charge will polarize the dielectric layers, which
will result in surface charges at the interfaces of these layers).
Hence, if computation speed is important, and if your technology is complicated, it might 
be worthwhile to simplify the layer structure somewhat, e.g. by merging layers for which
the $epsilon sub r$ are almost equal.

For small values of $r$, \fIspace3d\fP uses an algorithm based on simulated annealing to 
compute $g$. For large values of $r$, \fIspace3d\fP will switch to an interpolation method.

For the interpolation method, \fIspace3d\fP needs to divide the layer structure into a grid.
The actual computation of values on the grid is done by the technology compiler (\fItecc\fP), as a
preprocessing step. Thus, although computing Green's values on the grid may take quite 
some time, this time is only spent while compiling the technology file, and not when
\fIspace3d\fP is actually processing a layout.
But note that this computation is not available in the public version.

.H 2 "Diffused Conductors"
.sY %SE_DIFFUSED% \n(H1.\n(H2
Diffused conductors
(which for example implement the MOS transistor source and drain regions)
are described in a somewhat different way than the poly-silicon
and the metal conductors.
The approach is illustrated in Figure %FG_DiffusedCap%.
.DS
.PS 5i
copy "../space3dman/diffmod.pic"
.PE
.fG "Illustration of the heuristic approach to incorporate diffusion \
capacitances, physical structure (a) \
and 3D capacitance model (b)." %FG_DiffusedCap%
.DE
Figure %FG_DiffusedCap%.a shows a cross-sectional view
of a diffused conductor.
The capacitance model employed by
.P= space3d
for such a conductor
is shown in Figure %FG_DiffusedCap%.b,
where the diffused interconnect is replaced
by a thin sheet conductor.
Therefore, the user must specify in the element definition file
a zero thickness for the conductor.
The sheet conductor is positioned
half the thickness of the field oxide above the
ground plane, which is flat and continuous,
and must be thought of as
modeling the top side of the diffused conductors.
.P
Initially,
the 3D capacitance extraction method 
will compute 3D capacitances between
all conductors and ground.
The 3D capacitances between non-diffused conductors
mutually, between non-diffused conductors and ground and between
non-diffused conductors and diffused conductors are inserted in the 
extracted circuit.
However, the 3D capacitances between diffused conductors and ground
and between diffused conductors mutually are better represented by
junction capacitances that are computed using
an area/perimeter method.
.N(
Therefore, the 3D capacitances between diffused conductors
and ground,
and between diffused conductors mutually,
are discarded by the program.
The junction capacitances that replace these capacitances
have to be specified separatedly by the user in
the element definition file.
.N)
See also Section %SE_NON3DCAP%.
.P
Although this approach is purely heuristic,
its results are satisfactory when the width of the diffusion
paths is large enough compared to the height of the sheet conductors
above the ground plane.
.P
.N(
A conductor is defined as a diffused conductor within
.P= space3d
if and only if in the element definition file
the type of the conductor is specified as 'n' or 'p'.
.N)
.H 2 "Gate Capacitances"
.sY %SE_GATECAP% \n(H1.\n(H2
When extracting 3D capacitances,
.P= space3d
assumes that the gate-channel capacitances of field-effect transistor
are included in the simulation model that is used
for the extracted transistor.
Therefore it discards the 3D capacitance to ground for
conductor parts that are a gate of a field-effect transistor
and that are directly above the transistor area as defined in the element
definition file.
.P
Also the 3D coupling capacitances between gates and diffused
conductors (drain/source areas; see Section %SE_DIFFUSED%) 
can be discarded by the program, depending whether they are
present in the SPICE or other simulation model for the device.
This is achieved by turning on the parameter
cap3d.omit_gate_ds_cap
in the parameter file.
.H 2 "Non-3D Capacitances"
.sY %SE_NON3DCAP% \n(H1.\n(H2
When extracting 3D capacitances, non-3D capacitances that are 
specified in the element definition file are not extracted,
except for capacitances between a diffused conductor and ground
and capacitances between diffused conductors mutually
(see also Section %SE_DIFFUSED%).
However, when the parameter
cap3d.all_non3d_cap
is set, also all non-3D capacitances that are defined
in the element definition file, are extracted.
