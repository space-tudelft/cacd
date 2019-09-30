.T= "Updating Helios to Version 1.4.2b"
.DS 2
.rs
.sp 1i
.B
.S 15 20
UPDATING HELIOS
FROM
VERSION 1.4.2a
TO
VERSION 1.4.2b
.S
.sp 1
.I
S. de Graaf
.sp 1
.R
Circuits and Systems Group
Faculty of Electrical Engineering
Delft University of Technology
The Netherlands
.DE
.sp 2c
.ce
Report ET-CAS 99-03
.ce
November 12, 1999
.sp |9i
.DS
.in +5
.S 9
Copyright \s+3\(co\s-3 1999 by the author.

Last revision: December 15, 2003.
.S
.in -5
.DE
.SK
.S=
.H 1 "INTRODUCTION"
This report describes the modifications made to the
.P= helios
1.4.2a (November version) after requests out of the \*(sP users group.
.P
This work is carried out for the \*(sP project till 12 November 1999.
.br
Some modifications are done by A.J. van Genderen.

.nr Hu 2 \" Level of this .HU heading
.HU "REFERENCES"
.nf
For \*(sP see:

	The usage of \*(sP is described in the user manuals:

	- Space Tutorial
	- Space User's Manual

.H 1 "DISCUSSION OF CHANGES IN THE SOURCE FILES"
For a list of changes see appendix B.
.P
The following change requests are done:
.H 2 "Circuit Reduction Extract Options Form"
.nf
Change
	Retain nodes with at least [ 4 ] connections
into
	Retain nodes with at least [ 4 ] resistor connections

Change
	Omit coupling junction areas < [ 0.04 ] times ground
into
	Omit coupling junction areas < [ 0.04 ] times ground area
.H 2 "Manual Page Window"
Can not find
.P= helios
manual page.
.H 3 "Default Value of Empty Text Fields"
It is currently not possible to tell
.P= helios ,
that you want to use the
.P= space
default value for an item.
That this item must be left out the "helios.def.p" file.
If a text field is left empty you get a zero value returned from function atoi() or atof().
If this value is not in the range for the item, you get the minimal accepted value.
.H 3 "The Value of net_node_sep"
Why is a value of 't' not acceptable for the net_node_sep?
The net_node_sep can only be a punctuation character.
.H 3 "Junction Cap. also for Accurate 3D"
The junction capacitance setting must also be in the "Accurate Details" window.
I added the "jun_caps3d" keyword for the
.P= helios
defaults file.
.H 3 "Constant Strings"
By some C++ compilers you may not change the contents of ``constant strings''.
This gives a runtime bus error (core dump).
I have changed the "adminvar.c" source file, thus that no constant strings need
to be modified anymore.
I use now the private function strTok() in place of strtok().
I use also another method for printing the spaces between the keywords and the values.
.H 3 "Initial Boolean Values"
I have changed the initial specified boolean values from "off"/"on" into "0"/"1".
This is shorter and easier to detect.

.H 1 "APPENDICES"
.SP 2
.HU "APPENDIX A -- SCCS DELTA'S"
.SP 2
.TS
box;
l | l l | l l
l | n l | n l.
FILE:	PREVIOUS:		NEW:
name	bytes	delta/date	bytes	delta/date
_
Makefile	4065	3.3   98/12/03	--	--
README.helios	3011	3.5   99/11/02	--	--
adminvar.c	89362	3.9   99/11/01	89541	3.10 99/11/10
adminvar.c	--	--      	89212	3.11 99/11/12
callbacks.c	127820	3.10 99/11/02	127798	3.11 99/11/10
elements.h	2397	3.1   98/04/01	--	--
externs.h	1087	3.2   99/10/01	--	--
get_db.c	1551	3.2   98/06/11	--	--
glue.c  	250	3.1   98/04/01	--	--
helios.icon	4841	3.1   98/04/01	--	--
helios.message	80665	3.6   99/11/01	80740	3.7   99/11/12
helios.res	1457	3.4   99/11/01	--	--
option.h	3801	3.4   99/11/01	3827	3.5   99/11/12
realist.c	15313	3.15 99/11/01	15526	3.16 99/11/12
realist.h	1567	3.2   99/10/01	--	--
run_space.c	15926	3.4   99/11/01	15375	3.5   99/11/10
sram.xpm	68077	3.1   98/04/01	--	--
xmrealist.c	633387	3.11 99/11/01	636466	3.12 99/11/12
xmrealist.h	28054	3.5   99/11/01	28207	3.6   99/11/12
xmrealist.xd	285681	3.11 99/11/01	287056	3.12 99/11/12
.TE
