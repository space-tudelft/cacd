.T= "Partial Cap2d Extraction with Space Cap3d"
.DS 2
.rs
.sp 1i
.B
.S 15 20
Partial Cap2d Extraction
with Space Cap3d
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
Report EWI-ENS 08-04
.ce
Sep 9, 2008
.sp |9i
.DS
.in +5
.S 9
Copyright \s+3\(co\s-3 2008 by the author.

Last revision: Oct 13, 2008.
.S
.in -5
.DE
.SK
.S=
.H 1 "INTRODUCTION"
This application note discuss the implementation of partial 2D capacitance extraction while using the 3D capacitance extraction mode.
The partial 2D capacitance extraction mode can be useful for some parts in the layout where 3D extraction fails or is less accurate.
For example, when in the layout MIM capacitances are specified.
MIM capacitances are metal1 to metal2 capacitances with a conduction MIM layer between them.
The MIM layer narrows the gap between the two metal layers and is connected to one of the metal layers (for example metal2).
The distance between the metal1 and MIM layer becomes in this case inappropriate small.
In such a case it is maybe more useful to use the 2D capacitance extraction mode.
Because the schur dimensions become smaller the extraction shall also be faster.
.P
I have investigated the 3D capacitance extraction problem with a simple "t1" cell and
have used as starting point the demo \fBsram\fP example directory.
The project directory uses the "scmos_n" process and a lambda of 0.25 micron.
.br
The layout of cell "t1" contains two metal conductors above each other (see LDM below).
.fS I
% cat t1.ldm
ms t1
term cmf 6 30 12 20 f
term cms 6 30 12 20 s
me
.fE
The dimension of each conductor is dx=24 and dy=8 units (6 by 2 micron).
Thus each conductor has a perimeter of 16 micron and an area of 12 micron^2.
.P
I used the following technology file for the 2D/3D capacitance extraction:
.fS I
% cat t1.s
unit vdimension    1e-6  # um
unit a_capacitance 1e-6  # aF/um^2
unit e_capacitance 1e-12 # aF/um

conductors :
    cond_mf : cmf  : cmf  : 0.045   # metal first
    cond_ms : cms  : cms  : 0.030   # metal second

capacitances :
    acap_mf_ms :  cmf cms : cmf cms : 86.33625
    ecap_mf_ms : !cmf -cmf !cms -cms : -cmf -cms : 20

vdimensions :
    vdim_mf : cmf  : cmf : 1.70 0.70 # 2.4
    vdim_ms : cms  : cms : 2.80 0.70
 
dielectrics :
    SiO2   3.9   0.0
    air    1.0   5.0
.fE
The z-distance between the two conductors is 0.4 micron (see vdimensions).
.br
I used the following parameter file "t1.p" for the extraction:
.fS I
BEGIN cap3d
be_mode           0c
be_window         20
max_be_area       20
END cap3d
.fE
This gives for example the following cap3d extraction:
.fS I
% space3d -C3 -E t1.t -P t1.p t1
% xsls t1

network t1 (terminal f, s)
{
    cap 1.310002f (s, f);
    cap 448.4035e-18 (s, GND);
    cap 711.7672e-18 (f, GND);
}
.fE
You see that there are three capacitances, two ground caps
and one couple cap between the terminals "f" and "s".
I used a large be_window, thus the complete layout is extracted in one strip.
I used also a large max_be_area, thus no tiles where refined.
The schur matrix dimension is 12, because there are 12 faces (center spiders).
.P
The following tables show what happens when i change the z-distance between
the two conductors (i lower the height of vdim_ms).
.P
.ps -1
.TS
box;
l s s s s s s
l c s s || c s s
l| l| l| l || l| l| l.
cell t1: max_be_area 20 => 12 faces
_
	be_mode 0c	be_mode 0g
_
d	cap (s, f)	cap (s, GND)	cap (f, GND)	cap (s, f)	cap (s, GND)	cap (f, GND)
_
0.4	1.310002f	448.4035e-18	711.7672e-18	1.369053f	472.997e-18	747.7688e-18
0.2	2.400741f	447.0079e-18	693.5715e-18	2.482993f	471.3857e-18	727.5944e-18
0.1	4.51126f	445.3707e-18	683.5609e-18	4.613935f	469.1622e-18	716.6982e-18
0.01	41.85221f	442.8611e-18	674.1515e-18	41.97653f	465.8044e-18	706.3397e-18
0.001	414.8399f	442.5706e-18	673.1669e-18	414.9213f	465.3425e-18	705.2803e-18
0.0001	4.146612p	442.4478e-18	673.1613e-18	4.141934p	465.3088e-18	705.1597e-18
.TE
.TS
box;
l s s s s s s
l| l| l| l || l| l| l.
cell t1: max_be_area 8 => 20 faces
_
d	cap (s, f)	cap (s, GND)	cap (f, GND)	cap (s, f)	cap (s, GND)	cap (f, GND)
_
0.4	1.311813f	458.4563e-18	719.0441e-18	1.372215f	473.1009e-18	748.0071e-18
0.2	2.402475f	456.9011e-18	700.1009e-18	2.489224f	471.58e-18	727.7712e-18
0.1	4.512204f	455.0071e-18	689.9215e-18	4.622648f	469.4228e-18	716.8317e-18
0.01	41.85282f	452.5077e-18	680.3097e-18	41.98676f	466.038e-18	706.5479e-18
0.001	414.8448f	452.2917e-18	679.2373e-18	414.9521f	465.5727e-18	705.4987e-18
0.0001	4.14172p	452.2001e-18	679.1993e-18	4.143654p	465.4527e-18	705.4647e-18
.TE
.TS
box;
l s s s s s s
l c s s || c s s
l| l| l| l || l| l| l.
cell t1: max_be_area 2 => 52 faces
_
	be_mode 0c	be_mode 0g
_
d	cap (s, f)	cap (s, GND)	cap (f, GND)	cap (s, f)	cap (s, GND)	cap (f, GND)
_
0.4	1.314351f	465.7617e-18	729.7088e-18	1.374543f	475.6444e-18	750.3513e-18
0.2	2.396848f	464.1708e-18	710.279e-18	2.497719f	474.0816e-18	729.8953e-18
0.1	4.483736f	462.2121e-18	700.0046e-18	4.643547f	471.9579e-18	718.8718e-18
0.01	39.3684f	459.5685e-18	690.3953e-18	42.03862f	468.7256e-18	708.4689e-18
0.001	262.068f	458.5616e-18	690.1069e-18	415.0244f	468.2671e-18	707.4214e-18
0.0001	NAN	NAN	NAN	4.143557p	468.2047e-18	707.331e-18
.TE
.ps +1
The NAN problem in the schur module can only be solved, when we change the mesh.
The space between the two conductors can be filled with a contact area.
In that case the top faces of the metal1 conductor and the bottom faces of the metal2
conductor are not made.
Thus there are 16 lesser faces.
The sidewall faces of the contact can be flagged, thus they are not done.
Because there are lesser faces, the values of the GND capacitances are changed:
.P
.ps -1
.TS
box;
l s s s s s s
l| l| l| l || l| l| l.
cell t1: max_be_area 2 => 36 faces
_
d	cap (s, f)	cap (s, GND)	cap (f, GND)	cap (s, f)	cap (s, GND)	cap (f, GND)
_
0.4	cap2d	463.6631e-18	731.6343e-18	cap2d	471.9593e-18	754.0059e-18
0.2	cap2d	461.7845e-18	712.6315e-18	cap2d	469.5282e-18	734.1785e-18
0.1	cap2d	459.6584e-18	702.3628e-18	cap2d	466.7476e-18	723.4487e-18
0.01	cap2d	456.8923e-18	692.6535e-18	cap2d	462.8814e-18	713.2794e-18
0.001	cap2d	456.5694e-18	691.6569e-18	cap2d	462.3827e-18	712.2354e-18
0.0001	cap2d	456.5366e-18	691.557e-18	cap2d	462.3311e-18	712.1307e-18
.TE
.ps +1
The couple cap (s, f) must now be calculated with a cap 2D method.
.br
For max_be_area=20 the results are as follows:
.ps -1
.TS
box;
l s s s s s s
l| l| l| l || l| l| l.
cell t1: max_be_area 20 => 10 faces
_
d	cap (s, f)	cap (s, GND)	cap (f, GND)	cap (s, f)	cap (s, GND)	cap (f, GND)
_
0.4	cap2d	444.3699e-18	715.559e-18	cap2d	469.6386e-18	751.0907e-18
0.2	cap2d	442.4028e-18	697.449e-18	cap2d	467.2316e-18	731.268e-18
0.1	cap2d	440.3521e-18	687.5401e-18	cap2d	464.4574e-18	720.5158e-18
0.01	cap2d	437.6014e-18	678.087e-18	cap2d	460.5787e-18	710.2934e-18
0.001	cap2d	437.2637e-18	677.1222e-18	cap2d	460.078e-18	709.2406e-18
0.0001	cap2d	437.2293e-18	677.0254e-18	cap2d	460.0262e-18	709.135e-18
.TE
.ps +1
.H 1 "CAP 2D EXTRACTION"
A 2D couple capacitance can be calculated with the following formula:
.fS I
C = e0 * er * A / d = 8.855e-12 * 3.9 * 12e-12 / 0.4e-6 = 1036.035 aF
.fE
For the area cap in the technology file i use the following value:
.fS I
acap_mf_ms = 1036.035 / 12 = 86.33625 aF / um^2
.fE
For the edge cap in the technology file i use 20 aF / um.
.br
Thus i get the following total 2D couple capacitance:
.fS I
area_cap = 86.33625 * 12 um^2 = 1036.035 aF
edge_cap = 20.00000 * 16 um   =  320.000 aF
                                --------- +
             total couple cap = 1356.035 aF
.fE
A 2D extraction with space gives the following result:
.fS I
% space3d -C -E t1.t -P t1.p t1
% xsls t1

network t1 (terminal f, s)
{
    cap 1.356035f (s, f);
}
.fE
For other distances the area_cap can be calculated with the following formula:
.fS I
area_cap = 1036.035 aF * 0.4 / d
.fE
Other example cells:
.fS I
ms t2                            ms t3
term cmf 0 40 0 40 f             term cmf 0 400 0 400 f
term cms 0 40 0 40 s             term cms 0 400 0 400 s
me                               me

perim = 40 um                    perim =  400 um
area = 100 um^2                  area = 10000 um^2
.fE
Cap 2D extraction results for all cells and distances:
.TS
box;
l| l| l| l.
d	t1 total_cap	t2 total_cap	t3 total_cap
_
0.4	1.356035f	9.433625f	871.3625f
0.2	2.39207f	18.06725f	1.734725p
0.1	4.46414f	35.3345f	3.46145p
0.01	41.7614f	346.145f	34.5425p
0.001	414.734f	3.45425p	345.353p
0.0001	4.14446p	34.5353p	3.453458n
.TE
.H 1 "CAP 3D EXTRACTION RESULTS FOR CELLS T2 AND T3"
.ps -1
.TS
box;
l s s s s s s
l c s s || c s s
l| l| l| l || l| l| l.
cell t2: max_be_area 20 => 48 faces
_
	be_mode 0c	be_mode 0g
_
d	cap (s, f)	cap (s, GND)	cap (f, GND)	cap (s, f)	cap (s, GND)	cap (f, GND)
_
0.4	9.492534f	1.036256f	3.055058f	9.496113f	1.056767f	3.129756f
0.2	18.76569f	1.066303f	2.999039f	18.32628f	1.070052f	3.085702f
0.1	38.47932f	1.106859f	2.942624f	35.83787f	1.077935f	3.058651f
0.01	NAN	NAN	NAN	359.351f	1.154236f	2.961804f
0.001	NAN	NAN	NAN	5.281847p	2.312276f	1.80174f
0.0001	NAN	NAN	NAN	NAN	NAN	NAN
.TE
.TS
box;
l s s s s s s
l| l| l| l || l| l| l.
cell t2: max_be_area 20 => 32 faces (modified mesh)
_
d	cap (s, f)	cap (s, GND)	cap (f, GND)	cap (s, f)	cap (s, GND)	cap (f, GND)
_
0.4	cap2d	1.007617f	3.082217f	cap2d	1.042962f	3.141611f
0.2	cap2d	1.018964f	3.044504f	cap2d	1.053094f	3.100782f
0.1	cap2d	1.022926f	3.024415f	cap2d	1.055664f	3.078962f
0.01	cap2d	1.02479f	3.005495f	cap2d	1.055329f	3.058616f
0.001	cap2d	1.024885f	3.003553f	cap2d	1.055053f	3.056558f
0.0001	cap2d	1.024894f	3.003358f	cap2d	1.055022f	3.056352f
.TE
.TS
box;
l s s s s s s
l| l| l| l || l| l| l.
cell t3: max_be_area 20 => 528, 1088, 528, 1056, 528, 1056, 1088 faces (0c:18s, 0g:41.5s)
_
d	cap (s, f)	cap (s, GND)	cap (f, GND)	cap (s, f)	cap (s, GND)	cap (f, GND)
_
0.4	864.3864f	11.41757f	213.6499f	870.2568f	12.72133f	212.4652f
0.2	1.696078p	11.53479f	213.4048f	1.735898p	12.39525f	212.6313f
0.1	3.410182p	11.15113f	213.7041f	3.457199p	12.53488f	212.3878f
0.01	23.86224p	6.781229f	217.9842f	33.85941p	12.57858f	212.2259f
0.001	NAN	NAN	NAN	289.4314p	12.5533f	212.2366f
0.0001	NAN	NAN	NAN	NAN	NAN	NAN
.TE
.TS
box;
l s s s s s s
l| l| l| l || l| l| l.
cell t3: max_be_area 20 => 272, 576, 272, 544, 272, 544, 576 faces (0c:4.3s, 0g:11.2s)
_
d	cap (s, f)	cap (s, GND)	cap (f, GND)	cap (s, f)	cap (s, GND)	cap (f, GND)
_
0.4	cap2d	11.66699f	213.3999f	cap2d	11.9386f	213.2468f
0.2	cap2d	11.97055f	212.9664f	cap2d	12.13178f	212.8907f
0.1	cap2d	12.12591f	212.7254f	cap2d	12.21012f	212.7059f
0.01	cap2d	12.231f	212.5282f	cap2d	12.25614f	212.5395f
0.001	cap2d	12.24032f	212.5087f	cap2d	12.25708f	212.5239f
0.0001	cap2d	12.24124f	212.5067f	cap2d	12.2573f	212.5223f
.TE
.ps +1
.H 1 "DISCUSSION ABOUT THE METHOD USED"
The used method skips both the bottom and top planes.
.fS I
+-----------------------+
| conductor m2          |
+  -  -  -  -  -  -  -  + <- bottom skipped

+  -  -  -  -  -  -  -  + <- top skipped
| conductor m1          |
+-----------------------+
.fE
The other planes produce also a couple cap between conductors m1 and m2.
This couple cap can be left out by flagging the used spiders in the planes.
A 2D edge couple cap must be defined to add the effect of the edges.
When one of the conductors is longer than the other there are more edge effects.
It is better, to calculate all edge effects with the cap 3D method.
However it looks that the calculated values are two times the values which are expected.
Therefor i implemented another method which has a better expectance (see picture below).
.fS I
+-----------------------+
| conductor m2          |
+  -  -  -  -  -  -  -  + <- bottom skipped (core face)

+-----------------------+ <- top flagged as cap2d face
| conductor m1          |
+-----------------------+
.fE
The new method skips only one plane.
The faces in the other plane are flagged as cap2d.
All couple caps connected to these cap2d face spiders are left out.
The not skipped couple caps values looks to give the value for all edge effects.
See the extraction results for be_mode 0c and 0g in the tables below.
.ps -1
.TS
box;
l s s s s s s
l| l| l| l || l| l| l.
cell t1: max_be_area 20 => 11 faces
_
d	ecap (s, f)	cap (s, GND)	cap (f, GND)	ecap (s, f)	cap (s, GND)	cap (f, GND)
_
0.4	309.5748e-18	450.5521e-18	709.61e-18	273.8529e-18	472.2039e-18	748.5608e-18
0.1	445.0401e-18	457.5858e-18	671.302e-18	411.9228e-18	481.9925e-18	703.8193e-18
0.001	524.5773e-18	459.8639e-18	655.8728e-18	513.2061e-18	485.2236e-18	685.3983e-18
0.0001	525.437e-18	459.8834e-18	655.7255e-18	514.4498e-18	485.2437e-18	685.2247e-18
.TE
.TS
box;
l s s s s s s
l| l| l| l || l| l| l.
cell t1: max_be_area 2 => 44 faces
_
d	ecap (s, f)	cap (s, GND)	cap (f, GND)	ecap (s, f)	cap (s, GND)	cap (f, GND)
_
0.4	263.0461e-18	459.6849e-18	735.7093e-18	271.1581e-18	472.1261e-18	753.8421e-18
0.1	379.612e-18	467.1323e-18	695.0754e-18	407.4516e-18	481.1787e-18	709.6203e-18
0.001	452.7083e-18	469.5975e-18	679.0705e-18	507.5824e-18	484.4502e-18	691.2374e-18
0.0001	453.517e-18	469.6199e-18	678.9181e-18	508.8171e-18	484.4725e-18	691.0631e-18
.TE
.ps +1
.H 1 "HOW TO CHOICE FOR CAP2D"
As first implementation, we can request space3d to skip a certain couple of vdimensions.
We can add a parameter to do so.
In my test case, i test for the vdimensions distance, but this is maybe not a good idea.
Because any distance can be given by some user.
I don't implemented a distance parameter, but hard coded some minimal distance.
I added a normal cap2d rule to the technology file and specified parameter
"cap3d.all_non3d_cap" to let space3d know not to skip any 2D caps.
Maybe space3d can also calculate the needed cap2d area cap, because the area and distance
and the dielectric values are known.
We can also specifiy to space3d not to skip a certain cap2d element name.
.P
We can also add a rule to the technology file, where we specify that a pair of vdimensions
must be done in cap2d.
This can be done after the vdimensions section or be part of this section.
Tecc can add a special cap2d element, which may not be skipped by space3d.
Tecc can calculate the value for this cap2d element.
The special cap2d element specifies also the involved vdimension conductors.
.P
I have now changed tecc and have added the following technology file rule:
.fS I
vdimensions :
    ver_cmf : cmf : cmf : 1.70 0.70
    ver_cms : cms : cms : 2.80 0.70
    omit_cap3d : ver_cmf ver_cms
.fE
Tecc calculates a cap2d surface cap value after the "dielectrics" section is done.
The "omit_cap3d" clause always add a SURFCAP3DELEM to the technology file.
The SURFCAP3DELEM is used in the recognize mesh phase to toggle the bottom and top faces involved.
The SURFCAP3DELEM value is used for the cap2d surface cap calculation.
If the value is zero, no cap2d surface cap is calculated.
.P
If you don't want that tecc calculates a value for the cap2d cap, you can specify
a cap2d value yourself.
For example:
.fS I
    omit_cap3d : ver_cmf ver_cms : 86.33625
.fE
The value is dependent of "unit a_capacitance".
Note that, if you specify a zero value, no cap2d surface cap is calculated.
I have also added the "keep_cap2d" rule.
The following example gives the same result as above:
.fS I
capacitances :
    acap_cms_cmf :  cms cmf : cms cmf : 86.33625
    ecap_cms_cmf : !cms -cms !cmf -cmf : -cms -cmf : 20

vdimensions :
    ver_cmf : cmf : cmf : 1.70 0.70
    ver_cms : cms : cms : 2.80 0.70
    omit_cap3d : ver_cmf ver_cms : 0
    keep_cap2d : acap_cms_cmf
.fE
.H 1 "NEW CAP3D EXTRACTION RESULTS"
The following tables show the new results for cap3d with partial cap2d.
.ps -1
.TS
box;
l s s s s s s
l c s s || c s s
l| l| l| l || l| l| l.
cell t1: max_be_area 20 => 11 faces (modified mesh)
_
	be_mode 0c	be_mode 0g
_
d	cap (s, f)	cap (s, GND)	cap (f, GND)	cap (s, f)	cap (s, GND)	cap (f, GND)
_
0.4	1.34561f	450.5521e-18	709.61e-18	1.309888f	472.2039e-18	748.5608e-18
0.2	2.459357f	455.1913e-18	685.3417e-18	2.421163f	478.5073e-18	720.4374e-18
0.1	4.58918f	457.5858e-18	671.302e-18	4.556063f	481.9925e-18	703.8193e-18
0.01	41.95754f	459.6683e-18	657.3374e-18	41.94263f	485.002e-18	687.133e-18
0.001	414.9386f	459.8639e-18	655.8728e-18	414.9272f	485.2236e-18	685.3983e-18
0.0001	4.144665p	459.8834e-18	655.7255e-18	4.144654p	485.2437e-18	685.2247e-18
.TE
.TS
box;
l s s s s s s
l| l| l| l || l| l| l.
cell t2: max_be_area 20 => 40 faces (modified mesh)
_
d	cap (s, f)	cap (s, GND)	cap (f, GND)	cap (s, f)	cap (s, GND)	cap (f, GND)
_
0.4	9.508636f	1.036904f	3.054083f	9.418409f	1.067013f	3.118546f
0.2	18.34224f	1.062414f	3.002778f	18.26145f	1.0959f	3.059253f
0.1	35.75212f	1.075453f	2.973911f	35.69594f	1.110794f	3.025369f
0.01	346.742f	1.086251f	2.946283f	346.7327f	1.123502f	2.992259f
0.001	3.454868p	1.087427f	2.943284f	3.454867p	1.124574f	2.988879f
0.0001	34.53592p	1.08757f	2.942959f	34.53592p	1.124677f	2.988541f
.TE
.TS
box;
l s s s s s s
l| l| l| l || l| l| l.
cell t3: max_be_area 20 => 400, 832, 400, 800, 400, 800, 832 faces (0c:9.5s, 0g:23.6s)
_
d	cap (s, f)	cap (s, GND)	cap (f, GND)	cap (s, f)	cap (s, GND)	cap (f, GND)
_
0.4	874.2887f	11.35244f	213.7156f	871.5565f	12.11442f	213.0711f
0.2	1.739841p	11.74394f	213.1956f	1.736909p	12.47391f	212.5524f
0.1	3.467863p	11.96462f	212.8904f	3.46506p	12.65764f	212.2648f
0.01	34.55078p	12.1151f	212.649f	34.54822p	12.81333f	211.9911f
0.001	345.3615p	12.12844f	212.6255f	345.359p	12.82342f	211.9665f
0.0001	3.453467n	12.12978f	212.6231f	3.453464n	12.82478f	211.9637f
.TE
.ps +1
You see that for cell t3 the partial cap2d method is almost 2 times faster than a complete cap3d extraction.
Thus you see, if you want to have a faster extraction, you must reduce the mesh.
But more important by this partial cap2d method is, that you don't get the NAN problem.
.P
The next page gives the extraction results of cell t3 for a more reduced mesh.
.ps -1
.TS
box;
l s s s s s s
l c s s || c s s
l| l| l| l || l| l| l.
cell t3: max_be_area 40 => 200, 416, 200, 400, 200, 400, 416 faces (0c:2.6s, 0g:11.8s)
_
	be_mode 0c	be_mode 0g
_
d	cap (s, f)	cap (s, GND)	cap (f, GND)	cap (s, f)	cap (s, GND)	cap (f, GND)
_
0.4	874.4772f	12.28361f	212.1965f	871.9261f	11.93117f	212.8592f
0.2	1.741247p	12.37502f	211.9669f	1.737583p	12.26204f	212.3849f
0.1	3.470509p	12.34236f	211.9077f	3.466027p	12.46857f	212.0825f
0.01	34.55377p	12.50335f	211.6521f	34.54893p	12.59793f	211.8399f
0.001	345.3645p	12.51913f	211.6258f	345.3597p	12.60872f	211.8159f
0.0001	3.45347n	12.5208f	211.6231f	3.453465n	12.60974f	211.8135f
.TE
.TS
box;
l s s s s s s
l| l| l| l || l| l| l.
cell t3: max_be_area 80 => 104, 224, 104, 208, 104, 208, 224 faces (0c:0.73s, 0g:7.8s)
_
d	cap (s, f)	cap (s, GND)	cap (f, GND)	cap (s, f)	cap (s, GND)	cap (f, GND)
_
0.4	875.2398f	11.96969f	212.5992f	872.5675f	11.72428f	212.5733f
0.2	1.740425p	12.44732f	212.0022f	1.737961p	12.09793f	212.0568f
0.1	3.468734p	12.61832f	211.7436f	3.466538p	12.28055f	211.7759f
0.01	34.5517p	12.79865f	211.4668f	34.54938p	12.45853f	211.4836f
0.001	345.3624p	12.8141f	211.4403f	345.3603p	12.46359f	211.4648f
0.0001	3.453467n	12.81565f	211.4376f	3.453465n	12.46505f	211.462f
.TE
.TS
box;
l s s s s s s
l| l| l| l || l| l| l.
cell t3: max_be_area 800 => 16, 36, 16, 32, 16, 32, 36 faces (0c:0.07s, 0g:0.92s)
_
d	cap (s, f)	cap (s, GND)	cap (f, GND)	cap (s, f)	cap (s, GND)	cap (f, GND)
_
0.4	872.5975f	14.42184f	212.7341f	875.1432f	18.59759f	219.9812f
0.2	1.737913p	14.68976f	212.2159f	1.741201p	19.21398f	219.2046f
0.1	3.466272p	14.7679f	211.925f	3.469961p	19.52752f	218.7906f
0.01	34.54882p	14.96373f	211.5892f	34.55298p	19.9838f	218.271f
0.001	345.3595p	14.97789f	211.5545f	345.3638p	20.01112f	218.2311f
0.0001	3.453465n	14.97931f	211.551f	3.453469n	20.01381f	218.2271f
.TE
.TS
box;
l s s s s s s
l| l| l| l || l| l| l.
cell t3: max_be_area 8000 => 7, 16, 7, 14, 7, 14, 16 faces (0c:0.06s, 0g:0.34s)
_
d	cap (s, f)	cap (s, GND)	cap (f, GND)	cap (s, f)	cap (s, GND)	cap (f, GND)
_
0.4	874.3799f	13.56267f	216.4429f	875.4465f	15.59632f	218.1946f
0.2	1.739879p	13.66499f	215.9064f	1.741631p	15.7458f	217.5723f
0.1	3.468125p	13.69821f	215.588f	3.470514p	15.77855f	217.2227f
0.01	34.5510p	13.71437f	215.2613f	34.55438p	15.75932f	216.8805f
0.001	345.3617p	13.71512f	215.2261f	345.3652p	15.75259f	216.8447f
0.0001	3.453467n	13.7152f	215.2226f	3.45347n	15.75184f	216.8411f
.TE
.ps +1
In the last table you see the minimal mesh, this depends on the "cap3d.be_window".
.br
Thus, it is maybe a good idea not to reduce the partial cap2d mesh.
.br
To get this done, we must flag all faces of a partial cap2d mesh.
.H 1 "CMIM EXTRACTION RESULTS"
Now follows a more realistic extraction example of a "cmim" capacitance.
.P
The following technology file is used:
.fS I
% cat cmim.s

unit vdimension  1e-6  # um

colors :
    cmf   blue
    cms   green
    cog   red

conductors :
    cond_mf : cmf       : cmf  : 0.045      # first metal
    cond_ms : cms       : cms  : 0.030      # second metal
    cond_cg : cog       : cog  : 0.030      # cmim layer

contacts :
    cont_fs : cmf !cog cva cms : cmf cms : 0.1
    cont_cs :      cog cva cms : cog cms : 0.1

vdimensions :
    ver_cmf        : cmf  : cmf : 1.70 0.70
    ver_cog        : cog  : cog : 2.42 0.28
    ver_cms        : cms  : cms : 2.80 0.70
    omit_cap3d : ver_cmf ver_cog
 #  omit_cap3d : ver_cog ver_cms

dielectrics :
   SiO2   3.9   0.0
   air    1.0   5.0

.fE
The following parameter file is used:
.fS I
% cat cmim.p

disp.draw_be_mesh

BEGIN cap3d
be_mode          0c
be_window        20
max_be_area     200
END cap3d

.fE
See the next two pages for the extraction results.
Only "cap3d.be_mode=0c" is used.
The pictures are made with "cap3d.be_window=20" and "cap3d.max_be_area=200".
.TS
box;
l s s s s s
l l l l l l.
cell cmim: omit_cap3d for cmf/cog
_
be_window	max_be_area	cap (f, s)	cap (f, GND)	cap (s, GND)	real time
_
40	200	123.6099p	1.836616p	37.52563f	14.7
40	400	123.5857p	1.836158p	42.56402f	5.8
_
20	200	123.6023p	1.843837p	40.17355f	5.3
20	400	123.5931p	1.841476p	52.68501f	2.7
_
10	200	123.5910p	1.868847p	96.00037f	2.1
10	400	123.5949p	1.873557p	98.72101f	1.7
.TE
.F+
.PSPIC "an0804/fig1.ps" 6i
.F-
The above picture shows the modified mesh, which gives no problems for "space3d".
You see that it is more orthogonal than the mesh of the following picture.
.TS
box;
l s s s s s
l l l l l l.
cell cmim: omit_cap3d for both cmf/cog and cog/cms
_
be_window	max_be_area	cap (f, s)	cap (f, GND)	cap (s, GND)	real time
_
40	200	123.5997p	1.836524p	37.48174f	10.6
40	400	123.5927p	1.836102p	41.8949ff	4.3
_
20	200	123.6008p	1.843736p	41.40739f	3.9
20	400	123.5934p	1.840953p	50.22652f	2.0
_
10	200	123.5939p	1.868867p	95.99144f	1.6
10	400	123.5942p	1.873576p	98.67894f	1.2
.TE
.F+
.PSPIC "an0804/fig2.ps" 6i
.F-
The above picture shows the unmodified mesh, which results in "space3d: Encountered NAN in schur module".
This happens when edges are split too far from there original x or y point, because
there is already a spider found.
