.aH "Summary of Command-Line Options"
.sY %AX_OptionOverview% \n(H1
The program
.P= space 
(or
.P= space3d )
has the following options:
.M(
.ME -c -cap
Extract capacitances to substrate.
.ME -C -coupcap
Extract coupling capacitances as well as capacitances to substrate.
This option implies
.O= -c -cap "" .
.ME -l -latcap
Also extract lateral coupling capacitances,
implies
.O= -C -coupcap "" .
.ME -3 ""
Use a boundary-element technique for 3-dimensional capacitance extraction.
Use with
.O= -c -cap 
or
.O= -C -coupcap "" .
.ME -r -res
Extract resistances for high-resistivity (non-metal) interconnect.
.ME -R -allres
Extract all resistances, also low-resistivity metal interconnect.
To use this option, it must be specified in the first option argument list.
And this option argument must start with \fB-%\fP.
.ME -z -resmesh
Apply mesh refinement for resistance extraction,
implies
.O= -r -res "" .
.ME -G -usefrequency
Extract RC models that are accurate up to a certain frequency.
.ME -b -simplesub 
Use a simple but fast method to compute substrate resistances.
.ME -B -3dsub 
Use a boundary-element technique to compute substrate resistances.
.ME -F -flat
Set flat extraction mode,
i.e. produce a flattened netlist.
.ME -T -top
In hierarchical mode, only extract the top cell(s).
.ME -I -noincremental
Unset incremental (hierarchical) mode:
do not skip sub-cells for which
the circuit is up-to-date.
Cannot be used with the
.O= -F -flat
option.
.ME -D -mindepth depth
Selectively unset incremental (hierarchical) mode for all cells
.br
at level <= \fIdepth\fP
(default \fIdepth\fP = 1).
.ME -u ""
Do not automatically run the preprocessors
.I makeboxl(1ICD)
and
.I makegln(1ICD).
.ME -n -noreduc
Do not apply the circuit reduction heuristics.
.ME -t -torpos
Add positions of devices and sub-cells to the extracted circuit.
.ME -v -verbose
Print run-time information (verbose mode).
.ME -h ""
Print help information.
.ME -i ""
Print statistics, implies
.O= -v -verbose "" .
.ME -k -select
Selective resistance extraction, resistances are
only extracted for specified interconnects.
.ME -j -unselect
Selective resistance extraction, resistances are
extracted for all but specified interconnects.
.ME -x -backannotation
Generate layout back-annotation information, implies
.O= -t -torpos "" .
.ME -a "" sec
Make space report its progression every \fIsec\fP seconds.
Space also reports its progression when it receives an ALARM
signal,
such a signal can be send by the command ``kill -ALRM \fIpid\fP'',
where \fIpid\fP is the process id.
.ME -e -elem xxx
Use the file space.\fIxxx\fP.t
in the ICD process library
as the element definition file.
.ME -E -elemfile file
Use \fIfile\fP as the element definition file.
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
.aH "Hierarchy and Terminals"
.sY %AX_Terminals% \n(H1
With hierarchical extraction,
the extracted circuit is most accurate when the following guidelines
are followed.
.I(
.I=
The area occupied by a terminal should be minimal.
This is to improve the accuracy of hierarchical resistance extraction.
It is recommended to use the minimal width design rule to construct a
terminal box (useless error reports from a 
design rule checker are prevented this way).
.I=
Do not overlap terminals (in the same interconnect layer) of two instances,
but instead have them abut each other.
Also, when a cell contains an instance of a child-cell:
do not let
the interconnect of the top-cell overlap the
terminals of the child-cell, but make them abut.
This is to improve the accuracy of hierarchical capacitance extraction.
.I=
When possible, do not make layout patterns at a higher hierarchical level 
in the area occupied by instantiated child-cells,
even if 
this does not change the functional behavior of the child-cell.
Whenever possible,
global wiring above instances should be contained in these instances.
This is
to more accurately
determine the parasitic coupling capacitance.
.I=
If the above rules do not apply to one or more sub-cells of a
particular design,
selectively set the macro status
for each of these cells 
in order to enforce a flat extraction extraction of these
sub-cells, even in case of hierarchical extraction
(see Section %SE_MIXHF%).
This may especially be useful for sea-of-gates circuits.
.I)
.aH "Solving Problems"
.sY %AX_PROBLEMS% \n(H1
.I(
.I= "Design rule correctness of the circuit"
Often,
the element definition file of
.P= space
will be created under the assumption that the layout to be extracted is free of
design rule errors.
For example,
the rules for transistor recognition in the
.P= scmos_n
example technology do not require the presence or absence of the
n-well mask since this is already enforced\(emunder the assumption
of design-rule correctness\(emby the presence or absence
of the n-channel implant mask.
So, in the case of unexpected extraction results,
make sure that the layout is free of design rule errors.
.I= "Size of compiled element definition file"
The element definition file of
.P= space
is compiled by the program
.P= tecc
into a form that is read in by
.P= space .
The size of this compiled file is mostly dependent on
the number of masks defined for the technology.
For the
.P= scmos_n
example technology, the size of this file is approximately 7 Kbyte.
In some occasions (e.g. for other technologies),
the size of this file may become too large.
Then, the size of the file can be limited by lowering the value of the 
.I maxkeys
parameter in the element definition file
(see Section %SE_KEYLIST%).
However,
extraction will run somewhat slower then.
.I= "New version of compiled element definition file"
The element definition file is reorganized.
It contains two key lists and these key lists are encoded.
Thus, the size of this compiled file is much smaller.
As a consequence,
.P= space
does not more read older file versions.
.I= "Negative resistances and/or capacitances"
When negative resistances and/or capacitances
occur in the extraction output, read the note about mesh generation
in Section %SE_RESISTEXT%.
.I= "Long extraction times with resistance extraction"
Sometimes, when resistances are extracted, arrays of small contacts 
will drastically increase the extraction time.
In that case, merge the contact arrays into large contacts, using the
resize statement (see Section %SE_RESIZEMASK%).
Further, long extraction times may occur (1) when well resistances are
extracted - especially in combination with the option
.O= -z -resmesh 
- (2) when metal resistances are extracted or (3) when coupling
capacitances are extracted.
Improvements may then be obtained by 
the use of the parameter
.I max_delayed
(see Section %SE_ELIM_ORDER%, for cause (1), (2) and (3)),
the parameter
.I equi_line_ratio
(see Section %SE_EQUI_LINE%, for cause (2))
and the parameter
.I frag_coup_cap
(see Section %SE_FRAG_COUP%, for cause (3)).
.I)
.aH "Element Definition File for CMOS Example Process"
.sY %AX_ElemFile% \n(H1
.L{
.nf
.S 9 11
#
# space element definition file for scmos_n example process
#
# masks:
# cpg - polysilicon interconnect        ccp - contact metal to poly
# caa - active area                     cva - contact metal to metal2
# cmf - metal interconnect              cwn - n-well
# cms - metal2 interconnect             csn - n-channel implant
# cca - contact metal to diffusion      cog - contact to bondpads
#
# See also: maskdata

unit resistance    1     # ohm
unit c_resistance  1e-12 # ohm um^2
unit a_capacitance 1e-6  # aF/um^2
unit e_capacitance 1e-12 # aF/um
unit capacitance   1e-15 # fF

maxkeys 13

colors :
    cpg   red
    caa   green
    cmf   blue
    cms   gold
    cca   black
    ccp   black
    cva   black
    cwn   glass
    csn   glass
    cog   glass
    @sub  pink

conductors :
  # name    : condition     : mask : resistivity : type
    cond_mf : cmf           : cmf  : 0.045       : m    # first metal
    cond_ms : cms           : cms  : 0.030       : m    # second metal
    cond_pg : cpg           : cpg  : 40          : m    # poly interconnect
    cond_pa : caa !cpg !csn : caa  : 70          : p    # p+ active area
    cond_na : caa !cpg  csn : caa  : 50          : n    # n+ active area

fets :
  # name : condition    : gate d/s
    nenh : cpg caa  csn : cpg  caa     # nenh MOS
    penh : cpg caa !csn : cpg  caa     # penh MOS

contacts :
  # name   : condition        : lay1 lay2 : resistivity
    cont_s : cva cmf cms      : cmf  cms  :   1    # metal to metal2
    cont_p : ccp cmf cpg      : cmf  cpg  : 100    # metal to poly
    cont_a : cca cmf caa !cpg : cmf  caa  : 100    # metal to active area

.ne 6
junction capacitances ndif :
  # active area capacitances
  # name    :  condition                 : mask1 mask2 : capacitivity
    acap_na :  caa       !cpg  csn !cwn  :  caa @gnd : 100  # n+ bottom
    ecap_na : !caa -caa !-cpg -csn !-cwn : -caa @gnd : 300  # n+ sidewall

junction capacitances pdif :
    acap_pa :  caa       !cpg  !csn cwn      :  caa @gnd : 500 # p+ bottom
    ecap_pa : !caa -caa !-cpg !-csn cwn -cwn : -caa @gnd : 600 # p+ sidewall

capacitances :
  # polysilicon capacitances
    acap_cpg_sub :  cpg                !caa :  cpg @gnd : 49 # bottom to sub
    ecap_cpg_sub : !cpg -cpg !cmf !cms !caa : -cpg @gnd : 52 # edge to sub

  # first metal capacitances
    acap_cmf_sub :  cmf           !cpg !caa :  cmf @gnd : 25
    ecap_cmf_sub : !cmf -cmf !cms !cpg !caa : -cmf @gnd : 52

    acap_cmf_caa :  cmf      caa !cpg !cca !cca :  cmf caa : 49
    ecap_cmf_caa : !cmf -cmf caa !cms !cpg      : -cmf caa : 59

    acap_cmf_cpg :  cmf      cpg !ccp :  cmf  cpg : 49
    ecap_cmf_cpg : !cmf -cmf cpg !cms : -cmf  cpg : 59

  # second metal capacitances
    acap_cms_sub :  cms      !cmf !cpg !caa :  cms @gnd : 16
    ecap_cms_sub : !cms -cms !cmf !cpg !caa : -cms @gnd : 51

    acap_cms_caa :  cms      caa !cmf !cpg :  cms caa : 25
    ecap_cms_caa : !cms -cms caa !cmf !cpg : -cms caa : 54

    acap_cms_cpg :  cms      cpg !cmf :  cms cpg : 25
    ecap_cms_cpg : !cms -cms cpg !cmf : -cms cpg : 54

    acap_cms_cmf :  cms      cmf !cva :  cms cmf : 49
    ecap_cms_cmf : !cms -cms cmf      : -cms cmf : 61

    lcap_cms     : !cms -cms =cms    : -cms =cms : 0.07

#EOF
.L}
.S=
.aH "Parameter File for CMOS Example Process"
.sY %AX_ParamFile% \n(H1
.fS
#
# space parameter file for scmos_n example process
#
min_art_degree    3    
min_degree        4
min_res         100      # ohm
max_par_res      20  
no_neg_res       on
min_coup_cap      0.05
lat_cap_window    6.0    # micron
max_obtuse      110.0    # degrees
equi_line_ratio   1.0
.fE
.aH "Element Definition File for Bipolar Example Process"
.sY %AX_BipElemFile% \n(H1
.L{
.nf
.S 9 11
#
# space element-definition file for DIMES-01
#
# masks:
# bi - intrinsic base bi-npn (optional) dp - deep p-well
# bn - buried n-layer                   ic - interconnect
# bs - intrinsic base bs-npn (optional) in - second interconnect
# bw - intrinsic base bw-npn            sn - emitter bs-npn (optional)
# ci - channel p-jfet (optional)        sp - extrinsic base bs-npn (optional)
# co - contact window                   wn - shallow n-layer
# ct - second contact window            wp - extrinsic base bw/bi-npn
# dn - deep n-well
#
# See also: Design manual DIMES-01 process

unit c_resistance  1e-12
unit a_capacitance 1e-03
unit e_capacitance 1e-09

maxkeys 10

new : !dp : epi

colors :
    bn glass
    dp green
    dn magenta
    sp red
    sn red
    bs lightBlue
    bi brown
    ci brown
    cw glass
    wp red
    bw lightBlue
    wn red
    co white
    ic blue
    ct white
    in red

conductors :
	
  condIC   : ic          : ic  : 0.044 : m
  condIN   : in          : in  : 0.019 : m
  condEPI  : epi !wp !sp : epi : 0     : n
  condEPI1 : epi  wp !sp : epi : 0     : n   # Epi under WP
  condEPI2 : epi !wp  sp : epi : 0     : n   # Epi under SP
  condBN   : bn          : bn  : 20    : n
  condDP   : dp          : dp  : 8     : p   # deep P-well
  condDN   : dn          : dn  : 4     : n   # deep N-well
  condSP   : sp          : sp  : 25    : p
  condSN   : sn          : sn  : 29    : n
  condBS   : bs !sn      : bs  : 1200  : p
  condBS1  : bs  sn      : bs  : 6000  : p
  condBI   : bi !wn      : bi  : 1400  : p
  condBI1  : bi  wn      : bi  : 6000  : p
  condCI   : ci !wn      : ci  : 6000  : p
  condCI1  : ci  wn      : ci  : 30    : p
  condWP   : wp          : wp  : 25    : p
  condBW   : bw !wn      : bw  : 600   : p
  condBW1  : bw  wn      : bw  : 7000  : p
  condWN   : wn          : wn  : 40    : n   # shallow N-layer

fets :

  jfet : wn ci bn : wn ci

bjts :

  npnBS : bn sn bs epi               : ver : sn bs epi
  npnBW : bn wn bw epi               : ver : wn bw epi
  npnBI : bn wn bi epi               : ver : wn bi epi
  pnpWP : bn !wp -wp !bw !bi !ci epi : lat : -wp epi =wp
  pnpSP : bn !sp -sp !bs epi         : lat : -sp epi =sp

.ne 3
connects :

  connBS  : sp bs          : sp bs
  connDN  : dn epi         : dn epi
  connWN  : wn dn          : wn dn
  connBI  : bi wp          : bi wp
  connCI  : ci wp          : ci wp
  connBW  : bw wp          : bw wp
  connDP  : dp wp          : dp wp
  connSN  : sn epi !bs !dn : sn epi
  connSN1 : sn dn          : sn dn
  connBN  : bn epi         : bn epi

contacts :

  contIN : ic ct in  : ic  in :   0
  contSN : ic co sn  : ic  sn : 120
  contSP : ic co sp  : ic  sp :  16
  contWN : ic !co wn : ic  wn :  80
  contWP : ic co wp  : ic  wp : 160
  contDN : ic co dn  : ic  dn :  80

capacitances :

  capBS  : ic  bs !sn             : ic  bs  : 0.109
  capEPI : ic !bn !dp             : ic  epi : 0.123
  capSP  : ic  sp !co !bs         : ic  sp  : 0.119
  capSN  : ic  sn !co             : ic  sn  : 0.077
  capBI  : ic  bi !wn             : ic  bi  : 0.110
  capCI  : ic  ci !wn             : ic  ci  : 0.051
  capWP  : ic  wp !co !bi !bw !ci : ic  wp  : 0.124
  capBW  : ic  bw !wn             : ic  bw  : 0.122
  capDP  : ic  dp !wp !bw         : ic  dp  : 0.123
  capDN  : ic  dn !wn !sn !co     : ic  dn  : 0.081

#EOF
.L}
.S=
.aH "Parameter File for Bipolar Example Process"
.sY %AX_BipParamFile% \n(H1
.fS
#
# space parameter file for DIMES-01 process
#
min_art_degree    3    
min_degree        4
min_res          10      # ohm
max_par_res      20  
no_neg_res       on
min_coup_cap      0.05
lat_cap_window    6.0    # micron
lat_base_width    3.0    # micron
max_obtuse      110.0    # degrees
equi_line_ratio   1.0
.fE
.aH "Control File for Bipolar Example Process"
.sY %AX_BipControlFile% \n(H1
.EQ
delim off
.EN
.fS
# library_files specifies which file(s) contain
# the appropriate model definitions.

include_library  spice3f3.lib

# model indicates which predefined models can be
# used for which group of devices and it includes
# the ranges for area (AE), perimeter (PE) and
# (for lateral pnp) base width (WB).

model bw101a  npnBW  npn (
        AE 4e-12 8e-12 4e-11
        PE (2*$AE / 2.00e-06 + 4.00e-06)
)
model bw10x  npnBW  npn (
        AE 4e-12 2e-10
        PE 8e-06 6e-05
)
model bs101a  npnBS  npn (
        AE 3.60e-11
        PE 2.40e-05
)
model bi101a  npnBI  npn (
        AE 8.00e-12
        PE 1.20e-05
)
model wp102c  pnpWP  pnp (
        AE 3.60e-11
        PE 2.4e-05
        WB 3.00e-06
)
.fE
.aH "Library File for Bipolar Example Process"
.sY %AX_BipLibraryFile% \n(H1
.fS
# In this file, the models are described for the different bipolar
# devices of the DIMES-01 process. It is allowed to create models
# for which the parameters are defined by a substitution equation.

unity Q_electron      1.602e-19
unity N_intrinsic     1.045e+20
unity Gummel_base     7.500e+06
unity C0s_wn_bw       1.900e-03
unity C0e_wn_bw       2.800e-09
unity C0s_bw_epi      0.290e-03
unity C0s_bn_sub      0.151e-03

model bw10x  npn (Is=($Q_electron*$N_intrinsic/$Gummel_base)*$AE Nf=1
             Ikf=3.00e+07*$AE+6.00e+01*$PE Bf=117 Br=4 Vaf=55 Var=4
             Ikr=5.00e+07*$AE+1.00e+02*$PE Xtb=1.5 Eg=1.17 Xti=2.5
             Cje=$C0s_wn_bw*$AE+$C0e_wn_bw*$PE Vje=0.78 Mje=0.28 Tf=20p
             Xtf=(4.70e-02*$AE+1.90e-02*$PE)^2 Tr=100p Mjc=0.32 Vjc=0.67
             Cjc=$C0s_bw_epi*$AE Cjs=$C0s_bn_sub*$AE Vjs=0.45 Mjs=0.26)

model bw101a npn (Is=0.018f Bf=117 Nf=1 Vaf=55 Ikf=4.1m Br=4 Nr=1 Var=4
             Ikr=45u Rb=600 Irb=0.15m Rbm=30 Re=14 Rc=200 Xtb=1.5
             Eg=1.17 Xti=2.5 Cje=50f Vje=0.78 Mje=0.28 Tf=20p Cjc=75f
             Vjc=0.67 Mjc=0.32 Xcjc=1 Tr=100p Cjs=0.24p Vjs=0.45 Mjs=0.26)

model bs101a npn (Is=0.050f Bf=100 Nf=1 Vaf=40 Ikf=5m Ise=1f Ne=2 Br=0.5
             Nr=1 Var=5 Ikr=5m Isc=10f Nc=1.2 Rb=350 Irb=1m Rbm=50 Re=25
             Rc=150 Eg=1.17 Xti=3 Cje=95f Vje=0.8 Mje=0.26 Tf=30p Cjc=100f
             Vjc=0.75 Mjc=0.33 Xcjc=1 Tr=100p Cjs=0.25p Vjs=0.45 Mjs=0.26)

model bi101a npn (Is=0.018f Bf=120 Nf=1 Vaf=58 Ikf=4.2m Br=4 Nr=1 Var=5.8
             Ikr=45u Rb=600 Irb=0.15m Rbm=30 Re=14 Rc=200 Xtb=1.5 Eg=1.20
             Xti=2.5 Cje=40f Vje=0.77 Mje=0.24 Tf=30p Cjc=75f Vjc=0.65
             Mjc=0.32 Xcjc=1 Tr=100p Cjs=0.24p Vjs=0.45 Mjs=0.26)

model wp102c pnp (Is=0.183f Bf=89 Nf=1 Vaf=13 Ikf=0.5m Ise=0.37f Ne=2 BR=17
             Nr=1 Var=10 Ikr=0.4m Isc=1.4f Nc=2 Rb=80 Irb=10 Rbm=0 RE=25
             Rc=150 Xtb=1.5 Eg=1.20 Xti=2.5 Cje=95f Vje=0.67 Mje=0.34
             Tf=90p Cjc=387f Vjc=0.70 Mjc=0.40 Xcjc=0.3 Tr=1n Cjs=0.60p
             Vjs=0.52 Mjs=0.31)
.fE
.EQ
delim $$
.EN
