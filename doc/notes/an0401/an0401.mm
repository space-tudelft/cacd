.T= "Space 3D Substrate Extraction By Using FEMLAB"
.DS 2
.rs
.sp 1i
.B
.S 15 20
SPACE 3D SUBSTRATE EXTRACTION
BY USING FEMLAB
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
Report EWI-ENS 04-01
.ce
March 26, 2004
.sp |9i
.DS
.in +5
.S 9
Copyright \s+3\(co\s-3 2004 by the author.

Last revision: December 13, 2004.
.S
.in -5
.DE
.SK
.S=
.H 1 "INTRODUCTION"
Normally, the 
.P= space3d
layout to circuit extractor uses
for accurate (3D) substrate resistance extraction (option \fB-B\fP) the
.P= makesubres
program to calculate the substrate resistances.
See also the "Space 3D Substrate Extraction Application Note" (report EWI-ENS 03-04).
For the calculation,
the
.P= makesubres
program uses a substrate spider mesh, Green's functions and Schur matrix inversion.
The calculated results are written in the cell layout view "subres" file.
In the
.P= space3d
extract pass, the results are read again and the substrate resistances
are added to the extracted circuit network.
See the figure below.
.P
.F+
.PSPIC "an0304/fig1a.ps" 5i
.F-
Currently,
.P= makesubres
is a symbolic link to the
.P= space3d
program.
In the future, this part may be replaced by an user program.
This program must use the same extraction environment,
.P= space
parameter file
and technology file and must know the extracted cell name and project path.
.P
.F+
.PSPIC "an0401/fig1c.ps" 5i
.F-
Now,
as an alternative,
FEMLAB (Finite Element Method) can be used by
.P= space3d
to calculate the substrate resistances.
See the explanation of the
.P= makefem
program in the following section.
.N(
You can use FEMLAB instead of the
.P= makesubres
method, by setting the
.P= makesubres
symbolic link to the
.P= makefem
program.
.N)
.N(
The
.P= makefem
program can only be used with
.P= space3d
that supports the new "subres" file format.
.N)
.H 1 "THE MAKEFEM PROGRAM"
The
.P= makefem
program reads the substrate contacts and prepares input for using FEMLAB.
After running FEMLAB, the output of FEMLAB is converted to stream "subres".
For an overview see the following flow chart:
.P
.F+
.PSPIC "an0401/fig2.ps" 5i
.F-
.N(
The
.P= runfemlab
program is a shell script, that defines the
.P= matlab
run command.
The "startup.m" file is default read by the
.P= matlab
program and defines for
.P= matlab
what must be done.
Note that the "runfemlab" and "startup.m" file must exist in the current working directory.
.N)
In appendix A, a listing is given of a number of files.
As example, the cell "sub3term" is used
(see the "Space Substrate Res. Extraction User's Manual").
Note that the ``image part'' is also written to stream "subres".
These substrate capacitance values can be outputted by
.P= space3d
when option \fB-C\fP is used.
.P
The
.P= makefem
program creates for each substrate tile a 'rect2' or 'poly2' object.
A number of these tiles can be one substrate contact.
To get a better mesh (see appendix B), the tile objects must be joined together
with the union operator '+'.
Note that the internal borders must be removed with the 'geomdel' function.
However, also this mesh is not yet optimal, because points of internal borders are not removed from the contact boundary.
.P
Appendix B shows the mesh with internal boundary, with and without extra border point.
.br
Appendix C gives a flow chart about how the
.P= makefem
program can be tested.
.br
Appendix D shows the output for one big substrate contact of cell "coilgen".
.H 1 "MAKEFEM OPTIONS"
Command line:
.fS I
makefem [-e \fIdef\fP | -E \fIfile\fP] [-p \fIdef\fP | -P \fIfile\fP] [-S \fIparam\fP=\fIvalue\fP] [-hiv] cell
.fE
Options explanation:
.DS 2
.TS
box, tab(|);
l l.
\fB-e,-E\fP|for specifying a technology file (see \fIspace\fP)
_
\fB-p,-P\fP|for specifying a parameter file (see \fIspace\fP)
_
\fB-S\fP|for specifying additional parameters (see \fIspace\fP)
_
\fB-h\fP|for getting help info
_
\fB-i\fP|for statistical information
_
\fB-v\fP|for setting verbose mode
.TE
.DE
.nr Ej 0
.H 1 "MAKEFEM PARAMETERS"
.H 2 "fem.param_verbose (default: off)"
Prints something verbose about read parameters.
.H 2 "fem.verbose (default: off)"
Verbose mode.
Equal to option
.B -v .
.H 2 "fem.print_time (default: off)"
Prints the used time of modules of the
.P= makefem
program.
.H 2 "elim_sub_node (default: off)"
Can be put "on" to eliminate the substrate node "SUBSTR".
.H 2 "fem.xoverlap (default: 20)"
Cell boundary x-overlap in microns (must be \(>= 0).
.H 2 "fem.yoverlap (default: 20)"
Cell boundary y-overlap in microns (must be \(>= 0).
.H 2 "fem.zbottom (default: 20)"
The deepth in microns of the substrate.
The position of the substrate backcontact (must be > 0).
The deepth must be greater than the specified layers.
.H 2 "fem.backcontact (default: on)"
Can be put "off", when you don't want to add a backside contact.
.H 2 "fem.cs_mask (no default)"
To generate a channelstop area,
the channelstop mask must be specified.
The program reads the channelstop area from the cell \fImask\fP_gln stream.
.H 2 "fem.cs_bbox (no default)"
This bounding box can be used, when the channelstop mask has infinity gln edges.
This parameter has four arguments (xl xr yb yt) and must
be specified in database units.
.H 2 "fem.cs_extension (default: 0)"
When the channelstop mask has infinity gln edges, the cell bounding box is
used to give the channelstop mask a finity dimension.
Parameter "fem.cs_extension" can be used to change the dimension and must
be specified in database units.
The parameter is not used, when parameter "fem.cs_bbox" is specified.
.H 2 "fem.cs_sigma (default: 0.0)"
The channelstop sigma is default 100 times the sigma of the first sublayer.
.H 2 "fem.cs_thickness (default: 0.5 micron)"
The thickness of the channelstop area (must be > 0).
.H 2 "sub3d.makefem (default: off)"
For the
.P= space3d
program.
To use
.P= makefem
in place of the
.P= makesubres
program.
.nr Ej 1
.H 1 "MAKEFEM ARRAY"
The
.P= makefem
array is one bigger than the number of substrate contacts,
because one big backcontact is added to the number of contacts.
This backcontact is eliminated with Gaussian elimination.
The backcontact is the last entry in the array.
The values on the diagonal from left-top to bottom-right are positive.
The other values are all negative (see the "femsub.out" file).
The mean value of each pair of values is taken and the sign is changed.
The new value is put in the upper-right array part.
.fS I
g0,1 = -(g0,1 + g1,0) / 2
.fE
See also the array pictures below.
.P
.F+
.PSPIC "an0401/fig3.ps" 5i
.F-
.N(
The calculated substrate capacitance values are divided by 2*pi.
.N)

The diagonal values must also be corrected, before writing to the "subres" file.
.br
Thus:
.fS I
g0,0 -= (g0,1 + g0,2 + g0,3)
g1,1 -= (g0,1 + g1,2 + g1,3)
g2,2 -= (g0,2 + g1,2 + g2,3)
g3,3 -= (g0,3 + g1,3 + g2,3)
.fE
.H 1 "SUBSTRATE GEOMETRY PICTURE"
The substrate geometry of cell "sub3term" is specified in a MATLAB format
(see the "step1_geometry.m" file).
A picture of this substrate geometry is given below.
.P
.F+
.PSPIC "an0401/fig4.ps" 6i
.F-
.N(
The substrate contact "c1" starts at position x=0 and y=0.
.br
The substrate contact bounding box has an overlap of 20 microns.
.br
Contact "c4" is the additional backcontact.
.N)
.nr Ej 0

.H 1 "THE COILGEN EXAMPLE"
Depending on substrate parameters "sep_sub_term" and "sub_term_distr_\fImask\fP" the
files "cont_bln" and "subres" are different.
The
.P= makefem
program must change the number of tiles found in the "cont_bln" file
into the correct number of substrate contacts for the "subres" file.
.P
The following table gives some statistics:
.DS 2
.TS
box, tab(|);
l r r.
mode|number of tiles|number of contacts
_
sep_sub_term=off, not distr.|34|1
sep_sub_term=on, not distr.|36|4
sep_sub_term=on, distr_m5/m6|126|126
.TE
.DE

See appendix D for one result of the "coilgen" example (sep_sub_term=off, not distr.).
.nr Ej 1
.H 1 "MAKEFEM CONTACT BOUNDARY IMPROVEMENT"
As explained before (see appendix D),
there can be left extra points on the substrate contact boundaries after internal border deletion.
Thus, i implemented another method for the substrate border construction.
.P
Out of the substrate tiles a list of substrate border points is extracted.
This list is stored by each substrate contact.
After the substrate contacts are ready, the border points are outputted as big "poly2" polygons.
The "geomdel" function is not more needed, because internal borders don't need to be deleted.
If there are holes in a substrate contact, then there are extra internal polygons,
which must be substracted from the external border polygon.
The following figure explains the data structure added to a substrate contact.
.F+
.PSPIC "an0401/fig6.ps" 5i
.F-
As seen in the example, parts of the external border polygon may be stored
in separate boundary lists.
And must be put together, when finished.
.P
Appendix E gives a dump of the data structure for the boundary of cell "coilgen".
.br
Appendix F gives the results for the improved method for cell "coilgen".
.P
I added new code to
.P= makefem
source files "enumtile.c" and "input.c".
I added three new functions, (1) init_boundary, (2) update_boundary and (3) merge_boundaries.
The functions are called by enumPair.
Function init_boundary is called for each new boundary.
This init can only happen for a vertical edge.
Note that two points are also created for a vertical edge with a edge length of zero.
Function update_boundary can also call init_boundary, when update fails
to find the edge connection.
In that case, it must be the start of an internal boundary.
Function merge_boundaries is called, when two substrate contacts are joined together.
.SK
.H 2 "Special Contact Boundary Cases"
When two substrate contacts touch each other, but are not joined together,
then they can add an extra point on the boundary of each other.
Also two substrate contacts are never put together, when they touch each other
with a single point.
See as an example the figure below:
.F+
.PSPIC "an0401/fig7.ps" 4i 2i
.F-
In this special case ends the boundary of substrate contact 1 on a vertical edge.
Thus, the boundary of substrate contact 1 is constructed with 5 points.
This is not repaired on output.
It is not important, because the point of substrate contact 2 is also touching the contour of substrate contact 1.
.P
The boundary of substrate contact 2 starts with a vertical edge, which edge length is zero.
In this special case the init_boundary must be done (see enumPair), when on both sides
of the zero vertical edge is a substrate contact tile.
However, this does not happen (as expected) in the case below:
.F+
.PSPIC "an0401/fig8.ps" 4i 2i
.F-
The enumPair's 1 to 3 are all done for a zero length vertical edge.
The edges 'a' and 'b' are done first by a call to tileDeleteEdge.
The last one gives enumPair(tile1,tile3).
.br
New edge 'c' is not bundled with edge 'b', because edge 'b' xr is equal to thisX.
.br
The tileInsertEdge of edge 'c' gives enumPair(tile2,tile3) and the tileInsertEdge of
.br
edge 'd' gives enumPair(tile2,tile4).
.H 1 "APPENDICES"
.SP
.nr Hu 2
.HU "APPENDIX A -- List of files"
.nf
femtech.in
.sp -.4
==============
.S 8
.ft C
#contacts
contacts 3
backcontact 1
#subdomain
layers 2
epsilon 11.7 11.7
sigma 6.7 2000
#frequence
frequence 1
omega 1e6
.ft
.S

step1_geometry.m
.sp -.4
==============
.S 8
.ft C
% New geometry 1
fem.sdim={'x','y','z'};

% Geometry of substrate contacts (in meters)
wp=[0 1 0;0 0 1;0 0 0];
s1=rect2(0,1e-6,0,1e-6);
s2=rect2(15e-7,35e-7,3e-6,4e-6);
s3=rect2(4e-6,5e-6,0,1e-6);
c1=embed(s1,wp);
c2=embed(s2,wp);
c3=embed(s3,wp);

% Geometry of substrate blocks (in meters)
% cell bbox: 0 5e-6 0 4e-6
% subc bbox: 0 5e-6 0 4e-6
xl=0;
xr=5e-6;
yb=0;
yt=4e-6;
x=2e-5; % delta x overlap
y=2e-5; % delta y overlap
z=2e-5; % delta z to z-bottom

% bottom plane also has to be defined as separate contact
c4=embed(rect2(xl-x,xr+x,yb-y,yt+y),[0 1 0;0 0 1;-z -z -z]);

b1=block3(xr-xl+2*x,yt-yb+2*y,7e-6,'corner',[xl-x yb-y -7e-6]);
b2=block3(xr-xl+2*x,yt-yb+2*y,z-7e-6,'corner',[xl-x yb-y -z]);

clear s f c p
s.objs={b1,b2};
s.name={'B1','B2'};
f.objs={c1,c2,c3,c4};
f.name={'C1','C2','C3','C4'};
c.objs={};
c.name={};
p.objs={};
p.name={};
drawstruct=struct('s',s,'f',f,'c',c,'p',p);
fem.draw=drawstruct;

fem.geom=geomcsg(fem);
.ft
.S
.SK
femsub.out
.sp -.4
==============
.S 8
.ft C
frequence 1.000000e+08 
real part
 2.934393e-05 -3.443189e-06 -2.061320e-06 -2.383258e-05
 -3.443059e-06 3.969221e-05 -3.421877e-06 -3.280838e-05
 -2.061767e-06 -3.422627e-06 2.954846e-05 -2.415300e-05
 -2.383914e-05 -3.282668e-05 -2.406534e-05 8.072944e-05
image part
 3.026643e-08 -3.711542e-09 -2.237690e-09 -2.253931e-08
 -3.677760e-09 4.089951e-08 -3.661541e-09 -2.809899e-08
 -2.241087e-09 -3.693202e-09 3.047047e-08 -2.126881e-08
 -2.427372e-08 -3.341893e-08 -2.450418e-08 8.219131e-08
.ft
.S
.sp .5
subres
.sp -.4
==============
.S 8
.ft C
c 1 1 xl 0 yb 0 g 4.125942e-05 c 6.739797e-09
nr_neigh 2
c 2 3 xl 60 yb 120 g 5.609761e-05 c 9.036076e-09
nc 1 g 8.286918e-06 c 1.307583e-09
nr_neigh 2
c 3 7 xl 160 yb 0 g 4.160057e-05 c 6.729553e-09
nc 1 g 5.620003e-06 c 8.918057e-10
nc 2 g 8.321587e-06 c 1.288846e-09
nr_neigh 2
.ft
.S
.sp .5
cont_bln
.sp -.4
==============
.S 8
.ft C
% dbcat -vs cont_bln sub3term
=> layout/sub3term/cont_bln
  xl:   xr:   yb:   yt:   ct:
    0    40     0     0     0 (BN 0)
    0    40    40    40  1024 (EN 0)
   60   140   120   120     0 (BN 0)
   60   140   160   160  1024 (EN 0)
  160   200     0     0     0 (BN 0)
  160   200    40    40  1024 (EN 0)
.ft
.S
.sp .5
runfemlab
.sp -.4
==============
.S 8
.ft C
#
matlab -nodisplay -nojvm > /dev/null
.ft
.S
.sp .5
startup.m
.sp -.4
==============
.S 8
.ft C
main;
quit;
.ft
.S
.sp .5
main.m
.sp -.4
==============
.S 8
.ft C
# FEMLAB algorithm implemented by W.Qiang
.ft
.S
.SK
.HU "APPENDIX B -- Three different mesh results"
.F+
.PSPIC "an0401/mesh2.ps" 5i 3i
.F-
.HU "APPENDIX C -- Testing makefem"
.F+
.PSPIC "an0401/fig5.ps" 5i
.F-
In place of using
.P= matlab ,
we can generate
.P= matlab
output with the
.P= sub2out
program
to test the working of the
.P= makefem
program.
Result subres (1) must be equal to subres (2).
.SK
.HU "APPENDIX D -- Example of coilgen"
.nf
.S 8
.ft C
% New geometry 1
fem.sdim={'x','y','z'};

% Geometry of substrate contacts (in meters)
wp=[0 1 0;0 0 1;0 0 0];
s1=rect2(-69e-6,-54e-6,-17e-6,-1e-5);
s2=rect2(-69e-6,-54e-6,1e-5,17e-6);
s3=poly2([-54e-6 -54e-6 -48e-6 -48e-6],[-2237e-8 -1e-5 -1e-5 -2837e-8]);
s4=poly2([-54e-6 -54e-6 -48e-6 -48e-6],[1e-5 2237e-8 2837e-8 1e-5]);
s7=poly2([-45e-6 -45e-6 -4e-5 -4e-5],[-1864e-8 1864e-8 2364e-8 -2364e-8]);
s5=poly2([-48e-6 -48e-6 -2237e-8 -2237e-8],[-2837e-8 -1988e-8 -4551e-8 -54e-6]);
s6=poly2([-48e-6 -48e-6 -2237e-8 -2237e-8],[1988e-8 2837e-8 54e-6 4551e-8]);
s10=poly2([-2237e-8 -2237e-8 -1988e-8 -1988e-8],[-54e-6 -4551e-8 -48e-6 -54e-6]);
s11=poly2([-2237e-8 -2237e-8 -1988e-8 -1988e-8],[4551e-8 54e-6 54e-6 48e-6]);
s8=poly2([-4e-5 -4e-5 -1864e-8 -1864e-8],[-2364e-8 -1657e-8 -3793e-8 -45e-6]);
s9=poly2([-4e-5 -4e-5 -1864e-8 -1864e-8],[1657e-8 2364e-8 45e-6 3793e-8]);
s14=poly2([-1864e-8 -1864e-8 -1657e-8 -1657e-8],[-45e-6 -3793e-8 -4e-5 -45e-6]);
s15=poly2([-1864e-8 -1864e-8 -1657e-8 -1657e-8],[3793e-8 45e-6 45e-6 4e-5]);
s16=rect2(-1657e-8,1657e-8,-45e-6,-4e-5);
s17=rect2(-1657e-8,1657e-8,4e-5,45e-6);
s18=poly2([1657e-8 1657e-8 1864e-8 1864e-8],[-45e-6 -4e-5 -3793e-8 -45e-6]);
s19=poly2([1657e-8 1657e-8 1864e-8 1864e-8],[4e-5 45e-6 45e-6 3793e-8]);
s12=rect2(-1988e-8,1988e-8,-54e-6,-48e-6);
s13=rect2(-1988e-8,1988e-8,48e-6,54e-6);
s22=poly2([1988e-8 1988e-8 2237e-8 2237e-8],[-54e-6 -48e-6 -4551e-8 -54e-6]);
s23=poly2([1988e-8 1988e-8 2237e-8 2237e-8],[48e-6 54e-6 54e-6 4551e-8]);
s20=poly2([1864e-8 1864e-8 4e-5 4e-5],[-45e-6 -3793e-8 -1657e-8 -2364e-8]);
s21=poly2([1864e-8 1864e-8 4e-5 4e-5],[3793e-8 45e-6 2364e-8 1657e-8]);
s26=poly2([4e-5 4e-5 4261e-8 4261e-8],[-2364e-8 -226e-8 35e-8 -2103e-8]);
s27=poly2([4e-5 4e-5 4261e-8 4261e-8],[296e-8 2364e-8 2103e-8 35e-8]);
s28=poly2([4261e-8 4261e-8 45e-6 45e-6],[-2103e-8 2103e-8 1864e-8 -1864e-8]);
s29=poly2([45e-6 45e-6 4615e-8 4615e-8],[-574e-8 504e-8 389e-8 -459e-8]);
s30=poly2([4615e-8 4615e-8 4685e-8 4685e-8],[-459e-8 389e-8 459e-8 -389e-8]);
s24=poly2([2237e-8 2237e-8 48e-6 48e-6],[-54e-6 -4551e-8 -1988e-8 -2837e-8]);
s31=poly2([4685e-8 4685e-8 48e-6 48e-6],[-389e-8 459e-8 574e-8 -504e-8]);
s25=poly2([2237e-8 2237e-8 48e-6 48e-6],[4551e-8 54e-6 2837e-8 1988e-8]);
s32=poly2([48e-6 48e-6 5039e-8 5039e-8],[-2837e-8 2837e-8 2598e-8 -2598e-8]);
s33=poly2([5039e-8 5039e-8 54e-6 54e-6],[-2598e-8 -35e-8 -396e-8 -2237e-8]);
s34=poly2([5039e-8 5039e-8 54e-6 54e-6],[-35e-8 2598e-8 2237e-8 326e-8]);
c1=embed(geomdel(s34+s33+s25+s23+s13+s11+s6+s4+s2+s31+s30+s29+s28+s27+s26+s21+ ...
                 s20+s19+s18+s17+s16+s15+s14+s9+s8+s7+s32+s24+s22+s12+s10+s5+s3+s1),wp);

% Geometry of substrate blocks (in meters)
xl=-69e-6; xr=54e-6; yb=-54e-6; yt=54e-6;
x=2e-5; % delta x overlap
y=2e-5; % delta y overlap
z=2e-5; % delta z to z-bottom

% bottom plane also has to be defined as separate contact
c2=embed(rect2(xl-x,xr+x,yb-y,yt+y),[0 1 0;0 0 1;-z -z -z]);

b1=block3(xr-xl+2*x,yt-yb+2*y,z,'corner',[xl-x yb-y -z]);

clear s f c p
s.objs={b1};    s.name={'B1'};
f.objs={c1,c2}; f.name={'C1','C2'};
c.objs={};      c.name={};
p.objs={};      p.name={};
drawstruct=struct('s',s,'f',f,'c',c,'p',p);
fem.draw=drawstruct;

% This is for visual feedback:
fem.geom=geomcsg(fem);

.ft
.S
.F+
.PSPIC "an0401/coil2.ps" 4.5i
.F-
.F+
.PSPIC "an0401/coil.ps" 4.5i
.F-
.SK
.ce
The following picture shows the border points for cell "coilgen".
.ce
As you can seen, there are a number of extra points.

.F+
.PSPIC "an0401/extra.ps" 5i
.F-
.SK
.HU "APPENDIX E -- Dump of boundary points for coilgen"
.nf
.S 8
.ft C
pt: 54000 3260     <------ boundary 1 : top
pt: 54000 22370
pt: 22370 54000
pt: -22370 54000
pt: -54000 22370
pt: -54000 17000
pt: -69000 17000
pt: -69000 10000
pt: -48000 10000
pt: -48000 19880
pt: -19880 48000
pt: 19880 48000
pt: 48000 19880
pt: 48000 5740
pt: 46150 3890
pt: 45000 5040
pt: 45000 18640
pt: 18640 45000
pt: -18640 45000
pt: -45000 18640
pt: -45000 -18640
pt: -18640 -45000
pt: 18640 -45000
pt: 45000 -18640
pt: 45000 -5740
pt: 46850 -3890
pt: 48000 -5040
pt: 48000 -19880
pt: 19880 -48000
pt: -19880 -48000
pt: -48000 -19880
pt: -48000 -10000
pt: -69000 -10000
pb: 54000 -3960    <------ boundary 1 : bottom
pb: 54000 -22370
pb: 22370 -54000
pb: -22370 -54000
pb: -54000 -22370
pb: -54000 -17000
pb: -69000 -17000
-----------------
pt: 42610 350      <------ boundary 2 : top
pt: 40000 2960
pt: 40000 16570
pt: 16570 40000
pt: -16570 40000
pt: -40000 16570
pb: 42610 350      <------ boundary 2 : bottom
pb: 40000 -2260
pb: 40000 -16570
pb: 16570 -40000
pb: -16570 -40000
pb: -40000 -16570
-----------------
pt: 54000 3260     <------ boundary 3 : top
pt: 50390 -350
pb: 54000 -3960    <------ boundary 3 : bottom
pb: 50390 -350
.ft
.S
.SK
.HU "APPENDIX F -- Results for the improved coilgen"
.nf
.S 8
.ft C
% New geometry 1
fem.sdim={'x','y','z'};

% Geometry of substrate contacts (in meters)
wp=[0 1 0;0 0 1;0 0 0];
s1=poly2([-69e-6 -48e-6 -48e-6 -1988e-8 1988e-8 48e-6 48e-6 4685e-8 45e-6 45e-6 ...
 1864e-8 -1864e-8 -45e-6 -45e-6 -1864e-8 1864e-8 45e-6 45e-6 4615e-8 48e-6 ...
 48e-6 1988e-8 -1988e-8 -48e-6 -48e-6 -69e-6 -69e-6 -54e-6 -54e-6 -2237e-8 ...
 2237e-8 54e-6 54e-6 5039e-8 54e-6 54e-6 2237e-8 -2237e-8 -54e-6 -54e-6 -69e-6], ...
[-1e-5 -1e-5 -1988e-8 -48e-6 -48e-6 -1988e-8 -504e-8 -389e-8 -574e-8 -1864e-8 ...
 -45e-6 -45e-6 -1864e-8 1864e-8 45e-6 45e-6 1864e-8 504e-8 389e-8 574e-8 ...
 1988e-8 48e-6 48e-6 1988e-8 1e-5 1e-5 17e-6 17e-6 2237e-8 54e-6 ...
 54e-6 2237e-8 326e-8 -35e-8 -396e-8 -2237e-8 -54e-6 -54e-6 -2237e-8 -17e-6 -17e-6]);
s2=poly2([-4e-5 -1657e-8 1657e-8 4e-5 4e-5 4261e-8 4e-5 4e-5 1657e-8 -1657e-8 -4e-5], ...
[1657e-8 4e-5 4e-5 1657e-8 296e-8 35e-8 -226e-8 -1657e-8 -4e-5 -4e-5 -1657e-8]);
c1=embed(s1-s2,wp);

% Geometry of substrate blocks (in meters)
% cell bbox: -69e-6 54e-6 -54e-6 54e-6
% subc bbox: -69e-6 54e-6 -54e-6 54e-6
xl=-69e-6;
xr=54e-6;
yb=-54e-6;
yt=54e-6;
x=2e-5; % delta x overlap
y=2e-5; % delta y overlap
z=2e-5; % delta z to z-bottom

% bottom plane also has to be defined as separate contact
c2=embed(rect2(xl-x,xr+x,yb-y,yt+y),[0 1 0;0 0 1;-z -z -z]);

b1=block3(xr-xl+2*x,yt-yb+2*y,z,'corner',[xl-x yb-y -z]);

clear s f c p
s.objs={b1};
s.name={'B1'};
f.objs={c1,c2};
f.name={'C1','C2'};
c.objs={};
c.name={};
p.objs={};
p.name={};
drawstruct=struct('s',s,'f',f,'c',c,'p',p);
fem.draw=drawstruct;

% This is for visual feedback:
fem.geom=geomcsg(fem);
.ft
.S

========================================
Mesh statistics of FEMLAB:
.TS
box, tab(|);
l r r.
method|nodes|elements
_
old|3921|16598
new|2220|9560
.TE
.SK
.F+
.PSPIC "an0401/bla1.ps" 5i 10c
.F-
.F+
.PSPIC "an0401/bla2.ps" 5i 10c
.F-
