.T= "Space Application Note About Using Process Data"
.DS 2
.rs
.sp 1i
.B
.S 15 20
SPACE APPLICATION NOTE
ABOUT
USING PROCESS DATA
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
Report ET-CAS 00-06
.ce
November 21, 2000
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
This report describes the usage of technology data in the \*(sP system.

.H 1 "READING PROCESS DATA"
To read the "maskdata",
.P= space
calls the database interface function
dmGetMetaDesignData.
This function retrieves from the project key the
process id to use for finding the "maskdata" file.
A project key can be
used after that a project is successfully opened with dmOpenProject.
Function dmOpenProject stores the process id in "pkey -> procid" and
sets "pkey -> procpath" to a null string.
If the "procid" is not a number, "procid" is set to -9 and "procpath"
contains the path to the process directory.
Note that "procpath" can be DM_MAXPATHLEN (= 1024) long.
Function dmGetMetaDesignData uses the "procpath" if it is set.

.F+
.PSPIC an0006/proc.ps 12c
.F-
.FG "Reading process data."

Note that the "procpath" (at this moment) can not be a process name.
In that case it must be a process id (number) which must be in the
"ICDPATH/lib/process/processlist".
Note that the "procpath" may be
a relative directory path, but it may not begin with a digit.

.H 1 "TECC PROCESS DATA FLOW"
The program
.P= tecc ,
the \*(sP technology compiler, uses also the "maskdata"
file for converting the technology input file into a tabular one.
Default,
.P= tecc
uses the process id or path from the current project.
If the CWD environment variable is not set, the current working directory
is the current (default) project path.
Note that this default is superceded
by the setting of the ICDPROCESS environment variable.
The "maskdata" file path can also be specified with the
.B -m
option.
The process name or path can also be specified with the
.B -p
option.
Note that only one of these options may be specified.
If the process name is not a process name from the "processlist",
it is used as a process path.

.F+
.PSPIC an0006/teccproc.ps 12c
.F-
.FG "Tecc process data flow."

Note that the "maskdata" used by
.P= tecc
and
.P= space
must be identical!

.H 1 "GETPROC PROCESS DATA FLOW"
The program
.P= getproc
can display the contents of the "maskdata" file.
Default,
.P= getproc
uses the process id or path from the current project.
If the CWD environment variable is not set, the current working directory
is the current (default) project path.
A process id, name or path can also be specified as program argument.
If the process name is not in the "processlist", it must be a path.
If the argument is a path, then is the process id and name unknown.
The basename of the path can maybe used as process name.
.P
With the
.B -m
option
.P= getproc
displays the contents of the specified "maskdata" file.
In that case the process name and id displayed is unknown.
With the
.B -p
option
.P= getproc
displays the contents of the "processlist" file.

.H 1 "REFERENCES"
.nf
For \*(sP see:

	The usage of \*(sP is described in the user manuals:

	- Space Tutorial, October 2000
	- Space User's Manual, September 2000
	- Space3D Cap. Extraction User's Manual, October 2000
	- Space Substrate Res. User's Manual, May 2000

. \".TC 2 1 3 0
