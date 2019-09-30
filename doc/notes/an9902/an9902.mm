.T= "Updating Helios to Version 1.4.2a"
.DS 2
.rs
.sp 1i
.B
.S 15 20
UPDATING HELIOS
FROM
VERSION 1.4.2
TO
VERSION 1.4.2a
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
Report ET-CAS 99-02
.ce
November 2, 1999
.sp |9i
.DS
.in +5
.S 9
Copyright \s+3\(co\s-3 1999 by the author.

Last revision: December 12, 2003.
.S
.in -5
.DE
.SK
.S=
.H 1 "INTRODUCTION"
This report describes the modifications made to the
.P= helios
version 1.4.2 (October version)
after requests out of the \*(sP users group.
.P
This work is carried out for the \*(sP project till 2 November 1999.

.HU "REFERENCES"
.nf
For \*(sP see:
.P
	The usage of \*(sP is described in the user manuals:
.P
	- Space Tutorial
	- Space User's Manual

.H 1 "DISCUSSION OF CHANGES IN THE SOURCE FILES"
For a list of changes see appendix B.
.H 2 "Change Requests"
.I(
.I=
When the "helios.defaults" file cannot be (over)written, is the error message not nice.
.I=
The text "Accept" and "Accept Options" is used both.
.I=
In the ``New Technology File Dialog'' is no "Accept" button.
.I=
Sometimes the "Load settings" command has no effect in ``Extract Options''?
.I=
The "Selected Cell" field is not cleaned by going to another project.
.I=
The option
.B -t
by
.P= cgi
(Import Layout) is obsolete!
The option
.B -w
does exist.
They are independent of each other.
.I=
Change the name
.P= xdali
into
.P= dali .
The manual page of
.P= dali
is not on the list.
.I=
If there are a lot of cells in the view of a project (P505 for example), the
.P= helios
interface slows done.
Implemented another (private) ISROOT function.
.I=
There is a empty line on the bottom of the list of cells field.
.I=
In ``Name of New Database'' is the Databases text field too small.
.I=
The program
.P= mkpr
is now used to make new projects.
.I=
The program
.P= rmpr
is now used without option
.B -a .
This option removes all files and directories from a not empty project.
.I=
Implemented a shorter prompt (choices 0 to 4).
.I=
Removed the hidden Job Control mnemonic 'P'.
.I=
You can now also specify to
.P= helios
which project you want to open.
.I=
Updated file "README.helios".
.I=
Updated the
.P= helios
tutorial document.
.I=
Updated the
.P= helios
manual page.
.I)

.H 1 "APPENDICES"
.HU "APPENDIX A -- SCCS DELTA'S"
.S 11
.TS
box;
l | l l | l l
l | n l | n l.
FILE:	PREVIOUS:		NEW:
name	bytes	delta/date	bytes	delta/date
_
Makefile	4065	3.3   98/12/03	--	--
README.helios	2762	3.4   98/07/07	3011	3.5   99/11/02
adminvar.c	89089	3.8   99/10/04	89362	3.9   99/11/01
callbacks.c	120096	3.8   99/10/01	127764	3.9   99/11/01
callbacks.c	--	--      	127820	3.10 99/11/02
elements.h	2397	3.1   98/04/01	--	--
externs.h	1087	3.2   99/10/01	--	--
get_db.c	1551	3.2   98/06/11	--	--
glue.c  	250	3.1   98/04/01	--	--
helios.icon	4841	3.1   98/04/01	--	--
helios.message	80403	3.5   99/10/05	80665	3.6   99/11/01
helios.res	1755	3.3   99/09/29	1457	3.4   99/11/01
option.h	3801	3.3   99/10/01	3801	3.4   99/11/01
realist.c	18434	3.14 99/10/01	15313	3.15 99/11/01
realist.h	1567	3.2   99/10/01	--	--
run_space.c	16123	3.3   99/10/01	15926	3.4   99/11/01
sram.xpm	68077	3.1   98/04/01	--	--
xmrealist.c	637382	3.10 99/10/04	633387	3.11 99/11/01
xmrealist.h	28045	3.4   99/10/01	28054	3.5   99/11/01
xmrealist.xd	286299	3.10 99/10/04	285681	3.11 99/11/01
.TE
.S
.P
.B "Helios tutorial" :
.TS
box;
l | l l | l l
l | n l | n l.
FILE:	PREVIOUS:		NEW:
name	bytes	delta/date	bytes	delta/date
_
body.mm 	100463	4.23 99/10/05	100448	4.24 99/11/02
title.mm	1035	4.14 99/10/05	1035	4.15 99/11/02
about.ps	84391	4.2   99/10/05	84271	4.3   99/11/02
helios.ps	139429	4.2   99/10/05	138447	4.3   99/11/02
jobs.ps 	160535	4.2   99/10/05	144514	4.3   99/11/02
.TE
.P
.B "Helios manual" :
.TS
box;
l | l l | l l
l | n l | n l.
FILE:	PREVIOUS:		NEW:
name	bytes	delta/date	bytes	delta/date
_
helios.1	3006	4.1   98/07/06	4901	4.2   99/11/02
.TE
