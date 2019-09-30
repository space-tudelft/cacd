.T= "Adding Multi-layer FEM Support to Tecc"
.DS 2
.rs
.sp 1i
.B
.S 15 20
ADDING MULTI-LAYER FEM
SUPPORT
TO THE TECC PROGRAM
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
Report EWI-ENS 04-12
.ce
October 29, 2004
.sp |9i
.DS
.in +5
.S 9
Copyright \s+3\(co\s-3 2004 by the author.

Last revision: April 20, 2005.
.S
.in -5
.DE
.SK
.S=
.H 1 "INTRODUCTION"
This document explains how the multi-layer FEM is implemented in the
.P= tecc
program.
.br
A new "wafer" statement is added for Eelco Schrik to the technology file.
And also the resize possibility of new masks.
.P
See appendix A for the BEM/FEM note of Eelco Schrik, which explains the multi-layer FEM method.
See appendix B for the pseudo code for handling the "wafer" statement.
See the picture below for an explanation of combined BEM/FEM.
.P
.F+
.PSPIC "an0412/fig1.ps" 5i 3i
.F-
.P
The real situation is a little bit more complicated then the picture shows.
.br
But the intention is, to model the substrate area around the contact more accurate.

The following picture gives a side-view of a FEM modeled substrate contact.
.P
.F+
.PSPIC "an0412/fig2.ps" 4.5i
.F-
.P
.H 1 "THE WAFER STATEMENT"
One or more wafer definition statements can be specified in the technology file.
This must be done before the "conductors" definition section.
The
.P= tecc
program expands these statements to a number of "new", "conductor" and "contact" statements.
The conductors of the first "wafer" statement are called "w1_1", "w1_2", etc.
(The conductors of a second "wafer" statement are called "w2_1", "w2_2", etc.)
The "w1_1" layer, is the top layer, and can be connected with an interconnect layer.
The "w1_\fIn\fP" layer, is the lowest layer, and can be connected with the substrate.
This is automatically done, when option "subconn=off" is not specified.
.fS
wafer: conditions : conductivity : thickness : #layers [ : [options]...]
                        (S/m)       (micron)
options are:
    viamask = mask      (default: no viamask used)
    subconn = on | off  (default: on)
    restype = m | p | n (default: p)
.fE
For example, the following "wafer" statement
.fS
wafer: !csn !cwn : 1000 : 0.5 : 3 : restype=n subconn=off
.fE
is internally expanded to:
.fS
new: !csn !cwn : w1_1
new: !csn !cwn : w1_2
new: !csn !cwn : w1_3

conductors:
  # name   : condition : mask : resistivity : type
  cnd$w1_1 : !csn !cwn : w1_1 : 8000        : n
  cnd$w1_2 : !csn !cwn : w1_2 : 4000        : n
  cnd$w1_3 : !csn !cwn : w1_3 : 8000        : n

contacts:
  # name   : condition : lay1 lay2 : resistivity
  cnt$w1_1 : !csn !cwn : w1_1 w1_2 : 2.5e-10
  cnt$w1_2 : !csn !cwn : w1_2 w1_3 : 2.5e-10
.fE
Another example with a resized channel-stop mask "cs1" is given in appendix C.
.H 1 "APPENDICES"
.SP
.nr Hu 2
.HU "APPENDIX A -- BEM/FEM NOTE (October 29, 2004) by Eelco Schrik"
.HU "NOTE ON COMBINED BEM/FEM AND BEM/MULTILAYERFEM"
.HU "OLD APPROACH (INCLUDING A FEW TRICKS)"
A simple combined BEM/FEM extraction could be run in the following way:
.fS
space3d -ziBFv -P space.p -E space.t <cell_name>
.fE
The parameter file
.B space.p
would look something like:
.fS
low_sheet_res           0 
min_res                 0

BEGIN sub3d 
be_shape                4
be_mode                 0g
max_be_area             inf
be_window               inf 
END sub3d

sub_term_distr_cs       on
elim_sub_con            on
elim_sub_node           on
.fE
.HU "... WITH TRADITIONAL 2DFEM"
A very simple technology file
.B space.s
could look something like:
.fS
conductors:
  # name    : condition : mask : resistivity : type
    cond_mf : cmf       : cmf  : 0           : m    
    cond_cs : cs        : cs   : 2000        : p   

contacts: 
  # name    : condition : lay1 lay2 : resistivity
    cont_b  : cmf cs    : cmf  cs   : 0    
    cont_ca : cs        : cs   @sub : 0   

sublayers: 
  # name       conductivity  top 
    substrate       10       0.0 
.fE
In this case, the cs-layer (representing channel-stop) is the layer where
the 2DFEM operates, and the 0-resistivity contact of the cs-layer to the
substrate makes the combination to the BEM.
The interface mesh on the BEM/FEM interface
is defined by dummy structures (typically some kind of chessboard),
that cause extra tile-divisions in the cs-layer, which will
cause the grid of 2DFEM and BEM node points to be (artificially/manually) refined.
.P
The cs-layer has a sheet-resistivity of 2000 Ohm/sq,
representing a 1000 S/m layer of 0.5 micron thick.
.HU "... WITH MULTIPLE-LAYER 2DFEM"
When using a multiple layer 2DFEM, we have to define a few extra
conductor-layers in the technology file, and connect them through contact
statements as follows:
.fS
unit c_resistance  1e-12 # Ohm um^2

conductors:
  # name    : condition : mask : resistivity : type
    cond_cs1 : cs1      : cs1  : 12000       : p
    cond_cs2 : cs2      : cs2  :  6000       : p
    cond_cs3 : cs3      : cs3  :  6000       : p
    cond_cs4 : cs4      : cs4  : 12000       : p
    
contacts:
  # name    : condition : lay1 lay2 : resistivity
    cont_b  : cs1 cs2   : cs1  cs2  : 166.667 
    cont_c  : cs2 cs3   : cs2  cs3  : 166.667
    cont_d  : cs3 cs4   : cs3  cs4  : 166.667
    cont_e  : cs4       : cs4  @sub : 0

sublayers:
  # name       conductivity  top
    substrate       10       0.0
.fE
The resistance values in the conductor listing are obtained by
dividing the channel-stop layer into multiple uniform layers of equal thickness,
and assigning a sheet-resistivity to each of them.
Then, similar to the standard 2DFEM approach in interconnect resistance,
the sheet-resistivities are divided over the top and bottom planes of the layer.
Therefore, cs2 and cs3 have a parallel connection of 2 sheet
resistivities, which makes them half as large as the sheet resistivities of cs1 and cs4.
The contact resistances can be found in a similar way,
by observing that they form the vertical connections between the layers;
they are directly derived from the thickness of the layers in
the vertical discretization.
The connection from the FEM area to the
underlying substrate (the BEM area), takes place through the 0-resistance
contact from the bottom layer (cs4) to @sub.
.P
Typically, the top-layer (cs1) in the vertical discretization would now be
used to attach vias to, e.g.:
.fS
cont_a : cmf via cs1 : cmf cs1 : <via_resistivity>
.fE
Or it would be used to attach interconnect capacitances to, e.g.:
.fS
acap_cmf_cs : cmf cs1 <other boolean entries> : cmf cs1 : <area_cap>
.fE
The parameter file
.B space.p
will have to be supplemented with a statement 'sub_term_distr_cs4',
which (together with a chessboard of dummy
structures) controls the discretization on the BEM/FEM interface.
.P
In initial experiments with this approach, the maskdata file was
supplemented with additional layer names for the cs1-cs4 layers.
However,
a better approach would probably be that we use 'new' statements in the
space technology file.
At this moment, this approach has not yet been tried, though.
.HU "NEW APPROACH (GENERIC)"
The idea is to automatically induce the multiple-layer 2DFEM (actually 3DFEM)
by including a special statement in the space technology file as follows:
.fS
wafer:
<name>:<conditions>:<conductivity>:<thickness>:<#layers>
.fE
From the conductivity, thickness and #layers, the appropriate conductivity
and contact values (see above) can be calculated for the multiple layer
FEM (this would typically happen in the background).
.P
The number of layers may be optional, but then a default value should
be available.
It is, however, difficult to consistently derive this
default value from the thickness and conductivity (and possibly also
typical via-size) of the FEM-domain.
At the moment, a user-defined number of layers seems the best choice.
.P
The mesh for the combined BEM/FEM method should only be generated
according to the substrate features (e.g. channel-stop layer, or other
relevant features) and substrate contacts (vias, wells, transistors and
shadow-contacts from interconnect).
That is, the mesh should not depend
on features that have little (or nothing) to do with the substrate
(i.e. high interconnect layers, or vias connecting high interconnect layers).
This is a form of a-priori model reduction. The implementation
should be combined with a generic meshing approach (better than the
current setup with the chessboard of dummy structures), but there is
still some discussion possible about how the meshing can be done (see
below under 'discussion topics')
.HU "FUTURE ADDITIONS"
The multiple-layer 2DFEM is only necessary near contact areas, at some
distance away, the traditional 2DFEM can be used.
At this moment, the
transition from multilayer 2DFEM to regular 2DFEM is done manually
by growing a symbolic mask (using 'resize').
However, the symbolic
mask cannot be defined by a 'new' statement in the technology file;
the resize operation only works on physical masks (apparently only
physical masks produce gln-files).
Therefore, we currently copy entries
(typically corresponding to vias) in the appropriate ldm-file to the
symbolic mask (possibly newly defined in the maskdata file), and then
enter the modified cell into the database with 'cldm -f'.
The technology
may then look as follows (with 'rbc' the symbolic resizeable mask derived 
from some via statement, or possibly an interconnect capacitance statement):
.fS
unit c_resistance  1e-12 # Ohm um^2

resize: rbc : rbc : 0.25e-6

conductors:
  # name    : condition : mask : resistivity : type
    cond_cs1 : cs1      : cs1  : 12000       : p
    cond_cs2 : cs2      : cs2  :  6000       : p
    cond_cs3 : cs3      : cs3  :  6000       : p
    cond_cs4 : cs4      : cs4  : 12000       : p
    
contacts:
  # name    : condition     : lay1 lay2 : resistivity
    cont_b  : cs1 cs2  rbc  : cs1  cs2  : 166.667 
    cont_c  : cs2 cs3  rbc  : cs2  cs3  : 166.667
    cont_d  : cs3 cs4  rbc  : cs3  cs4  : 166.667

    cont_e  : cs1 cs2 !rbc  : cs1  cs2  : 0 
    cont_f  : cs2 cs3 !rbc  : cs2  cs3  : 0
    cont_g  : cs3 cs4 !rbc  : cs3  cs4  : 0
    cont_h  : cs4           : cs4  @sub : 0

sublayers:
  # name       conductivity  top
    substrate       10       0.0
.fE
In this way, the vertical discretization will be applied when 'rbc' is present.
When 'rbc' is not present, the vertical resistances will be zero,
such that the horizontal layers are placed in parallel,
which then reduces to the traditional 2DFEM.
In the future this approach might be automated and optimized.
.P
An alternative formulation of this approach is by using the 'connects' 
statement instead of explicitly defining the 0-resistance contacts
between the vertical discretization layers as follows:
.fS
connects:
    conn_cs12 : cs1 cs2 !rbc : cs1 cs2
    conn_ca23 : cs2 cs3 !rbc : cs2 cs3
    conn_ca34 : cs3 cs4 !rbc : cs3 cs4
.fE
Note, however, that a connects statement between cs4 and @sub is
(currently) not (yet) supported; the 'contacts' statement, as shown
earlier, is still necessary.
.P
Using either the contacts formulation or the connects formulation, the 
extraction results will be exactly identical (!).
At this moment, however, 
it's not yet entirely clear whether there are situations in which either of
both methods should be preferred.
.P
Connects statements may also be used for horizontal connections.
If, for example,
the layers cs1-cs4 connect to a doping pattern that needs only a 
single layer (say a layer that conducts 10 times better), we can do:
.fS
connects:
    conn_cs1hcl : -cs1 !cs1 hcl : cs1 hcl
    conn_cs2hcl : -cs2 !cs2 hcl : cs2 hcl
    conn_cs3hcl : -cs3 !cs3 hcl : cs3 hcl
    conn_cs4hcl : -cs4 !cs4 hcl : cs4 hcl
.fE
The formulation could also be the other way around:
.fS
connects:
    conn_hclcs1 : -hcl !hcl cs1 : hcl cs1
    conn_hclcs2 : -hcl !hcl cs2 : hcl cs2 
    conn_hclcs3 : -hcl !hcl cs3 : hcl cs3 
    conn_hclcs4 : -hcl !hcl cs4 : hcl cs4
.fE
At this moment, these two formulations have not (!) yet been tested in 
practice; it is also not yet clear whether either of these formulations 
should be preferred over the other.
.P
According to the initial approach, we divide the FEM domain into multiple
layers with equal thickness.
However, if the FEM domain contains a
difficult doping profile, it may at some point be necessary to divide
the FEM domain into multiple layers with varying thicknesses and varying properties.
We can already do this using the tricks from the 'old' approach,
but this may require significant work from the user.
.HU "DISCUSSION TOPICS"
Currently the meshing on the BEM/FEM interface is artificially induced
by a 'chessboard' of dummy structures.
An automated version of this approach may be as follows.
In combined BEM/FEM extraction,
the BEM mesh can typically be coarse.
Perhaps the BEM mesh (with user-defined granularity)
can be used as the initial mesh on the BEM/FEM interface,
after which the FEM mesh is placed along the BEM mesh for the combined model.
The original concept of dual meshing may be applied here, but at
this moment it is not yet sure whether this is preferable over the way
things are currently implemented in SPACE.
.P
Should the channel-stop be modeled as a layer that is ON TOP of
the substrate, or EMBEDDED IN the substrate.
In this case,
it is important to consider how deep the wells typically are,
and how far transistors actually extend vertically into the substrate.
Furthermore,
it is important to consider how the SPICE (BSIM) MODELS are built up;
they consider the transistor as a whole, but how large is the 'box'
they place around it.
The box will extend some (yet unknown) distance
into the substrate, which may give us indications of how to model the
channel-stop layer appropriately.
.SK
.HU "APPENDIX B -- Pseudo code for handling 'wafer' statement (by E. Schrik)"
.nf
.S 8
.ft C
/* pseudo code for handling 'wafer' statement:
/
/ wafer:
/ name : condition : conductivity : thickness : #layers
/
/ - conductivity in Siemens per meter (S/m)
/ - thickness in microns
*/

layerThickness = thickness / (#layers - 1);

/* First the horizontal sheet-resistances */

baseSheetRes = (1/conductivity) * (1/(layerThickness * 1e-6));

/*
/ We find the Sheet Resistivities (SR) for the cs-layers as follows:
/
/ SR_cs[1]           (top layer)          = 2 * baseSheetRes (see cs1)
/ SR_cs[2]           (intermediate layer) =     baseSheetRes (see cs2)
/       :
/ SR_cs[#layers - 1] (intermediate layer) =     baseSheetRes (see cs3)
/ SR_cs[#layers]     (bottom layer)       = 2 * baseSheetRes (see cs4)
*/

for (it = 1; it <= #layers; ++it) {
    if (it == 1 || it == #layers)
	SR_cs[it] = 2 * baseSheetRes;
    else
	SR_cs[it] = baseSheetRes;
}

/* Now the vertical contact-resistances */

baseContactRes = (1/conductivity) * layerThickness * 1e-6;

/*
/ We find the vertical Contact Resistivities (CR) as follows:
/
/ CR_cs[1]_cs[2]                 = baseContactRes
/ CR_cs[2]_cs[3]                 = baseContactRes
/ CR_cs[3]_cs[4]                 = baseContactRes
/       :
/ CR_cs[#layers - 1]_cs[#layers] = baseContactRes
*/

for (jt = 1; jt < #layers; ++jt) {
    CR_cs[jt]_cs[jt+1] = baseContactRes;
}
.ft
.S
.SK
.HU "APPENDIX C -- Wafer statement example"
.nf
.S 8
.ft C
input specification:
---------------------------------------------------------------
new: !csn !cwn : cs1
new: cca : rbc

resize: rbc : rbc : 0.25e-6

wafer: cs1 : 1000 : 0.5 : 4 : viamask=rbc

conductors:
  # name  : condition : mask : resistivity : type
  cond_mf : cmf       : cmf  : 0.045       : m    # first metal

contacts:
  # name  : condition    : lay1 lay2 : resistivity
  cont_mf : cmf w1_1 cca : cmf  w1_1 : 0

sublayers:
  # name    conductivity  top
  substrate      10       0
---------------------------------------------------------------

output result:
---------------------------------------------------------------
new: !csn !cwn : cs1
new: cca : rbc

resize: rbc : rbc : 0.25e-6

new: cs1 : w1_1
new: cs1 : w1_2
new: cs1 : w1_3
new: cs1 : w1_4

conductors:
  # name   : condition : mask : resistivity : type
  cnd$w1_1 : cs1       : w1_1 : 12000       : p
  cnd$w1_2 : cs1       : w1_2 :  6000       : p
  cnd$w1_3 : cs1       : w1_3 :  6000       : p
  cnd$w1_4 : cs1       : w1_4 : 12000       : p
  cond_mf  : cmf       : cmf  :     0.045   : m

contacts:
  # name   : condition   : lay1 lay2 : resistivity
  cnt$w1_1 : cs1         : w1_4 @sub : 0
  cnt$w1_2 : cs1  rbc    : w1_1 w1_2 : 1.666667e-10
  cnt$w1_3 : cs1  rbc    : w1_2 w1_3 : 1.666667e-10
  cnt$w1_4 : cs1  rbc    : w1_3 w1_4 : 1.666667e-10
  cnt$w1_5 : cs1 !rbc    : w1_1 w1_2 : 0
  cnt$w1_6 : cs1 !rbc    : w1_2 w1_3 : 0
  cnt$w1_7 : cs1 !rbc    : w1_3 w1_4 : 0
  cont_mf  : cmf cs1 cca : cmf  w1_1 : 0

sublayers:
  # name    conductivity  top
  substrate      10       0
---------------------------------------------------------------
.ft
.S
.SK
.P
.F+
.PSPIC "an0412/fig3.ps" 4i
.F-
.P
.SK
.HU "APPENDIX D -- P/Nwell substrate side-view"
.P
.F+
.PSPIC "an0412/fig4.ps" 4i
.F-
.P
.HU "APPENDIX E -- Transistor substrate side-view"
.P
.F+
.PSPIC "an0412/fig5.ps" 4i
.F-
.P
