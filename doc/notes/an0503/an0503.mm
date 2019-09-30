.T= "How The Nspice Program Works"
.DS 2
.rs
.sp 1i
.B
.S 15 20
HOW THE
NSPICE PROGRAM
WORKS
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
Report EWI-ENS 05-03
.ce
June 20, 2005
.sp |9i
.DS
.in +5
.S 9
Copyright \s+3\(co\s-3 2004 by the author.

Last revision: June 20, 2005
.S
.in -5
.DE
.SK
.S=
.H 1 "INTRODUCTION"
This document describes how the
.P= nspice
utility program works in combination with the
.P= spice
simulator.

The program
.P= nspice
is in real an unix shell-script and
consists out of two programs.
A preprocessor program
.P= nspice_pp ,
which prepares the input for the
.P= spice
simulator program.
And second, a back-substitute program
.P= nspice_bs ,
which changes the simulator output ".ana" file
(the signal node-numbers into node-names).

But first of all, the
.P= nspice
program calls the
.P= xspice
program to extract a spice netlist (".spc" file) of the cell out of the design database.
The following figure gives an overview.
.P
.F+
.PSPIC "an0503/fig1.ps" 5i
.F-
.P
.H 1 "NSPICE_PP"
The \fInspice_pp\fP program consists out of the following source modules:
.DS C
.TS
box;
l | l.
main_pp.c	define.h
readCir.c	type.h
readCom.c	extern.h
res.c
cmd_l.l
cmd_y.y
.TE
.DE
The include (".h") files and "readCir.c" file are also used by \fInspice_bs\fP.
.P
The program contains a lexical analyser and parser to read the ".cmd" file.
For detailed information consult the manual page of \fInspice\fP.
The ".cmd" file contains
.P= spice
commands, which are added to the spice netlist in the ".spc" file.
.P
First the command file is parsed by routine readCom to make a list of all signals,
which are specified with "set" commands.
And it makes also a list of all found "plot" commands.
It also reads a number of "option" commands.
Of which "option sigunit" is most important, this option is used to set the time-scaling.
With "option simperiod" can the simulation end-time be set.
If not specified, the end-time is calculated from the found "set" commands.
Note that the end-less simuli cannot be used for it.
The "option simlevel=3" is needed to generate ``\fB.print\fP ...'' cards
and a ``\fB.tran\fP ...'' card.
The level must be '\fB3\fP' to get them.
.P
The spice section of the command file is divided in two parts.
The parts begin and are separated from each other with the '*%' marks.
The whole spice section must be a comment for the sls parser.
Thus, this section is not read with routine readCom.
.P
Second, routine readCir is used to read the name-list of the cell netlist,
which specifies the relation between node-names and node-numbers.
The program
.P= xspice
should generate this name-list after the cell netlist,
else the
.P= nspice_pp
program cannot work.
The format of this name-list must be:
.fS
* namelist cellname
*
*   nodenr name ...
.fE
Third, routine writeSpiceCom is used to write to "stdout" the spice commands
which must be added to the ".spc" file.
This routine shall first read the ".cmd" file for the spice section.
It reads in first part the keywords, which specify "tstep", "trise", "tfall", etc.
The value "tstep" must be specified to generate the a valid ``\fB.tran\fP ...'' card.
Also, when "trise" and/or "tfall" are missing, the value of "tstep" is used instead.
You can also specify a "vhigh" voltage, to use another value then 5 volt.
.P
Thus, the sls "set" commands are converted to spice "pwl" stimuli commands.
But only for those signal nodes, for which the node number is not equal to zero (the ground node).
.SK
Each transition from logic 'l' to 'h' at a time 't' is changed into two time points.
The first time point 't1' is equal to (t - trise / 2) and the second time point 't2'
is equal to (t + trise / 2).
Thus, each transition from logic 'h' to 'l' at a time 'T' is changed into
(T - tfall / 2) and (T + tfall / 2).
See the following figure.
.P
.F+
.PSPIC "an0503/fig2.ps" 5i
.F-
.P
Of coarse the values of "tstep", "trise" and "tfall" must be small enough
in accordance with the specified "option sigunit" value.
.H 1 "NSPICE_BS"
The \fInspice_bs\fP program consists out of the following source modules:
.DS C
.TS
box;
l | l.
main_bs.c	define.h
readCir.c	type.h
	extern.h
.TE
.DE
The include (".h") files and "readCir.c" file are also used by \fInspice_pp\fP.
.P
First, routine readCir is used to read the name-list of the cell netlist (from the ".spc" file),
which specifies the relation between node-names and node-numbers.
.P
Second, routine backsubSpc is used to read the ".ana" spice output file.
First, it scans the ".ana" file, to see if there are "*error*" or "*ERROR*" messages.
If found, it print these error messages and exits the program.
.P
When there are no errors, the program shall start to back-substitute the node numbers
to node names.
The node names can be found in the name-list.
For spice3 output, the node numbers can be found on "Index time" lines.
Between "v(..)" are these numbers found and changed into names.
Everything is copied to an ".axa" temporary file.
.P
If nothing goes wrong, (a) at least one "Index time" line found,
and (b) all node numbers are found in the name-list.
In that case, the ".axa" file is moved to the ".ana" file
and overwrites the old ".ana" file.
