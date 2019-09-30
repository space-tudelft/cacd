.T= "Eelco Test Cell Substrate Examples"
.DS 2
.rs
.sp 1i
.B
.S 15 20
EELCO TEST CELL
SUBSTRATE
EXAMPLES
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
Report EWI-ENS 05-06x
.ce
November 21, 2005
.sp |9i
.DS
.in +5
.S 9
Copyright \s+3\(co\s-3 2005-2006 by the author.

Last revision: April 27, 2006.
.S
.in -5
.DE
.SK
.S=
.H 1 "Eelco TEST CELL"
The test cell from Eelco Schrik is saved in directory:
.fS I
~simon/eelco/substratestorm_vs_space
.fE
.nf
See e-mail from Eelco Schrik (d.d. Thu, 22 Sep 2005 16:06).
It contains the tar-file of the project directory and some info.
The layout example (test cell) uses a lambda of 0.03125 um.
With that value is the transistor gate-length exact 250 nm.
.fi
For that gate-length are usable transistor-models available (see
file "spice3f3_new.lib" in subdirectory "proc").
.P
The README file (in the project) contains some extra info, how the standard SubstrateStorm doping-profile
can be adapted, thus that it can be compared with the SPACE substrate calculations (see appendix A).
.br
See: /opt/cad/cadence/SNA3.1/doc/substormtcttut/substormtcttut.pdf
.P
The cell "oscil_nwellguard" is a ring oscillator, which contains 9 invertor sub-cells ("inv").
See the figure below:
.F+
.PSPIC "an0506/fig1.ps" 5.5i
.F-
The cell has 6 terminals. Two power connections ("vss" and "vdd").
The "vss" interconnect is a closed metal1 ring and has a substrate contact.
The "vdd" interconnect has n-well contacts (in each "inv" one).
The "sens" terminal is also a substrate contact.
The n-well guard-ring has a contact with terminal "vdd2".
The total layout is using only 7 different masks (no metal2 is used):
.TS
allbox;
l l.
cmf	metal first
cpg	polysilicon
caa	active area
cwn	n-well region
csn	n-channel implant
cca	contact cmf - caa/cwn/@sub
ccp	contact cmf - cpg
.TE
The "inv" sub-cell contains 4 terminals ("vdd", "vss", "in" and "out").
The "caa" mask is only used in "inv" (2x) for the 2 transistors.
The "ccp" mask is also only used in the sub-cell (1x), this,
to connect the output (cmf) to the input gates (cpg) of a next invertor.
Mask "csn" is also used to define a n-well contact.
However, this rectangle is a little too short defined on the right side.
This gives extra mesh lines.
See also the
.P= space
technology file in appendix B.
The following figure shows a slice for the invertor.
.F+
.PSPIC "an0506/fig2.ps" 5.5i
.F-
.H 2 "Layout of inv"
.F+
.PSPIC "an0506/inv.ps" 5.5i
.F-
.SK
.H 2 "Layout of oscil_nwellguard"
.F+
.PSPIC "an0506/osc.ps" 5.5i
.F-
Note that the dimensions of the guard-ring are not symmetrical (see figure below).
.F+
.PSPIC "an0506/fig3.ps" 3.0i
.F-
This can result in extra tile splitting in the
.P= space
program by a be_window of 5 micron (see figure below).
The white dots give the window positions.
.F+
.PSPIC "an0506/ext1.ps" 5.5i
.F-
You get three separate "pink" substrate area's when all substrate terminals are merged
(parameter sep_sub_term=off) by the
.P= space
extraction.
.SK
Thus the substrate is modeled with three substrate terminals.
These nodes are connected with each other by substrate resistors and they are
connected to the SUBSTR node.
See the listing of the resulting SLS-network below.
Two substrate terminals have a contact with the cell interconnect and
are connected to terminals "vss" and "sens".
The guard-ring n-well area (node 8) has only a capacitive connection (26.5 fF) to terminal "vdd2".
The capacitor is not included, because capacitors where not extracted.
Note that the bulk connections of the nenh and penh transistors are not included
(there are 9 of each).
.fS
network oscil_nwellguard (terminal out, in, vdd, vss, sens, vdd2)
{
    nenh w=1.375u l=250n (in, out, vss);
    penh w=1.625u l=250n (in, out, vdd);
    penh w=1.625u l=250n (1, in, vdd);
    nenh w=1.375u l=250n (1, in, vss);
    nenh w=1.375u l=250n (out, 2, vss);
    penh w=1.625u l=250n (out, 2, vdd);
    ...
    ...
    res 27.11588k (sens, 8);
    res 676.0169k (sens, vss);
    res 65.0086k (sens, SUBSTR);
    res 80.39587k (8, vss);
    res 9.031278k (8, SUBSTR);
    res 3.214768k (vss, SUBSTR);
}
.fE
The example shows, that it is not a good idea to merge the substrate area's.
Because different types (p/n) of substrate area's are merged together.
.P
With parameter "sep_sub_term=on" you get a better result (see figure below).
.F+
.PSPIC "an0506/ext2.ps" 5.5i
.F-
This result has 118 substrate tiles, but adjacent tiles with same color (same causing conductor)
are merged.
What results in 59 different substrate terminals or nodes.
The big pink area is the n-well area of the transistors, this is one substrate terminal.
The small pink area's are the transistor gates, which have no causing conductor.
Note that only the tiles, which has some connection with the substrate (resistive, capacitive or bulk)
are taken into account.
The other (black) tiles are not taken into account.
.P
The
.P= space
extractor has also a 3rd possibility to get more substrate terminals.
When parameter sub_term_distr_\fImask\fP is specified for a substrate causing conductor mask,
then this mask is not more merged.
This works only in combination with interconnect res extraction and the causing conductor must not have
a low sheet res value.
The table below shows howmany extra substrate terminals we get for each mask:
.TS
allbox;
l r.
NO	59
cmf	+33
cpg	+59
caa	+108
cwn	+283
ALL	+483
.TE
To let it work for the n-well mask, you must give the cwn conductor a sheet res value > 0.
.br
The figure below shows the substrate tile splitting and terminals for all distributed.
.F+
.PSPIC "an0506/ext3.ps" 5.5i
.F-
The
.P= space
extractor has also a 4th possibility to do the substrate res extraction.
With the combined BEM/FEM method, the total substrate area is taken into account.
Because this method sets all masks distributed, you get even more substrate terminals,
because all black area's are also taken into account.
To use this method, you must define wafer configurations and bem_depth in the technology file.
You must also decide, how big the substrate bounding box must be.
Note that
.P= space
is default using a substrate extension of be_window width.
Note that
.P= space
is also doing a vertical tile splitting (using the be_window width).
.P
For the above test cell, the BEM/FEM method is using 711 substrate tiles/terminals.
.br
See the figure below.
.F+
.PSPIC "an0506/ext4.ps" 5.5i
.F-
The used wafer configurations for the BEM/FEM technology file are also listed in appendix B.
Note that the conductivity values and the number of layers is hypothetical chosen.
Because a ``pdiff'' layer is missing for the process the test cell is using,
i must use a different definition method for the contacts.
Thus, the pcontact cross-section is specified with the dimension of the via mask "cca".
This is not the case for the ncontact.
The ncontact cross-section is specified with the dimension of the "csn" mask.
See figure below.
.F+
.PSPIC "an0506/fig4.ps" 5i
.F-
.P
Important is to note, that the "cwn" and "caa" conductors are taken over by the FEM
wafer cross-section.
However, this is not done for the transistor gates (conductor "cpg" must be used).
Therefor, conductor "cpg" may not be specified as 'p' or 'n' type conductor.
.P
Note that these wafer configurations can be compared with the eight doping profiles
cross-sections for a CMOS process which SubstrateStorm is using.
.H 2 "Some Space Test Results of oscil_nwellguard"
makesubres result:  (executed on host "europa", DELL Precision 450, Red Hat Linux)
.TS
allbox;
l r r r r.
tiles	subtiles	subterms	mem(Mb)	utime(s)
730	711	711	5.04	7.3
.TE
.P
space3d -%2Br results for !elim_sub_term_node:
.TS
allbox;
l r r r r r r r.
tf#	cap_nr	res_nr	nodes	subnodes	subterms	mem(Mb)	utime(s)
1	0	52883	383	324	711	13.2	26.8
2/4	0	52883	383	324	711	15.8	34.8
5	0	443	79	20	711	23.6	79.3
.TE
space3d -%2Br results for elim_sub_term_node:
.TS
allbox;
l r r r r r r r.
tf#	cap_nr	res_nr	nodes	subnodes	subterms	mem(Mb)	utime(s)
1	0	443	79	20	711	14.6	38.9
2/4	0	443	79	20	711	14.9	42.3
5	0	443	79	20	711	23.6	76.8
.TE
.P
space3d -%2BrC results (between braces cap_assign_type=2) for !elim_sub_term_node:
.TS
allbox;
l r r r r r r r.
tf#	cap_nr	res_nr	nodes	subnodes	subterms	mem(Mb)	utime(s)
1	72147	52883	383	324	711	16.2(16.2)	32.5(32.5)
2	72144	52883	383	324	711	17.8(17.8)	36.5(36.5)
3	72455	52883	383	324	711	18.5(18.5)	43.9(47.2)
4	72455	52883	383	324	711	18.5(19.2)	43.6(49.8)
5	2383	443	79	20	711	32.6(30.2)	123.7(108.3)
.TE
.P
space3d -%2BrC results (between braces cap_assign_type=2) for elim_sub_term_node:
.TS
allbox;
l r r r r r r r.
tf#	cap_nr	res_nr	nodes	subnodes	subterms	mem(Mb)	utime(s)
1	2383	443	79	20	711	15.7(15.7)	46.4(46.4)
2	2380	443	79	20	711	17.0(17.0)	59.5(59.5)
3	2383	443	79	20	711	17.3(17.9)	59.0(59.2)
4	2383	443	79	20	711	17.3(18.6)	59.5(62.3)
5	2383	443	79	20	711	32.6(30.2)	119.3(109.3)
.TE
See appendix C for the used space technology files.
.SK
When we look to the numbers, we see that there are 711 subterms (substrate terminals / tiles).
This number is the sum of all not nwell tiles (pdefault, pcontact, nsd, nchannel) and
all nwell tiles (ndefault, ncontact, psd, pchannel).
.TS
box;
c s | r s
l r | l r.
not nwell	nwell (guard ring)
=
pdefault	252	ndefault	121 (14)
pcontact	6	ncontact	48   (3)
nsd	136	psd	126   (0)
nchannel	13	pchannel	9   (0)
_
total	407	total	304 (17)
.TE
When you look more accurate to the above numbers, you see that the nchannel and pchannel numbers are almost exact
the number of gates of the nenh/penh transistors (18).
There are 4 nchannel tiles more, because the top 4 nenh transistors are subdivided by the horizontal be-window line.
However, the source/drain area's (nsd / psd) are subdivided a lot more than needed.
That is, because contacts to first metal are laying on these area's.
Maybe, we can optimize this in the future.
.P
When all substrate nodes (subnodes) are eliminated (elim_sub_term_node=on), then there are 20 subnodes left.
This is of the 9 nenh transistors (9 source pins and 9 bulk pins) and 2 terminals (vss and sens) in the non nwell area.
The total number of nodes (79) is the sum of the total number of transistor pins (18 * 4) and the number of terminals (6)
and the SUBSTR node.
.P
When parameter elim_sub_term_node=off, then the non nwell area is default eliminated (because it is distributed and
is connected to the BEM substrate area).
Only the nwell substrate terminals (304) are not eliminated.
Thus, we get 324 subnodes (20 + 304) and 383 nodes (79 + 304).
.P
When we look to the utime (user time) numbers, we see that substrate node elimination cost a lot of time (about 40% more).
This is not the case for technology file number 5, because by that example there is p-fem under the nwell area.
Thus, all substrate terminals are eliminated by default.
Note that the user time numbers are not so accurate, because these numbers are also depending of other system activities.
But they are more accurate than the real time used (which is depending of the system load).
.H 1 "APPENDICES"
.SP
.nr Hu 2
.HU "APPENDIX A -- Eelco README File"
.nf
Cell "oscil_nwellguard" is the basic ring oscillator cell used in the thesis
by Eelco Schrik. The used technology in the thesis is a simple modification
of a SubstrateStorm example doping profile.

The SubstrateStorm example doping profiles can be found in the examples
directory provided with the SubstrateStorm installation:
.fS I
$SUBSTRATESTORMHOME/etc/examples/tct/cmos/lightlyDoped/profiles/
.fE
The files in this directory contain samples of the doping profile in the
following way:
.fS I
<depth>      <n-type concentration>    <p-type concentration>
                (Electrons)               (Holes)
.fE
Concentrations are in cm^-3. Note that 'depth' does not mean 'thickness' and
that the first depth sample may not be at depth zero, because the oxide may
also be taken into account, which typically contains no charge-carriers, but
does have a certain thickness itself. For example, the first 3 lines
from 'pdefault.txt' are as follows:
.fS I
0.70420003         4727.1001               4.4227001e+16
0.72359997         3730.5                  5.6043001e+16
0.74910003         2911.5                  7.1808001e+16
.fE
The modifications used in the thesis by Eelco Schrik are straightforward:
.DL
.LI
all p-type carrier-concentrations in the SubstrateStorm example profiles
have to be multiplied by 2
.LI
all n-type carrier-concentrations in the SubstrateStorm have to be
multiplied by 0.5.
.LE
.P
NOTE: Nd * Na = Ni * Ni   (Ni is constant?)
.P
NOTE: SubstrateStorm uses a FDM method. Each mesh 3D-box has a center node,
which is connected with 6 resistors. Each going to a center node of a face
of the box. Pairs have an equal value. Half of the resistors is eliminated
(or removed). That is maybe the reason of the factor 2.
.SK
.HU "APPENDIX B -- Space Technology File"
.nf
.S 10
.ft C
unit resistance    1     # ohm
unit c_resistance  1e-12 # ohm um^2
unit a_capacitance 1e-6  # aF/um^2
unit e_capacitance 1e-12 # aF/um

###begin: BEM/FEM additions #############

new : !cwn !caa !csn !cca : pdefault
new : !cwn !caa !csn  cca : pcontact
new : !cwn  caa  csn !cpg : nsd
new : !cwn  caa  csn  cpg : nchannel

new :  cwn !caa !csn      : ndefault
new :  cwn !caa  csn      : ncontact
new :  cwn  caa !csn !cpg : psd
new :  cwn  caa !csn  cpg : pchannel

set bem_depth 6

wafer : pdefault :  100 6 1
wafer : pcontact :  100 6 1
wafer : nsd      :  100 1 1 : restype=n
wafer : nsd      :  100 5 1
wafer : nchannel :  100 6 1

wafer : ndefault : 1000 6 1 : restype=n subconn=off
wafer : ncontact : 1000 6 1 : restype=n subconn=off
wafer : psd      : 1000 1 1
wafer : psd      : 1000 5 1 : restype=n subconn=off
wafer : pchannel : 1000 6 1 : restype=n subconn=off

###end: BEM/FEM additions #############

conductors :
  # name    : condition     : mask : resistivity : type
    cond_mf : cmf           : cmf  : 0.045       : m  # first metal
    cond_pg : cpg           : cpg  : 40          : m  # poly interc.
    cond_pa : caa !cpg !csn : caa  : 70          : p  # p+ active area
    cond_na : caa !cpg  csn : caa  : 50          : n  # n+ active area
    cond_well : cwn         : cwn  : 0           : n  # n-well

fets :
  # name : condition    : gate d/s : bulk
    nenh : cpg caa !cwn : cpg  caa : @sub  # nenh MOS
    penh : cpg caa  cwn : cpg  caa : cwn   # penh MOS

contacts :
  # name   : condition         : lay1 lay2 : resistivity
    cont_p : ccp cmf cpg       : cmf  cpg  :  0   # metal to poly
.SK
    cont_a : cca cmf caa !cpg  cwn
           | cca cmf caa !cpg !cwn
                               : cmf  caa  :  0   # metal to act. area
    cont_w : cca cmf  cwn  csn : cmf  cwn  :  0   # metal to n-well
    cont_b : cca cmf !cwn !csn : cmf  @sub :  0   # metal to substrate

junction capacitances nwell :
    acap_cw :  cwn             :  cwn @sub : 100
    ecap_cw : !cwn -cwn        : -cwn @sub : 540

junction capacitances ndif :
    acap_na :  caa       !cpg  csn  !cwn :  caa @sub : 100
    ecap_na : !caa -caa !-cpg -csn !-cwn : -caa @sub : 300

junction capacitances pdif :
    acap_pa :  caa       !cpg  !csn cwn      :  caa cwn : 500
    ecap_pa : !caa -caa !-cpg !-csn cwn -cwn : -caa cwn : 600

capacitances :
  # polysilicon capacitances
    acap_cpg_sub :  cpg               !caa !cwn :  cpg @sub : 57.5
    acap_cpg_cwn :  cpg               !caa  cwn :  cpg cwn  : 57.5
    ecap_cpg_sub : !cpg -cpg          !caa !cwn : -cpg @sub : 52.5
    ecap_cpg_cwn : !cpg -cpg          !caa  cwn : -cpg cwn  : 52.5

  # first metal capacitances
    acap_cmf_sub :  cmf     !cca !cpg !caa !cwn :  cmf @sub : 20.3
    acap_cmf_cwn :  cmf     !cca !cpg !caa  cwn :  cmf cwn  : 20.3
    ecap_cmf_sub : !cmf -cmf     !cpg !caa !cwn : -cmf @sub : 40.9
    ecap_cmf_cwn : !cmf -cmf     !cpg !caa  cwn : -cmf cwn  : 40.9

    acap_cmf_caa :  cmf     !cca !cpg  caa      :  cmf caa  : 20.3
    ecap_cmf_caa : !cmf -cmf caa !cpg           : -cmf caa  : 40.9

    acap_cmf_cpg :  cmf     !ccp  cpg           :  cmf cpg  : 57.5
    ecap_cmf_cpg : !cmf -cmf      cpg           : -cmf cpg  : 52.5

sublayers :
  # name       conductivity  top
  # substrate  10.0          0.0 # !bemfem
    substrate  10.0         -6.0 #  bemfem
.ft
.S
.SK
.HU "APPENDIX C -- Used Space Technology Files"
.nf
.S 10
.ft C
tf# 1
-------------------------------------
Wafer configuration of appendix B.
With only area caps.
-------------------------------------

tf# 2
-------------------------------------
Wafer configuration of appendix B with 2 layers.
With only area caps.
-------------------------------------

tf# 3
-------------------------------------
Wafer configuration of appendix B with 2 layers.
With also edge caps (7), not ecap_na/ecap_pa.
-------------------------------------

tf# 4
-------------------------------------
Wafer configuration of appendix B with 2 layers.
With all edge caps (9).
-------------------------------------

tf# 5
-------------------------------------
Wafer configuration with extra p-fem below n-well.
This gives distributed fem/bem contacts also below
the n-well area. Note that distributed substrate
contacts are always eliminated.
With all edge caps (9).
-------------------------------------
wafer : pdefault :  100 6 2
wafer : pcontact :  100 6 2
wafer : nsd      :  100 1 2 : restype=n
wafer : nsd      :  100 5 2
wafer : nchannel :  100 6 2

wafer : ndefault : 1000 5 2 : restype=n
wafer : ndefault : 1000 1 2
wafer : ncontact : 1000 5 2 : restype=n
wafer : ncontact : 1000 1 2
wafer : psd      : 1000 1 2
wafer : psd      : 1000 4 2 : restype=n
wafer : psd      : 1000 1 2
wafer : pchannel : 1000 5 2 : restype=n
wafer : pchannel : 1000 1 2
.ft
.S
