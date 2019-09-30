.T= "Space \s-2ICDPROCESS\s0 Application Note"
.DS 2
.rs
.sp 1i
.B
.S 15 20
SPACE ICDPROCESS
APPLICATION NOTE
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
Report ET-CAS 01-03
.ce
May 8, 2001
.sp |9i
.DS
.in +5
.S 9
Copyright \s+3\(co\s-3 2001-2004 by the author.

Last revision: December 3, 2003.
.S
.in -5
.DE
.SK
.S=
.H 1 "INTRODUCTION"
This report describes the environment variable \s-1ICDPROCESS\s0,
which is used by some \*(sP programs.
.P
See the file "/users/space/Shadow/Updates.extern":
.fS
27 Oct 2000
-----------
Using the environment variable ICDPROCESS, a default process can now be
set.  This environment variable can denote a process name, a process
number or a process directory.  This variable is used (if defined) by
mkpr and by helios when creating a new project directory, and by tecc
when compiling an element definition file.
Moreover, the process directory can have a file "default_lambda" that
contains a default lambda value for the new project that is created.
Both mkpr and tecc now have an option -p that can be used to specify a
process name, a process number or a process directory, and the option
-P with mkpr has become obsolete.

2 Nov 2000
----------
Added tool getproc to obtain information (mask names etc.) about the
current process (see "icdman getproc").
.fE
See also file "/users/space/Shadow/Updates.intern".
.P
See also the manual pages of
.P= getproc ,
.P= tecc ,
.P= mkpr
and
.P= helios .

.H 1 "TECHNOLOGY DATAPATH STRUCTURE"
.F+
.PSPIC an0103/tpath.ps 12c
.F-
.P
The technology data is all stored in a technology data directory.
In the above figure is one technology data directory shown, named "scmos_n".
This IC Design system directory can be found on the \s-1$ICDPATH\s0,
which is an environment variable (default path is "/usr/cacd").
.P
Note that IC Design programs are stored in the directory "\s-1$ICDPATH\s0/bin".
This directory must be added to your shell program search path.
.P
When an user design project database is opened, with database function dmOpenProject,
the init-file ".dmrc" is read.
This file contains a process identification number and process lambda value (in micron).
This process identification number is used for finding the named technology data directory.
This identification number name relation is specified in the
IC Design system "processlist" file.
.P
The technology data of an user design project database is only retrieved if needed.
The "maskdata" information is get with the following function call:
.fS I
mdata = (DM_PROCDATA *) dmGetMetaDesignData (PROCESS, project);
.fE
The \s-2\fCproject\fP\s0 key is used for process identification.

.H 1 "TECHNOLOGY DATABASE FUNCTIONS"
.F+
.PSPIC an0103/tfunc.ps 11c
.F-
.P
Normally the maskdata is only get once for the current project.
The current project is normally the current working directory,
but it can be set with the environment variable CWD.
.P
Thus, if the current project imports other projects,
then these projects must be able to use the same maskdata.
.P
When function dmGetMetaDesignData is used to get the maskdata for
different projects and these projects have the same process identification,
then the maskdata is shared between the different projects.
Thus, the project keys point to the same maskdata.
.P
Note that the init-file ".dmrc" may contain a process directory path
in place of the process identification.
.br
This maskdata cannot be shared by using function dmGetMetaDesignData.

.H 1 "REFERENCES"
.nf
For \*(sP see:

	The usage of Space is described in the user manuals:

	- Space Tutorial, October 2000
	- Space User's Manual, September 2000
	- Space3D Cap. Extraction User's Manual, October 2000
	- Space Substrate Res. User's Manual, May 2000

.TC 2 1 3 0
