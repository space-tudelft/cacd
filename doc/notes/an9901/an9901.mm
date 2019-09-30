.T= "Updating Helios to Version 1.4.2"
.DS 2
.rs
.sp 1i
.B
.S 15 20
UPDATING HELIOS
FROM
VERSION 1.4.1
TO
VERSION 1.4.2
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
Report ET-CAS 99-01
.ce
October 4, 1999
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
version 1.4.1 user interface.
There is added new code for the new retrieval tool
.P= xvhdl .
A number of other programs had new options (
.P= highlay ,
.P= space ", ...)."
.P
The
.P= space
program had also a number of new variables.
A lot of other modifications were done.
I made the interface more consistend.
.P
The forms may now stay open, sub-forms become insensitive if its setting
is currently not used.
The settings in the open forms are used by clicking on 'extract' or 'retrieve'.
A 'cancel' or 'accept' of the main 'extract' or 'retrieve' form is also used
by the sub-forms.
This is not true for the 'set defaults' button.
.P
The format of the "helios.messages" file is a little changed.
It contains now also the index list of the manual pages.
A number of bugs is also repaired.
.P
This work is carried out for the \*(sP project in the period 18 Aug. - 4 Oct. 1999.

.H 1 "SCCS DELTA'S"
.TS
box;
l | l l | l l
l | n l | n l.
FILE:	PREVIOUS:		LAST/NEW:
name	bytes	date	bytes	delta/date
_
Makefile	--	--      	4065	3.3 12/03/98
README.helios	--	--      	2762	3.4 07/07/98
adminvar.c	125653	07/07/98	89141	3.7 10/01/99
--      	--	--      	89089	3.8 10/04/99
callbacks.c	148563	07/15/99	120096	3.8 10/01/99
elements.h	--	--      	2397	3.1 04/01/98
externs.h	1159	04/01/98	1087	3.2 10/01/99
fallback.h	--	--      	2407	--  07/09/98
get_db.c	--	--      	1551	3.2 06/11/98
glue.c  	--	--      	250	3.1 04/01/98
helios.icon	--	--      	4841	3.1 04/01/98
helios.message	66570	06/17/98	83003	3.4 10/01/99
--      	--	--      	80403	3.5 10/05/99
helios.res	2201	07/02/98	1755	3.3 09/29/99
option.h	3362	06/11/98	3801	3.3 10/01/99
realist.c	18817	07/15/99	18434	3.13 10/01/99
--      	--	--      	18434	3.14 10/01/99
realist.h	1201	04/01/98	1567	3.2 10/01/99
run_space.c	15776	07/15/99	16123	3.3 10/01/99
sram.xpm	--	--      	68077	3.1 04/01/98
xmrealist.c	587738	07/15/99	637549	3.8 10/01/99
--      	--	--      	637549	3.9 10/04/99
--      	--	--      	637382	3.10 10/04/99
xmrealist.h	27887	06/17/98	28045	3.4 10/01/99
xmrealist.xd	256411	07/15/99	286306	3.8 10/01/99
--      	--	--      	286306	3.9 10/04/99
--      	--	--      	286299	3.10 10/04/99
.TE
.nr Ej 0
.H 1 "REFERENCES"
.nf
For \*(sP see:
.P
	The usage of \*(sP is described in the user manuals:
.P
	- Space Tutorial
	- Space User's Manual

