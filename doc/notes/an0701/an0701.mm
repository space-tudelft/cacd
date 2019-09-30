.T= "Using and testing tabs"
.DS 2
.rs
.sp 1i
.B
.S 15 20
Using
and
Testing Tabs
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
Report EWI-ENS 07-01
.ce
Sep 25, 2007
.sp |9i
.DS
.in +5
.S 9
Copyright \s+3\(co\s-3 2007 by the author.

Last revision: Nov 20, 2007.
.S
.in -5
.DE
.SK
.S=
.H 1 "INTRODUCTION"
The
.P= tabs
program (see manual page: technology abstraction tool)
is used to generate 2D capacitance data for the
.P= space
technology file.
.P
.P= Tabs
uses the more accurate 3D capacitance extraction method with a large number of basic cells
to calculate these 2D capacitance data.
Thus, the specified technology file needs to contain 3D technology data
(the vdimension list and dielectric structure).
.P
.P= Tabs
is a tcl-script and must be started in a project directory.
The ICDPATH environment variable is used to find the system technology directory (if needed).
Thus, the following command line can be used:
.fS I
% tabs
.fE
In this case the default technology file "space.def.s" is used,
of which the place is specified in the project ".dmrc" file.
When this file is in a system directory, it can possible not be changed.
.br
Thus, more likely,
you specify a technology file on the command line (for example "tech.s"):
.fS I
% tabs -s tech.s
.fE
.P= Tabs
uses a nominal_spacing to work with.
The default nominal_spacing is equal to your project lambda value.
If you want to use another nominal_spacing,
you must specify the
.B -S
option, like this:
.fS I
% tabs -S 0.5 -s tech.s
.fE
The value must be specified in microns.
.P
Use option
.B -i
when there are already capacitance 2D rules in the technology file.
In that case, they are ignored (not removed).
.P
Normally,
.P= tabs
is calculating all capacitances between all conductors found in the vdimensions.
The
.B -L
option can be used to reduce this behaviour.
.P
Note that
.P= tabs
uses a cache, it remembers what is calculated.
.P
Note that the new version of
.P= tabs
can handle more than 3 dielectric layers.
.TS
box;
l l l l l.
precision	be_window_factor	max_be_area_factor	be_mode	max_complexity
_
low	1.0	4.0	0c	100
medium	2.0	4.0	0g	500
high	2.0	4.0	0g	1000
veryhigh	5.0	4.0	1g	1500
.TE
The default precision, which
.P= tabs
is using, is "medium" (see option \fB-p\fP).
The "cap3d.be_window" is equal to the max. feature size multiplied by be_window_factor.

.H 1 "NEW VERSION OF TABS"

The new version of
.P= tabs
does its work more secure for edge capacitance calculations.
It uses another (larger) default edge ratio.
And it does more iteration steps to find the best last value going to the infinity case.

.H 2 "Other changes are:"

The directory structure in /tmp is changed.
There is now only used one project directory for all cap3d test cells.
All cells use now the same technology file.
Which only needs to be compiled ones.
The technology compiler
.P= tecc
is used two times.
The first time, to get the tcl-table with technology data to be used.
The
.P= tabs
program generates another technology file to be used.
The second time,
.P= tecc
is used to compile this generated technology file.
When there are more than 3 dielectric interfaces,
.P= tecc
is requested to used the unigreen method (using the option \fB-u\fP).
Note that the new version of
.P= tecc
uses a cache for the unigreen technology files.
Thus, if this is a known case, not much needs to be done.
However, if it is the first time, it can cost more than 3 hours to
generate the unigreen technology files.
Note, don't edit your technology file to skip some vdimensions.
This is important information for the technology compiler to use
some fixed points in the z-direction.
There is a new
.P= tabs
option (\fB--skip-vdim\fP) to skip some vdimension names.

.H 2 "Other new options are:"

Option \fB-V\fP (or \fB--verboser\fP) to put on
.P= space3d
verbosity.
Note that the normal verbosity mode is become less verbose.
.P
Option \fB-R\fP (or \fB--edge-ratio\fP) to specify another edge ratio
for the calculation of edge capacitances.
.P
A number of \fB--skip\fP options, to skip for example the lateral
capacitance generation part.

.H 2 "TESTING TABS"

The changes are made to
.P= tabs
after trying out some test cases.
This tests are not incorporated in this report yet.

You can run your own tests to compare the results of a cap3d extraction
with a cap2d extraction.
There is a
.P= tabs-verify
program (tcl-script) which can be used to do that.
