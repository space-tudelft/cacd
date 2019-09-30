.T= "Updating Helios to Version 1.4.4"
.DS 2
.rs
.sp 1i
.B
.S 15 20
UPDATING HELIOS
FROM
VERSION 1.4.3
TO
VERSION 1.4.4
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
Report ET-CAS 00-05
.ce
November 14, 2000
.sp |9i
.DS
.in +5
.S 9
Copyright \s+3\(co\s-3 2000-2004 by the author.

Last revision: December 16, 2003.
.S
.in -5
.DE
.SK
.S=
.H 1 "INTRODUCTION"
This report describes the modifications made to the
.P= helios
1.4.3 version
after requests out of the \*(sP users group.
.P
This work is carried out for the \*(sP project from 20 to 27 October 2000.
.br
Some modifications were carried out by Arjan van Genderen (AvG).

.nr Hu 2 \" Level of this .HU heading
.HU "CHECKED IN HELIOS SOURCES"
.fS I
SCCS/s.realist.c:    D 3.20 00/10/26 13:34:37 space
New version; NewButton and NewTechButton are always sensitive. (SdeG)
SCCS/s.xmrealist.xd: D 3.15 00/10/26 13:37:00 space
New version; Process_name textfields are now editable. (SdeG)
SCCS/s.xmrealist.c:  D 3.15 00/10/26 13:37:09 space
New version; Process_name textfields are now editable. (SdeG)
SCCS/s.callbacks.c:  D 3.17 00/10/26 13:43:31 space
The Process_name can now also be a Process_path; It also looks for
env.var. ICDPROCESS and tries to read a default_lambda file.  (SdeG)

SCCS/s.xmrealist.c:  D 3.16 00/10/27 09:46:57 space
changed 1999 into 2000 (AvG)
SCCS/s.helios.res:    D 3.5 00/10/27 10:24:18 space
more info about path to helios.icon and sram.xpm (AvG)
SCCS/s.README.helios: D 3.6 00/10/27 10:24:20 space
more info about path to helios.icon and sram.xpm (AvG)
.fE
.HU "CHECKED IN HELIOS TUTORIAL"
.fS I
SCCS/s.body.mm:  D 4.31 00/10/26 16:35:06 space
about ICDPROCESS (AvG)
SCCS/s.body.mm:  D 4.30 00/10/23 14:50:46 space
added ICDPROCESS and default_lambda (AvG)
SCCS/s.title.mm: D 4.19 00/10/23 14:49:48 space
(AvG)
.fE
.HU "CHECKED IN HELIOS MANUAL"
.fS I
SCCS/s.helios.1: D 4.3 00/10/23 13:49:11 space
added ICDPROCESS and default_lambda (AvG)
.fE
.HU "REFERENCES"
.nf
The usage of \*(sP is described in the user manuals:
.P
	- Space Helios Tutorial
	- Space Tutorial
	- Space User's Manual

.H 1 "DISCUSSION OF CHANGES IN THE SOURCE FILES"
For a list of changes see appendix B.
.H 2 "Discussion of realist.c"
When there were no processes found in the process list, the NewButton was set insensitive.
This is not longer needed because you can specify your own process path.
.br
A database (.dmrc) can also use another process path for the technology data.
.P
Also the NewTechButton does not more be set insensitive.
.H 2 "Discussion of xmrealist.xd and xmrealist.c"
The process_name textfields in the "New Project" and "New Technology"
dialogs must be set editable.
(XmNeditable = true; XmNcursorPositionVisible = true;)
.P
I have changed the text '[microns]' into '[micron]' in the "New Project" dialog window.
Like in the "Params & Files" window.
.H 2 "Discussion of callbacks.c"
Added #include's for "DIR *dp" and opendir() usage. (to test for readable techn.dir.)
.P
Added TechnologyPath for the storage of the current opened database technology path
in the ".dmrc" file:
.fS
> static char TechnologyPath[DM_MAXPATHLEN];
.fE
.H 3 "Added 3 Service Functions:"
.fS
> int isProcessName (char *name)
.fE
Function isProcessName tests of the name, a name of a process
out of the processlist is.
.fS
> char *strip2 (char *name)
.fE
Function strip2 strips leading and trailing spaces from
a string read from a textfield.
.br
It removes a possible second string part.
It returnes a string without spaces.
.fS
> char *lambdaStr (char *s)
.fE
Function lambdaStr converts its string argument to a floating
point number in the "%f" format (0.000000).
This format has standard 6 digits behint the dot.
.br
The function strips unneeded '0' digits, but it leaves
always 1 '0' digit behind the dot (if there is nothing else behind it).
If gives back the converted string in a static buffer.
.br
The result string maybe is incorrect, because it is converted to zero.
This can now be easy tested with atof(str) <= 0.0?
.SK
.H 3 "Function MakeNewDataBase:"
Changed "float lambda;" into "char *lambda;".
Now function lambdaStr() is used to make an uniform lambda string.
The corrected lambda string is put back into the form with:
.fS
>     XmTextFieldSetString (NewDbWin->LambdaText, lambda);
.fE
Removed "int processNumber;".
Function isProcessName() is now used.
.P
Added "struct stat buf;" for function stat().
This function is used for testing the newDatabaseName.
.P
Changed the 'space in it' message for a correct database name.
.P
Changed the 'incorrect database path' message.
There can be a file or directory already with that name.
In that case
.P= mkpr
shall fail.
Because a stat() is done, we know this before calling
.P= mkpr .
.P
Function isProcessName() is now used for testing the specified processName.
If the name is not a known process name, it must be the name of a directory path.
This is checked with a call to opendir().
.P
The program
.P= mkpr
is changed.
It creates now default extended projects.
The
.B -e
option does not more be given.
To make explicit old databases the
.B -o
option must be used.
.P
Note that the lambda value can now be longer than 3 digits behind the dot!
Maximum 6 digits may now be used.
See function lambdaStr().
.H 3 "Function ChangeToOtherDatabase:"
Changed some detail.
The versionNumber is not more read with sscanf() "%g"
into a double, but we use now directly atoi() and "%d".
.P
.P= Helios
tests now if the processIndex can be read with "%d" from the ".dmrc" file.
If this fails, helios expects that it is a process name/path entry.
Note that a process name/path must not start with a digit.
This path is saved in TechnologyPath.
.P
Now function lambdaStr() is used to format the lambda value.
The lambda value can now be longer than 3 digits behind the dot.
.H 3 "Function ListSelectionOKCallback:"
The selectionText is not more static.
The value is freed before leaving.
I also fixed a bug here.
Sometimes the program tries to free selectionText more than ones.
.P
I have changed ListSelectionOKCallback() for the "Process Selection".
After the <OK> button is clicked for a "Process Selection" automatically
the file "default_lambda" is checked and possible is filled in.
If no process is selected, the process name from the TextField is used.
This name is possibly filled in by the user and thus he can set a default.
Note that the "default_lambda" file is a new process feature!
.P
Because the process name field is now editable, it must be stript.
The process name can also be an user defined name or path.
.H 3 "Function NewTechDefaultCallback:"
If you click on the <Defaults> button in the "New Technology" dialog, the
process name field and the maskdata path field are filled in.
If the environment variable ICDPROCESS is set, this name/path is used.
Else the name/path of the current open database is used.
This path can be set in the TechnologyPath global variable.
.H 3 "Function NewTechOKCallback:"
Because the process name field is now editable, it must be stript.
The process name can also be an user defined name or path.
Also the program
.P= tecc
is modified.
The user can also give a path to the program with option
.B -p .
Note that, if the process name field is not filled in, no
.B -p
option is given to the
.P= tecc
program.
In that case
.P= tecc
uses its own default.
.H 3 "Function OpenWindow:"
The NEW_DATABASE case is changed.
After clicking on the "New Project"
button function OpenWindow() initialize the form dialog window.
Before the "New Project" dialog window pops-up the fields are now filled
in as follows:
.P
.nf
(1) Project Name field:
      Is initialized with the value of the current opened database.
(2) Process Name field:
      Is initialized with the value of (a) the environment variable ICDPROCESS
      or else with (b) the TechnologyName or else (c) TechnologyPath
      or else (d) not initialized.
      Note that (b) or (c) is the value of the last opened database.
(3) Lambda Value field:
      Is possible initialized with (a) the default value of the process name/path
      or else (b) the value of current opened database or (c) not initialized.
(4) Extended toggle:
      Is now always initialized with the value TRUE.

.nr Ej 0
.H 1 "NOTES"
Note that
.P= mkpr
and
.P= tecc
can get a process_path with the option
.B -p .
These programs must handle this situation.
.P
Note that
.P= mkpr
does not ask for process name or lambda value anymore!
If it can find a default, it shall use that default!
.P
Maybe we can also introduce an ICDLAMBDA environment variable?

.nr Ej 1
.H 1 "APPENDICES"

.HU "APPENDIX A -- SCCS DELTA'S"
.B "Helios sources" :
.S 11
.TS
box;
l | l l | l l
l | n l | n l.
FILE:	PREVIOUS:		NEW:
name	bytes	delta/date	bytes	delta/date
_
Makefile	4065	3.3   98/12/03	--	--
README.helios	3011	3.5   99/11/02	3150	3.6   00/10/27
adminvar.c	89212	3.11 99/11/12	--	--
callbacks.c	142193	3.16 00/07/05	144186	3.17 00/10/26
elements.h	2397	3.1   98/04/01	--	--
externs.h	1087	3.2   99/10/01	--	--
get_db.c	1551	3.2   98/06/11	--	--
glue.c  	250	3.1   98/04/01	--	--
helios.icon	4841	3.1   98/04/01	--	--
helios.message	80996	3.10  00/07/12	--	--
helios.res	1457	3.4   99/11/01	1588	3.5   00/10/27
option.h	3844	3.6   00/06/15	--	--
realist.c	15766	3.19 00/07/14	15600	3.20 00/10/26
realist.h	1567	3.2   99/10/01	--	--
run_space.c	15375	3.5   99/11/10	--	--
sram.xpm	68077	3.1   98/04/01	--	--
xmrealist.c	643965	3.14 00/07/14	643699	3.15 00/10/26
xmrealist.c	--	--      	643699	3.16 00/10/27
xmrealist.h	28392	3.7   00/06/15	--	--
xmrealist.xd	293476	3.14 00/07/14	293343	3.15 00/10/26
.TE
