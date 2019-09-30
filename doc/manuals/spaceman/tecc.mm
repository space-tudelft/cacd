.H 1 "Developing Space Element Definition Files"
.sY %SE_MKELEM% \n(H1
.H 2 "Introduction"
The element definition file describes how
.P= space
recognizes the circuit elements 
(transistors, contacts, interconnection layers etc.)
from the layout definition of the cell.
The element definition file further contains the values for the different
interconnect capacitances, the sheet resistances
for the interconnect layers, etc.
.P
The program
.P= tecc
acts as a pre-processor for technology descriptions
for the
.P= space
layout to circuit extractor.
From a user-defined element-definition source file,
.P= tecc
produces a compiled element-definition file
that can be used as input for
.P= space.
.H 2 "Invocation and Command Line Options"
The program
.P= tecc
is invoked as follows:
.DS I
\fCtecc\fI [-sn] [-m maskdatafile] [-p process] file\fR
.DE
The user-defined input file should
have the extension '.s', while the compiled  output file
will have the same name but with '.t' substituted for '.s'.
The compiled output file is an ascii file,
hence it can easily be exchanged between different types of machines.
.H 3 "Command-Line Options"
The following options can be specified:
.M(
.ME -s -silent
Silent mode.
This will suppress some diagnostics information, see section %SE_TECHDIAG%.
.ME -n -nocompress
Do not compress table-format element-definition file.
This option is useful during element-definition file development.  
It makes 
.P= tecc
run faster, and 
.P= space 
somewhat slower.
.ME -m -maskdata maskdatafile
.br
Specifies the maskdatafile.
Default, the maskdatafile is obtained from the process directory.
.ME -p -process process
.br
Specifies the process.
The default process is determined by the current project
directory.
This option allows to run
.P= tecc
outside a project directory.
.M)
.H 2 "Example Technology"
In this section, we will use a double metal Nwell CMOS
process as an example.
This technology is known in the system under the name 'scmos_n',
and the technology files can be inspected.
The masks that are used are the following:
.DS I
.TS
tab (:);
l l.
_
maskname:description/purpose
_
cpg:polysilicon interconnect
caa:active area
cmf:metal interconnect
cms:metal2 interconnect
cca:contact metal to diffusion
ccp:contact metal to poly
cva:contact metal to metal2
cwn:n-well
csn:n-channel implant
cog:contact to bondpads
_
.TE
.DE
The process is similar to the Mosis n-well scmos process.
An element definition file for this process is included
as appendix %AX_ElemFile%, and a parameter file as
appendix %AX_ParamFile%.
.P
For illustrating the features that are only meaningful to
bipolar processes, 
we will use the bipolar DIMES-01 process
as example. 
An element definition file for this process is included
as appendix %AX_BipElemFile%.
The standard masks of this process are:
.DS I
.TS
tab (:);
l l.
_
maskname:description/purpose
_
bn:buried N-layer
dp:deep P-well; island isolation
dn:deep N-well; collectorplug of all NPNs
wp:extrinsic base of the NPNs
bw:intrinsic base of the BW-NPN
wn:shallow N-layer; washed emitter implantation
co:contact windows in wp, sn and sp
ic:interconnect layer
ct:second contact window for in-ic contact
in:second interconnect layer
_
.TE
.DE
.H 2 "The Element Definition File"
.sY %SE_THE_ELEM% \n(H1.\n(H2
.H 3 "General"
The element-definition file defines the circuit elements
that can be recognized from the layout description.
For each of the elements, at least a
.I name
and a
.I "condition list"
have to be specified.
In this subsection, names and condition lists are defined.
They are illustrated in the next subsections.
.P
The name of an element is used to identify the element when
error messages are generated or, for transistor elements,
to identify the element in the circuit that is extracted.
It is not allowed to use the same name in more than one element 
definition.
.P
A condition list specifies
how the presence of a particular element depends on the
presence or absence of the different masks.
The condition list is a boolean expression where
the masks are used as the variables of the expression.
In the expression, the AND operation is performed using
simple concatenation, the OR operation is performed using
the '|' character (AND has precedence over OR)
and the INVERT operation is performed
using the '!' character.
Parentheses may be used to nest the expression.
.E(
Examples of condition lists are:
.fS
    caa !cpg !csn
.fE
.fS
    cca cmf caa !cpg ( cwn !csn | !cwn csn )
.fE
.E)
.P
References to masks in adjacent areas can be made by preceding
the mask names with a '-' character (specifying an edge mask),
or a '=' character (specifying an opposite edge mask).
The following (cross-section) gives an example.
.P
.DS
.PS 3i
copy "../spaceman/cross.pic"
.PE
.DE
.S=
.P
When preceding the mask name with a '-' character,
a reference is made to a mask that is in an adjacent area.
When preceding the mask name with a '=' character,
a reference is made to a mask that is in an area
that is opposite to the area that contains the masks preceded 
with the '-' sign.
The above can be used to define the elements that
are present on the boundary between two regions of 
different mask combinations
(e.g. edge capacitances)
or elements that are present between the boundaries
themselves (e.g. lateral capacitances between
parallel wires).
.P
In the following, the syntax and semantics of the
element-definition file is described.
The element-definition file may contain specifications
for, among other things, the units, the key list, the new masks, 
the mask colors, 
the conductor elements, the transistor elements,
the connect and contact elements and the capacitance elements.
.N(
Each of the specifications in Sections %SE_UNITLIST%-%SE_CAPELEM%
is optional but their order is fixed.
.N)
.H 3 "Comment"
Comments can be included in the element-definition file by
preceding them with a '#' character.
All text following the '#' character, until the end of the line,
will be skipped as comment.
.E(
.fS
    # this is comment
.fE
.E)
Comment can be included at any place in the element definition file.
.br
.ne 6
.H 3 "Unit specification"
.sY %SE_UNITLIST% \n(H1.\n(H2.\n(H3
Syntax:
.DS I
\fCunit\fP \fIvariable\fP \fIvalue\fP
  .
  .
.DE
With an unit command, the unit of variable values used in one
of the element lists can be specified.
The string \fIvariable\fP can be represented by the following key word:
.VL 20 2
.LI \fIresistance\fP 
to specify the unit for sheet resistance (see conductors),
.LI \fIc_resistance\fP 
to specify the unit for contact resistance (see contacts),
.LI \fIa_capacitance\fP 
to specify the unit for area capacitance,
.LI \fIe_capacitance\fP 
to specify the unit for edge capacitance
or - if the capacitance is specified by means of
(distance, capacitivity) pairs -
also for lateral capacitance,
.LI \fIcapacitance\fP 
to specify the unit for other (lateral) capacitance,
.LI \fIdistance\fP 
to specify the unit for distance in
(distance, capacitivity) pairs,
.LI \fIresize\fP 
to specify the unit for resizing masks.
.LE
.P
For other key words see the "Space 3D Capacitance Extraction User's Manual".
.br
The unit of each variable is specified by \fIvalue\fP, expressed
in S.I. units.
.E(
The following gives some examples of unit specifications
(between comment the new unit is described).
.fS
unit resistance    1     # ohm
unit c_resistance  1e-12 # ohm um^2
unit a_capacitance 1e-6  # aF/um^2
unit e_capacitance 1e-12 # aF/um
unit capacitance   1e-15 # fF
unit distance      1e-6  # um
unit resize        1e-6  # um
.fE
.E)
.H 3 "The key list"
.sp 0.15
.N(
You may skip this subsection until or unless you run
into problems with the size of the compiled technology file,
due to a large number of masks and/or element definitions.
.N)
.sY %SE_KEYLIST% \n(H1.\n(H2.\n(H3
Syntax:
.DS I
\fCkeys:\fP \fImask1 mask2 ... maskN\fP
.DE
During extraction,
.P= space
uses a hash table to speed-up the recognition of the elements.
This hash table is constructed based on the masks that are specified
in the key list.
When a mask is specified in the key list,
.P= space
is capable of recognizing elements
without separately checking the element conditions
that refer to the presence or absence of that mask.
Adjacent masks (that are specified by preceding them with a '-' character)
may also be used as a key mask.
Note that the size of the hash table will be proportional to 2 ^ N, 
where N is the number of key masks.
By carefully choosing the key masks, the speed
of element recognition will be optimal, while the hash table
will be not too large.
Actually,
.P= space
will use two key lists,
one for surface elements and one for edge elements.
.P
When no key list is specified, but
.DS I
.ft C
maxkeys \fImaxkeys [ maxkeys2 [ maxedgekeys ] ]
.R
.DE
is specified instead,
.P= tecc
will select the key masks itself.
Up to a maximum of \fImaxkeys\fP surface mask keys for the surface element key list
will be used.
The most frequently used masks,
specifying element conditions,
will be selected.
Optional,
up to a maximum of \fImaxkeys2\fP surface mask keys for the edge element key list
will be used.
And optional,
up to a maximum of \fImaxedgekeys\fP edge mask keys are also used
for the edge element key list.
.N(
A specification of many key masks or a very large value for \fImaxkeys\fP
may cause
.P= tecc
to run out of memory, or it may
result in an excessive long running time for
the program.
.N)
.P
When neither a key list nor a maximum number of keys
is specified,
.P= tecc
will assume that \fImaxkeys\fP and \fImaxkeys2\fP are equal to 12
and \fImaxedgekeys\fP is equal to 0.
.E(
When specifying for the complete element description as found 
in the CMOS technology library
.fS
    maxkeys 13
.fE
.P= tecc
will find a key list that is equivalent to
.fS
    -- keys: cms cpg cmf caa cwn csn cca ccp cva
    -- keys2: cms cpg cmf caa cwn csn
    -- number of keys: 6 + 3 (9)
    -- number of keys2: 6 + 0 (6)
.fE
To get also six edge masks into the edge key list (keys2), specify:
.fS
    maxkeys 13 6 6
.fE
But, normally no edge mask keys are used for the key list,
because there are normally too many surface masks.
Besides that,
.P= space
is using an improved hash table and runs also fast with small key lists.
.E)
.H 3 "New mask specification"
.sY %SE_NEWMASK% \n(H1.\n(H2.\n(H3
Syntax:
.DS I
\fCnew\fP: \fIcondition_list : name\fP
  .
  .
.DE
This command allows to create a new mask from the combination
of other masks.
The new mask, given by \fIname\fP, is defined everywhere where
the combination of masks satisfies the condition list of the specification.
The characters '-' and '=' may not be used in the condition list for a 
new mask.
.E(
.fS
new : caa !cpg !csn : pdf   # mask pdf defines p+ active area
new : caa !cpg  csn : ndf   # mask ndf defines n+ active area
.fE
.fS
new : caa ( cwn !csn | !cwn csn ) : cta   # contact area
.fE
.fS
new : !dp : epi    # mask epi defines an epitaxial layer
.fE
.E)
.H 3 "Resize mask specification"
.sY %SE_RESIZEMASK% \n(H1.\n(H2.\n(H3
Syntax:
.DS I
\fCresize\fP:\fI condition_list : mask : value\fP
  .
  .
.DE
This command allows to grow masks with a certain value
(when a positive value is specified)
or shrink masks with a certain value
(when a negative value is specified).
The
.I mask
that is specified must be in the
.I condition_list
that is specified,
or it must be a newly created mask name.
In the last case, the new mask becomes a real mask.
For one mask, more than one resize statement may be specified.
.E(
The following grows the cpg mask with 0.01 micron.
.fS
resize : cpg : cpg : 0.01e-6
.fE
.E)
Apart from using the resize statement to model the difference
between "mask dimensions" and "on-chip dimensions", it
may also be used to e.g. merge arrays of small contacts
into bigger contacts.
This may be desirable, sometimes, in order to improve the 
efficiency of resistance extraction.
.E(
The following two statements will merge all 'cva' contacts 
that are within 0.5 micron of each other.
This is achieved by first growing the 'cva' layout object with 
0.25 micron, and next shrinking the newly obtained objects with 
0.25 micron.
.fS
resize : cva : cva : 0.25e-6
resize : cva : cva : -0.25e-6
.fE
.E)
.H 3 "Mask colors"
The colors that are used to display the different masks/conductors when using
.P= Xspace
may be specified as follows:
.P
Syntax:
.DS I
\fCcolors\fP :
.P
   \fImask color\fP
     .
     .
.DE
For a more detailed description, see the
"Xspace User's Manual".
.ne 9
.H 3 "The conductor list"
.sY %SE_RESELEM% \n(H1.\n(H2.\n(H3
Syntax:
.DS I
\fCconductors\fP [\fItype\fP] :
.P
   \fIname : condition_list : mask : sheet-resistivity\fP [: \fIcarrier_type\fP]
     .
     .
.DE
The conductor list contains the definitions for the conducting
layers in the circuit.
For each conductor specification, a specification of the
actual conductor mask and a specification of the
sheet-resistivity (in ohms) is required.
For bipolar devices in particular, it is also necessary to
specify the \fIcarrier-type\fP of the conductor. 
The type can be
\fIn\fP for n doped conductors, \fIp\fP for p doped
conductors and \fIm\fP otherwise.
The default carrier-type is \fIm\fP.
.P
Conducting layers in adjacent areas are connected
to each other
\fIif and only if\fP they have the same conductor mask and the same
carrier type.
If either the conductor mask is different or the carrier type 
is different, the conducting layers are not connected to
each other.
.P
Optionally, a type may be specified for the conductor list.
.E(
The following conductor section is appropriate for the CMOS
example technology:
.fS
conductors :
    cond_mf : cmf           : cmf  : 0.045 : m  # first metal
    cond_ms : cms           : cms  : 0.030 : m  # second metal
    cond_pg : cpg           : cpg  : 40    : m  # poly interconnect
    cond_pa : caa !cpg !csn : caa  : 70    : p  # p+ active area
    cond_na : caa !cpg  csn : caa  : 50    : n  # n+ active area
.fE
The following describes some conductors for the bipolar
example technology:
.fS
    condIC  : ic          : ic  : 0.044 : m
    condBW1 : bw !wn      : bw  : 600   : p
    condBW2 : bw wn       : bw  : 7000  : p
    condWN  : wn          : wn  : 40    : n   # shallow N-layer
.fE
.E)
Default, when extracting resistances, linear resistances
will be extracted for a conductor.
However, when a conductor type is specified with the conductor list,
the extracted resistances for all conductors in that list will be of the
specified type.
.E(
.fS
conductors rdif :
    cond_pa : caa !cpg !csn : caa  : 70 : p    # p+ active area
    cond_na : caa !cpg  csn : caa  : 50 : n    # n+ active area
.fE
.E)
In this case, a resistor model corresponding to the specified conductor 
type must be specified using the control file of
.P= xspice
(see Section %SE_MODELINLIB%).
.P
An element definition file may contain more than one conductor list.
.br
.ne 6
.H 3 "The field-effect transistor list"
.sY %SE_TORELEM% \n(H1.\n(H2.\n(H3
Syntax:
.DS I
\fCfets\fP :
.P
   \fIname : condition_list : mask_g mask_ds\fP [ (\fIcondition_list\fP) ] [ : \fIconnect_b\fP ]
     .
     .
.DE
For a field-effect transistor (e.g. MOS transistor), 
the name and the condition list are followed
by a specification of the gate mask \fImask_g\fP and
the drain/source mask \fImask_ds\fP.
The gate mask and the drain/source mask must be masks
that are defined as a conductor in the conductor list.
Optionally, in parentheses, a condition list 
for the drain/source region can be specified.
When capacitance extraction is enabled, this information will 
be used to compute the parameters ad, as, pd, ps, nrs and nrd 
(see SPICE User's Manual) for the transistor.
Further, optionally, at the end of the specification after a colon,
a bulk connection \fIconnect_b\fP can be specified for the transistor.
This connection may consist of
(1) a mask that is specified as a conductor in
the conductor list, (2) the string "@sub" to denote
the substrate area below the transistor gate, or (3)
the notation "%(\fIcondition_list\fP)" to denote a substrate
area described by the condition list.
When case (3) is used, the area specified by the condition list 
must have an overlap with the transistor gate area.
.E(
The MOS transistors can be defined as follows:
.fS
fets :
    nenh : cpg caa  csn : cpg caa     # nenh MOS
    penh : cpg caa !csn : cpg caa     # penh MOS
.fE
Note that in this example, the n-well mask 'cwn' has not been used,
since it is assumed that this mask is, respectively, present or absent
because the layout is free of design-rules errors
(see also appendix %AX_PROBLEMS%).
.P
In case when the extraction of the parameters ad, as, pd, ps, nrs 
and nrd is required (to describe the properties of the transistor 
drain/source region), the following specification can be used:
.fS
fets :
    nenh : cpg caa  csn : cpg caa (caa !cpg  csn)   # nenh MOS
    penh : cpg caa !csn : cpg caa (caa !cpg !csn)   # penh MOS
.fE
.E)
When no bulk terminals are specified (as in the above transistor
definition example),
the program
.P= xspice
will add appropriate bulk terminal connections
when a SPICE circuit description is retrieved from the database.
The actual bulk connections for field-effect transistors may be extracted
by specifying also a bulk connection for each transistor.
.E(
When 'cwn'
is defined as a conductor mask,
the actual bulk connections may be extracted
by using the following transistor definition:
.fS
fets :
    nenh : cpg caa  csn : cpg caa : @sub   # nenh MOS
    penh : cpg caa !csn : cpg caa : cwn    # penh MOS
.fE
In case of substrate resistance extraction, the above
specification will result in the creation of substrate
contact directly under the 'nenh' transistor gate area.
When the user wants to create a substrate terminal for
the 'nenh' transistor under the gate area as well as under
the drain and source areas of the transistor, the 
following specification may be used:
.fS
fets :
    nenh : cpg caa  csn : cpg caa : %(caa csn !cwn)
.fE
The condition '!cwn' has been used here to prevent that substrate
contacts are also generated for well contacts.
.E)
.ne 5
.H 3 "The bipolar transistor list"
.sY %SE_BITORELEM% \n(H1.\n(H2.\n(H3
Syntax:
.DS I
\fCbjts\fP :
.P
   \fIname : condition_list : type : mask_em mask_ba mask_co\fP [ : \fIconnect_b\fP ]
     .
     .
.DE
For a bipolar junction 
transistor
the name, the condition list and
the transistor-type ("ver" for vertical or "lat" for lateral)
are followed by a specification of the emitter mask \fImask_em\fP,
the base mask \fImask_ba\fP, the collector mask \fImask_co\fP.
These masks must be defined as a conductor in the conductor list.
Optionally, after a colon, a bulk connection \fIconnect_b\fP
may be specified for the transistor.
This connection may consist of
(1) a mask that is specified as a conductor in
the conductor list, or (2)
the notation "%(\fIcondition_list\fP)" to denote a substrate
area described by the condition list.
When case (2) is used, the area specified by the condition list
must have an overlap with the transistor area.
.E(
The bipolar transistors can be defined as follows:
.fS
bjts :
    npnBW :      wn  bw epi : ver :  wn  bw epi : %(bw wn)
    pnpWP : !wp -wp !bw epi : lat : -wp epi =wp
.fE
.E)
.ne 6
.H 3 "The connect list"
.sY %SE_CONNELEM% \n(H1.\n(H2.\n(H3
Syntax:
.DS I
\fCconnects\fP :
.P
   \fIname : condition_list : mask1 mask2\fP
     .
     .
.DE
The connect elements connect different semiconductor regions of
the same carrier-type. 
They define the connectivity relation between
the different conductors.
\fIMask1\fP and \fImask2\fP are
the conductor masks that are connected.
Connect elements can not be used to connect conducting layers
that have a different carrier-type.
Note: the connection of conductor layers via a contact or via
should be specified in the contact list (see section %SE_CONTELEM%).
.E(
.fS
connects :
    connBW : bw wp      : bw wp    # connect bw and wp
    connBN : bn epi     : bn epi   # connect epi and buried layer
    connSN : sn epi !dn : sn epi   # connect epi and sn
.fE
.E)
.ne 6
.H 3 "The contact list"
.sY %SE_CONTELEM% \n(H1.\n(H2.\n(H3
Syntax:
.DS I
\fCcontacts\fP [\fItype\fP] :
.P
   \fIname : condition_list : mask1 mask2 : resistivity\fP
     .
     .
.DE
The contact elements connect different conductors that are 
on top of each other.
\fIMask1\fP and \fImask2\fP are 
the conductor masks that are connected by the contact.
For \fImask1\fP or \fImask2\fP also the string "@sub" or
the "%(\fIcondition_list\fP)" notation may be used.
The first specifies a connection to the substrate area 
directly below the contact area. 
The second specifies a connection to a substrate 
area as specified by the condition list.
In the second case, the substrate area described by the condition 
list must have an overlap with the contact area.
The resistivity parameter specifies
the contact resistance in ohm square meter for a 
(hypothetical) contact of 1 meter * 1 meter.
Optionally, a type may be specified for the contact list.
.ne 8
.E(
.fS
contacts :
    cont_s : cva cmf cms      : cmf cms : 1e-12   # metal to metal2
    cont_p : ccp cmf cpg      : cmf cpg : 100e-12 # metal to poly
    cont_a : cca cmf caa !cpg : cmf caa : 100e-12 # metal to active
    cont_b : cca cmf !cwn !csn: cmf @sub: 100e-12 # metal to sub.
.fE
.E)
Default, when extracting resistances, linear resistances
will be extracted for a contact.
However, when a contact type is specified with the contact list,
the contact resistances
for all contacts in that list will be of the specified type.
An element definition file may contain more than one contact list.
.H 3 "The capacitance list"
.sY %SE_CAPELEM% \n(H1.\n(H2.\n(H3
Syntax:
.DS I
[\fCjunction\fP] \fCcapacitances\fP [\fItype\fP] :
.P
   \fIname : condition_list : mask1 [ mask2 ] : capacitivity\fP
     .
     .
.DE
In the capacitance list, coupling capacitances,
ground capacitances and substrate capacitances
can be defined.
.P
Coupling capacitances are defined by using a conductor mask
for both \fImask1\fP and \fImask2\fP.
.P
Ground capacitances are defined by using the string
"@gnd" for either \fImask1\fP or \fImask2\fP,
or by omitting \fImask2\fP.
In this case, the capacitance will on one side 
be connected to a node that is called "GND".
.P
Substrate capacitances are defined by using 
the string "@sub" or the notation
"%(\fIcondition_list\fP)" for either \fImask1\fP or \fImask2\fP.
In this case, the capacitance will on one side be connected
- when no substrate resistances are extracted - 
to a node that is called "SUBSTR", or 
- when substrate resistances are
extracted, see the "Space Substrate Resistance Extraction User's Manual"
- to a node that corresponds to a substrate terminal.
In case of substrate resistance extraction, the use of the 
string "@sub" will denote the substrate area directly under 
the capacitance area,
while the notation "%(\fIcondition_list\fP)" is used to
denote a substrate area specified by the condition list,
which must enclose the capacitance area.
.P
Capacitances are further distinguished between
surface capacitances, edge capacitances and lateral capacitances.
.P
To define edge capacitance, masks preceded
with a '-' character are used in
the condition list, and either \fImask1\fP 
or \fImask2\fP, or both, have to be preceded with
a '-' character to denote an edge of an interconnection.
.P
To define lateral capacitances, masks preceded
with a '-' character and mask preceded with a '=' character
are used in the condition list, and either \fImask1\fP
or \fImask2\fP is preceded with a '-' character to denote
one edge of an interconnection and the other mask is preceded
with a '=' character to denote another (opposite) 
edge of an interconnection.
.P
For surface capacitances, \fIcapacitivity\fP is
the capacitance per square meter.
For edge capacitances and lateral capacitances, the capacitance can be 
specified in two different ways.
.P
For edge capacitances, if only one value is specified (as in the above),
\fIcapacitivity\fP is the capacitance per meter edge length.
.P
For lateral capacitances,
if only one value is specified (as in the above), \fIcapacitivity\fP is the
capacitance for a configuration where the spacing between two
parallel wires is equal to length of the two wires.
In that case, it is assumed that the lateral coupling capacitance 
is proportional to 
the distance between the two wires and inverse proportional
to their spacing, see Section %SE_LATCAP%.
.P
Edge capacitances and
lateral capacitances can also be defined as follows.
.DS I
.if n .ta 44 54
.if t .ta 2.95i 3.65i
   \fIname : condition_list : mask1 mask2 : distance1 capacitivity1\fP
	\fIdistance2 capacitivity2\fP
		.
		.
.DE
In this case, for edge capacitances, the distance, capacitivity pairs 
specify the capacitance per meter edge length for a given distance to a 
neighboring wire that is of the same type.
In that case,
a lateral coupling capacitance must also be defined for these wires.
If no lateral coupling capacitance is defined or
if the distance between the wire and the neighboring wire is larger than 
the maximum distance for which an edge capacitance is specified, the 
edge capacitance is equal to the edge capacitance that is specified with
the maximum distance.
.P
For the lateral capacitances,
the distance, capacitivity pairs specify the capacitance 
between
two parallel wires of a length of 1 meter for different values of the
distance between them.
.P
For both edge capacitances and lateral capacitances,
capacitances for other configurations are found
from an interpolation between two distance, capacitivity pairs
(see Section %SE_LATCAP% and Section %SE_EDGECAP%).
.E(
The following three lines specify
first metal bottom to ground capacitance,
first metal sidewall to ground capacitance and
the lateral coupling capacitance between two parallel first metal lines
under the condition that no second metal is present:
.fS
capacitances:
    acap_cmf_sub :  cmf                !cpg !caa : cmf @gnd: 25e-06
    ecap_cmf_sub : !cmf -cmf      !cms !cpg !caa :-cmf @gnd: 52e-12
    lcap_cmf     : !cmf -cmf =cmf !cms !cpg !caa :-cmf =cmf: 30e-18
.fE
The following line specifies the coupling capacitance between the edge 
of a first metal wire and the edge of a second metal wire that are on top 
of each other:
.fS
capacitances:
    eecap_cmf_cms : !cmf -cmf !cms -cms : -cmf -cms : 23e-12
.fE
The first three capacitances may (more accurately) be specified as 
a function of the distance between two neighboring wires as follows:
.fS
capacitances:
    acap_cmf_sub :  cmf                !cpg !caa : cmf @gnd: 25e-06
    ecap_cmf_sub : !cmf -cmf      !cms !cpg !caa :-cmf @gnd: 
                                                        1e-6 30e-12  
                                                        2e-6 46e-12 
                                                        4e-6 50e-12  
                                                        8e-6 52e-12
.fE
.sp -0.5
.fS
    lcap_cmf     : !cmf -cmf =cmf !cms !cpg !caa :-cmf =cmf: 
                                                        1e-6 27e-12  
                                                        2e-6  8e-12
                                                        4e-6  3e-12  
                                                        8e-6  1e-12
.fE
.E)
.P
There may be more than one capacitance list in an element definition file.
Optionally, for each capacitance list a type may be specified
after the keyword "capacitances".
In that case, all capacitance definitions in that list are of the 
specified type, and the extracted capacitances will also have that type.
.P
Normally, the positive node and the negative node of the extracted
capacitance will be arbitrarily connected to the layers that are
specified with \fImask1\fP and \fImask2\fP.
However, for capacitance lists for which a type is specified,
if the keyword "junction" is used before the keyword "capacitances"
for all elements in that list
\fImask1\fP specifies the positive node of the element
and \fImask2\fP specifies the negative node of the element.
For junction capacitances, the method of extraction is further 
determined by the parameter
.I jun_caps
(see Section %SE_JUNCAP%).
Lateral coupling capacitances may not be specified for junction capacitances.
.P
.E(
Junction capacitances of type 'ndif' for n diffusion areas
and junction capacitances of type 'pdif' for
p diffusion areas may be specified as follows:
.fS
junction capacitances ndif :
    acap_na:  caa       !cpg  csn  !cwn :  caa @gnd : 100e-6
    ecap_na: !caa -caa !-cpg -csn !-cwn : -caa @gnd : 300e-12
.fE
.fS
junction capacitances pdif :
    acap_pa:  caa       !cpg  !csn cwn     : caa cwn : 500e-6
    ecap_pa: !caa -caa !-cpg !-csn cwn -cwn:-caa cwn : 600e-12
.fE
.E)
In order to simulate a circuit that contains junction capacitances,
a corresponding (diode) model has to specified for each type
('ndif' and 'pdif' in the above example)
using the control file of
.P= xspice.
.H 2 "Diagnostics"
.sY %SE_TECHDIAG% \n(H1.\n(H2
Without the
.O= "-s" -silent
option,
.P= tecc
will print information about the hash table that is being constructed
for element recognition.
This information can be used to tune
the specification of the key masks.
.P
Furthermore, 
.P= tecc
will check if legal mask names are used and if conductor masks
are used with transistor, contact and capacitance definitions.
During extraction itself it will be checked if appropriate conductor
elements are present;
for transistor elements and capacitance elements this is required,
for contact elements this is not required.
