.T= "Adding Tools Xspf and Xspef to Helios"
.DS 2
.rs
.sp 1i
.B
.S 15 20
ADDING RETRIEVAL TOOLS
XSPF AND XSPEF
TO THE HELIOS PROGRAM
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
Report ET-CAS 98-01
.ce
April 3, 1998
.sp |9i
.DS
.in +5
.S 9
Copyright \s+3\(co\s-3 1998 by the author.

Last revision: December 12, 2003.
.S
.in -5
.DE
.SK
.S=
.H 1 "INTRODUCTION"
This report describes the modifications made to the
.P= helios
user interface.
There are added two buttons for the new retrieval tools
.P= xspf
and
.P= xspef
in the "Retrieval\ Options\ Menu".
.P
A number of other modifications were also done.
Now you can change also between two different projects.
.P
The
.P= helios
interface is itensively tested and many other problems are also repaired.
.P
This work was carried out for the \*(sP project in the period 17 February - 1 April 1998.

.H 1 "TOOL XSPF"
Tool
.P= xspf
generates output in the Cadence Standard Parasitic Format (SPF).
The output is in proposed Detailed SPF (DSPF).
.fS I
Usage: xspf [-defghiknoptuvxy -z name -C file] cell
.fE
As you see, there are two types of arguments: options and the cell name.
Each option or a list of options starts with a minus '-' sign.
Some options have a separate argument, like
.B -z
and
.B -C .
The cell name is always the last argument and must be specified.
The cell name is the name of the top circuit (network).
.P
The following options can be specified:
.P
.ta 2.7c
    \fB-d\fP	use original instance names
    \fB-e\fP	expand name arrays into single names
    \fB-f\fP	output to file "\fIcell\fB.spf\fR"
    \fB-g\fP	add large ground resistors for vnets
    \fB-h\fP	hierarchical mode (all local sub-cells)
    \fB-i\fP	imported mode (also imported sub-cells)
    \fB-k\fP	generate for all cells \fB.subckt\fP lines
    \fB-n\fP	add terminal for n-bulk to all cells
    \fB-o\fP	omit model definitions
    \fB-p\fP	add terminal for p-bulk to all cells
    \fB-t\fP	don't output unconnected instances/cells
    \fB-u\fP	don't autom. add terminals for n-/p-bulk
    \fB-v\fP	verbose mode
    \fB-x\fP	nodes starting with prefix \fBgnd\fP/\fBGND\fP are 0-node
    \fB-y\fP	nodes starting with prefix \fBvss\fP/\fBVSS\fP are 0-node
    \fB-z\fP \fIname\fP	nodes starting with prefix \fIname\fP are 0-node
    \fB-C\fP \fIfile\fP	use \fIfile\fP in place of default control file
.P
For NELSIS Release 4 also the option:
.P
    \fB-T\fP	use database stream in place of output file

.H 2 "Default control file"
When the file "./\fBxspicerc\fP" exists, then this file is read as the default control file,
else the file "$ICDPATH/lib/process/\fIproc\fP/\fBxspicerc\fP" is tried to read.

.H 1 "TOOL XSPEF"
Tool
.P= xspef
generates output in the Standard Parasitics Exchange Format (SPEF).
.P
The following usage message is generated when you type the command:
.fS I
% xspef
xspef 2.19 27-Oct-1997

Usage: xspef [-efhivxy -z name -C file] cell
.fE
As you see, there are two types of arguments: options and the cell name.
Each option or a list of options starts with a minus '-' sign.
Some options have a separate argument, like
.B -z
and
.B -C .
The cell name is always the last argument and must be specified.
The cell name is the name of the top circuit (network).
.P
The possible number of options is shorter then for tool
.P= xspf .
Because the output format is almost completely different from SPF.
For an explanation of the options, see tool
.P= xspf .
For option
.B -f
counts that the extention of the output files is "\fB.spef\fP".
Note that in hierarchical
mode for each circuit a separate output file is generated.
.P
The following options don't exist, because...
.P
    \fB-d\fP	original instance names can't be generated,
.br
	because only instance numbers are used by SPEF.
.P
    \fB-g\fP	because vnets are not used by SPEF.
.P
    \fB-k\fP	because SPEF generates no \fB.subckt\fP lines.
.P
    \fB-n,-p,-u\fP	because there are never bulk terminals added by the SPEF format.
.P
    \fB-o\fP	because there are never model definitions generated.
.P
    \fB-t\fP	because there are never instances of cells generated.

.H 1 "TESTING"
The following test projects are used for testing:
.fS I
crand/
rand_cnt/
switchbox/
test/
.fE
.H 2 "Using the Helios user interface"
The
.P= space
extractor can easely be started via the
.P= helios
user interface.
Start
.P= helios
in the background.
.P= Helios
comes up with two windows.
Close the "QuickRefWin" window and go with the mouse to the "helios" window.
.P
The
.P= helios
interface remembers the last opened database.
When you want to open another database, click on "Database\(->Open" and select the correct one.
For extraction, select a layout cell (for example "substr_1").
To start an extraction, go to the "Extractor" menu.
In the "Extractor" menu, click on the "Extract" button to start directly
.P= space .
.P
When you want to specify other extraction options,
click in the menu on "Extraction Options...".
Note that
.P= helios
generates a parameter file "helios.def.p",
which is given to the extractor with option
.B -P .
.P
When you want to add some other parameters, click on "More Options"
to include your file at end of the
.P= helios
parameter file.
.P
For hierarchical extraction the following additional parameters must be
specified:
.fS I
hier_name_sep     /
inst_term_sep     :
term_is_netname   on
leaf_terminals    on
.fE

.H 1 "REFERENCES"
.nf
For \*(sP see:

	The usage of \*(sP is described in the user manuals:

	- Space Tutorial
	- Space User's Manual

