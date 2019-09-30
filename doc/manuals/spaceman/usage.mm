.EQ
define afm2 'roman { [ aF / mu m sup 2 ]}'
define afm 'roman { [ aF / mu m ] }'
define Ceb 'C sub eb'
define Cet 'C sub et'
define Cpp 'C sub pp'
define Cppt 'C sub ppt'
define Cppb 'C sub ppb'
define Cl  'C sub l'
define Ce  'C sub e'
.EN
.H 1 "Space Program Usage"
.H 2 "Introduction"
This chapter will describe
.P= space .
Its many characteristic features are discussed in separate sections
within this chapter.
Each of the features is controlled by one or more of the following:
.I(
.I= "Command-Line Options"
These are mainly flags that can be used to enable or disable some
aspects of
the extraction process
or otherwise influence the behavior of
.P= space .
For example,
capacitance extraction is disabled by default,
but can be turned on by using the
.O= -c -cap
option.
Some options can also be specified in the parameter file.
Options overrule specifications in the parameter file.
.I= "The Element Definition File"
In this file,
it is described how
.P= space
can recognize the circuit elements
(transistors, contacts, interconnects etc.)
from the layout.
This file also contains unit values for capacitances and resistances.
By default,
.P= space
reads this file from the appropriate directory of the ICD process library.
See also Section %SE_ELEMDEFFILE% and Chapter %SE_MKELEM%.
.I= "The Parameter File"
This file contains values for several variables that
control the extraction process,
like minimum values of resistances to be retained in the extracted netlist.
Also this file is default read
from the appropriate directory of the ICD process library.
Some parameters can also be specified as options.
Options overrule specifications in the parameter file.
See also Section %SE_PARAMFILE%.
.I)
The relevant aspects of each of these control mechanisms
are discussed in each section,
and are summarized at the end of each section.
An overview of all
command-line options is given in appendix %AX_OptionOverview%.
.H 2 "General"
.H 3 "Invocation and Basic Options"
.P= Space
is normally invoked via the
Unix command interpreter, or shell,
as follows:
.DS I
\fCspace\fI [options] cellname [cellname ...]
.R
.DE
The
.I cellname
arguments specify which layouts are to be extracted,
at least one cellname has to be specified.
The options influence the behavior of
.P= space ,
as will be shown in the rest of this chapter.
A basic option is
.O= -v -verbose "" ,
which makes
.P= space
prints some useful information during the extraction process.
.H 3 "Hierarchical and Flat Extraction"
By default,
.P= space
operates in \fBhierarchical mode\fP.
In this mode,
the circuit that is produced has the same hierarchical structure
as the layout from which it is derived.
.P= Space 
will traverse the hierarchy itself,
it is thus only necessary to specify the root(s) of the tree(s)
to be extracted on the command line.
To forbit traversal use option \fB-T\fP.
Note that
.P= space 
does never extract imported sub-cells (cells of other projects).
.P
.P= Space
operates in \fBflat mode\fP when using the
.O= -F -flat
option.
In flat mode,
the layout will be fully instantiated
(flattened)
before extraction and the circuit will have no sub-cells
(except library and device cells, see \fIxcontrol(1ICD)\fP).
.H 3 "Mixed Flat/Hierarchical Extraction"
.sY %SE_MIXHF% \n(H1.\n(H2.\n(H3
In \fBhierarchical mode\fP,
it is possible to perform a mixed flat/hierarchical extraction
and by
defining one or more layout cells to have the macro status
using \fIxcontrol(1ICD)\fP.
.br
In that case, in contrast
to the instances of other cells,
all instances of macro cells are flattened before extraction.
When a macro cell has one or more child cells itself, the macro status
of each of these child cells determines if the instances of these
cells are also flattened.
The use of macros may for example be advantageous when using
small sub-cells for which it is cumbersome to define terminals 
(see Section %SE_USAGETERM%),
or when extracting sea-of-gates circuits,
where the image is in a separate cell that is instantiated ``under''
all other cells.
.P
In \fBflat mode\fP, it is also possible to perform a mixed flat/hierarchical extraction.
This can be done by setting the library or device status for
one or more sub-cells with \fIxcontrol\fP.
.br
In that case, the layout of the whole circuit will be expanded,
expect for the instances of library and device cells, which will
be included as instances in the extracted netlist.
.H 3 "Incremental Operation"
In \fBhierarchical mode\fP,
.P= space
operates in
.I "incremental mode"
by default.  
In incremental mode,
.P= space
does not extract sub-cells for which the circuit is up-to-date.
.P= Space
uses an internal algorithm to select cells for
(re-)extraction, as follows:
.I(
.I=
extract all cells that have not yet been extracted,
.I=
extract all cells of which the layout is newer than the extracted circuit,
.I=
extract all cells that have one or more child cells
of which the layout is newer than the extracted circuit,
.I=
extract all cells that have one or more child cells
for which the extraction status (see \fIxcontrol(1ICD)\fP)
has been changed after the last extraction of the cell,
.I=
extract cells that are at a depth <=
.I depth,
where
.I depth
is specified with the option
.O= -D -mindepth depth.
The root of the tree is at depth 1,
its children are at depth 2, and so on.
By default, \fIdepth\fP = 1, so 
cells that are are given
as argument with
.P= space
are always extracted.
.I)
Incremental mode is disabled completely with the
.O= -I -noincremental
option.
In this non-incremental mode,
all cells in the tree are extracted.
(Note that this option is equal to specifying a large number for
\fIdepth\fP.)
The options
.O= -I -noincremental
and
.O= -D -mindepth
are 
useful when changing extraction style,
e.g. to extract all cells with coupling capacitances if they
were done with substrate capacitances only.
.H 3 Terminals
.sY %SE_USAGETERM% \n(H1.\n(H2.\n(H3
An electrical network or circuit as extracted by
.P= space ,
is built up from primitive elements
(transistors, capacitors and resistors),
subcircuits (when the input layout is hierarchical)
and terminals.
.I Terminals
(sometimes also called
.I ports
or
.I pins )
are connectors to the outside world
and are used for building up the hierarchical structure of the circuit.
The extracted netlist specifies the connectivity among all these elements.
.P
For
.P= space
to be able to produce these terminals in the extracted circuit,
the input layout must also have terminals.
Layout terminals are named rectangular areas or points
(this is dependent on how they are specified in the layout
input file or using the layout editor) in an interconnect
layer of the layout.
.P= Space
uses the terminals in the layout to create, name and connect
the terminals in the circuit
according to
a one-to-one correspondence between layout terminals
and circuit terminals.
Terminal names must be unique within a cell.
.P
For flat extraction,
only the topmost cell must have terminals.
For hierarchical extraction,
the topmost cell and all the other cells in the hierarchy
must have terminals, unless
the cell is used as a macro (see Section %SE_MIXHF%).
.P= Space
identifies a connection from a cell being extracted
(the parent cell)
to a sub-cell (or child cell)
when a layout feature of the parent cell touches or overlaps
a terminal of the child cell.
.P= Space
identifies a connection between two sub-cells when a terminal
of one sub-cell touches or overlaps a terminal of another sub-cell.
.N(
If a sub-cell does not have terminals,
set the macro status for the sub-cell,
or use flat extraction only.
.N)
In case of hierarchical extraction (without setting the macro status),
.P= space
would not detect such connections.
In fact,
.P= space
would be unable to represent those non-hierarchical connections
in a hierarchical circuit netlist.
The program will not give a warning in case of
such non-hierarchical connections.
.N(
If a sub-cell has terminals but the sub-cell is connected
via layout polygons of the sub-cell that are only extensions
the real terminal areas of the sub-cell,
set the interface type for the sub-cell to free (or freemasks)
(using \fIxcontrol(1ICD)\fP)
or use flat extraction only.
.N)
By setting the interface type for the sub-cell to free (or freemasks),
the layout of the sub-cell will be expanded in the father cell
(with freemasks only for certain masks)
and the sub-cell will be appropriately connected via its terminals.
Note, however, that other elements may then also be recognized in
the parent cell (see also Section %SE_LIBRARYCELLS%).
.H 3 "Limitations of Hierarchical Extraction"
Hierarchical extraction is also impossible
(in the sense that it results in an incorrect circuit)
if the functionality of the child cells is modified
by layout features in the parent cells.
For example,
a polysilicon feature in a parent cell that overlaps
an active area feature in a child cell modifies the circuit
of the child cell by creating a transistor in it.
Flat extraction is not a problem in this case
and will result in a correct circuit.
.P
While flat extraction is necessary 
in some cases as mentioned above,
it is recommended when it is required to get the highest accuracy
of the parasitics extracted.
One reason for this is the fact that (parasitic)
coupling capacitances between layout features
of different cells don't fit in
a hierarchical circuit description.
However,
flat extraction is often not a significant burden because
.P= space
is very fast.
Also,
the extracted circuit is often used for simulation
and most simulators must flatten the circuit anyway,
since internally they only can work on a flattened netlist.
.P
Appendix %AX_Terminals% gives some guidelines
for how to construct a layout
such that a hierarchical extraction by
.P= space
will suffer least from these difficulties and hence
is more accurate.
.H 3 "Command-Line Options"
.M(
.ME -v -verbose
Set verbose mode.
.ME -F -flat
Set flat extraction mode, i.e. produce a flattened netlist.
.ME -T -top
In hierarchical mode, only extract the top cell(s).
.ME -I -noincremental
Unset incremental (hierarchical) mode:
do not skip sub-cells
.br
for which the circuit is up-to-date (\fIdepth\fP = $inf$).
.ME -D -mindepth depth
Selectively unset incremental (hierarchical) mode for all cells
.br
at level <= \fIdepth\fP
(default \fIdepth\fP = 1).
.ME -u ""
Do not automatically run the preprocessors
.I makeboxl(1ICD)
.br
and
.I makegln(1ICD).
.M)
.H 3 "Parameter File"
When the parameter
.I flat_extraction
is specified in the parameter file,
a flat extraction is performed.
.DS I
.TS
tab(:);
l c c c c.
_
parameter:type:unit:default:suggestion
_
flat_extraction:boolean:-:off:-
_
.TE
.DE
.H 2 "Extraction of Field-effect Transistors"
.P= Space
recognizes field-effect transistors (e.g. MOS transistor)
from the different mask combinations
in the layout (see Section %SE_THE_ELEM%).
The extractor will calculate the width and length of each field-effect
transistor.
For that matter,
.P= space
uses perimeter and surface formulas,
as described below.
When the
.O= -t -torpos
option is used,
the position of each transistor is specified in the extraction
output.
.H 3 "Drain-source connections"
For each field-effect transistor 
.P= space
will extract one
gate connection, optionally a bulk connection,
and one drain/source connection 
for each set of drain/source areas that are connected.
As an example, for the following layout of a transistor
(which has 5 drain/source areas, divided into 2 different sets),
.P= space
will extract one gate connection,
possibly a bulk connection (if this connection is specified in the element
definition file),
one drain connection and one source connection.
.PS 2.8i
copy "../spaceman/5dsTor.pic"
.PE
If a field-effect transistor is used as a capacitor and has only
one connected set of drain/source areas,
two connections will be generated that are however attached to the same net.
If a field-effect transistor has more than two drain/source connections,
only two of these connections appear in the extracted circuit.
(These will in general be the connections that are connected
to other parts of the circuit; dangling connections are not used
in this case.)
.H 3 "Transistor width and length calculation"
For each field-effect transistor, 
its width and length are calculated and stored in the attributes w and l.
The transistor width and length are calculated from
.DS
.EQ I ""
length ~=~ { per sub g } over { N sub g }
.EN
.sp 1
.EQ I ""
width ~=~ { A } over { length }
.EN
.DE
where 
.in +1c
.sp 0.3
$per sub g$ is the part of the perimeter of the transistor where
the gate extends the active area (see below),
.sp 0.3
$N sub g$ is the number of separate areas where this occurs, and
.sp 0.3
$A$ is the total area of the transistor.
.in -1c
.DS
.PS 3.5i
copy "../spaceman/perexam.pic"
.PE
.DE
.P
.ne 4
If the transistor has no gate overlaps ($ N sub g ~=~ 0 $)
the width and length of the transistor are calculated from:
.DS
.EQ I
width ~=~ { per sub tot } over { 2 }
.EN
.sp 1
.EQ I
length ~=~ { A } over { width }
.EN
.DE
where 
.in +1c
$per sub tot$ is the total perimeter of the transistor.
.in -1c
.H 3 "Transistor drain/source parameters calculation"
When a condition list for the drain/source region of the transistor
is specified in the element definition file (see Section %SE_TORELEM%),
and when capacitance extraction is enabled,
the area, perimeter and number of equivalent squares for both drain and 
source region will be computed.
In the perimeter of the drain and source, the edge that coincides
with the transistor channel, is not included.
When more than one transistor is connected to a drain/source region, the
area value that is obtained for the total drain/source region is subdivided 
over the different transistors in proportion to the widths of the 
transistors.
Apart from some constant that represents the length of the 
subdivided drain/source regions, the perimeter value is split 
in a similar way.
The number of equivalent squares $nrx$ for the drain/source
region of a transistor is computed from:
.DS
.EQ I
nrx ~=~ { ax } over { width sup 2 }
.EN
.DE
where $ax$ is the area of the drain/source region and $width$ is the
width of the transistor.
.H 3 "Command-Line Options"
Extraction of field-effect transistors is controlled by the following option:
.M(
.ME -t -torpos
Add positions of devices and sub-cells to the extracted circuit.
.M)
.H 3 "Element Definition File"
See section %SE_TORELEM% for the specification of the field-effect
transistor elements.
. \" .SK    \" NOTE NOTE NOTE !!!!!! This is done to prevent title at end of page
.H 2 "Bipolar Device Extraction"
.sY %SE_BIPEXT% \n(H1.\n(H2
.P= Space
recognizes bipolar junction transistors 
(BJT's)
by combining
semiconductor regions of type 'n' and 'p'.
Bipolar junction transistors are divided into two groups:
vertical and lateral transistors.
For the vertical transistors, 
.P= space
calculates the emitter area and perimeter. They are stored
in the attributes area and length, respectively.
For the lateral transistors, the width of the base
- the smallest distance between collector and emitter -
is also calculated and stored in the attribute basew.
For both groups, the
.O= -t -torpos
option enables
.P= space
to output the position of the emitter for each transistor.
.H 3 "Terminal connections"
For each bipolar junction transistor,
.P= space
will extract only one connection for each device terminal
(collector, base, emitter and optionally a substrate terminal).
Multi-emitter transistors are extracted as separate
devices (one for each emitter). 
. \" Setting the
. \" .I merge_par_bjts
. \" parameter enables that multi-emitter transistors that are in
. \" parallel (emitters connected to the same node) are represented
. \" by one single device. A scaling parameter is then stored in the
. \" attribute scalef that contains the number of transistors the
. \" device represents.
When a lateral transistor is recognized that has the same mask
for both the emitter and the collector connections,
it is assumed that the collector has the larger area.
.H 3 "Command-Line Options"
Extraction of bipolar devices is controlled by the following option:
.M(
.ME -t -torpos
Add positions of devices and sub-cells to the extracted circuit.
.M)
.H 3 "Parameter File"
The following parameters for the parameter file control the
extraction of bipolar transistors. The
.I lat_base_width
parameter defines the maximum base width that a lateral
transistor can have. Lateral transistor configurations
with a width larger than this value are not recognized
as being transistors.
.DS I
.TS
tab(:);
l c c c c.
_
parameter:type:unit:default:suggestion
_
. \" merge_par_bjts:boolean:-:off:on
lat_base_width:real:micron:0:3
_
.TE
.DE
.H 3 "Element Definition File"
See section %SE_BITORELEM% for the specification of the bipolar
transistor elements.
.H 2 "Capacitance Extraction"
.H 3 "Capacitance Definitions"
Space uses an area/perimeter based capacitance extraction method.
To describe the capacitance model employed by
.P= space,
refer to the following figure and definitions:
.PS 3i
copy "../spaceman/capdef.pic"
.PE
.VL 20n
.LI "Bottom plate"
Lowest conductor, e.g. substrate, active area, polysilicon or first metal.
.LI "Top plate"
Can for example be second metal, but can also be absent.
.LI "Middle plate"
Can for example be polysilicon or first metal.
.LI "Neighbor plate"
Not necessarily in same layer as middle plate.
.LI "$Cppb$"
Parallel-plate
capacitance from middle-plate to bottom-plate.
This can be a substrate capacitance or a cross-over coupling capacitance.
.LI "$Cppt$"
Parallel-plate
capacitance from middle-plate to top-plate,
this is a cross-over coupling capacitance.
.LI "$Ceb$"
Edge capacitance of middle conductor to bottom conductor.
This can be a substrate capacitance or a fringe-coupling capacitance.
.LI "$Cet$"
Edge capacitance of middle conductor to top conductor.
This is a fringe-coupling capacitance.
.LI "$Cl$"
Lateral coupling capacitance between two parallel conductors.
.LE
.P
.H 3 "Parallel Plate Capacitances"
The parallel plate capacitances $C sub ppt$ and $C sub ppb$ between two wires 
that overlap over an area $A$ are calculated from
.DS
.EQ I
C sub ppt ~=~ c sub ppt ~ A
.EN
.sp 0.5
.EQ I
C sub ppb ~=~ c sub ppb ~ A
.EN
.DE
where $c sub ppt$ and $c sub ppb$ are capacitances 
per square meter.
.H 3 "Lateral Coupling Capacitances"
.sY %SE_LATCAP% \n(H1.\n(H2.\n(H3
The value of a lateral coupling capacitance can be computed in two 
different ways, dependent on how the capacitance
is specified in the element definition file.
.P
One possibility is that a single value for the capacitance is specified
in the element definition file.
In that case,
the lateral coupling capacitance between two parallel wires
that are at a distance
$d$ over a length $l$ is calculated from
.DS
.EQ I
C sub l ~=~ c sub l ~ l over d
.EN
.DE
where $c sub l$ is the value that is specified in the element
definition file 
($c sub l$ corresponds to the capacitance
for a configuration where the distance between both wires
is equal to their length).
.P
Another possibility, which allows to
specify more accurate capacitance values, is that the 
the lateral coupling capacitance is specified as a function of different
distances between the wires.
In that case,
the specification of the lateral capacitance
has one or more (distance, capacitivity) pairs
that each specify the capacitance between 
two parallel wires of a length of 1 meter for one particular
distance between the wires.
The lateral coupling capacitance for other configurations is found 
from an interpolation between two (distance, capacitivity) pairs.
For the interpolation, the function
.DS
.EQ I
C sub l ~=~ l ~ a over { d sup p }
.EN
.DE
is used if the capacitance for both adjacent points is larger than zero 
and $p ~>=~ 1$,
and 
.DS
.EQ I
C sub l ~=~ l ~ ( a over { d } ~+~ b )
.EN
.DE
is used otherwise.
If the actual distance is larger or smaller than any specified distance,
an extrapolation is done using the above functions.
.P
Lateral coupling capacitance extraction is controlled by
the 
.I lat_cap_window
parameter from the parameter file.
This parameter specifies in microns the distance over which
lateral coupling capacitance is considered significant.
When the wires are at a distance that is larger than 
.I lat_cap_window,
no lateral coupling capacitance will be extracted for these wires.
.H 3 "Edge Capacitances"
.sY %SE_EDGECAP% \n(H1.\n(H2.\n(H3
In addition to the edge-to-bottom capacitance
$C sub eb$ and the edge-to-top capacitance $C sub et$,
also edge-to-edge capacitances between wires that are on
top of each other can computed, as $C sub ee$ in the
figure below.
.PS 1.3i
copy "../spaceman/eecapdef.pic"
.PE
.P
One possibility to specify edge capacitance in the element definition file 
is by specifying a single value $c sub e$ which denotes the edge capacitance 
per meter edge length.
The edge capacitance $C sub e$ between one wire that overlaps the edge
of another wire over a length $l$, or between two wires
of which the edges coincide over a length $l$, is then calculated from
.DS
.EQ I
C sub e ~=~ c sub e ~ l
.EN
.DE
.P
However, if also lateral coupling capacitances are extracted,
these simple equations do not hold anymore, as the values
of $C sub eb$, $C sub et$ and $C sub ee$ are effected by the lateral 
capacitances.
.P= Space
offers two ways of dealing with this, either by using an
heuristical method, or by allowing the user to
specify explicitly how large the edge capacitances become
if other conductors are nearby.
.P
The heuristical method uses the fact that
experiments show that
for conductors that are not too narrow and/or too close to other conductors,
the sum of the ground capacitances and the coupling capacitances
that are connected to that conductor is approximately constant.
Therefore, for each conductor edge to which a lateral coupling capacitance
is connected,
.P= space
tries to keep the sum of the ground capacitance and the coupling capacitances
constant by decreasing the value of the other (non-lateral) edge 
capacitances that are connected to that edge.
This is achieved by subtracting from each non-lateral edge capacitance 
$C sub {ex}$ (where $C sub {ex}$ is $C sub {et}$, $C sub {eb}$ or $C sub {ee}$)
that is connected to the edge, a value 
$minimum ~ roman "{" C sub {ex} , ~ {C sub {ex} ~ C sub l ~ 
compensate_lat_part } over { C sub { ex-all }} roman "}" $, 
where $C sub { ex-all }$ is the sum of all connected
non-lateral edge capacitances and
.I compensate_lat_part
is a parameter.
Note that this parameter has a default value 1 (for full compensation).
But if you want to have no full compensation, set the value of parameter
.I compensate_lat_part
smaller than 1.
.P
The second way of dealing with the effect of lateral coupling on
the edge capacitances, is by explicitly 
specifying in the element definition file
(distance, capacitivity) pairs, similar to the possibility
used for lateral coupling capacitances (see previous section).
(Note, to use this second way, parameter
.I compensate_lat_part
must be greater than 0.)
Each pair gives the edge capacitance per meter edge length
for given distance to a neighboring conductor that is of the same type
(e.g. both wires are of type metal1).
If ($d sub max$, $c sub e$) is the pair with
maximal specified distance
(and therefore also maximal edge capacitance), 
$C sub e ~=~ c sub e ~ l$ for all distances larger than
$d sub max$ or for a situation where no neighboring wire
of the same type is present.
The edge capacitance for other configurations is found from
an interpolation between two (distance, capacitivity) pairs.
The following
interpolation function is used:
.DS
.EQ I
C sub e ~=~ c sub e ~ ( 1 ~-~ b ~ exp ~ ( ~-~ p ~ d ) ~ ) ~ l
.EN
.DE
If the actual distance is smaller than any specified distance,
the same formula is used, with $b$ fixed at 1.
These formulas apply both for $C sub eb$, $C sub et$ and $C sub ee$.
Edge capacitances that are explicitly specified as a function of the
distance to neighbor wires, are not affected by the first method.
.H 3 "Coupling Capacitances When Extracting Only Substrate/Ground Capacitances"
When only substrate/ground capacitances are extracted and no coupling
capacitances (the option 
.O= -c -cap
is used and 
the option
.O= -C -coupcap
and
.O= -l -latcap 
are not used),
all non-lateral coupling capacitances that are specified
in the element definition file are added
as ground capacitances to the two wires
to which the coupling capacitance
is connected.
.H 3 "Extracting Capacitances of Different Types"
For each capacitance that is extracted it is possible to specify
its type (see Section %SE_CAPELEM%).
Unlike parallel capacitances that are of a same type,
parallel capacitances of a different type are not joined
during extraction.
.ne 6
.H 3 "Extracting Junction Capacitances"
.sY %SE_JUNCAP% \n(H1.\n(H2.\n(H3
Default, junction capacitances 
(see Section %SE_CAPELEM%)
will be extracted as linear capacitances.
.P
If the parameter
.I jun_caps
is set to "non-linear" the extracted capacitance will be of the specified type.
.P
If the parameter
.I jun_caps
is set to "area" the extracted capacitance will be of the specified type and,
moreover,
the value of the extracted capacitance will specify the area of the element
(if only an area capacitivity is specified for that capacitance type)
or the edge length of the element (if only edge
capacitivity is specified for that capacitance type).
If
.I jun_caps
is set to "area", and
both area capacitivity and edge capacitivity are specified
for one junction type,
the value of the extracted capacitance will be equal
to the extracted capacitance value divided by the area capacitivity
(so in this case, effectively, the total vertical and horizontal
junction area is extracted).
.P
If the parameter
.I jun_caps
is set to "area-perimeter" the extracted capacitance will be of the specified 
type and the area and perimeter of the element will be represented by
the instance parameters
.I area
and
.I perim
in the database.
See section %SE_MODELINLIB% for then how to convert these parameter names to
the parameter names that are appropriate for the simulator that is used.
.P
If the parameter
.I jun_caps
is set to "separate" the extracted capacitance will be of the specified 
type and the area and perimeter of each capacitance that is specified in
the capacitance list for that type will separatedly be represented with
parameters 
.I area<nr>
and
.I perim<nr>
where 
.I <nr>
denotes that it is the \fI<nr>\fP-th. area or the \fI<nr>\fP-th. perimeter
element in the list.
This may e.g. be useful when it is required that for one junction
capacitance element the perimeter adjacent to the gate oxide and the 
perimeter adjacent to the field oxide are specified as separate parameters.
(See again section %SE_MODELINLIB% for how to convert the parameter names to
the parameter names that are appropriate for the simulator that is used.)
.P
Note that junction capacitances of drain/source areas can also be
extracted as drain/source area and perimeter information attached to
the MOS transistors, see Section %SE_TORELEM%.
.H 3 "Name of Ground or Substrate Node"
Ground or substrate capacitances are on one side connected to
a node that is called "GND" (if @gnd is used as terminal mask 
in the element definition
file) or "SUBSTR" (if @sub is used as terminal mask
in the element definition file),
see Section %SE_CAPELEM%.
These names can be changed using respectively the
.I name_ground
parameter and the
.I name_substrate
parameter from the parameter file.
.ne 5
.H 3 "Command-Line Options"
Capacitance extraction is controlled by the following options:
.M(
.ME -c -cap
Extract capacitances to substrate/ground.
.ME -C -coupcap
Extract coupling capacitances as well as capacitances to substrate.
This option implies
.O= -c -cap "" .
.ME -l -latcap
Also extract lateral coupling capacitances, implies
.O= -C -coupcap "" .
.M)
.H 3 "Parameter File"
The following parameter for the parameter file controls the extraction
of capacitances.
.DS I
.TS
tab(:);
l c c c c.
_
parameter:type:unit:default:suggestion
_
lat_cap_window:real:micron:0:3-5
compensate_lat_part:real:-:1:1
jun_caps:string:-:linear:area-perimeter
name_ground:string:-:GND:-
name_substrate:string:-:SUBSTR:-
_
.TE
.DE
.H 3 "Element Definition File"
See section %SE_CAPELEM% for the specification of the unit capacitance values 
(i.e. $c sub ppt$, $c sub ppb$, $c sub et$, $c sub eb$ and $c sub l$)
for the different interconnection layers.
.H 2 "Resistance Extraction"
.sY %SE_RESISTEXT% \n(H1.\n(H2
.H 3 "General"
.sY %SE_RESISTGEN% \n(H1.\n(H2.\n(H3
When extracting resistances,
.P= space
applies finite element techniques to
construct a fine resistance mesh that models
resistive effects in detail.
An example of a layout and the finite element mesh produced
for the polysilicon mask is in the figure below:
.DS
.PS 4.8i
A:[
copy "../spaceman/select.pic"
]
B:[
copy "../spaceman/selPolyMesh.pic"
] with .w at 11/10 <A.w, A.e>
.PE
.DE
The finite element mesh is equivalent to a detailed
resistance network and
.P= space
initially constructs this detailed resistance network to model the resistances.
Then it applies a Gaussian elimination
(or, equivalently, a star-triangle transformation)
node reduction technique to simplify the network and to
find the final resistances.
The final network will in general contain
(1) the nodes that are terminals,
(2) the nodes that are labels,
(3) the nodes that are transistor connections
(gate, source, drain, emitter etc.),
(4) the nodes that are introduced by
the algorithm in Section %SE_MOREDETAILRC%,
(5) the nodes that are connected to resistances of different types,
(6) - if metal resistances are not extracted or when equi-potential
lines are detected (see Section %SE_DEGREE% and Section %SE_EQUI_LINE%) - 
the nodes that correspond to equi-potential
regions, and
(7) - if substrate resistances are
extracted (see the Space Substrate Resistance Extraction User's Manual) -
the nodes that represent substrate terminals.
This is described in more detail in
Section %SE_NETWREDHEUR%.
.P
When extracting resistances together with capacitances,
the extractor will add lumped capacitances to the nodes of the
initial resistance network to model the distributed capacitive effects.
In that case,
the node reduction will proceed such that
the Elmore time constants
between the nodes in the final network
are unchanged with respect to their value in the fine RC mesh.
This will guarantee 
that the electrical transfer function
of the final network
closely matches that of the fine RC mesh
and, consequently,
that of the actual circuit.
.P
The extraction of resistances together with (ground) capacitances
is illustrated below.
In (a) it is shown how a T shaped interconnection with terminals
$Ta$, $Tb$ and $Tc$ is subdivided
into finite elements, which are all rectangles in this case.
In (b) the RC network is shown that models the resistive
and capacitive effects in detail.
In (c) the final network is shown that is obtained after
eliminating the internal nodes and collapsing the nodes
that belong to the same terminal.
.br
.DS
.S 8
.EQ
gsize 8
.EN
.PS 3.6i
copy "../spaceman/rcmesh.pic"
.PE
.S=
.EQ
gsize 12
.EN
.DE
.H 3 "Improved Accuracy"
.sY %SE_RESMESH% \n(H1.\n(H2.\n(H3
By default,
.P= space
uses a rather coarse finite element mesh to compute resistances.
The accuracy of the resistances as obtained with this mesh
may not always be sufficient.
In particular,
obtuse triangles in the mesh may be generated 
that have a bad influence on the accuracy of the method and that
produce negative resistances in the detailed resistance network.
Under some circumstances, negative resistances
(and negative capacitances as a by-product of the network
reduction heuristics) may also appear in the final network.
.P
To obtain an improved accuracy with resistance extraction,
the option
.O= -z -resmesh "" ,
may be used.
On the penalty of a somewhat longer extraction time,
this option generates
a finer finite-element mesh that contains (almost) no
obtuse triangles.
As a result, more accurate resistance values
are found and negative resistances do not occur in the output
network.
.P
When using the option
.O= -z -resmesh "" ,
the parameter 
.I max_obtuse
specifies the maximum (obtuse) angle of a corner of a triangle
of the finite element mesh.
A smaller value for
.I max_obtuse
(use 90 $<=$ 
.I max_obtuse 
$<=$ 180)
results in a finer finite element mesh,
but also in longer extraction times.
.N(
The usage of the option
.O= -z -resmesh "" 
may result in large finite element meshes, 
and hence long extraction times, 
when resistances are extracted for 
large rectangular areas like e.g. wells.
.N)
.H 3 "Extracting Resistances of Different Types"
For each conductor it is possible to specify the type
of the resistance that is extracted
(see Section %SE_RESELEM% and Section %SE_CONTELEM%).
Parallel resistances of different types will not be joined during
extraction.
Neither will nodes be eliminated that are connected 
to resistances of different types.
.H 3 "Selective Resistance Extraction"
Selective resistance extraction is possible by specifying
interconnects in a file called 'sel_con' and by using
either the option 
.O= -k -select
or
.O= -j -unselect "" .
When using the option 
.O= -k -select "" ,
resistances will only be extracted for the interconnects
that are specified in the file 'sel_con'.  
When using the option
.O= -j -unselect "" ,
resistances will be extracted for all interconnects 
except for the interconnects that are specified in the
file 'sel_con'.  
The format of the file 'sel_con' is as follows.
On each line, an x position, an y position and a maskname is
specified.  
When an interconnect has the specified mask on
the specified layout position, that interconnect is specified 
in the file.
.H 3 "Contact Resistances"
When extracting resistances,
with each contact area a resistance 
is associated that has a value
.DS
.EQ I
R ~=~ r over A
.EN
.DE
where 
.in +1c
.br
$r$ is the contact resistance in ohm square meter, and
.br
$A$ is the area of the contact.
.in -1c
.P
The contact resistance $R$ is subdivided over the corners of
the contact as shown in the figure below.
.DS
.PS 2.0i
copy "../spaceman/contres.pic"
.PE
.DE
The top plane and bottom plane (in dashed lines) represent the two layers that 
are connected by the contact, which on their turn may also be represented by
resistors if resistances are extracted for these layers.
The value of $r$ should be tuned to give
accurate results for typical size contacts.
.H 3 "Elimination Order"
.sY %SE_ELIM_ORDER% \n(H1.\n(H2.\n(H3
To reduce memory usage,
.P= space
creates the detailed RC mesh while scanning the layout from
left to right.
It eliminates each internal node as soon as this is possible,
i.e. as soon as all network elements that are connected to the node
are known.
However, this strategy will in general result
in an elimination order that is not optimal with respect to
computation time.
Since the cost of each elimination is proportional to the square
of the number resistances that are connected to the node,
it is often more efficient to first eliminate the node that has
the lowest number of resistances connected it, then
the node that has - after the previous operation - the lowest number
of resistances connected to it, etc..
To allow
.P= space
to select for elimination a node with a low number resistances
connected to it, a buffer can be defined
in which the nodes are temporarily stored that are ready for elimination.
When the buffer becomes full, the node with lowest degree
is selected for elimination and it is removed from the buffer.
The size of the buffer is defined using the parameter
.I max_delayed.
A large value of
.I max_delayed
may result in significantly faster extraction times,
but a too large value of
.I max_delayed
will also result in large memory usage.
A large value for
.I max_delayed
will in general be useful for circuits that contain
large rectangular area for which resistances are extracted
such as n-wells.
.ne 10
.H 3 "Command-Line Options"
Resistance extraction is controlled by the following options:
.M(
.ME -r -res
Extract resistances for high-resistivity (non-metal) interconnect.
.ME -z -resmesh
Apply mesh refinement for resistance extraction (implies
.O= -r -res "" ).
.ME -k -select
Selective resistance extraction, resistances are
only extracted for specified interconnects.
.ME -j -unselect
Selective resistance extraction, resistances are
extracted for all but specified interconnects.
.M)
The threshold value to determine whether an interconnect is high-resistive
or low-resistive is specified by the parameter
.I low_sheet_res
in the parameter file
(default, 
.I low_sheet_res
= 1 ohm per square).
.H 3 "Parameter File"
Resistance extraction is controlled by
the following parameters from the the parameter file:
.DS I
.TS
tab(:);
l c c r r
l c c r r.
_
parameter:type:unit:default:suggestion
_
max_obtuse:real:degree:90.0:110.0
low_sheet_res:real:ohm:1.0:1.0
low_contact_res:real:ohm.m2:0.1e-12:0.1e-12
max_delayed:integer:-:100000:500-200000
_
.TE
.DE
.H 3 "Element Definition File"
See section %SE_RESELEM% for the specification of the sheet resistance values
for the interconnection layers,
and see section %SE_CONTELEM% for the specification of the contact resistances.
.H 2 "Frequency Dependent Number of RC Sections"
.sY %SE_MOREDETAILRC% \n(H1.\n(H2
In some occasions (for high frequencies)
the RC network that is extracted
for the interconnections
may contain too few elements to allow
a simulation of
the extracted circuit afterwards
with sufficient accuracy.
For example, if an interconnection has two terminals, the extractor
will extract an RC network consisting of one resistor and two capacitors
(one $pi$-section),
while an accurate simulation of the interconnection may require at least
two $pi$-sections.
In that case,
the option
.O= -G -usefrequency
may be used.
This option causes that,
instead of eliminating
.I all 
the internal nodes
in the initial RC mesh (see Section %SE_RESISTGEN%),
a Selective Node Elimination (SNE) is performed in which,
besides the nodes that are normally retained in the extracted network
(terminals, transistor connections, etc; see Section %SE_RESISTGEN%),
also other nodes are retained.
.P
When using the the option
.O= -G -usefrequency "" ,
a parameter
.I sne.frequency
has to be specified in the parameter file
that specifies the maximum signal frequency that occurs
in the extracted circuit.
The complexity of the extracted network will be a function of the
parameter
.I sne.frequency.
The higher the value of
.I sne.frequency
the more nodes will be retained in the final network,
such that the reduced network accurately models
the detailed network (i.e. the network before reduction).
If
.I sne.frequency
$<=$ 0,
no additional nodes are retained.
.P
As an example we consider the spiral resistor that is shown below.
.F+
.PSPIC "../spaceman/spiral_lay2.eps" 2.4i
.F-
The RC network that is default extracted for it is shown below in (a).
When using the option
.O= -G -usefrequency
and when using
for parameter
.I sne.frequency
the value 50e9,
one non-terminal node of the initial RC mesh
will be retained in the final RC network
and the RC network as shown in (b) is obtained.
.DS
.PS 4.4i
copy "../spaceman/spiral_rc.pic"
.PE
.DE
.H 3 "Command-Line Options"
More detailed RC network extraction
is controlled by the following option:
.M(
.ME -G -usefrequency
Extract RC models that are accurate up to a certain frequency.
.M)
.H 3 "Parameter File"
More detailed RC network extraction
is controlled by
the following parameter from the the parameter file:
.DS I
.TS
tab(:);
l c c c c
l c c c n.
_
parameter:type:unit:default:suggestion
_
sne.frequency:real:Hz:1e9:1e9
_
.TE
.DE
.H 2 "Network Reduction Heuristics"
.sY %SE_NETWREDHEUR% \n(H1.\n(H2
When extracting resistances and capacitances,
.P= space
can apply
some heuristics 
to further reduce the number of elements 
(resistors, capacitors and nodes)
in the final network
by neglecting irrelevant detail.
These heuristics include
.I(
.I=
Retaining of nodes corresponding to equi-potential
regions (i.e. pieces of interconnect for which no resistance
is extracted)
in order to prevent the creation of complete resistance graphs
on the terminal nodes of large conductors
(e.g. power and ground lines, 
clock lines and large busses with many connections).
.I=
Merging of (terminal) nodes that
are connected by a small resistance.
.I=
Removal of large resistances that are shunted by a low
resistivity path.
.I=
Reconnecting small coupling capacitances to ground.
.I)
.P
Below, we will introduce these heuristics,
and for illustration we use the following layout:
.DS 
.PS 3.6i
copy "../spaceman/lay.pic"
.PE
.DE
In general, after Gaussian elimination but before applying the network
reduction heuristics, the network will contain
(1) the nodes that are terminals,
(2) the nodes that are labels,
(3) the nodes that are transistor connections 
(gate, source, drain, emitter etc.),
(4) the nodes that are introduced by 
the algorithm in Section %SE_MOREDETAILRC%,
(5) the nodes that are connected to resistances of different types,
(6) - if metal resistances are not extracted or when equi-potential
lines are detected (see Section %SE_DEGREE% and Section %SE_EQUI_LINE%) - 
the nodes that correspond to equi-potential 
regions, and
(7) - if substrate resistances are 
extracted (see the Space Substrate Resistance Extraction User's Manual) -
the nodes that represent substrate terminals.
.P
For the above example, when assuming that non-metal resistances
and ground and coupling capacitances are extracted, and when
assuming that a, b, c, d and e are either terminals
or gate, drain or source connections,
the network that is initially extracted
will have the following form:
.DS
.PS 3.4i
copy "../spaceman/netw.pic"
.PE
.DE
In the above figure, node m corresponds to the piece of metal that 
connects the three
different poly branches.
If also metal resistances are extracted, or if
no network reduction heuristics are applied,
this node will not be present.
.H 3 "min_art_degree and min_degree heuristic
.sY %SE_DEGREE% \n(H1.\n(H2.\n(H3
The 
.I "articulation degree"
of a node is defined as the number of pieces
in which the resistance graph would break if the node
and its connected resistances were removed.
Nodes that
correspond to pieces of metal for which no resistances are extracted
(e.g. node m in the last figure)
will often have an articulation degree > 1.
If a node has an
articulation degree <
.I min_art_degree
and if 
(1) the node has no terminals or transistors connected to it,
(2) the node has not been introduced by the 
algorithm in Section %SE_MOREDETAILRC%,
(3) the node is not connected to resistances of different types,
and
(4) the node does not represent a substrate terminal
(see the Space Substrate Resistance Extraction User's Manual),
the node will be eliminated.
If an equi-potential node has an articulation degree >= 
.I min_art_degree,
or if it does not satisfy one of the 
4
above conditions,
the node will be retained in the final network.
.P
The 
.I degree
of a node is equal to
the number of resistances connected to the node.
Nodes with a degree >= 
.I min_degree 
and an articulation degree > 1
will also be retained in the final network.
.E(
Node m is not eliminated if its articulation degree (3 in this case)
is more than or equal to 
.I min_art_degree, 
or if its articulation degree is more than one and its degree (4 in this case)
is more than or equal to 
.I min_degree.
Otherwise, the node is eliminated.
.E)
N.B. To find the articulation
degree of a node, the extractor does not take into
account interconnection loops.
.H 3 "min_res heuristic"
This heuristic deletes small resistances from
the network via Gaussian elimination of one of the nodes 
that is connected to the resistance.
If a resistor has
an absolute value that is less than the
.I min_res
parameter,
and if the resistor is connected to a node that 
(1) is not connected to resistances of different types,
and (2) 
does not have an articulation degree >= 
.I min_art_degree,
or a degree >= 
.I min_degree 
(see the previous paragraph),
the resistance is deleted
by the elimination of that node.
Terminals and transistor connections of the node that is eliminated
are added to the other node that is connected to the resistor.
The deletion is done incrementally: each time when one or more node 
have been eliminated, the network is checked again to see if new small
resistances have arisen.
.E(
The resistors $R sub 1$, $R sub 2$, $R sub 3$, $R sub 4$ and $R sub 5$
are evaluated to see 
if their value is less than 
.I min_res.
If this is true then one of the nodes to which the resistor is connected to
is eliminated (not node m if this node
was retained because of its degree or articulation degree)
and its terminal label(s) and/or transistor connections
are attached to the other node.
The new network is again verified to see if there are any
resistors that are smaller than
.I min_res,
etc.
.E)
.H 3 "min_sep_res heuristic"
This heuristic deletes small resistances from
the network by joining the two nodes that are connected
by the resistance.
If two nodes are connected by a resistor
that has an absolute value that is less than
.I min_sep_res,
the resistor is deleted and the two nodes
are merged.
Note that while the 
.I min_res
heuristic does not affect the total
resistance between the remaining nodes, this heuristic, in general,
does.
.H 3 "max_par_res heuristic"
This heuristic prevents the occurrence of high-ohmic shunt paths
between two nodes.
If the ratio
of the absolute value of a resistor and its
minimum parallel resistance path (along positive resistors)
exceeds the value
of the
.I max_par_res
parameter,
then the resistor is simply removed.
.E(
Assume $R sub 3$ > 0, $R sub 4$ > 0 and $R sub 5$ > 0.
First it is checked if
$R sub 3$ / ($R sub 4$ + $R sub 5$) $>$ 
.I max_par_res. 
If this is true then $R sub 3$ will be removed.
If this is not true then it is checked 
if $R sub 4$ / ($R sub 3$ + $R sub 5$) $>$ 
.I max_par_res. 
If this is true then $R sub 4$ will be removed.
If this is not true then it is checked 
if $R sub 5$ / ($R sub 3$ + $R sub 4$) $>$ 
.I max_par_res.
If this is true then $R sub 5$ will be removed.
Recall that the elimination order is arbitrary.
.E)
.H 3 "no_neg_res heuristic"
If the
.I no_neg_res
heuristic is on,
all negative resistances will be removed from the network.
.H 3 "min_coup_cap heuristic"
If, for both nodes a coupling capacitance is connected to, it holds
that the ratio between the absolute value of the coupling capacitance
and the value of the ground/substrate capacitance of the same type of 
that node,
is less than the
.I min_coup_cap 
parameter,
then the value of the coupling capacitance is added to the ground 
capacitances of the two nodes
and the coupling capacitance is removed.
.E(
If $C sub 12$ / $C sub 1$ $<$ 
.I min_coup_cap 
and $C sub 12$ / $C sub 2$ $<$ 
.I min_coup_cap 
then $C sub 12$ is removed and its value is added to both
$C sub 1$ and $C sub 2$.
.E)
.H 3 "min_ground_cap heuristic"
If the absolute value of a ground/substrate capacitance is less than the
.I min_ground_cap
parameter, the ground/substrate capacitance is removed.
.H 3 "no_neg_cap heuristic"
If the
.I no_neg_cap
heuristic is on,
all negative capacitances will be removed from the network.
.H 3 "frag_coup_cap heuristic"
.sY %SE_FRAG_COUP% \n(H1.\n(H2.\n(H3
In contrast to the other heuristics,
this heuristic is applied during Gaussian elimination
(see Section  %SE_RESISTGEN%).
When eliminating a node and redistributing a coupling capacitance that
is connected to that node over the nodes that are adjacent,
this heuristic decides whether or not the adjacent node receives (a part of)
the coupling capacitance.
When $Rmin$ is the minimum of the absolute values of the resistances
that are connected to the node that is eliminated,
and when $R$ is the value of the resistance between the node that is 
eliminated and an adjacent node, then the adjacent node
receives (a part of) the 
coupling capacitance if and only if
$Rmin$ / $| R |$ $>=$ 
.I frag_coup_cap.
Hence, a lower value of 
.I frag_coup_cap
will give more detail in the extracted network,
but it will also result in longer extraction times.
For the fastest and least accurate form of 
resistance and coupling capacitance extraction, set
.I "frag_coup_cap"
equal to 1.
.H 3 "min_coup_area, min_ground_area and frag_coup_area heuristics"
These heuristics are similar to their equivalences for capacitance
(\fImin_coup_cap, min_ground_cap\fP and \fIfrag_coup_cap\fP),
but are used instead when junction capacitances are extracted as 
area and perimeter elements (see Section %SE_JUNCAP%).
They only look at the value of the area parameter(s) of the elements,
not at the value of the perimeter parameter(s).
.H 3 "equi_line_ratio heuristic"
.sY %SE_EQUI_LINE% \n(H1.\n(H2.\n(H3
If, during resistance extraction,
for a rectangular piece of interconnect,
the ratio length/width is more than 
.I equi_line_ratio,
an equi-potential line is generated for that piece of interconnect.
The equi-potential line is placed at the middle of the rectangle,
perpendicular to the current flow.
This will introduce an extra equi-potential node 
that is treated similarly as the nodes that are introduced
by not extracting metal resistances
(see Section %SE_DEGREE%).
In general, this will
simplify the extracted network and reduce extraction time.
Especially when metal resistances are extracted,
the reduction in network complexity and extraction time
can be large.
Currently, equi-potential lines are not detected for all
interconnect rectangles.
.H 3 "keep_nodes"
It is possible to keep nodes of certain capacitance (and contact) elements
in the extracted network (these nodes are not eliminated).
You need to specify a string of element names after the
.I "keep_nodes"
parameter.
When you only want to keep one of the nodes of an element,
you must add '.1' or '.2' to the element name.
See for element names and the pin order the technology file.
.E(
keep_nodes  lcap_cms  ecap_cms_cpg.2
.E)
.br
.ne 10
.H 3 "Parameter File"
The heuristics are controlled by
the following parameters from the parameter file:
.DS I
.TS
tab(:);
l c c c c
l c c c n.
_
parameter:type:unit:default:suggestion
_
min_art_degree:integer:\(em:+\(if:3
min_degree:integer:\(em:+\(if:4
min_res:real:ohm:0:100
min_sep_res:real:ohm:0:10
max_par_res:real:\(em:+\(if:25
no_neg_res:boolean:\(em:off:on
min_coup_cap:real:\(em:\(mi\(if:0.04
min_ground_cap:real:farad:0:1e-15
no_neg_cap:boolean:\(em:off:on
frag_coup_cap:real:\(em:0:0.2
min_coup_area:real:\(em:\(mi\(if:0.04
min_ground_area:real:farad:0:1e-11
frag_coup_area:real:\(em:0:0.2
equi_line_ratio:real:\(em:+\(if:1.0
keep_nodes:string:\(em:\(em:\(em
_
.TE
.DE
.H 3 "Command-Line Options"
The following command-line option controls the heuristics:
.M(
.ME -n -noreduc
Do not apply the circuit reduction heuristics.
.M)
.H 2 "Library Cell Circuit Extraction"
.sY %SE_LIBRARYCELLS% \n(H1.\n(H2
When extracting a circuit that contains library cells
(standard cells, gate arrays)
the library cells themselves often need not
to be extracted, but only the interconnects
between the library cells.
To let
.P= space
perform an extraction in this way,
set the extraction status for each library cell
to "library" using the program 
.P= xcontrol(1ICD).
When a cell has a library status,
.P= space
will not extract this cell, but it will include
it as an instance in the extracted circuit,
no matter whether hierarchical extraction is used
or flat extraction.
.P
The description of the library cell itself
can be added to the database using a netlist
conversion tool like
.P= cspice(1ICD)
or
.P= csls(1ICD).
When, however, instead of a netlist description,
a model description needs to be specified for
the library cell, set the extraction status for that
cell to "device" using
.P= xcontrol(1ICD)
and specify the model in
the control file of the netlist retrieving tool
(see Section %SE_PRESPICE%) or use the tool
.P= putdevmod(1ICD)
to specify the device model.
.P
Sometimes it may be necessary to expand
(parts of) the library cell in the parent cell.
This may for example be the case when the cell connects
to the father cell via other layout polygons than its terminal areas,
or when the cell has feed-throughs that occur via other layout polygons
than its terminal areas.
In that case,
set the interface type for the cell
to free (or freemasks)
using the command
.P= xcontrol(1ICD).
This way, the contents of a library cell (for freemasks
only certain masks) will be expanded
its parent cell.
Note, however, that capacitances etc. inside
the cell will then also be extracted.
If this is not desired (because the capacitances
have already been accounted for in the
description that is available for the library cell),
one can use the following strategy:
Add a dummy mask to the library cell
and modify the element definition file
that is used for extraction
such that capacitances are only recognized
for positions where the dummy mask is not present.
.P
To prevent the replication of data,
it is useful to store the library cells in a separate 
project and next import this project in the project 
where the design is present.
.H 2 "Back Annotation"
.sY %SE_NAMING% \n(H1.\n(H2
.H 3 "Instance names"
An instance name for a layout cell can be specified
using a layout import tool (e.g.
.P= cgi )
or using the layout editor
.P= dali .
The corresponding instance in the extracted circuit
will have the same name.
If an instance in an extracted circuit is obtained from more than one 
level down in the hierarchy of that cell
(e.g. in case the instances that contain the relevant instance
are expanded in the top-level cell), the name of that instance is
obtained by concatenating the names of all the different instances 
that contain the instance.
The different instance names in this case are separated by a character 
that is specified by the parameter
.I hier_name_sep
(default the character '.').
If no instance name is specified in the layout,
or if a hierarchical instance name can not be constructed
because one of the instance names is missing,
the instance name in the extracted circuit will be generated by
the network retrieving tool that is used.
.P
The instance names of elements that are recognized from the mask
layout combinations (transistors, resistors, capacitors etc.) 
are generated by the extraction program or by the 
network retrieving tool that is used.
.H 3 "Net names"
.sY %SE_NETNAMESPEC% \n(H1.\n(H2.\n(H3
A net in the extracted circuit represents one conductor or
(defined from a layout point of view) one set of electrically connected
polygons.
If no resistances are extracted, a net is represented in the circuit by
exactly one node.
If resistances are extracted, a net may be represented in the circuit
by more than one node.
.P
The name of a net, among other things, may be used to determine the 
names of the nodes that are part of the net (see Section %SE_NODENAMESPEC%).
The specification of a net name can be done as follows:
.BL
.LI
By defining a label for the net.
The definition of a label requires the specification of
a name, a mask, an x, y position and (optionally) a class.
The name of the label is then used as the name for the net 
that is represented by the specified mask at the specified position.
If more than one label is attached to a net, the net receives the name
of the label that has the smallest x coordinate or (if
both x coordinates are equal) the smallest y coordinate.
Labels may be defined as follows:
.DL
.LI
Using the layout editor
.P= dali
in the annotate menu.
.LI
Using the program
.P= cgi.
.LE
.P
If the parameter
.I no_labels
is set, the labels that are defined for a cell will not be used
by the extractor.
.LI
By setting the parameter
.I term_is_netname.
This will cause that each terminal of the cell
(hence, not a terminal of a sub-cell !) is also interpreted as a label.
Again, if more than one label is attached to a net, the net receives the name
of the terminal that has the smallest x coordinate or (if
both x coordinates are equal) the smallest y coordinate.
.LI
By the use of inherited labels.
Inherited labels originate from labels and terminals that are defined
in sub-cells:
.P
If the parameter
.I hier_labels
is set, a label will be inherited from each label of a sub-cell
that is flattened in the extracted cell.
.P
If the parameter
.I hier_terminals
is set, a label will be inherited from each terminal of a sub-cell 
that is flattened in the extracted cell.
.P
If the parameter
.I leaf_terminals
is set, a label will be inherited from each terminal of a sub-cell 
that is
.I not 
flattened in the extracted cell.
.P
In each of the three above cases, the name of the inherited label 
is given by concatenating the names 
of all the different instances that contain the original label 
or terminal plus the name of the label or terminal itself.
The different instance names are separated 
by a character that is specified by the parameter
.I hier_name_sep
(default the character '.').
The instance name(s) and the label or terminal name are separated
by a character that is specified by the parameter
.I inst_term_sep
(default the character '.').
If not all instance names are defined for the inherited label, 
or if the parameter
.I cell_pos_name
is set,
the name will be based on the name of the cell that the
original label or terminal comes from,
the original label or terminal name and the position of the
inherited label.
.P
The name of an inherited label is used to name the net that is 
connected to it only if this net has not a normal label connected to it.
For the rest, inherited labels are used in the same way as normal
labels.
.LE
.P
.H 3 "Nodes"
A node represents a vertex in the circuit graph.
If no resistances are extracted, exactly one node will be created for a net.
If resistances are extracted (see Section %SE_RESISTEXT%),
more than one node may be created for a net.
.H 3 "Node names"
.sY %SE_NODENAMESPEC% \n(H1.\n(H2.\n(H3
Default, nodes in the circuit are assigned a name
that is an integer number.
However, the latter is overruled in the following ways:
.BL
.LI
A node in the circuit that represents a terminal of the cell receives
a name that is equal to the terminal name.
.LI
A node in the circuit that represents a (inherited) label of the cell 
receives a name that is equal to the label name.
.LI
If resistances are extracted, 
if the node belongs to a net that
has a name (see Section %SE_NETNAMESPEC%)
and if the node does not represent a terminal nor a label,
then the node has a name <netname><separator><number>,
where <netname> is the net name, <separator> is specified using
the parameter 
.I net_node_sep
(default it is the character '_') and <number> is
an integer number.
.LI
If the parameter
.I node_pos_name
is set,
each node has a name <prefix><mask>_<xpos>_<ypos>,
where <prefix> is a prefix that is specified using the
parameter
.I pos_name_prefix
(default it is an empty string),
and the tuple <mask>, <xpos> and <ypos> denotes a part of the layout
(mask, x coordinate and y coordinate) that corresponds to the node.
It is the point that has the smallest x value and, next, the smallest
y value.
.LE
.P
Note that it is possible that a node in the output netlist has more than 
one name,
also because during resistance extraction different nodes may be joined.
Depending on the netlist format that is used, not all names
may be part of the output netlist.
.H 3 "Positions of devices and sub-cells"
When the option
.O= -t -torpos
is used with
.P= space 
or when the parameter
.I component_coordinates
is set,
positions of devices and sub-cells are added to the extracted circuit.
.H 3 "Name length"
The maximum number of characters in an instance, node or net name is
determined by the project version number.
The project version number is given on the first line of the .dmrc
file and for e.g. version number 3 the maximum name length is 14,
for version number 301 the maximum name length is 32
and for version number 302 the maximum name length is 255.
The
.P= mkpr
program creates only projects for version number 302.
.P
Node or net names that are generated by
.P= space
and that are longer than the maximum name length allowed by the project 
version number, are converted to a shorter name.
A translation table \fIcell\fP.nmp will then give the mapping between
the original names and the new names.
The maximum name length can be decreased (e.g. because
the simulator that is used after the extraction can not handle
the long names) by specifying a new maximum length using the parameter
.I max_name_length
.P
When a long name is converted to a short name, the parameter
.I trunc_name_prefix
specifies a prefix string for the new name (default is "n").
.H 3 "More back-annotation information"
The use of the option
.O= -x -backannotation
or setting the parameter
.I backannotation
causes that
.P= space
will also generate layout back-annotation information
about the geometry of the different nets, the different transistors etc.
This information can
e.g. be used as input for the program
.P= highlay.
The option 
.O= -x -backannotation
or the parameter
.I backannotation
implies the option
.O= -t -torpos "" .
.H 3 "Parameter File"
Back annotation is controlled by
the following parameters from the parameter file:
.DS I
.TS
tab(:);
l c c 
l c c.
_
parameter:type:default
_
hier_name_sep:char:.
inst_term_sep:char:.
no_labels:boolean:off
hier_labels:boolean:off
hier_terminals:boolean:off
leaf_terminals:boolean:off
cell_pos_name:boolean:off
term_is_netname:boolean:off
net_node_sep:char:\&_
node_pos_name:boolean:off
pos_name_prefix:string:<empty>
max_name_length:int:32
trunc_name_prefix:string:n
component_coordinates:boolean:off
backannotation:boolean:off
_
.TE
.DE
.H 3 "Command-Line Options"
The following command-line options control back-annotation:
.M(
.ME -t -torpos
Add positions of devices and sub-cells to the extracted circuit.
.ME -x -backannotation
Generate layout back-annotation information, implies
.O= -t -torpos "" .
.M)
.H 2 "Element Definition Files"
.sY %SE_ELEMDEFFILE% \n(H1.\n(H2
.P= Space
is technology independent.
At start up,
it reads a tabular element definition file
specifying
how the different elements like conductors and transistors
can be recognized from the different mask combinations,
and which values should be used for for example conductor
capacitivity and conductor resistivity.
This tabular element file is constructed
from a user-defined element definition file by the
space technology compiler
.P= tecc.
.P
The default element definition file is
\fIspace.def.t\fP 
in the appropriate directory of the ICD process library.
However,
there can be several other element definition files for a particular
process.
For example,
the file 
\fIspace.max.t\fP
may contain an element description with worst-case capacitance
and resistance values.
If this file exists,
it can be read rather than the standard file by 
specifying
.O= -e -elem max
at the command line.
.P
The user can also prepare his own element definition file
and specify the name of that file with the 
.O= -E -elemfile
option.
For a description of how to prepare an element definition file,
see Chapter %SE_MKELEM%.
But usually it is most convenient
to modify a copy of the source of one of the ``official''
element definition tables.
These sources are also in the ICD process library.
.H 3 "Command-Line Options"
The command line options are as follows:
.M(
.ME -e -elem xxx
Use the file 
space.\fIxxx\fP.t
in the ICD process library
as the element definition file.
.ME -E -elemfile file
Use \fIfile\fP as the element definition file.
.M)
.H 2 "Parameter Files"
.sY %SE_PARAMFILE% \n(H1.\n(H2
The parameter file for
.P= space
contains values for several variables that control the extraction process.
For example, 
it contains all parameters that control the network
reduction heuristics.
.P
The default parameter file is
\fIspace.def.p\fP
in the appropriate directory of the ICD process library.
However,
an alternative parameter file in the ICD process library can be used
by using the
.O= -p -param 
option at the command line,
and other files can be used by specifying the
.O= -P -paramfile
option at the command line.
.P
Parameters can be of different types, e.g. real, integer, string and boolean.
If a parameter is of type boolean, its value can be either on or off.
If the name of a boolean parameter is included in the parameter
file, but no value is specified, this is equivalent to specifying the
value on.
.P
Some parameters have a name
.I class.parameter
(e.g. sne.frequency).
In this case, a group of parameters of the same class may be used 
without the prefix "\fIclass.\fP"
if they are included between the lines "BEGIN \fIclass\fP" 
and "END \fIclass\fP".
E.g.
.fS I
BEGIN sne
frequency  1e9
END sne
.fE
is equivalent to
.fS I
sne.frequency  1e9
.fE
.P
The value of a parameter in the parameter file can also
be specified using the option
.O= -S "" "param=value" .
This sets parameter \fIparam\fP to the value \fIvalue\fP
and overrides the setting in the parameter file.
(-S \fIparam\fP is equivalent to -S \fIparam=\fPon.)
.P
Some parameters can also be specified as options,
e.g. the use of the option
.O= -F -flat
is equivalent to specifying the parameter
.I flat_extraction
in the parameter file.
Options overrule specifications in the parameter 
file and specifications using the option
.O= -S "" "param=value" .
.br
.ne 6
.H 3 "Command-Line Options"
The command line options are as follows:
.M(
.ME -p -param xxx
Use the file 
space.\fIxxx\fP.p
in the ICD process library
as the parameter file.
.ME -P -paramfile file
Use \fIfile\fP as the parameter file.
.ME -S "" "param=value"
Set parameter \fIparam\fP to the value \fIvalue\fP,
overrides the setting in the parameter (.p) file.
(-S \fIparam\fP is equivalent to -S \fIparam=\fPon.)
.M)
