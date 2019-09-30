.H 1 "The Interpolation Method"
.sY %SE_INTER%  \n(H1
.H 2 "Introduction"
The interpolation method can quickly compute substrate 
resistances for large circuits.
The method is illustrated by the following figure
which shows a configuration of two square terminals
on top of a substrate.
.DS
.PS 2.2c
copy "../spacesubman/submod.pic"
.PE
.DE
The interpolation method uses the notion of a
(virtual) substrate node (default name "SUBSTR")
to which all substrate terminals are directly connected via 
resistance (see the above figure:
resistance $R sub a$ and resistance $R sub b$ connect respectively
terminal $A$ and
terminal $B$ to the substrate node).
Direct coupling resistances between substrate terminals
are only computed between terminals that are ``neighbors'' of each other
(resistance $R sub ab$ for terminal pair $A$-$B$).
The values of the resistance are computed using interpolation
formulas based on area and perimeter information and based on
the distances between the terminals.
.H 2 "Network Structure"
Direct coupling resistances between substrate terminals
are only computed between terminals that are ``neighbors'' of each other.
Whether or not two terminals are neighbors is determined
from a Delaunay triangulation in which the corners of the
substrate terminals are the nodes of the Delaunay triangulation.
.[
Fast Computation Substrate
Genderen
.]
An example of a Delaunay triangulation for a set of substrate terminals
is shown in the figure below at the left
(terminals are grey).
.F+
.PSPIC -L "../spacesubman/newdelau.eps" 2.7i 1.5i
.sp -1.7i
.PSPIC -R "../spacesubman/newresis.eps" 2.7i 1.5i
.F-
A property of the Delaunay triangulation is that it is a planar
graph that connects nodes that are "neighbors" of each other.
Therefore, the interpolation method computes a direct coupling
resistance between two terminals if, and only if,
the terminals are connected by at least one edge of the Delaunay 
triangulation.
As a result, for the figure above at the left, direct coupling resistances are 
computed as shown in the figure above at the right.
Recall that the terminals are also coupled to each other via
the substrate node.
.H 2 "Resistance Computation"
The resistance between a terminal $A$ and the virtual substrate node
is computed from the following formula
.DS
.EQ
R sub a ~ = ~ { 1 } over { G sub { a } } ~ = ~
{ 1 } over { L ~ P sub a sup m ~  A sub a sup n }
.EN
.DE
where
$P sub a$ is the perimeter of terminal $A$, $A sub a$ is the area
of terminal $A$,
and $L$, $n$ and $m$ are empirical fitting parameters.
.P
The direct coupling resistance between a terminal $A$ and a
terminal $B$ is computed from the formula
.DS
.EQ
R sub { ab } ~ = ~
{ K ~ d sup p } over { sqrt { A sub a } ~+~ sqrt { A sub b } }
.EN
.DE
where
$A sub a$ and $A sub b$ are the areas of the terminals,
$d$ is the minimum distance between the terminals
and $p$ and $K$ are empirical fitting parameters.
.P
When the distance between two terminals is decreased,
a part of the current between the terminals that normally flows
via the substrate node will flow via the direct coupling
resistance.
This is
modeled by subtracting a fraction of the total direct
coupling conductance that is connected
to a terminal from the conductance
between that terminal and the substrate node.
.P
The fitting parameters $L$, $m$, $n$,
$p$ and $K$ are automatically computed by
.P= space
from the resistance values for some typical substrate terminal
configurations
(see Section %SE_TECHINTER%).
These values are 
obtained via measurement on the chip, or
by using the boundary-element method that is described in 
Section %SE_BEM%.
.H 2 "Determining the Parameters"
The parameters for the interpolation method in the element
definition file 
(see Section %SE_TECHINTER%) must be determined from results
for some standard terminal configurations, that are obtained
using some other method.
Usually this method will be the boundary-element method 
that can be applied by
.P= space
(see Section %SE_BEM%),
but also other programs can be used for this, or 
results from measurements on the chip can be used.
.P
The application of the boundary-element method of space
to find the parameters for the interpolation method,
is done using the tool
.P= subresgen
(see "icdman subresgen").
This tool automatically generates the
parameters from a description of the substrate layers 
in a space element definition file.
However, for completeness, it is described below
how the parameters can be computed using another method.
.H 3 "Computation of selfsubres entries"
For different terminal sizes do the following:
.BL
.LI
For a single terminal with area $A$ and perimeter $P$, compute 
its substrate resistance $Rsub1$ to the virtual substrate node
(default this node is called SUBSTR in
.P= space
).
.LI
Take the above single terminal and put a sufficiently wide 
substrate ring terminal around it at a short distance (e.g. 
as close as different substrate terminals can be).  
The substrate resistance to the virtual substrate node 
is now $Rsub2$.
.LE
.P
The parameters for a line in the selfsubres list 
are then:
.br
.nf
      $A$    $P$    $Rsub1$    $(1/Rsub2)/(1/Rsub1)$
.fi
.TE
.H 3 "Computation of coupsubres entries"
For different terminal sizes and different distances do the
the following:
.BL
.LI
For a single terminal with area $A$ and perimeter $P$, compute
its substrate resistance $Rsub1$ to the virtual substrate node
(this part is similar to the first part that is described above).
.LI
For two of the above terminals at a distance $D$, compute the
substrate resistance $Rsub3$ to the virtual substrate node
for each terminal, and the direct coupling substrate $Rcoup$
between them.
.LE
.P
The parameters for a line in the selfsubres list 
are then:
.br
.nf
      $A$    $A$    $D$    $Rcoup$    $(1/Rsub1 - 1/Rsub3)/(1/Rcoup)$
.fi
.TE
.H 2 "Example of CMOS Ring Oscillator"
The following example consists of a CMOS ring oscillator.
To run the example, first create a project, e.g. with name "suboscil",
for a "scmos_n" process with lambda is 0.1$mu$.
.fS I
% mkpr -l 0.1 suboscil
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
directory /usr/cacd/share/demo/suboscil (supposing a \fB/usr/cacd\fP installation).
.fS I
% cd suboscil
% cp /usr/cacd/share/demo/suboscil/* .
.fE
The layout description is put into the database using the program
.P= cgi.
.fS I
% cgi oscil.gds
.fE
Use the layout editor
.P= dali
to view the layout:
.fS I
% dali oscil
.fE
To view also the sub-cells, use the mouse,
click on "DB_menu", then on "all_exp" and then on "maximum".
To leave the program, click on "-return-" and "-quit-".
.F+
.PSPIC "../spacesubman/oscil.eps" 4.8i
.F-
.SP
The source of the used
.P= space
default process technology file is as follows:
.EQ
delim off
.EN
.fS
% cat /usr/cacd/share/lib/process/scmos_n/space.def.s
#
# space element definition file for scmos_n example process
# with transistor bulk connections and substrate terminals 
# for substrate contacts and nmos bulk connections, and
# with information for 3D capacitance extraction and 
# substrate resistance extraction.
#
# masks:
# cpg - polysilicon interconnect        ccp - contact metal to poly
# caa - active area                     cva - contact metal to metal2
# cmf - metal interconnect              cwn - n-well
# cms - metal2 interconnect             csn - n-channel implant
# cca - contact metal to diffusion      cog - contact to bondpads
#
# See also: maskdata
.fE
.fS
unit resistance    1     # ohm
unit c_resistance  1e-12 # ohm um^2
unit a_capacitance 1e-6  # aF/um^2
unit e_capacitance 1e-12 # aF/um
unit capacitance   1e-15 # fF
unit vdimension    1e-6  # um
unit shape         1e-6  # um
.fE
.fS
maxkeys 13
.fE
.fS
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
.fE
.fS
conductors :
  # name    : condition     : mask : resistivity : type
    cond_mf : cmf           : cmf  : 0.045       : m    # first metal
    cond_ms : cms           : cms  : 0.030       : m    # second metal
    cond_pg : cpg           : cpg  : 40          : m    # poly interconnect
    cond_pa : caa !cpg !csn : caa  : 70          : p    # p+ active area
    cond_na : caa !cpg  csn : caa  : 50          : n    # n+ active area
    cond_well : cwn         : cwn  : 0           : n    # n well
.fE
.fS
fets :
  # name : condition    : gate d/s : bulk
    nenh : cpg caa  csn : cpg  caa : @sub  # nenh MOS
    penh : cpg caa !csn : cpg  caa : cwn   # penh MOS
.fE
.fS
contacts :
  # name   : condition         : lay1 lay2 : resistivity
    cont_s : cva cmf cms       : cmf  cms  :   1   # metal to metal2
    cont_p : ccp cmf cpg       : cmf  cpg  : 100   # metal to poly
    cont_a : cca cmf caa !cpg cwn !csn
           | cca cmf caa !cpg !cwn csn
                               : cmf  caa  : 100   # metal to active area
    cont_w : cca cmf cwn csn   : cmf  cwn  :  80   # metal to well
    cont_b : cca cmf !cwn !csn : cmf  @sub :  80   # metal to subs
.fE
.fS
junction capacitances ndif :
  # name    :  condition                 : mask1 mask2 : capacitivity
    acap_na :  caa       !cpg  csn !cwn  :  caa @gnd : 100  # n+ bottom
    ecap_na : !caa -caa !-cpg -csn !-cwn : -caa @gnd : 300  # n+ sidewall
.fE
.fS
junction capacitances nwell :
    acap_cw :  cwn                  :  cwn @gnd : 100  # bottom
    ecap_cw : !cwn -cwn             : -cwn @gnd : 800  # sidewall
.fE
.fS
junction capacitances pdif :
    acap_pa :  caa       !cpg  !csn cwn      :  caa cwn : 500 # p+ bottom
    ecap_pa : !caa -caa !-cpg !-csn cwn -cwn : -caa cwn : 600 # p+ sidewall
.fE
.fS
capacitances :
  # polysilicon capacitances
    acap_cpg_sub :  cpg                !caa !cwn :  cpg @gnd : 49
    acap_cpg_cwn :  cpg                !caa  cwn :  cpg cwn  : 49 
    ecap_cpg_sub : !cpg -cpg !cmf !cms !caa !cwn : -cpg @gnd : 52
    ecap_cpg_cwn : !cpg -cpg !cmf !cms !caa  cwn : -cpg cwn  : 52
.fE
.fS
  # first metal capacitances
    acap_cmf_sub :  cmf           !cpg !caa !cwn :  cmf @gnd : 25
    acap_cmf_cwn :  cmf           !cpg !caa  cwn :  cmf cwn  : 25
    ecap_cmf_sub : !cmf -cmf !cms !cpg !caa !cwn : -cmf @gnd : 52
    ecap_cmf_cwn : !cmf -cmf !cms !cpg !caa  cwn : -cmf cwn  : 52
.fE
.fS
    acap_cmf_caa :  cmf      caa !cpg !cca :  cmf  caa : 49
    ecap_cmf_caa : !cmf -cmf caa !cms !cpg : -cmf  caa : 59
.fE
.fS
    acap_cmf_cpg :  cmf      cpg !ccp :  cmf  cpg : 49
    ecap_cmf_cpg : !cmf -cmf cpg !cms : -cmf  cpg : 59
.fE
.fS
  # second metal capacitances
    acap_cms_sub :  cms      !cmf !cpg !caa !cwn :  cms @gnd : 16
    acap_cms_cwn :  cms      !cmf !cpg !caa  cwn :  cms cwn  : 16
    ecap_cms_sub : !cms -cms !cmf !cpg !caa !cwn : -cms @gnd : 51
    ecap_cms_cwn : !cms -cms !cmf !cpg !caa  cwn : -cms cwn  : 51
.fE
.fS
    acap_cms_caa :  cms      caa !cmf !cpg :  cms caa : 25
    ecap_cms_caa : !cms -cms caa !cmf !cpg : -cms caa : 54
    acap_cms_cpg :  cms      cpg !cmf :  cms cpg : 25
    ecap_cms_cpg : !cms -cms cpg !cmf : -cms cpg : 54
    acap_cms_cmf :  cms      cmf !cva :  cms cmf : 49
    ecap_cms_cmf : !cms -cms cmf      : -cms cmf : 61
.fE
.fS
    lcap_cms     : !cms -cms =cms    : -cms =cms : 0.07
.fE
.fS
vdimensions :
    v_caa_on_all : caa !cpg           : caa : 0.30 0.00
    v_cpg_of_caa : cpg !caa           : cpg : 0.60 0.50
    v_cpg_on_caa : cpg caa            : cpg : 0.35 0.70
    v_cmf        : cmf                : cmf : 1.70 0.70
    v_cms        : cms                : cms : 2.80 0.70
.fE
.fS
dielectrics :
   # Dielectric consists of 5 micron thick SiO2
   # (epsilon = 3.9) on a conducting plane.
   SiO2   3.9   0.0
   air    1.0   5.0
.fE
.fS
sublayers :
  # name       conductivity  top
    substrate  6.7           0.0
.fE
.fS
selfsubres :
#   Generated by subresgen on 10:56:15 13-5-2003
#     area    perim          r   rest
      0.64      3.2    81286.8   0.01  # w=0.8 l=0.8  
      0.64        4   73678.39   0.01  # w=0.4 l=1.6  
      0.48      3.2   88205.12   0.01  # w=0.4 l=1.2  
      2.56      6.4    40643.4   0.01  # w=1.6 l=1.6  
      2.56        8    36839.2   0.01  # w=0.8 l=3.2  
      1.92      6.4   44102.56   0.01  # w=0.8 l=2.4  
     10.24     12.8    20321.7   0.01  # w=3.2 l=3.2  
     10.24       16    18419.9   0.01  # w=1.6 l=6.4  
      7.68     12.8   22051.22   0.01  # w=1.6 l=4.8  
     40.96     25.6   10160.85   0.01  # w=6.4 l=6.4  
     40.96       32   9209.799   0.01  # w=3.2 l=12.8  
     30.72     25.6   11025.61   0.01  # w=3.2 l=9.6  
    163.84     51.2   5080.426   0.01  # w=12.8 l=12.8  
    163.84       64   4604.902   0.01  # w=6.4 l=25.6  
    122.88     51.2   5512.804   0.01  # w=6.4 l=19.2  
    655.36    102.4   2540.212   0.01  # w=25.6 l=25.6  
    655.36      128   2302.451   0.01  # w=12.8 l=51.2  
    491.52    102.4   2756.403   0.01  # w=12.8 l=38.4  
   2621.44    204.8   1270.106   0.01  # w=51.2 l=51.2  
   2621.44      256   1151.225   0.01  # w=25.6 l=102.4  
   1966.08    204.8   1378.201   0.01  # w=25.6 l=76.8  
.fE
.fS
coupsubres :
#   Generated by subresgen on 10:56:15 13-5-2003
#    area1      area2     dist          r    decr
      0.64       0.64      1.6     648598  0.873512  # w=0.8 d=1.6 
      0.64       0.64      3.2    1101504  0.925946  # w=0.8 d=3.2 
      0.64       0.64      6.4    1996617  0.959256  # w=0.8 d=6.4 
      0.64       0.64     25.6    7341756  0.988935  # w=0.8 d=25.6 
      2.56       2.56      3.2   324299.1  0.873515  # w=1.6 d=3.2 
      2.56       2.56      6.4   550752.1  0.925953  # w=1.6 d=6.4 
      2.56       2.56     12.8   998307.9  0.959253  # w=1.6 d=12.8 
      2.56       2.56     51.2    3670877  0.988967  # w=1.6 d=51.2 
.fE
.fS
     10.24      10.24      6.4   162149.6  0.873515  # w=3.2 d=6.4 
     10.24      10.24     12.8     275376  0.925950  # w=3.2 d=12.8 
     10.24      10.24     25.6   499154.2  0.959259  # w=3.2 d=25.6 
     10.24      10.24    102.4    1835439  0.988967  # w=3.2 d=102.4 
    655.36     655.36     51.2   20268.69  0.873515  # w=25.6 d=51.2 
    655.36     655.36    102.4      34422  0.925953  # w=25.6 d=102.4 
    655.36     655.36    204.8   62394.28  0.959257  # w=25.6 d=204.8 
    655.36     655.36    819.2   229429.8  0.988985  # w=25.6 d=819.2 
.fE
.fS
#EOF
.fE
The above element definition file also contains a description 
of the substrate layers but this information is not used when 
using the interpolation method for substrate resistance computation.
.P
The following parameter file is used for this example:
.fS
% cat param.p
# directory: demo/suboscil

BEGIN sub3d            # Data for the boundary-element method
be_mode          0g
max_be_area      1     # micron^2
edge_be_ratio    0.01
edge_be_split    0.2
be_window        10    # micron
END sub3d

min_art_degree          3      # Data for network reduction
min_degree              4
min_res               100      # ohm
max_par_res            20  
no_neg_res             on
min_coup_cap            0.05
lat_cap_window          6.0    # micron
max_obtuse            110.0    # degrees
equi_line_ratio         1.0

disp.save_prepass_image  on    # Data for Xspace
.fE
Then
.P= space3d
is run in flat mode with the option
.B -b
for interpolated substrate resistance
extraction and with the option
.B -C
for coupling capacitance extraction, as follows:
.fS I
% space3d -vF -P param.p -bC oscil
.fE 
The output is a circuit description containing
transistors, capacitances and substrate resistances.
This output can be inspected by running
.P= xspice
with e.g. using the option
.B -a
and with "oscil" as an argument.
.fS I
% xspice -a oscil
.fE
Alternatively
.P= Xspace
can be used to extract the circuit.
.fS I
% Xspace -P param.p
.fE
Click button "oscil" in the menu "Database",
click button "inter. sub. res." and "coupling cap." in the menu "Options",
click button "DrawSubTerm", "FillSubTerm" and "DrawSubResistor"
in the menu "Display",
and click "extract" in the menu "Extract".
This will yield the following picture:
.F+
.PSPIC "../spacesubman/Xoscil.eps" 5.0i
.F-
The picture shows the direct coupling resistances that are
computed between the substrate contacts and the bulk connections
of the n-MOS transistors.
The resistances to the substrate node, that are also
computed, are not shown.
.P
If you have
.P= spice
available, you can run a spice simulation
to inspect the noise on the terminal "sens"
that is caused by the substrate coupling effects.
(Check the script "nspice" to see if spice is called correctly).
In order to run
.P= spice,
use the simulation interface
.P= simeye.
.fS I
% simeye
.fE
Click on the "Simulate" menu and choice the "Prepare" item.
Select in the "Circuit:" field cell name "oscil" and
in the "Stimuli:" field file name "oscil.cmd" (click on it).
Choice simulation "Type: spice" and click on the "Run" button.
This will yield the following result:
.F+
.PSPIC "../spacesubman/simeye.eps" 5.0i
.F-
One may zoom in on the signal "sens" after clicking on the "View" menu "ZoomIn" item.
Draw with the mouse a rubber box zoom in area around the signal part to zoom in.
.br
Set "Options" menu item "DetailZoomON" for detailed "ZoomIn" on one signal.
.br
To leave the program, choice item "Exit" in the "File" menu.
