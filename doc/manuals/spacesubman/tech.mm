.H 1 "Technology Description"
.sY %SE_TECH% \n(H1
.H 2 "Introduction"
For substrate resistance extraction, the space element definition file
should contain definitions of 
\fIsubstrate terminals\fP.
Substrate terminals are conducting polygons on top
of the substrate between which
substrate resistances are computed.
Currently, the following elements can be used to specify
substrate terminals:
.BL
.LI
contacts,
.LI
MOS transistors,
.LI
bipolar transistors,
.LI
capacitances.
.LE
.P
This is explained in more detail below.
.P
Also the substrate conductivity
and substrate resistances for typical terminal configurations 
are specified in the element definition file.
This information is used by respectively
the boundary-element method (see Section %SE_BEM%)
and the interpolation method (see Section %SE_INTER%)
to compute the substrate resistances.
.P
For basic information about the development of an element definition (technology) file,
see the Space User's Manual.
.H 2 "Substrate Terminals"
Normally,
when no 2D capacitance extraction is done
or when a 3D capacitance extraction is done,
the program shall also use the substrate terminals of not generated 2D surface capacitances.
Set the following parameter "on" to omit the unused ones.
.DS I
omit_unused_sub_term  \fIboolean\fP    (default: off)
.DE
Normally, all substrate terminal areas that are adjacent
are merged into one substrate terminal.
For the boundary-element method, substrate terminal areas 
that are adjacent are not merged when
the terminal areas are defined
by different conductor masks
(via a contact or capacitance element)
and the following parameter is set:
.DS I
sep_sub_term  \fIboolean\fP    (default: off)
.DE
Default, each substrate terminal is modeled as an
ideally conducting polygon and
one node is created for the substrate terminal that 
is connected to the substrate resistances that are computed 
as well as to the element(s) that define the substrate terminal.
To more accurately model distributed effects,
a substrate terminal can however 
also be modeled by more nodes.
This is achieved with the boundary-element method
when the terminal areas are defined by a conductor
(via a contact or capacitance element) for which interconnect
resistances are extracted, and the following
parameter is set
.DS I
sub_term_distr_<mask>  \fIboolean\fP    (default: off)
.DE 
where <mask> is the name of the corresponding conductor mask.
In this case,
(1) different adjacent substrate terminal areas 
that result from the layout
fragmentation due to the different mask combinations are not joined, and
(2) for each substrate terminal area 4 or 3 nodes (depending on
whether the area is a rectangle or triangle)
will be created at the corners of the substrate terminal area.
The nodes will connect to the nodes of resistance mesh of the conductor
that defines the substrate terminal area - either via a contact or via a 
capacitance (depending on whether the terminal is defined
via a contact or capacitance element) - as well as to the substrate
resistances that are computed for the substrate terminal area.
.H 2 "Contacts"
A contact in the element definition file specifies a substrate terminal
(and hence is a substrate contact)
(1) if one of the two masks that are specified with the contact
is replaced by the string "@sub"
or (2) if one of the two masks is replaced by
the notation "%(\fIcondition_list\fP)".
In the first case, the area of the substrate terminal is defined
by the area of the contact.
In the second case, the area of the substrate 
terminal is defined by the condition list
between the parentheses.
The contact connects the substrate terminal to the other mask that 
is specified with the contact,
possibly via resistances if a contact resistance is specified.
.E(
The following specifies a substrate contact that defines
a substrate terminal and that directly connects the substrate terminal
(contact resistance is 0) to the first metal
layer "cmf".
.fS
contacts :
    cont_b : cca cmf !cwn !csn : cmf @sub : 0.0 # metal to sub.
.fE
Alternatively the contact could have been specified as follows.
.fS
contacts :
    cont_b : cca cmf !cwn !csn : cmf %(cca !cwn !csn) : 0.0 
.fE
.E)
.H 2 "MOS Transistors"
A MOS transistor in the element definition file
specifies a substrate terminal 
(1) if the string "@sub" is used for its bulk connection
or (2) if the notation "%(\fIcondition_list\fP)" is used
for its bulk connection.
In the first case, the area of the substrate terminal is defined
by the area of the transistor gate.
In the second case, the area of the substrate 
terminal is defined by the condition list
between the parentheses.
When the second case is used, the area specified by the condition 
list must have an overlap with the transistor gate area.
The bulk connection of the transistor is then connected to
the substrate terminal.
.E(
The following specifies a transistor that has as a bulk
connection a substrate terminal.
The area of substrate terminal is equal
to the area of the transistor gate.
.fS
fets :
    nenh : cpg caa csn : cpg caa : @sub   # nenh MOS
.fE
In the following example the area of the substrate terminal
includes the transistor gate as well as the drain/source areas.
.fS
fets :
    nenh : cpg caa csn : cpg caa : %(caa csn)  # nenh MOS
.fE
.E)
.H 2 "Bipolar Transistors"
A bipolar transistor in the element definition file
specifies a substrate terminal if the notation "%(\fIcondition_list\fP)" 
is used for its bulk connection.
The area of the substrate terminal is defined by the condition list
between the parentheses.
The area specified by the condition list must have an overlap with 
the transistor area.
The bulk connection of the transistor is then connected to
the substrate terminal.
.E(
The following specifies a bipolar transistor that
has as a bulk connection a substrate terminal.
.fS
bjts :
    npnBW : bw wn : ver : wn bw epi : %(bw wn)  # ver. NPN
.fE
.E)
.H 2 "Capacitances"
A capacitance in the element definition file
specifies a substrate terminal 
(1) if the string "@sub" is used
for one of the terminal masks of the capacitance
or (2) if the notation "%(\fIcondition_list\fP)" is used
for one of the masks.
For case (1):
If the capacitance is a surface capacitance,
the area of the substrate terminal
is defined by the area of the capacitance.
The corresponding terminal of the capacitance is then connected
to the substrate terminal.
If the capacitance is an edge capacitance,
the edge capacitance must be adjacent to a substrate terminal
that is defined by another element (e.g. surface capacitance)
and the corresponding terminal of the capacitance is then connected
this substrate terminal.
For case (2): 
The area of the substrate
terminal is defined by the condition list
between the parentheses.
The area of the substrate terminal must then coincide
or overlap the area of the capacitance.
When substrate resistances are extracted and when no substrate terminal
is recognized underneath the capacitance to substrate,
the capacitance for that part of the layout is ignored.
.P
A capacitance that defines a substrate terminal
can be either a normal (linear) capacitance or 
a junction capacitance.
.E(
The following specifies bottom and side-wall
capacitances to the substrate of a diffusion area.
They are modeled on top of the substrate
(i.e. the thickness of the diffusion area is not taken
into account when the substrate resistances are computed).
.fS
junction capacitances ndif :
    acap_na:  caa       !cpg  csn  !cwn :  caa @sub: 100e-6
    ecap_na: !caa -caa !-cpg -csn !-cwn : -caa @sub: 300e-12
.fE
.E)
.SP
.N(
Be careful not to specify too many capacitances
as capacitances with a substrate terminal.
This may result in (a large number of) large substrate terminals.
.N)
.N(
Although well-substrate capacitances can also be modeled
using the above method, this should be done carefully since the wells
may define large substrate terminals (equipotential areas)
on top of the substrate.
The resistive coupling between the elements within a well can best 
be modeled by defining the well as a conductor.
.N)
.P
Capacitances are not used as substrate terminals
during substrate resistance extraction if the string (pin) "@gnd"
is used instead of the string "@sub" or instead of
the "%(\fIcondition_list\fP)" notation.
.br
.ne 10
.H 2 "Substrate Conductivity"
.sY %SE_TECHBEM% \n(H1.\n(H2
Syntax:
.DS I
\fCsublayers\fP :
   \fIname\fP  \fIconductivity\fP  \fItop\fP
     .
     .
.DE
Specifies the conductivity of the substrate.
This information is used by the boundary-element method
to compute the resistances between the substrate terminals
(see Section %SE_BEM%).
It is specified in
the element definition file after the specification
of the capacitances and the specification of the information that
is used for 3D capacitance extraction.
.P
In the current release, the maximum number
of different substrate layers is 2.
For each layer,
\fIname\fP is an arbitrary label that will be used for error messages etc,
\fIconductivity\fP is a real number giving the conductivity of that
layer in Siemens/meter,
and \fItop\fP specifies (in microns) the top of the substrate layer.
The positive z direction is out of the substrate.
Therefore, the value of \fItop\fP must be $<= ~ 0$.
The layers are enumerated starting from the top.
For the first substrate layer, \fItop\fP must be zero.
If a second substrate layer is present,
the bottom of the first layer is at the top
of the second layer.
The bottom of the last substrate layer
is at minus infinity.
.E(
.fS
sublayers :
  # name       conductivity  top
    epi        6.7           0.0
    substrate  2.0e3        -7.0
.fE
.E)

Note that in this release it is possible to specify a conducting back side
(grounded substrate).
This can be specified with a special last (2nd or 3th) sublayer with the name "metalization".
A finite substrate thickness is specified with the top value.
Note that the conductivity value of this entry is not used, but must hold a dummy value.
However, a conductivity value for the medium above the substrate is derived from
the first sublayer.
This first value is default divided by 100.
Another divisor can be specified with parameter "sub3d.neumann_simulation_ratio".
This parameter can be specified in the technology file (see example below).
.E(
.fS
sublayers :
  # name       conductivity  top
    epi        6.7           0.0
    substrate  2.0e3        -7.0
 metalization  dummy_value -20.0
 neumann_simulation_ratio : 1000
.fE
.E)
.H 2 "Substrate Permittivity"
.sY %SE_TECHBEM% \n(H1.\n(H2
Syntax:
.DS I
\fCsubcaplayers\fP :
   \fIname\fP  \fIpermittivity\fP  \fItop\fP
     .
     .
.DE
Specifies the relative permittivity of the substrate.
This information is used by the boundary-element method
to compute the capacitances between the substrate terminals (see Section %SE_BEM%).
It is specified in the element definition file after the specification
of the "sublayers" part.
The
.I top
must be specified like in the "sublayers" part.
In the current release, the maximum number
of different subcaplayers is 2.
.H 2 "Typical Substrate Resistances"
.sY %SE_TECHINTER% \n(H1.\n(H2
The interpolation method
uses typical substrate resistances for standard terminal configurations
to compute the substrate resistances
(see Section %SE_INTER%).
This information is specified in the element definition file,
after the (possible) specification of the substrate conductivity.
It consists of a list of so-called "selfsubres" entries and a list
of so-called "coupsubres" entries.
.P
.DS I
\fCselfsubres\fP :
   \fIarea\fP  \fIperimeter\fP  \fIresistance\fP  \fIrest\fP
     .
     .
.DE
This is a list that specifies for different substrate terminals
the resistance between that substrate terminal and the substrate node
(see Section %SE_INTER%).
Each entry consists of a specification
of (1) \fIarea\fP: the area of a substrate terminal (in square microns),
(2) \fIperimeter\fP: its perimeter (in microns),
(3) \fIresistance\fP: its resistance to the substrate node,
and (4) \fIrest\fP: the part of the \fIconductance\fP to the substrate node
(= 1/resistance to the substrate node)
that is a lower bound for the conductance to the substrate
node (i.e. if the conductance to the substrate node is decreased
because direct coupling resistances are connected to the substrate
terminal - see below - the conductance can not decrease below this value).
To optimize the interpolation method, the entries should preferably
be organized in groups of 3, where each second entry has an area equal 
to the area of the first entry and each third entry has a perimeter equal 
to the perimeter of the first entry (see the example below).
.DS I
\fCcoupsubres\fP :
   \fIarea1\fP  \fIarea2\fP  \fIdistance\fP  \fIresistance\fP  \fIdecrease\fP
     .
     .
.DE
This is a list that specifies for different pairs of substrate terminals
the direct coupling resistance between these terminals.
Each list entry consists of a specification
of (1) \fIarea1\fP: the area of a terminal,
(2) \fIarea2\fP: the area of another terminal,
(3) \fIdistance\fP: the minimum distance between these terminals,
(4) \fIresistance\fP: the corresponding direct coupling resistance,
and (5) \fIdecrease\fP: the part of the direct coupling \fIconductance\fP
(= 1/direct coupling resistance)
that is,
for each of the substrate terminals the direct coupling 
resistance is connected to,
subtracted from the conductance to the substrate node.
.P
Note that areas need to be specified in square microns and distances in microns.
.E(
.fS
selfsubres:
  # resistances to substrate node
  #   area    perim          r   rest
      0.48      3.2    88205.1   0.01 
      0.64      3.2    81286.8   0.01 
      0.64      4.0    73678.4   0.01 
      1.92      6.4    44102.5   0.01 
      2.56      6.4    40643.4   0.01 
      2.56      8.0    36839.2   0.01 
      7.68     12.8    22051.2   0.01 
     10.24     12.8    20321.7   0.01 
     10.24     16.0    18419.9   0.01 

coupsubres:
  # direct coupling resistances
  #  area1   area2   dist          r   decr
      0.64    0.64    1.6     648598   0.873512 
      0.64    0.64    3.2    1101504   0.925946 
      0.64    0.64    6.4    1996617   0.959256
      0.64    0.64   25.6    7341756   0.988935
      2.56    2.56    3.2     324299   0.873515
      2.56    2.56    6.4     550752   0.925953
      2.56    2.56   12.8     998308   0.959253
      2.56    2.56   51.2    3670877   0.988967

.fE
.E)
.N(
The above parameters, for the interpolation method,
can automatically
.br
be generated with tool
.P= subresgen
(see "icdman subresgen").
.N)
