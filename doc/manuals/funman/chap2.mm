.EQ
delim $$
gsize 11
gfont R
.EN
.H 1 "DESCRIPTION OF THE EXTENDED SLS-PACKAGE"
The extended simulator SLS consists of the programs used in the
Switch-Level Simulator plus one more program. The use of the package
furthermore changes due to the fact that an
executable simulator is created
whenever a new function block is used in a network.
In figure 1 the extended SLS package is shown.
.DS


.PS < ../funman/mfig1.pic


.FG "The extended SLS package"
.DE

.SK
The extended SLS-package involves four programs
.AL
.LI
\fIcfun\fP 
maps a function block description contained in one file
into a function block description in database format. The
function block description
is translated into C code, the C code is compiled and the
object file is placed into the database.
.LI
\fIcsls\fP 
maps a hierarchical \fIsls\fP network description, contained
in one or several files into a network description in database format.
.LI
\fIsls_exp\fP 
maps the network description into a binary (i.e non readable)
file which serves as input file for the \fIsls\fP simulator.
Furthermore, when the network contains calls to function blocks,
\fIsls_exp\fP 
creates an executable 
\fIsls\fP 
that contains the
behavioral description of these function blocks. Text
that is included in the file 'sls.ld_arg' is used as argument
when the executable 
\fIsls\fP 
is created.
.LI
\fIsls\fP 
is the created executable simulator program that
can be used similar to the original switch-level simulator.
.LE

Once a binary format file has been obtained for a network and an
executable simulator exists, different simulations can be done
without the time-consuming function block compilation and linking
step. A new executable will be created by 
\fIsls_exp\fP 
only when the network
contains a function block of which the compiled
description is not yet linked to the existing executable,
or when this function block is newer than the existing executable.
.SK
