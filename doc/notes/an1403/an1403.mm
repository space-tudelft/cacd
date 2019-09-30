.T= "Using Tabs"
.DS 2
.rs
.sp 1i
.B
.S 15 20
Using Tabs
.S
.sp 2
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
Report EWI-ENS 14-03
.ce
March 3, 2014
.sp |9i
.DS
.in +5
.S 9
Copyright \s+3\(co\s-3 2014 by the author.

Last revision: March 24, 2014.
.S
.in -5
.DE
.SK
.S=
.H 1 "INTRODUCTION"
The
.P= tabs
(technology abstraction) tool allows the user to automatically produce
2D capacitance extraction rules for the
.P= space
technology file.
This is very usefull,
because not always a 3D capacitance extraction is possible with the
.P= space3d
layout to circuit extraction program.
.P
The
.P= tabs
tool is made as a tcl-script, thus the source code is available,
and (if needed) can be changed by some more expert user.
The script is run by the
.P= cacdtcl
program, which is a
.P= tclsh
program with some build-in "cacd" database features.
.P
The
.P= tabs
tool needs to be run in the
.P= space
design environment with other tools
and the ICDPATH environment variable must be set.
.P
The
.P= tabs
tool reads a technology file with 3D capacitance extraction setup
and adds to this technology file rules for 2D capacitance extraction.
Besides the technology file also a technology "maskdata" file must be available.
The
.P= tabs
tool runs normally in a
.P= space
project directory.
Thus, it can find a "maskdata" file
and it shall default try to read the "space.def.s" technology file.
Examples:
.fS
% tabs
error: Directory `/home/simon/tmp' is not a project.
.fE
Ok, we change directory to a project directory and try again.
.fS
% cd proj
% tabs
error: Technology file `/home/simon/tmp/tech/space.def.s' is not writable.
.fE
Oeps, the example "scmos_n" technology file, we use, is not yet writable.
.fS
% chmod +w ../tech/space.def.s
% tabs
error: Technology file `/home/simon/tmp/tech/space.def.s' already
   contains `capacitance:' section. Use the `-i' option to ignore.
.fE
Of course, this "scmos_n" technology file contains already a "capacitance" section.
You see, that you may not easy change an existing technology file.
Maybe, you can better make a backup copy, before you try it again.
.fS
% cp ../tech/space.def.s ../tech/space.def.s_old
.fE
Of course, you can also copy the technology file to your project directory and run
.P= tabs
with the \fB-s\fP option:
.fS
% cp ../tech/space.def.s .
% tabs -i -s space.def.s
.fE
But we don't do this.
.br
See next page, we run
.P= tabs
using the technology directory.
.fS
% tabs -i
verbose: Computing vertical capacitances ...
progress: 1/268
+ space3d -3C -Scap3d.be_mode=0c -Scap3d.max_be_area=1490 \\
                                         -Scap3d.be_window=inf cell0001
+ space3d -3C -Scap3d.be_mode=0c -Scap3d.max_be_area=5958 \\
                                         -Scap3d.be_window=inf cell0002
progress: 2/268
 ...
progress: 132/268
+ space3d -3C -Scap3d.be_mode=0c -Scap3d.max_be_area=34.3 \\
                                         -Scap3d.be_window=inf cell0263
+ space3d -3C -Scap3d.be_mode=0c -Scap3d.max_be_area=34.3 \\
                                         -Scap3d.be_window=inf cell0264
progress: 133/268
+ space3d -3C -Scap3d.be_mode=0c -Scap3d.max_be_area=34.3 \\
                                         -Scap3d.be_window=inf cell0265
+ space3d -3C -Scap3d.be_mode=0c -Scap3d.max_be_area=34.3 \\
                                         -Scap3d.be_window=inf cell0266
verbose: Adding lateral capacitance rules to technology file ...
progress: 268/268
.fE
You see, that a lot of
.P= space3d
capacitance extraction runs are done to generate the 2D capacitance tables.
The
.P= tabs
tool has also written a summary file in the current working directory:
.fS
% cat tabs_summary.txt
tabs started at: Mon Mar 03 15:51:21 CET 2014
using techfile: /home/simon/tmp/tech/space.def.s
using maskdata: /home/simon/tmp/tech/maskdata
finished at: Mon Mar 03 15:51:37 CET 2014
.fE
But we want to know, what
.P= tabs
has changed into the "scmos_n" technology file:
.fS
% diff ../tech/space.def.s_old ../tech/space.def.s
114a115,294
> # GENERATED BEGIN: tabs-area-capacitance (edit this block at your own risk)
> # Added: Mon Mar 03 15:51:29 CET 2014
> ...
> # GENERATED BEGIN: tabs-edge-edge-capacitance (edit this block at your own risk)
> ...
> # GENERATED BEGIN: tabs-edge-surface-capacitance (edit this block at your own risk)
> # Added: Mon Mar 03 15:51:30 CET 2014
> # Precision: low (be_mode=0c error=0.1)
> # Edge-ratio: 10
> ...
> # GENERATED BEGIN: tabs-lateral-capacitance (edit this block at your own risk)
> # Added: Mon Mar 03 15:51:37 CET 2014
> # Precision: low (be_mode=0c error=0.1)
> # Edge-ratio: 10
> ...
> # GENERATED END
.fE
Thus,
.P= tabs
has not removed the old "capacitance" sections from the technology file!
.SK
We shall also copy the technology file to our current directory and see what happens:
.fS
% cp ../tech/space.def.s_old space.def.s
% tabs -s space.def.s -i
verbose: Computing vertical capacitances ...
progress: 1/268
progress: 2/268
progress: 3/268
progress: 4/268
progress: 5/268
 ...
progress: 268/268
.fE
He, the program
.P= space3d
is not more run!
This happens, because a cache directory is used.
We shall also perform a compare to see if this result is equal
to the result in the technology file directory.
.fS
% diff ../tech/space.def.s space.def.s
116c116
< # Added: Mon Mar 03 15:51:29 CET 2014
---
> # Added: Mon Mar 03 17:15:10 CET 2014
131c131
< # Added: Mon Mar 03 15:51:29 CET 2014
---
> # Added: Mon Mar 03 17:15:10 CET 2014
141c141
< # Added: Mon Mar 03 15:51:30 CET 2014
---
> # Added: Mon Mar 03 17:15:11 CET 2014
172c172
< # Added: Mon Mar 03 15:51:37 CET 2014
---
> # Added: Mon Mar 03 17:15:13 CET 2014
.fE
Yes, both results are equal!
We see only different time stamps.
.br
If we don't want to use the cache, we can disable the cache with option \fB--no-cache\fP.
.br
In that case
.P= space3d
is always run and no results are written to the cache.
.br
You can also disable the cache and test against the cache with option \fB--test-cache\fP.
.br
Note that my current cache path is "/home/simon/.cacd/cache/tabs4".
.fS
% tabs -s space.def.s -i --test-cache
 ...
progress: 133/268
+ space3d -3C -Scap3d.be_mode=0c -Scap3d.max_be_area=34.3 \\
                                         -Scap3d.be_window=inf cell0265
+ space3d -3C -Scap3d.be_mode=0c -Scap3d.max_be_area=34.3 \\
                                         -Scap3d.be_window=inf cell0266
verbose: Adding lateral capacitance rules to technology file ...
progress: 268/268
.fE
Does the resulting technology file now contain two times the generated sections?
.br
No, the new generated section(s) always replace the old generated section(s)!
.H 1 "HELP INFORMATION"
If you want to know which options the
.P= tabs
tool has, you can request for help:
.fS
% tabs -h
Usage: tabs [options]
Options:
  -h, --help                 Print this message.
  -v, --verbose              Turn on verbosity.
  -V, --verboser             Turn on space3d verbosity.
  -q, --quiet                Don't produce informational messages.
  -s, --sfile FILE.s         Use the given .s file. By default,
                             the space.def.s file of the current project.
  -m, --maskdata FILE        Use the given maskdata file. By default,
                             the maskdata file of the current project.
  -i, --ignore-preexisting   Ignore any preexisting capacitance rules
                             in the technology file.
  -p, --precision low|medium|high
                             Use the given level of precision (default low).
  -E VALUE                   To use another error-factor (default values for
                             the precisions are: 0.1|0.05|0.05).
  -L VALUE                   Set the number of layers over which capacitive
                             coupling is considered significant (default 2).
  -R, --edge-ratio VALUE     Set l/w ratio for edge caps (default 10).
  -Z, --z-window VALUE       Set the window in the z-direction for which
                             capacitive coupling is significant (in micron).
  --fake                     Fake run (it does not use the cache).
  --fix-vdim                 Fix vdim conditions with conductor conditions.
  --no-cache                 Disable cache (no writes). Always run space3d.
  --test-cache               Test against the cache. Always run space3d.
  --no-color                 Disable colored output.
  --omit-vias                Don't add via conditions to vertical caps.
  --skip-ecaps               Skip edge-surface capacitance part.
  --skip-lcaps               Skip lateral capacitance part.
  --add-lcaps2               Add lateral capacitance part2.
  --cf-lcaps  FACTOR         Using compensate factor for lcaps values.
  --cf-lcaps2 FACTOR         Using compensate factor for lcaps2 values
                             (values are multiplied with for example 0.9).
  --skip-vcaps               Skip vertical capacitance part. This part
                             computes the area and edge-edge capacitances.
  --skip-vdim NAME,...       Skip vdimension name(s).

.fE
You can, of course, use the help option in combination with other items on the command line.
But, when help is requested, the
.P= tabs
tool is only printing the help messages and stops running.
Note that help is also given, if you type something wrong.
.P
You can also show the manual page of the
.P= tabs
tool by using the
.P= icdman
command:
.fS
% icdman tabs
 ...
.fE
.H 1 "THE GENERATED CAP PARTS"
Default, four 2D capacitance parts are generated for the 2D capacitances section of the technology file.
A fifth part can optional be requested with option \fB--add-lcaps2\fP.
.br
The parts generate the following types of coupling capacitances:
.P
	1. area
.br
	2. edge-edge
.br
	3. edge-surface
.br
	4. lateral
.br
	5. lateral2 (optional)
.P
The lateral2 capacitances, part 5, are lateral capacitances between different conductor masks.
Note that only the specified conductors in the vdimensions section are used.
And, that for lateral capacitances only type 'm' conductors are selected.
.P
Note that the "area" and "edge-edge" capacitances, the parts 1 and 2,
can be skipped with option \fB--skip-vcaps\fP.
And that part 3, the "edge-surface" capacitances, can be skipped with option \fB--skip-ecaps\fP.
And that part 4, the "lateral" capacitances, can be skipped with option \fB--skip-lcaps\fP.
.H 2 "VERBOSE MODE"
The
.P= tabs
tool is default already verbose, that you see the "progress" and "space3d" invocations and
that you see verbose messages about which part is "computed" and which rules are "added".
The option \fB-v\fP shows also invocations of other commands which
.P= tabs
is using.
The option \fB-V\fP is almost equal to option \fB-v\fP,
but it also turns on the \fB-v\fP option of the
.P= space3d
program.
Some default verbose output is colored,
you can disable this with option \fB--no-color\fP.
The quiet option \fB-q\fP is not so quiet as you may thinking,
it only surpress the "progress" information.
.H 2 "OMITTING VIA'S"
With option \fB--omit-vias\fP,
in part 1, you can omit the addition of via masks to the condition of the rules
of the area capacitances.
Note that contact via's where not taken into account by older versions of the
.P= tabs
tool.
.H 2 "SKIPPING VDIM'S"
You can, of course, edit the technology file if you want to skip one or more vdimension conductors.
But, you can also use the \fB--skip-vdim\fP option to do so.
Thus, no couple capacitances are generated, which are connected to the skipped conductor(s).
.H 2 "A FAKE RUN"
You can experiment with the
.P= tabs
tool by using the \fB--fake\fP option.
It generates fake capacitances rules in the technology file and
it shows all the
.P= space3d
runs which need to be done, but they are not really executed.
It does not skip vdimensions or unneeded runs.
.H 2 "FIXING VDIM'S"
Option
.B --fix-vdim
tries to fix the vdimensions conditions.
It adjusts the conditions of vdimensions in the technology table, so that the
conditions only are true when there is a conductor present with the same mask.
.H 2 "COUPLING DISTANCE"
With options
.B -L
and
.B -Z
you can adjust the default capacitive coupling distance used.
Maybe you are missing some coupling caps between vdimensions.
Thus, you can set a higher number or z-distance.
.H 2 "PRECISION AND ERROR-FACTOR"
Where the algorithm is using convergence,
you wants to stop when values change less than given error-factor.
Default, with
.B low
precision, you gets an error-factor of "0.1".
By
.B medium
and
.B high
precision, you gets an error-factor of "0.05".
By
.B veryhigh
precision, you gets an error-factor of "0.01".
With option
.B -E
you may choice another error-factor independend of given precision.
Note that the precision also controls the mesh used.
With higher precision a smaller "cap3d.max_be_area" is used.
.H 2 "EDGE RATIO"
With option
.B -R
you can set the used edge-ratio.
The edge-ratio is default "10", but can be adjusted between "5" and "100".
The edge-ratio is used in the models for surface-edge and lateral capacitances.
A higher ratio gives maybe a more accurate model.
.H 1 "THE AREA CAP PART"
Tabs procedure "compute_vertical_capacitances" is used for the computation of
the vertical capacitances.
The used "cap3d.be_mode in this procedure is always "0c".
This includes the area capacitances: parallel plate capacitances
between metal layers and plate capacitances to ground.
Note, the model layout dimensions are in a lambda of 0.1 nm.
.P
The procedure is done for each vdimension mask which don't need to be skipped.
And the second vdimension mask must not be "out_of_reach" (option -Z or -L).
A two parallel square plates 3D tabs model is used.
C1 is calculated for plates with a side of L.
C2 is calculated for plates with a side of 2*L.
The model layout values are set into the "basic_layout_table".
The used "cap3d.max_be_area" is set into the "precision_table".
The procedure "basic_layout::compute_capacitances" is called to do a cap3D model extraction.
The C1 and C2 values can be retrieved between nodes "GND" (or "tcc") and "tp1c".
The formula for Cpp is:
.fS
    Cpp = (C2 - 2 * C1) / (2 * L * L * 1e-20)
.fE
An ideal Cpp is calculated with procedure "get_ideal_plate_capacitance_between_layers"
The rel_error is calculated with formula:
.fS
    rel_error = 1e4 * abs (Cpp_ideal - Cpp) / Cpp_ideal
.fE
A unique rule_name and a condition must be constructed for the capacitance rule.
Default is tried to add via condition.
The resulting condition is tried to be minimized.
The constructed capacitance rule existing out
(rule_name, condition, masks, Cpp_ideal, Cpp and rel_error) is appended to the "area_capacitance_rules".
Note that is chosen to use the Cpp_ideal value.
.P
The value for 'L' is calculated as follows:
.fS
    dz = z2b - z1t
    if (t1 > 0) {
        if (0.9*dz > t2) { L = 10*dz }
        else             { L = 30*dz }
    } else               { L = 20*dz } # i=ground
.fE
The used 'max_be_area' (in square micron) is calculated as follows:
.fS
    C1:  be_area = 1.9e-8 * L * L
    C2:  be_area = 4 * be_area
.fE
.H 1 "THE EDGE-EDGE CAP PART"
Tabs procedure "compute_vertical_capacitances" is also used for the computation of
the vertical edge-edge capacitances between metal layers.
For 'L' and 'max_be_area' see the "AREA CAP PART" section.
It uses the same two parallel square plates tabs model.
.br
The formula for Cee is:
.fS
    Cee = (4 * C1 - C2) / (8 * L * 1e-10)
.fE
Note that no edge-edge capacitance rule is outputted for a mask with zero thickness.
A unique rule_name and a condition must be constructed for the capacitance rule.
A not mask1 and not mask2 condition must be appended.
The resulting condition is tried to be minimized.
The constructed capacitance rule existing out
(rule_name, condition, masks and Cee) is appended to the "edge_edge_capacitance_rules".
Note that "UNIT e_capacitance" ensures that the Cee value gets the correct output value.

.F+
.PSPIC "an1403/fig1.ps" 4i
.F-

.H 1 "THE EDGE-SURFACE CAP PART"
Tabs procedure "compute_edge_surface_capacitances" is used for the computation of
the edge-surface capacitances.
You can use option \fB-M\fP to set the "cap3d.be_mode".
You can use option \fB-R\fP to set the used "edge-ratio" (default 10).
The procedure is done for each edge mask 'i' with thickness 't1' > 0.
For each mask 'i' a 'spacing' is used,
if not defined in the technology file 't1 / 2' is used.
The procedure remembers the last done 's_factor' with 'prev_factor'.
After the first surface mask 'j' not equal "@gnd" is done, flag 'first' becomes '0'.
For all other surface masks 'j' (not equal "@gnd") 'prev_factor' becomes the 'max_s_factor'.
.P
Edge mask 'i' is done in two directions, first in direction 'i_above' and second in direction 'i_below'.
In direction 'i_above' the rules for edge-bottom caps are done.
Surface mask 'j' lays below the edge mask 'i' and a sorted 'jlist' is used.
The last mask in this sorted list is the "@gnd" mask.
In that case node 'n1' becomes "GND" (otherwise 'n1' is "tcc").
Node 'n2' is by 'i_above' always "tp1c".
The capacitance 'ce' is found between this two nodes.
The model 'basic_layout_table' needs to have a 'size' of 2 (2 plates).
.F+
.PSPIC "an1403/fig2.ps" 4i
.F-
To compute 'Ce', we must do two cap3D extractions of the used layout model.
We use two different widths 'w' and 'w2' for the mask 'i' lanes.
The length 'L' of the lanes is not changed.
For the surface mask 'j' plane, we use a width 'W',
which is 3 times the width of the 'i' lanes plus 2 times the used spacing 's'.
The formula to compute 'Ce' is:
.fS
    Ce = (2 * C1 - C2) / (2 * L * 1e-10)
.fE
Note that the used 'w' is 4 times 'zdiff' (the distance between the plates).
And the used 'L' is 'w' multiplied with the used "edge-ratio".
And the used distance 's' is 'spacing' multiplied with 's_factor' (is 1,2,4,8,..,512).
For each 's' we compute a distance,edgecap pair.
The value of the new Ce must be greater than previous computed Ce.
If not, then the for-loop is stopped.
Also, if the new Ce is not significant greater than previous Ce, the for-loop is stopped.
.P
Each distance,edgecap pair is appended to the 'sub_rules' list.
Note that "UNIT distance" and "UNIT e_capacitance" ensures that the outputted
values are in the correct dimension.
There must be at least 1 sub_rule be produced (produced_sub_rules > 0).
If true, the edge-surface cap condition is constructed and minimized.
Only if the condition is valid, a unique rule_name is created and in that case the rule and
the sub_rules are appended to the edge_surface_capacitance_rules.
.P
The following "cap3d.max_be_area" is used for a (i,j) layout model:
.fS
w = 4 * zdiff
L = w * main_options(--edge-ratio)
p = main_options(--precision)
if (p == "veryhigh"){ a1 = L/64; a2 = w/4; if (a1 <= a2) p = "high"  }
if (p == "high")    { a1 = L/16; a2 = w/2; if (a1 <= a2) p = "medium"}
if (p == "medium")  { a1 = L/4 ; a2 = w  ; if (a1 <= a2) p = "low"   }
if (p == "low")     { a1 = L   ; a2 = L }
precision_table(be_area) = (a1+a2)/2 * L * 1e-8
.fE
In direction 'i_below' the rules for edge-top caps are done (see figure below).
In that case the surface mask 'j' lays above the edge mask 'i' and another sorted 'jlist' is used.
The model 'basic_layout_table' needs to have a 'size' of 3.
But only 2 plates are used, because the first plate becomes a dummy layer and is skipped in the used layout model.
Note that always the second layer is used for mask 'i'.
The capacitance 'Ce' can be found between nodes "tp1c" and "tp2c".

.F+
.PSPIC "an1403/fig3.ps" 4i
.F-

.H 1 "THE LATERAL CAP PART"
Tabs procedure "compute_lateral_capacitances" is used for the computation of
the lateral capacitances rules.
You can use option \fB-M\fP to set the "cap3d.be_mode" for this procedure.
And you can also use option \fB-R\fP to set the used "edge-ratio" (default 10).
Also a compensate factor \fB--cf-lcaps\fP can be used for this procedure.
The generated rules are appended to the 'lateral_capacitance_rules' list.
.P
The procedure is done for each edge mask 'i' with thickness 't1' > 0.
For each mask 'i' a 'spacing' is used,
if not defined in the technology file 't1 / 2' is used.
For each mask 'i' the 'w' and 'L' are set as a multiple of the 'spacing'
and 'L' is also multiplied by the "edge-ratio".
And 'w' and 'L' sets also the used "cap3d.max_be_area":
.fS
w = 8 * spacing
L = 4 * spacing * main_options(--edge-ratio)
m = L / 4
p = main_options(--precision)
if (p == "veryhigh"){ a1 = m/64; a2 = w/16; if (a1 <= a2) p = "high"  }
if (p == "high"   ) { a1 = m/16; a2 = w/ 8; if (a1 <= a2) p = "medium"}
if (p == "medium" ) { a1 = m/ 4; a2 = w/ 4; if (a1 <= a2) p = "low"   }
if (p == "low"    ) { a1 = m   ; a2 = w/ 2; }
precision_table(be_area) = (a1+a2)/2 * L * 1e-8
.fE
For each mask 'i' there are get two sorted masklists,
the 'jlist' for all masks below mask 'i' ("@gnd" is skipped)
and the 'klist' for all masks above mask 'i'.
Both lists are ended with '99'.
The following 'foreach' loops are done for each mask 'i':
.fS
    mask1 = technology_table(vdimensions:i:mask)
    foreach j in jlist {
	mask2 = j < 99 ? technology_table(vdimensions:j:mask) : "none"
	foreach k in klist {
	    mask3 = k < 99 ? technology_table(vdimensions:k:mask) : "none"
	    foreach s_factor in [1 2 4 8 16 32] {
		s = spacing * s_factor
		...
	    }
	}
    }
.fE
The 'basic_layout_table' is setup with the parameters for the used layout model.
The 'basic_layout_table' indices 0 and 1 are always used.
The 'size' of the table is 2, when 'mask3' (k) is not used,
else the 'size' of the table is 3.
When mask2 (j) is not used, the 'thickness' and 'z' value of index 0 are set to '0',
thus the plane below mask 'i' is skipped.
Note that index 1 is always used for mask 'i' and consists out of 3 lanes
with a width 'w' and length 'L' and distance 's'.
Two cap3D runs with a set layout model must be done to compute the 'Cl' capacitance.
The second run is done with a length '2*L'.
The lateral cap 'Cl' is computed with following formula:
.fS
    Cl = (C2 - C1) / (2 * L * 1e-10)
.fE
When the 'Cl' value is incorrect the foreach-loop is stopped.
Also, if no significant change is more found, than that value is as last done.
Each distance,latcap pair is appended to the 'sub_rules' list.
When all sub_rules are produced, a lateral cap condition is made
and a unique rule_name.
They are appended to the 'lateral_capacitance_rules' list for output.
The lateral cap C1 (C2) is get from both sides of node 'tp1c':

.F+
.PSPIC "an1403/fig4.ps" 4i
.F-

.H 2 "THE LATERAL2 CAP PART"
Tabs procedure "compute_lateral_capacitances2" is used for the computation of
the lateral capacitances rules between different masks.
The procedure is almost the same as the previous procedure.
.br
If set, it uses another compensate factor \fB--cf-lcaps2\fP for the procedure.
The generated rules are also appended to the 'lateral_capacitance_rules' list.
It computes now the lateral cap C1 (C2) between masks 'i' and 'm':

.F+
.PSPIC "an1403/fig5.ps" 4i
.F-

.H 1 "GENERATED TECHNOLOGY FILE"
As an example the 2D "capacitances:" section(s) of the "scmos_n" demo process is
generated with the
.P= tabs
tool.
First make a copy of the "space.def.s" technology file of the "scmos_n" process
and make sure that this file is writeable.
Second, edit the technology file and remove the 2D "capacitances:" section.
Don't remove the "junction capacitances" sections.
Now you are ready to run the
.P= tabs
tool,
see the commands used:
.fS
    % cp $ICDPATH/share/lib/process/scmos_n/space.def.s .
    % chmod +w space.def.s
    % vim space.def.s
    % tabs -s space.def.s -L 3
.fE
Option \fB-L 3\fP is needed, else layer "cms" is skipped.
Note, when you don't run
.P= tabs
in a project directory, you must also copy the "maskdata" file and add the \fB-m\fP option.
When you compare the generated results with the removed 2D "capacitances:" section,
you may have the following remarks.
Tabs does not generate capacitances to the "cwn" layer.
You can add these capacitance rules, they are equal to the generated capacitances to "@gnd".
Tabs has not generated a number of edge-surface capacitance rules!
This is indeed currently a problem.
These rules are skipped,
because of a directly found negative capacitivity value for the first distance,capacitivity pair.
The resulting technology file must look as follows (some parts are omitted):
.fS
#
# space element definition file for scmos_n example process
  ...

conductors :
  # name    : condition     : mask : resistivity : type
    cond_mf : cmf           : cmf  : 0.045       : m    # first metal
    cond_ms : cms           : cms  : 0.030       : m    # second metal
    cond_pg : cpg           : cpg  : 40          : m    # poly interconnect
    cond_pa : caa !cpg !csn : caa  : 70          : p    # p+ active area
    cond_na : caa !cpg  csn : caa  : 50          : n    # n+ active area
    cond_well : cwn         : cwn  : 0           : n    # n well
  ...

junction capacitances ndif :
  # name    :  condition                 : mask1 mask2 : capacitivity
    acap_na :  caa       !cpg  csn !cwn  : @gnd  caa : 100  # n+ bottom
    ecap_na : !caa -caa !-cpg -csn !-cwn : @gnd -caa : 300  # n+ sidewall

junction capacitances nwell :
    acap_cw :  cwn                  : @gnd  cwn : 100  # bottom
    ecap_cw : !cwn -cwn             : @gnd -cwn : 800  # sidewall

junction capacitances pdif :
    acap_pa :  caa       !cpg  !csn cwn      :  caa cwn : 500 # p+ bottom
    ecap_pa : !caa -caa !-cpg !-csn cwn -cwn : -caa cwn : 600 # p+ sidewall

# GENERATED BEGIN: tabs-area-capacitance (edit this block at your own risk)
# Added: Thu Mar 20 11:35:06 CET 2014
capacitances: # area-capacitances
    cpp_caa_cmf : !cpg caa cmf !cca : caa cmf : 24.665 # 24.676 (0.05% error)
    cpp_caa_cms : !cpg caa cms !cmf : caa cms : 13.812 # 13.807 (0.04% error)
    cpp_cpg_cmf : cpg caa cmf !ccp : cpg cmf : 53.124 # 53.048 (0.14% error)
    cpp_cpg_cms : cpg caa cms !cmf : cpg cms : 19.732 # 19.685 (0.24% error)
    cpp_cpg_cmf_1 : cpg !caa cmf !ccp : cpg cmf : 57.551 # 57.63 (0.14% error)
    cpp_cpg_cms_1 : cpg !caa cms !cmf : cpg cms : 20.312 # 20.267 (0.22% error)
    cpp_cmf_cms : cmf cms !cva : cmf cms : 86.326 # 86.106 (0.26% error)
    cpp_gnd_cpg : cpg caa : @gnd cpg : 98.659 # 98.018 (0.65% error)
    cpp_gnd_cpg_1 : cpg !caa : @gnd cpg : 57.551 # 56.236 (2.28% error)
    cpp_gnd_cmf : cmf !cpg !caa !cca : @gnd cmf : 20.312 # 20.272 (0.2% error)
    cpp_gnd_cms : cms !cpg !caa !cmf : @gnd cms : 12.332 # 12.316 (0.14% error)
# GENERATED END

# GENERATED BEGIN: tabs-edge-edge-capacitance (edit this block at your own risk)
# Added: Thu Mar 20 11:35:06 CET 2014
capacitances: # edge-edge capacitances
    cee_cpg_cmf : !cpg !cmf -cpg -caa -cmf : -cpg -cmf : 12.34
    cee_cpg_cms : !cpg !cms !cmf -cpg -caa -cms : -cpg -cms : 6.6169
    cee_cpg_cmf_1 : !cpg !cmf -cpg !-caa -cmf : -cpg -cmf : 11.68
    cee_cpg_cms_1 : !cpg !cms !cmf -cpg !-caa -cms : -cpg -cms : 6.6229
    cee_cmf_cms : !cmf !cms -cmf -cms : -cmf -cms : 19.366
# GENERATED END

# GENERATED BEGIN: tabs-edge-surface-capacitance (edit this block at your own risk)
# Added: Thu Mar 20 11:35:07 CET 2014
# Precision: low (be_mode=0c error=0.1)
# Edge-ratio: 10
capacitances: # edge-surface capacitances
    ecap_cpg_gnd : !cpg -cpg -caa : -cpg @gnd :
                                      2.5e-07 12.988
                                      5e-07 21.131
                                      1e-06 32.842
                                      2e-06 44.64
                                      4e-06 52.038
                                      8e-06 55.57

    ecap_cpg_gnd_1 : !cpg -cpg !-caa : -cpg @gnd :
                                      3.5e-07 12.607
                                      7e-07 19.869
                                      1.4e-06 29.936
                                      2.8e-06 39.658
                                      5.6e-06 45.841
                                      1.12e-05 48.554

    ecap_cmf_gnd : !cmf -cmf !cpg !caa : -cmf @gnd :
                                      3.5e-07 3.6625
                                      7e-07 6.8118
                                      1.4e-06 12.551
                                      2.8e-06 21.585
                                      5.6e-06 32.163
                                      1.12e-05 39.452
                                      2.24e-05 41.56

    ecap_cms_gnd : !cms -cms !cmf !cpg !caa : -cms @gnd :
                                      3.5e-07 1.8446
                                      7e-07 3.8911
                                      1.4e-06 7.8013
                                      2.8e-06 14.473
                                      5.6e-06 23.454
                                      1.12e-05 30.742
                                      2.24e-05 33.325
# GENERATED END

# GENERATED BEGIN: tabs-lateral-capacitance (edit this block at your own risk)
# Added: Thu Mar 20 11:35:10 CET 2014
# Precision: low (be_mode=0c error=0.1)
# Edge-ratio: 10
capacitances: # lateral capacitances
    lcap_cpg_none_cmf : !cpg -cpg -caa =cpg =caa cmf : -cpg =cpg :
                                      2.5e-07 88.403
                                      5e-07 33.098
                                      1e-06 14.174
                                      2e-06 10.087
                                      4e-06 0.59719

    lcap_cpg_none_cms : !cpg -cpg -caa =cpg =caa !cmf cms : -cpg =cpg :
                                      2.5e-07 110.18
                                      5e-07 56.087
                                      1e-06 24.975
                                      2e-06 8.5059
                                      4e-06 0.65662
                                      8e-06 0.003114

    lcap_cpg_none_none : !cpg -cpg -caa =cpg =caa !cmf !cms : -cpg =cpg :
                                      2.5e-07 121.44
                                      5e-07 68.424
                                      1e-06 37.274
                                      2e-06 18.59
                                      4e-06 7.7244
                                      8e-06 2.1706

    lcap_cpg_none_cmf_1 : !cpg -cpg !-caa =cpg !=caa cmf : -cpg =cpg :
                                      3.5e-07 27.414
                                      7e-07 17.55
                                      1.4e-06 2.0691

    lcap_cpg_none_cms_1 : !cpg -cpg !-caa =cpg !=caa !cmf cms : -cpg =cpg :
                                      3.5e-07 61.61
                                      7e-07 32.643
                                      1.4e-06 12.395
                                      2.8e-06 4.3901
                                      5.6e-06 0.074623

    lcap_cpg_none_none_1 : !cpg -cpg !-caa =cpg !=caa !cmf !cms : -cpg =cpg :
                                      3.5e-07 78.818
                                      7e-07 47.816
                                      1.4e-06 27.388
                                      2.8e-06 13.7
                                      5.6e-06 5.2779
                                      1.12e-05 1.1802

    lcap_cmf_cpg_none : !cmf -cmf =cmf cpg !caa !cms : -cmf =cmf :
                                      3.5e-07 75.103
                                      7e-07 52.654
                                      1.4e-06 28.394
                                      2.8e-06 13.254
                                      5.6e-06 3.9974
                                      1.12e-05 0.88224

    lcap_cmf_cpg_none_1 : !cmf -cmf =cmf cpg caa !cms : -cmf =cmf :
                                      3.5e-07 78.743
                                      7e-07 54.009
                                      1.4e-06 29.445
                                      2.8e-06 13.577
                                      5.6e-06 4.245
                                      1.12e-05 0.93599

    lcap_cmf_caa_cms : !cmf -cmf =cmf !cpg caa cms : -cmf =cmf :
                                      3.5e-07 26.289
                                      7e-07 25.696
                                      1.4e-06 5.571

    lcap_cmf_caa_none : !cmf -cmf =cmf !cpg caa !cms : -cmf =cmf :
                                      3.5e-07 103.28
                                      7e-07 64.888
                                      1.4e-06 38.925
                                      2.8e-06 20.589
                                      5.6e-06 7.9042
                                      1.12e-05 1.8049

    lcap_cmf_none_cms : !cmf -cmf =cmf !cpg !caa cms : -cmf =cmf :
                                      3.5e-07 30.746
                                      7e-07 28.134
                                      1.4e-06 7.479

    lcap_cmf_none_none : !cmf -cmf =cmf !cpg !caa !cms : -cmf =cmf :
                                      3.5e-07 107.27
                                      7e-07 67.184
                                      1.4e-06 41.149
                                      2.8e-06 22.503
                                      5.6e-06 9.2362
                                      1.12e-05 2.1809

    lcap_cms_cmf_none : !cms -cms =cms cmf : -cms =cms :
                                      3.5e-07 46.548
                                      7e-07 42.955
                                      1.4e-06 20.803
                                      2.8e-06 9.1318
                                      5.6e-06 1.7393
                                      1.12e-05 0.30806

    lcap_cms_cpg_none : !cms -cms =cms !cmf cpg !caa : -cms =cms :
                                      3.5e-07 103.53
                                      7e-07 65.333
                                      1.4e-06 38.963
                                      2.8e-06 19.869
                                      5.6e-06 6.8417
                                      1.12e-05 1.4481

    lcap_cms_cpg_none_1 : !cms -cms =cms !cmf cpg caa : -cms =cms :
                                      3.5e-07 104.02
                                      7e-07 65.663
                                      1.4e-06 39.283
                                      2.8e-06 20.158
                                      5.6e-06 7.0313
                                      1.12e-05 1.4963

    lcap_cms_caa_none : !cms -cms =cms !cmf !cpg caa : -cms =cms :
                                      3.5e-07 109.44
                                      7e-07 69.518
                                      1.4e-06 43.164
                                      2.8e-06 23.803
                                      5.6e-06 9.6526
                                      1.12e-05 2.29

    lcap_cms_none_none : !cms -cms =cms !cmf !cpg !caa : -cms =cms :
                                      3.5e-07 110.83
                                      7e-07 70.649
                                      1.4e-06 44.333
                                      2.8e-06 24.936
                                      5.6e-06 10.576
                                      1.12e-05 2.6333
# GENERATED END

vdimensions :
    v_caa_on_all : caa !cpg           : caa : 0.30 0.00
    v_cpg_of_caa : cpg !caa           : cpg : 0.60 0.50
    v_cpg_on_caa : cpg caa            : cpg : 0.35 0.70
    v_cmf        : cmf                : cmf : 1.70 0.70
    v_cms        : cms                : cms : 2.80 0.70
    
dielectrics :
   # Dielectric consists of 5 micron thick SiO2
   # (epsilon = 3.9) on a conducting plane.
   SiO2   3.9   0.0
   air    1.0   5.0

   ...
#EOF
.fE
