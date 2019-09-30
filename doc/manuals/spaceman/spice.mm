.H 1 "Preparing Simulation Input"
.sY %SE_PRESPICE% \n(H1
.H 2 "Introduction"
After extraction, a circuit description can be obtained
using on of the programs
.P= xedif,
.P= xnle,
.P= xpstar,
.P= xsls,
.P= xspice
or
.P= xvhdl,
depending on the desired netlist format.
Here will we discuss the retrieval of circuit
description in the SPICE format using the program
.P= xspice,
to prepare input for the SPICE simulator.
However, several things that are said here are also valid
for the other netlist formats.
For a more precise description of the retrieval
of other netlist formats, see the manual page
of the corresponding program.
.P
The model descriptions of the devices that occur in the 
SPICE circuit
are normally specified using the control file of
.P= xspice.
The model parameters may be specified as a function
of some layout parameters like the transistor emitter 
area and perimeter and the transistor base 
width.
In the control file also instance specific information 
can be specified like a prefix for each instance name 
of a device and the format for printing the instance 
parameters of each device.
.P
Optionally, the program
.P= putdevmod
can be used to specify a model for a device.
This method is useful if the model for the device consists 
of more than just a standard transistor model e.g. if it is described
by one or more subcircuits in combination with 
one or more transistor models.
.SK
.H 2 "Describing SPICE Models using the Control File of Xspice"
.sY %SE_MODELINLIB% \n(H1.\n(H2
The following table lists the layout parameters that are computed
by the extractor for the different devices.
.DS I
.TS
tab(:);
l l l.
_
symbol:description:devices
_
w:transistor width:fet transistors
l:transistor length:fet transistors
ad:drain area:fet transistors
as:source area:fet transistors
pd:drain perimeter:fet transistors
ps:source perimeter:fet transistors
nrd:equivalent drain squares:fet transistors
nrs:equivalent source squares:fet transistors
ae:emitter area:bjt transistors
pe:emitter perimeter:bjt transistors
wb:base width:lateral bjt transistors
v:value:resistors and (junction) capacitors
area:junction area:junction capacitors
area<nr>:junction area:junction capacitors
perim:junction perimeter:junction capacitors
perim<nr>:junction perimeter:junction capacitors
_
.TE
.DE
The parameters ad, as, pd, ps, nrd and nrs are only
computed for a fet transistor when a condition list
is specified for its drain/source region in the element
definition file and when capacitance extraction is
enabled.
Which of the layout parameters are computed for junction
capacitors (e.g. v, area or area<nr>)
depends on the value of the parameter
.I jun_caps
(see Section %SE_JUNCAP%).
.H 3 "The control file"
.EQ
delim off
.EN
The default name of the control file of
.P= xspice
is
\fIxspicerc\fP.
First,
.P= xspice
tries to read this file from the current working directory.
Otherwise, it tries to open it in the process directory.
.P
In the control file, the models are included by specifying
them in one or more separate files called the library files.
These files are then included in the control file as follows:
.DS I
\fCinclude_library\fP  \fIfile_name\fP
.DE
For how the models are described in the library file,
see Section %SE_SPICELIB%.
.P
Different models can be selected
for each extracted device, based on the values
of the parameters of the device.
This is specified by
the so-called global model-specifications.
The global model-specifications
are of the following format:
.DS I
\fCmodel\fP  \fIname\fP  \fIorig_name\fP  \fItype_name\fP \fC[(\fP \fIrange_specs\fP \fC)]\fP
.DE
The \fIorig_name\fP has to be equal to the extracted device name, which is
equal to the name specified in the element definition file. 
The \fItype_name\fP has to be equal to
one of the following SPICE standard device-models: \fInpn\fP,
\fIpnp\fP, \fInmos\fP, \fIpmos\fP, \fIr\fP, \fIc\fP or \fId\fP. 
Optionally, \fIrange_specs\fP can be specified for the device geometries
for which the model is valid. 
For each layout parameter a range can be
specified according to one of the following formats:
.DS I
\fIlayout_parameter\fP  \fItypical_val\fP
\fIlayout_parameter\fP  \fIlower_val\fP  \fIupper_val\fP
\fIlayout_parameter\fP  \fIlower_val\fP  \fItypical_val\fP  \fIupper_val\fP
.DE
The range values can be expressions in terms of other layout parameters that
are specified (the expression must be between parentheses and the name of the
other layout parameters must be preceded by a '$'-sign).
It is allowed to specify only the typical value or to omit this
value from the range specification.
.P
Based on the values that are specified for the layout parameters 
of one particular global model-definition, the following situations 
can be distinguished:
.I(
.I= "Only the typical values are specified"
The device model is only valid for one specific device. 
The parameters for this device model are all known. 
The model is selected if the layout parameters of the extracted
device exactly match the typical values as specified.
.I= "Both typical and upper/lower values are specified"
Extracted devices for which the layout parameters are within the specified
ranges are assigned to this model and a scaling factor is included. 
The scaling factor is based on the typical value of the emitter area.
Also for this model all model parameters are known and specified in the
library file.
.I= "Only upper/lower values are specified"
For the extracted devices that cannot be assigned to models as described above,
a so called \fIsubstitution model\fP is used. 
If the layout parameters of a device
are within the range specification of such a model, a device specific model
is created. 
The parameters of this model are expressions in terms of the
layout parameters and must be computed for each different 
device geometry, see Section %SE_SPICELIB%.
Additionally, the name of such a model is extended by a suffix that depends
on the values of the layout parameters.
.I)
.E(
The following gives an example of a control file for
.P= xspice.
It contains a global model-specification of a bipolar
vertical npn-transistor that is extracted as a
device with name npnBW. 
Under the conditions with respect to emitter area and
emitter perimeter that are listed
with the global model-specification, the device
npnBW is assigned a model bw101x that is
included in the library
file \fIspice3f3.lib\fP.
.fS
include_library spice3f3.lib

model bw101x npnBW npn (
    ae 4e-12 2e-11 
    pe (2*$ae / 2e-6 + 4e-6)
)
.fE
.E)
.P
Also instance-specific information for each device
can be specified in a control file.
The format of a specification of a prefix for each instance name
of a device is as follows:
.DS I
\fCprefix\fP  \fIname\fP  \fIprefix\fP
.DE
The \fIname\fP can be the \fIorig_name\fP or the \fItype_name\fP
of the device.
A new prefix overrides a previous specified prefix.
For SPICE, the first letter of the prefix must be a legal standard 
device-model letter (e.g. 'm' for MOS transistors).
For SPICE normally only one prefix letter is used.
To use more prefix letters (max. 7) specify the following keyword:
.DS I
\fClong_prefix\fP
.DE
.P
For a device of type \fItype_name\fP, a bulk voltage \fIvalue\fP
may be specified in the control file
according to the following format.
.DS I
\fCbulk\fP  \fItype_name\fP  \fIvalue\fP
.DE
If a bulk voltage is specified,
.P= xspice
will automatically add a bulk terminal for the device
and it will connect it to the specified potential.
Up to a maximum number of 2 different values for bulk voltages
may be specified in the control file.
.P
For a device,
instance parameters may be specified as follows:
.DS I
\fCparams\fP  \fIname\fP  [\fImodel_name\fP] { \fIparam_spec1\fP  \fIparam_spec2\fP  ... }
.DE
The \fIname\fP can be the \fIorig_name\fP or the \fItype_name\fP and can
optionally be followed by the \fImodel_name\fP.
A new params-statement overrides a previous specified params-statement.
With the params-statement the printing order of parameter values (with or
without parameter name) can be changed.
Normally invisible parameters can be made visible or used.
Standard visible parameters can be left out or changed.
The parameter specifications \fIparam_spec1\fP, \fIparam_spec2\fP etc.
each must have one of the following forms:
.DS I
\fIparameter\fP=\fIvalue\fP
\fIvalue\fP
\fIparameter\fP=\fI$intern_par\fP[<operator><value>]
\fI$intern_par\fP[<operator><value>]
.DE
with \fI$intern_par\fP denoting the actual value of a parameter that is
internally (in the database) called 'intern_par' (see the table
at the beginning of Section %SE_MODELINLIB% for a list of possible
parameter names).
If the \fI$intern_par\fP does not exist in the instance attribute-list,
the parameter specification is left out!
If the \fI$intern_par\fP is a standard visible parameter, it is no more
printed in the standard way.
If the "\fI$intern_par\fP"-forms have a leading '!' sign, they are not printed.
This is the way to skip a standard visible parameter.
If the "\fI$intern_par\fP"-forms have two leading '!' signs,
they are printed in the comment-part.
The "\fI$intern_par\fP"-forms can optionally be followed by an <operator> 
and a <value>.
This <value> may also be another internal parameter.
The operation is only done, if this internal parameter exists and is not zero.
This <operator> can be a '+', '-', '*' and '/'.
At last, you can additionally use the '@' <operator> with a <string>.
Denoting that the <string> must be printed after the value.
.P
Other program build-in internal parameters are:
.ta 2.2c
.DS I
\fImname\fP	the used model name
\fImsf\fP	the scale factor for scalable models (default 1)
.DE
.E(
The following specifies that for instances of the capacitance model 'ndif',
the area of the element is represented by the parameter 'area'
and the perimeter of the element is represented by the parameter 'pj'.
.fS
params ndif { area=$area pj=$perim }
.fE
The following uses square micron and micron as units
for respectively the area and the perimeter.
.fS
params ndif { area=$area*1e12 pj=$perim*1e6 }
.fE
.E)
.P
For a complete overview of all possible statements in the
control file, see the manual page of
.P= xspice.
.H 3 "The library file"
.sY %SE_SPICELIB% \n(H1.\n(H2.\n(H3
The library file contains the actual model definitions, possibly
as a function of the layout parameters.
Besides a specification of the model parameters, it is also allowed to
define so called unity parameters in a library file. 
These parameters
can be used in the expressions for the model parameters and are formatted
as follows:
.DS I
\fCunity\fP  \fIname\fP  \fIvalue\fP
.DE
The detailed model specifications are of the following format:
.DS I
\fCmodel\fP  \fIname\fP  \fItype_name\fP  \fC(\fP \fIpar_list\fP \fC)\fP
.DE
The \fItype_name\fP is normally a standard SPICE model name,
but can also be an alternative simulator model name.
For each parameter in the list \fIpar_list\fP the name is specified and its expression or value.
(But, also only a parameter name may be specified, or a parameter name and an equal sign and
another name in place of the expression.)
Each expression is an equation in terms of operators and operands.
Operands can be values or unity/layout parameters.
In the latter case, the operand must be preceded by a '$'-sign.
A value can be followed by an unit sign (a, f, p, n, u, m, k, M, G, T, P) in place of e-notation.
The operators that can be used are listed below:
.DS I
.TS
tab(:);
c l.
_
symbol:operator
_
+:addition
-:subtraction
*:multiplication
/:division
^:power
_
.TE
.DE
.E(
An example model specification of the model described above is as follows:
.fS
unity ISs_wn_bw 5.6e-7
unity ISe_wn_bw 4.9e-13

model bw101x npn (Is=$ISs_wn_bw*$ae+$ISe_wn_bw*$pe Bf=117 
\t\t\tVaf=55 Br=4 Xtf=(4.7e-2*$ae+1.9e-2*$pe)^2
\t\t\tXtb=1.5 Var=4 Tf=20p)
.fE
.E)
.EQ
delim $$
.EN
.H 2 "The Use of the Program Putdevmod"
.sY %SE_PUTDEVMOD% \n(H1.\n(H2
With the program 
.P= putdevmod
device model descriptions are stored into the database as a circuit cell.
This method is required if 
the model for the device consists of more than
just a standard transistor model e.g. if it is described
by one or more subcircuits in combination with one or more transistor
models.
Note that in order to use
.P= putdevmod
to store model descriptions for a cell, the extraction status 
of the cell must be set to "device" using the program
.P= xcontrol.
.H 3 "Syntax and semantics of the input file"
On the first line of an input file for the program
.P= putdevmod
there is the keyword "device" followed by the name of
the device.
On a next line the keywords "begin spicemod"
are specified to denote the beginning of the SPICE device information.
The next lines are
considered to be SPICE input that can directly be appended
to a SPICE network description.
When a SPICE circuit description is retrieved using
.P= xspice,
the SPICE model description of a device that has been added
to the database using
.P= putdevmod,
will automatically be added
at the end of the network description
if the device is part of the network
and if a model description of the device does not occur in a
library file (see the previous section).
The end of the SPICE input is denoted by
a line that contains the keyword "end".
.P
In the part of the input file that is used as SPICE input,
the terminals, a possible bulk potential and a prefix for instance
names for the device, may be included as SPICE comment.
This is done by specifying them on lines that start with
the comment character '*'.
.P
To specify the terminals, use the keyword "terminals" 
followed - on the same line and separated by spaces and/or tabs -
by the names of the terminals.
The order in which the terminals
are specified in this file
must agree with the order in which the terminals are
required in the SPICE circuit description (see the Spice User's Manual).
.P= Space
uses the following terminal names for the devices:
"g", "d" and "s" for the gate, drain and source of field-effect transistors,
"c", "b" and "e" for the collector, base and emitter of bipolar transistors,
and "n" and "p" for the terminals of resistors and capacitors.
.P
A bulk voltage (if appropriate) may be specified by the keyword "bulk"
followed by a floating point number.
.P
A prefix character for the instance names of the device
is specified by the keyword "prefix",
followed by a character.
.P
An example of an input file for
.P= putdevmod
is given below:
.fS I
device lnpn
begin spicemod
* terminals  c b e
* bulk       0
* prefix     q
\&.model lnpn npn
+    is=6.4e-16     bf=160       vaf=100   ikf=3e-2
+    ise=2.859e-15  ne=1.476     br=3      var=20
+    ikr=0.04       isc=1e-14    nc=1.15   rb=80
+    irb=2.0e-5     rbm=15       rc=150    re=2
+    cje=0.68e-12   mje=0.34     vje=0.71  fc=0.63
+    cjc=6.8e-13    mjc=0.33     vjc=0.55  xcjc=0.19
+    tf=4.2e-10     tr=7.0e-8    xtb=1.6   xti=3
+    eg=1.16        cjs=3.1e-12  mjs=0.35  vjs=0.5
end
.fE
