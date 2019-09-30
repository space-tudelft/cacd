.T= "Space Application Note About Options & Parameters"
.DS 2
.rs
.sp 1i
.B
.S 15 20
SPACE APPLICATION NOTE
ABOUT
OPTIONS & PARAMETERS
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
Report EWI-ENS 03-06
.ce
October 12, 2003
.sp |9i
.DS
.in +5
.S 9
Copyright \s+3\(co\s-3 2003 by the author.

Last revision: January 10, 2011.
.S
.in -5
.DE
.SK
.S=
.H 1 "INTRODUCTION"
The
.P= space
program has many options you can use on the command line.
This application note gives an overview of all command line options.
An describes each option in more detail.
.P
The
.P= space
program has also many parameters, you can set or change.
An individual parameter can also be set with option
.O= -S
on the command line.
Note that some parameters are also an option.
The parameters are normally specified in a parameter file.
Each technology has a default parameter file "space.def.p".
You can overwrite this default by putting a ".space.def.p" in your project.
An alternative technology parameter file "space.\fIxxx\fP.p" can be
chosen with option
.O= -p "" xxx .
You can also specify a parameter file with option
.O= -P "" pfile .

.H 2 "Space Program Names"
.DS C
.TS
box;
l|l.
Name	Description
_
space	Public domain 2D extractor.
	A subset of the licensed version.
_
space	The licensed 2D extractor.
	No 3D options -3BU (-%1) and -X.
_
space3d	The licensed 3D extractor.
	Executable without Xspace code.
_
Xspace	The licensed 3D extractor display tool.
	Runs the space3d extractor.
_
subspace	Substrate version.
_
makesubres	Substrate 3D special prepass.
_
makesubcap	Substrate 3D special prepass.
.TE
.DE

.H 2 "Space Program Arguments"
Space invocation requires one or more program arguments:
.fS
    space [options] cell ...
.fE
By space you must specify one or more cellnames,
however, by Xspace you can use only one (optional) cellname:
.fS
    Xspace [options] [cell]
.fE
.H 1 "SPACE PUBLIC VERSION"
The public domain space 2D version invocation is:
.fS
    space [-cFTIntihuvx] [-a time] [-D min_depth] [-E file | -e def]
       [-P file | -p def] [-S param=value] cell ...
.fE
\fBKeywords\fP:
.br
evaluation version,
no license needed,
no couple capacitance extraction,
no resistance extraction.

.nr Ej 0
.H 1 "SPACE NON PUBLIC VERSION"
.nf
The licensed space 2D version invocation is:
.fS
    space [-cClrzjkGbFTIntihuvx] [-a time]
       [-D min_depth] [-E file | -e def]
       [-P file | -p def] [-S param=value] cell ...
or
    space [-%RfgqwZ02] [-%L max_depth]
       [-cClrzjkGbFTIntihuvx] [-a time]
       [-D min_depth] [-E file | -e def]
       [-P file | -p def] [-S param=value] cell ...
.fE
\fBKeywords\fP:
.fi
couple and lateral capacitance extraction,
interconnect and 2D substrate resistance extraction,
moments, selective node elimination (SNE).

.H 1 "SPACE 3D VERSION"
The space 3D version is a non public version of space,
and must be used with a license.
This version supports all options and the display mode.
.nf
The space3d version invocation is:
.fS
    space3d [-cCl3UrzjkGbBFTIXntihuvx] [-a time]
       [-D min_depth] [-E file | -e def] [-s scene_file]
       [-P file | -p def] [-S param=value] cell ...
or
    space3d [-%RfgqwZ02] [-%L max_depth]
       [-cCl3UrzjkGbBFTIXntihuvx] [-a time]
       [-D min_depth] [-E file | -e def] [-s scene_file]
       [-P file | -p def] [-S param=value] cell ...
.fE
\fBKeywords\fP:
.fi
3D capacitance extraction,
3D substrate resistance extraction,
display functions.

See the following sections for a description of all used options.
.nr Ej 1
.H 1 "OPTIONS: Misc."
.S 10
.DS C
.TS
box;
l|l|l|l.
Op.	Code	Used Variable	Option Description
_
%	S	optSpecial	Must be specified to use special options.
0	S	skipPrePass0	Skip the mesh refinement prepass (see -z).
1	S	optOnlyPrePassB1	Do only the special -B prepass.
			Only used to start the special prepass program.
			No preprocessing is done (see -u).
			No last pass is done (see -Z).
			OBSOLETE in Version 5.4.3
2	S	optOnlyLastPass	Do only the last extract pass.
			No preprocessing is done (see -u).
			No prepasses are done (do not use -Z).
3	3	optCap3D	For accurate (3D) cap extraction.
			In combination with -C or -c (don't use -l).
			Sets the flat extraction mode (and optPseudoHier=0),
			when parameter "allow_hierarchical_cap3d" is "off"
			and option -H is not specified.
.TE
.DE
.S

\fBCodes\fP: 3=CAP3D/SUB3D, B=SUB_RES, C=SCENE, D=DISPLAY, E=SNE,
.br
         S=SPECIAL, I=INT_RES, M=MOMENTS, O=OPTEM, P=PUBLIC

\fBNotes\fP:
.br
Options followed with a colon ':' must have an additional argument.
A space between the option and that argument may be used, but is not required.
.br
Variable optIntRes is set for all types of interconnect resistance extractions.
Special options can only be used, when the first option argument starts with a percent sign '%'.
.H 1 "OPTIONS: A-Z"
.S 10
.DS C
.TS
box;
l|l|l|l.
Op.	Code	Used Variable	Option Description
_
A	-	-	Unused.
B	B3	optSubRes	For accurate (3D) substrate res extraction.
			Sets the flat extraction mode (and optPseudoHier=0),
			when parameter "allow_hierarchical_subres" is "off".
C	!P	optCoupCap	For couple cap extraction (see also -c).
D:	P	optMinDepth	Minimum extraction depth (ignored with -F).
E:	P	techFile	To specify a technology file (see also -e).
F	P	optFlat	Use flat extraction mode (default is hierarchical).
			Options -D and -I are ignored (optMinDepth=1).
G	EM	optSelectiveElimination	Use (-r or -z) and (-c or -C or -l).
H	S	optHier3D	Do hierarchical (3D) cap extraction by running
			program "master3d" (calls "decom", "space3d", etc.).
			OBSOLETE in Version 5.4.3
I	P	optMinDepth=INF	Use infinite extraction depth (ignored with -F).
J	S	optDsConJoin	Separate d/s boundaries for transistors.
			OBSOLETE in Version 5.4.3
K	S	optNoCore	Don't use the cell core.
			Adds the character 'K' before cellname.
			OBSOLETE in Version 5.4.3
L:	S	optMaxDepth	Maximum cell extraction depth.
M	S	optReadNetTerm	Use hierarchical net terminals.
			Write a "nethier" file on output.
			OBSOLETE in Version 5.4.3
N	S	optGenNetTerm	Write a "netterm" file on output.
			OBSOLETE in Version 5.4.3
O	S	optPlotCir	Write a "cir.pic" plotfile on output.
			OBSOLETE in Version 5.4.3
P:	P	paramFile	To specify a parameter file (see also -p).
Q	-	-	Unused.
R	S	optAllRes	Do interconnect res extraction, for all conductors
			with a sheet res > 0 ohm.
S:	P	paramSetOption	Set parameter(s) on command line.
T	P	optMaxDepth=1	Extract only the top cell (see also -L).
U	3	optEstimate3D	No calculation of Green's functions and no Schur
			matrix inversion (parameter "disp.be_mesh_only").
V	B3	optMatrixInfo	Prints a Schur matrix and bandwidth message.
			Only used in the special substrate version.
W	-	-	Unused.
X	D	optDisplay	Enables the display mode for space3d.
			Xspace: omit menubar and execute cell directly.
		optNoMenus	Sets the flat extraction mode, when parameter
			"expand_connectivity" (optPseudoHier) is "off".
Y	-	-	Unused.
Z	S	optOnlyPrePass	Do only the needed prepasses.
			No last pass is done (do not use -2).
.TE
.DE
.S
.B NOTE:
Remember, that the default
.P= space
extraction mode is
.B hierarchical .
.N)
.H 1 "OPTIONS: a-z"
.S 10
.DS C
.TS
box;
l|l|l|l.
Op.	Code	Used Variable	Option Description
_
a:	P	optAlarmInterval	Give a stateruler message after AlarmInterval
			seconds (also parameter "progress_timer").
b	B	optSimpleSubRes	For simple substrate res extraction.
			Sets the flat extraction mode (and optPseudoHier=0),
			when parameter "allow_hierarchical_subres" is "off".
c	P	optCap	For grounded cap extraction (overruled by -C).
d	S	optNoDim	Don't output w/l attributes for transistors.
			OBSOLETE in Version 5.4.3
e:	P	techDef	To specify a technology file (see also -E).
f	S	optFineNtw	Fine network, no node elimination.
g	SME	optExtractMomentsInit	Use (-r or -z) and (-c or -C or -l).
h	P	optHelp	Give complete help see message.
i	P	optInfo & -v	Verbose mode + info.
j	I	optInvertPrick	Selective interconnect res extraction (don't use -k).
k	I	optPrick	Selective interconnect res extraction (don't use -j).
l	!P	optLatCap	To include lateral cap extraction (see -C).
m	S	optMonitor	Produce a "mon.out" file, see also prof(1).
			ONLY if compiled with MONITOR.
n	P	optNoReduc	Don't do the circuit reduction heuristics.
o	-	-	Unused.
p:	P	paramDef	To specify a parameter file (see also -P).
q	S	optPrintRecog	For printing recognized elements.
r	I	optIntRes	Do interconnect res extraction, for all conductors
			with a sheet res > low_sheet_res ohm (see also -R).
s:	C	sceneFilepath	For writing display scene to file.
t	P	optTorPos	Output transistor x,y attributes, parameter
			"component_coordinates" (also set with -x).
u	P	optNoPrepro	Don't do the preprocessing step.
			Programs makeboxl, makegln and makesize are not run.
			Preprocessing status must be correct for the cell.
v	P	optVerbose	Verbose mode.
w	S	optWriteRecogs	Print element recognition count summary.
x	P	optBackInfo	Give back-annotation information.
y	O	optNetInfo	Write net information to "wiredata" file.
			Can only be used with license model "optem".
z	I	optResMesh	Mesh refinement and interconnect res extraction.
.TE
.DE
.S
.H 1 "SPACE SUBSTRATE VERSION"
The space substrate version invocation is:
.fS
subspace -B [-vi -VX] [-a time] [-e def | -E file] [-s scene_file]
                          [-p def | -P file] [-S param=value] cell ...
.fE
For a short description of all possible options, see the options sections.
.br
By option -V, function schurStartMatrix prints the message:
.fS
* matrix dimension = <row+1>, maximum bandwidth = <col+1>
.fE
The notation <...> must be replaced by an integer number.
Note that function cap3dEnd prints an additional newline.

.nr Ej 0
.H 1 "SPACE 3D SUBSTRATE PREPASS PROGRAM"
The space 3D substrate special prepass program invocation is:
.fS
space3d -%1 [-vi -X] [-e def | -E file] [-s scene_file]
                     [-p def | -P file] [-S param=value] [-Uhv] cell ...

    OBSOLETE in Version 5.4.3
.fE
or:
.fS
makesubres [-e def | -E file] [-p def | -P file] [-S param=value] [-Uhv] cell
.fE
For a short description of all possible options, see the options sections.
The same options are used for the
.P= makesubcap
program.
.nr Ej 1
.H 1 "SPACE PARAMETERS"
.H 2 "Parameter Blocks"
There are parameters for all kind of purposes.
There are parameters for general usage,
but also parameters aspecial for debugging.
And there are parameters for 3D capacitance extraction.
Parameters for some category can be placed in a parameter block for that specific category
or else can only be specified with the category name in front of it, for example:
.fS
    cap3d.be_window 10
\fRor\fP
    BEGIN cap3d
    be_window 10
    END
.fE
The following parameter blocks are possible:
.DS I
.TS
box;
l|l.
category	purpose
_
cap3d	capacitance 3D extraction
debug	space debugging
disp	display setting (Xspace)
fem	substrate extraction (makefem)
moments	cap moments extraction
scene	scene picture storage
schur	schur matrix inversion
sne	selective node elimination
sub3d	substrate res 3D extraction
subcap3d	substrate cap 3D extraction
.TE
.DE

.H 2 "Parameter On Command Line"
Parameters given on the command line (with option
.B -S )
has precedence above the parameters given in a
.P= space
parameter file.
When given on the command line,
the parameter and (possibly) its value must be separated with a '=' sign.
.br
For example:
.fS
    -S cap3d.be_window=10
\fRor\fP
    -S "keep_conductors=cond_pg cond_pa"
.fE
