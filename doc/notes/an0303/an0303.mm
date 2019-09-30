.T= "Space Incorrect Variables Application Note"
.DS 2
.rs
.sp 1i
.B
.S 15 20
SPACE INCORRECT VARIABLES
APPLICATION NOTE
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
Report EWI-ENS 03-03
.ce
January 31, 2003
.sp |9i
.DS
.in +5
.S 9
Copyright \s+3\(co\s-3 2003 by the author.

Last revision: December 9, 2003.
.S
.in -5
.DE
.SK
.S=
.H 1 "INTRODUCTION"
.H 2 "Problem Description"
We must look out for incorrect set variables (parameters)
in the
.P= space
program.
It leads to unpredicted
.P= space
program behaviour.
As an example i describe the following found mistake.
.P
Function cap3dInitParam() is only called by extract()
when variable optCap3D is true.
.P
Function cap3dInitParam() sets the variable extractDiffusionCap3d.
When extractDiffusionCap3d is unset, its value is false.
This is also the default value, when parameter "cap3d.omit_diff_cap"
is not used.
.P
The variable is used by Cap3D extraction.
However, function recognizeElements() is using this variable in all
.P= space
program passes, when optCap3DSave is true.
.P
Variable optCap3DSave is true in all passes, but variable optCap3D not.
The variable optCap3DSave is created, to remember the initial optCap3D value.
It is set in function optionImplications().
Function optionImplications() is called before extractTree() is done.
This is also true for
.P= Xspace ,
the interactive version of the
.P= space
program.
.P
The values of variables optCap3D/optCap3DSave are set as follows:
.DS C
.TS
allbox;
l l l.
Pass	Value optCap3D	Value optCap3DSave
_
initial 	\fBTrue\fP (-3c | -3C)	True
prePass0	False	True
\fBprePass1\fP	False(-b)/\fBTrue\fP(-B)	True
prePass2	False	True
\fBextrPass\fP	\fBTrue\fP	True
.TE
.DE
.DS C
.TS
allbox;
l l l.
Pass	Value optCap3D	Value optCap3DSave
_
initial 	False	False
prePass0	False	False
\fBprePass1\fP	False(-b)/\fBTrue\fP(-B)	False
prePass2	False	False
extrPass	False	False
.TE
.DE

Thus, in prePass1 by option -b it happens that
variable extractDiffusionCap3d is not set (because
function cap3dInitParam() is not called) and
in the extrPass is set (because
parameter "cap3d.omit_diff_cap" is set to "off").
.SK
This gives a problem in prePass1, because the value
of variable extractDiffusionCap3d can be the first time False and
the second time True.
See the following table:
.DS C
.TS
allbox;
l l l l.
Pass	Value optCap3D	Value extractDiffusionCap3d	Param Value
_
initial 	True	False	"off"
prePass1	False	False
extrPass	True	True
prePass1	False	True
extrPass	True	True
_
initial 	True	False	"on"
prePass1	False	True
extrPass	True	False
prePass1	False	False
extrPass	True	False
.TE
.DE

By
.P= Xspace ,
it is even possible, that the parameter is set to "on" again
(by given the menu command of rereading the parameter file).
Than arises the opposite situation (see also above).
.P
Note that, when extractDiffusionCap3d is true, no 2D cap elements
are used at all.
Thus, in the first case, the prePass1 is using the 2D caps
and the extrPass is not using the 2D caps.
Thus there are substrate terminals for these 2D caps.
Note that in the other case, there are also substrate terminals
in the prePass1 by recent generated technology files,
because there are also subcont elements generated in the technology
file for all 2D caps connected to substrate.
These subcont elements are not skipped!
.P
Note that for option -B extractDiffusionCap3d is set to true in prePass1.
Thus, no 2D cap elements are seen in prePass1 for option -B.
Thus, by old technology files, also no substrate terminals for used
2D diffusion caps are made in prePass1.

.nr Ej 0
.H 1 "CONCLUSION"
Because variable extractDiffusionCap3d is used in combination
with variable optCap3DSave, it is important to set extractDiffusionCap3d
like optCap3DSave before all passes (before the call to extractTree).
.P
To handle the generation of substrate terminals for 2D caps in prePass1
correctly, we must add 2D cap information to the present subcont elements.
