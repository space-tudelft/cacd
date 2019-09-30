.de p{
.DS 1
.ft FX
.S 9
..
.de p}
.ft
.S
.DE
..
.\" start and end of NOTE
.de N(
.P
.ne 5
.in +5n
.B NOTE:
.S=
.br
..
.de N)
.S=
.in -5n
.P
..
.tr #
.H 1 "INTRODUCTION"
This manual is meant as a guide to users who
want to simulate their network with the 
.I sls
simulator.
The acronym \fIsls\fP stands for \fISwitch-Level Simulator\fP,
and the simulator can be used for simulating the logical
and timing behavior of digital MOS circuits.
In the simulator transistors are modeled by grounded capacitors
and a switched resistor.
Each node in the network has a logic state O,
I or X (for unknown),
and each transistor has a state \fIon\fP,
\fIoff\fP or \fIundefined\fP.
Many characteristics of MOS circuits can be modeled accurately,
including:
ratioed,
complementary and precharged logic;
dynamic and static storage;
pass transistors;
busses;
and charge sharing.
Because the simulator performs local-event-driven simulation,
large networks with thousands of transistors
can be simulated in a reasonable time.
.P
The \fIsls\fP simulator is capable of simulating
a MOS transistor network at three levels:
.AL 
.LI
purely logic simulation based on network topology and transistor types,
without considering the actual circuit parameters.
.LI
logic simulation based on actual circuit parameters (transistor 
dimensions and interconnection resistances and capacitances are used
to determine logic states).
.LI
logic and timing simulation based on actual circuit parameters (transistor
dimensions and interconnection resistances and capacitances are used
to determine logic states and delays).
.LE
.P
Other important features of the simulator are:
.BL 4
.LI
piece-wise-linear voltage waveform approximations with timing simulation.
.LI
min-max delay simulation to account for circuit parameter deviations
and model accuracy.
.LI
mixed-level simulation for transistor-level, gate-level and function-level
circuits.
.LE
.P
The original switch-level model - which only allows a simulation
at the first level -
was introduced by R.E. Bryant.
Descriptions of it are given in [
.[
Bryant
ICCC 1982
.]] 
and [
.[
Bryant
IEEE Computers 1984
.]].
In [
.[
A.J. van Genderen
M.S. Thesis
.]], [
.[
P.M. Dewilde
Switch Level Timing
ICCAD
.]]
and [
.[
SLS
VLSI 89
Genderen
.]]
the principle of the \fIsls\fP simulator is described.
The limitations of the simulator can also be found there:
Due to its simple transistor model, \fIsls\fP is not as accurate
as a circuit simulator like SPICE [
.[
SPICE Guide Berkeley
.]],
while sometimes the transistor-level description of
analogue circuits like sense amplifiers can not be simulated correctly at all.
.P
Section 2 in this manual gives an overview of the \fIsls\fP package of programs.
.br
Section 3 describes the syntax and semantics of
the network description language.
.br
Section 4 contains the syntax and semantics of the command language.
.br
Section 5 gives some suggestions for solving troubles.
.P
The use of function-level circuit descriptions in the \fIsls\fP
simulator is explained in [
.[
Hol
.]].
.SK
.H 1 "DESCRIPTION OF THE SLS-PACKAGE"
The switch level simulation package consists of a number of programs.
In Figure 1 the relation between the three main programs is outlined.
In this figure you will find the following symbols:
.VL 12
.LI Rectangle
Rectangles stand for executable programs.
The name of a particular program is printed inside the rectangle.
.LI Punch-card
A punch-card symbol stands for one file.
A deck of punch-cards stands for multiple files.
.LI Arrow
An arrow denotes the information flow between
programs and files. Programs can read/write information
from/to several different files.
.LE
.SP 2
.PS
define pcard X
	[
		line right 3 then up 1.5 then left 2.5\
		then down 0.5 left 0.5 then down 1
	]
X
define file X
	[
		[pcard] $1 $2
	]
X
define files X
	[
		PC	: file($1, $2)
		move to PC.nw + (0.5, 0)
		line up 0.25 right 0.25 then right 2.5\
		then down 1.5 then left 0.25
		move to PC.nw + (0.5, 0)
		line up 0.5 right 0.5 then right 2.5\
		then down 1.5 then left 0.25
	]
X
define diagram X
	[
		Mkdb	: box ht 2 wid 3 "csls"
		move right 1
		Exp	: box ht 2 wid 3 "sls_exp"
		move right 1
		Sls	: box ht 2 wid 3 "sls"

		NTWFILE : [files("network", "file")] with .s at Mkdb.n + (0, 2) 
		CMDFILE : [file("command", "file")] with .s at Sls.n + (0, 2) 
		arrow from NTWFILE.s to Mkdb.n
		arrow from CMDFILE.s to Sls.n

		Db	:
		[
			Dbf	: [files("network", "db-format")]
			move right 1
			Bf	: [files("binary file","for sls")]
		] with .s at Exp.s + (0, -4.5)
		box dashed ht last [].ht+0.4 wid last [].wid+0.4 at last []
		"database" at Db.s + (0.0, -0.34) below
		line <-> from Mkdb.s to Db.Dbf.n
		line <-> from Exp.se + (-0.5, 0) to Db.Bf.n
		arrow from Db.Dbf.ne + (-0.5,0) to Exp.s
		arrow from Db.Bf.ne + (-0.5,0) to Sls.s

		Outf	: [file("\fIcell\fP.out", "file")] with .sw at Sls.se +\
			  (1.0, -0.5)
		Resf	: [file("\fIcell\fP.res", "file")] with .sw at Sls.se +\
			  (1.0, -2.5)
		Pltf	: [file("\fIcell\fP.plt", "file")] with .sw at Sls.se +\
			  (1.0, -4.5)
		arrow from Sls.se + (-0,0.25) to Outf.w
		arrow from Sls.se + (-0.25,0) to Resf.w
		arrow from Sls.se + (-0.5,0) to Pltf.w
	]
X
scale = 2.54
diagram
.PE
.SP
.FG "Information flow diagram of the SLS-package."
.SK
The SLS-package involves three main programs.
.AL
.LI
\fIcsls\fP#maps 
a hierarchical \fIsls\fP network description contained
in one or several files into a network description in database format.
.LI
\fIsls_exp\fP#maps 
the network description in database format into a
binary (i.e. non readable) file which serves as input file for the
switch level simulator. Actually a collection of binary files can be created
in the database, because first a binary file for each subnetwork
is created once. The expansion program links the binary files of
the subnetworks into one binary file of the total network.
Normally,
\fIsls_exp\fP
is automatically called by \fIsls\fP when starting a simulation.
.LI
\fIsls\fP#is the switch level simulation program and reads input
from two files:
.VL 17
.LI A#binary#file
This file is generated 
by \fIsls_exp\fP 
and contains the network description
in binary format.
.LI A#command#file
This file contains the simulation commands
and must be generated manually by the designer.
.LE
.P
the output files that can be generated by \fIsls\fR are:
.VL 11
.LI \fIcell\fP.out
This file is the normal readable and printable output in table format.
Here "\fIcell\fP" stands for the name of the cell that is simulated.
.LI \fIcell\fP.res
This file is not readable and serves as input for several post processing
facilities or as input for new simulations.
.LI \fIcell\fP.plt
This file is only generated when approximating voltage waveforms are plotted.
The file is not readable but can be used as input for a
post-processor.
.LI \fIcell\fP.dis
This file is optionally generated when information about the dynamic dissipation
in the circuit is requested.
.LE
.LE
.SP
Other programs that belong to the SLS-package involve post processing 
facilities
producing different sorts of output.
E.g. \fIlpsig\fP can be used to plot simulation output on a printer,
and \fIsimeye\fP is
a graphical signal display and editing program.
.P
For the invocation of the programs above, 
see the manual pages in the appendix of this manual.
.P
Apart from using the program
.I csls
to put a network description into the database,
also other programs may be used to generate an input network for
.I sls,
like the layout to circuit extraction program
.I space.
.SK
.H 1 "THE DESCRIPTION OF NETWORKS"
.H 2 "General conventions"
It is common practice to describe the syntax of a
computer language in some meta language.
The syntax definition of \fIsls\fP is described in the
meta language proposed by Wirth [
.[
Wirth diversity syntactic
.]].
This language has two types of symbols:
.VL 23
.LI terminal#symbols
These symbols are denoted by characters between double quote marks,
or words printed in \fIitalic\fP font.
They must be used literally or are described in the
table "lexical constructions".
.LI non-terminal#symbols
These are denoted by words in the ordinary font,
and each of them is described by a so-called production rule.
.LE
.SP
Each production rule of the sls syntax begins with a non-terminal, followed
by an equal-sign and a sequence of terminal and non-terminal symbols
and meta characters, and is terminated by a period.
The meta characters imply:
.VL 16
.LI Alternatives##|
A vertical bar between symbols denotes the choice of either one symbol
or another symbol. (i.e. a#|#b means either a or b)
.LI Repetition##{}
Curly brackets denote that the symbols in between may be present
zero or more times.
.br
(i.e. {#a#} stands for empty#|#a#|#aa#|#aaa#|#...)
.LI Optionality##[]
Square brackets denote that the symbols between them may be present.
.br
(i.e. [#a#] stands for a#|#empty)
.LI Parenthesis##()
Parenthesis serve for grouping of symbols in meta character expressions.
.br
(i.e. (#a#|#b#)#c stands for ac#|#bc)
.LE
.SP
Next the following rules are applied:
.DL
.LI
A comment begins with "/*" and is terminated by "*/" as in the C-language.
Every character in between is discarded by the parser. You are not
allowed to nest comments.
.LI
A name or identifier must begin with a letter
and may be followed by an arbitrary number of letters, digits and
underscore characters.
.LI
Uppercase and lowercase letters are distinct.
.LI
The blank, tab and newline are separators. Between two symbols of a production
rule of a non-terminal symbol, an arbitrary number of separators may appear.
.LI
Values are specified in S.I. units (e.g. meter, farad, ohm), 
and may appear with one of the following scaling factors:
.TS
center tab(/);
ll.
a/= 1.0e-18
f/= 1.0e-15
p/= 1.0e-12
n/= 1.0e-9
u/= 1.0e-6
m/= 1.0e-3
k/= 1.0e+3
M/= 1.0e+6
G/= 1.0e+9
.TE
.LI
Terminal symbols that are not literal are described in the
next table.
The difference between this syntax description (of terminal
symbols) and following syntax descriptions (of non-terminal symbols)
is that here it is not allowed 
to use separators between the symbols
that make up the production rule.
.SP
.DS
.TB "Lexical constructions"
.TS
center tab(/);
ll.
_
\fIpower_ten\fP/= "1"{"0"}[("a"|"f"|"p"|"n"|"u"|"m"|"k"|"M"|"G")].
\fIf_float\fP/= \fIfloat\fP[("a"|"f"|"p"|"n"|"u"|"m"|"k"|"M"|"G")].
\fIfloat\fP/= \fIinteger\fP["."\fIinteger\fP][\fIexponent\fP]
\fIexponent\fP/= ("D"|"E"|"d"|"e")[("-"|"+")]\fIinteger\fP.
\fIidentifier\fP/= \fIletter\fP {(\fIletter\fP | \fIdigit\fP | "_")}.
\fIstring\fP/= """ \fIcharacter\fP { \fIcharacter\fP } """.
\fIcharacter\fP/= \fIdigit\fP | \fIletter\fP | \fIspecial\fP.
\fIinteger\fP/= \fIdigit\fP {\fIdigit\fP}.
\fIletter\fP/= "a"|"b"|"c"|"d"|"e"|"f"|"g"|"h"|"i"|"j"
/| "k"|"l"|"m"|"n"|"o"|"p"|"q"|"r"|"s"|"t"
/| "u"|"v"|"w"|"x"|"y"|"z"
/| "A"|"B"|"C"|"D"|"E"|"F"|"G"|"H"|"I"|"J"
/| "K"|"L"|"M"|"N"|"O"|"P"|"Q"|"R"|"S"|"T"
/| "U"|"V"|"W"|"X"|"Y"|"Z".
\fIdigit\fP/= "0"|"1"|"2"|"3"|"4"|"5"|"6"|"7"|"8"|"9".
\fIempty\fP/= "".
\fIspecial\fP/= any character that is not a \fIletter\fP, \fIdigit\fP or """.
.TE
.DE
.LE
.SP 
.DS
Examples:
.sp 0.5
    \fIpower_ten\fP  :  10n  100p  1000u
    \fIf_float\fP  :  1.3e-9  2.0n  6.8k  0.0004
    \fIidentifier\fP  :  abcd  a123  this_IS_an_identifier 
    \fIstring\fP  :  "this is a string/$*&"
.DE
.SK
.H 2 "Network Description"
.DS
.TB "Syntax of the network part."
.TS
center tab(/);
ll.
_
network_decls/= network_decl {network_decl}.
network_decl/= \fInetwork identifier\fP decl_part ntw_body.
.TE
.DE
.SP
The network part begins with the keyword \fInetwork\fP followed
by the name of the network,
a declaration part describing the interface
to the outside world and a body specifying the instances of
devices and subnetworks.

.H 2 "Declarations"
.DS
.TB "Syntax of the declaration part."
.TS
center tab(/);
ll.
_
decl_part/= "(" term_decls ")".
term_decls/= term_decl {";" term_decl}.
term decl/= \fIterminal\fP term {"," term}.
term/= \fIidentifier\fP ["[" range_list "]"].
range_list/= range {"," range}.
range/= \fIinteger\fP ".." \fIinteger\fP.
.TE
.DE
.SP
The declaration part is bracketed by a left and right parenthesis.
It must contain at least one terminal declaration.
A terminal declaration begins with the keyword \fIterminal\fP
followed by a list of terminals.
A list of terminals consists of one or more terminals separated
by commas.
.DS
Examples:
.sp 0.5
    network register (terminal in[1..4,1..4], out[1..8])

    network latch (terminal in, out;
                            terminal phi[0..1], vss, vdd)
.DE

.H 2 "Network Structure"
The network body (ntw_body) is the statement part (stmt_part)
bracketed by curly brackets ({}).
The statement part may contain instance, net and null statements.
The scope of the terminal, instance and node names in the network
is local to the defined network.
.SP
.DS
.TB "Syntax of the network body."
.TS
center tab(/);
ll.
_
ntw_body/= "{" stmt_part "}".
stmt_part/= statement {statement}.
statement/= inst_stmt | net_stmt | null_stmt.
inst_stmt/= [inst_struct] inst_def.
inst_struct/= "{" \fIidentifier\fP [ "[" range_list "]" ] }"
/| "{" "." "[" range_list "]" }".
inst_def/= transistor_def | function_def | resistor_def
/| capacitor_def | call_def.
null_stmt/= ";".
net_stmt/= \fInet\fP "{" net_spec "}" ";".
net_spec/= vector_list | net_nodes ";".
vector_list/= "(" net_nodes ")" {"," "(" net_nodes ")"}.
net_nodes/= node_ref {"," node_ref}.
transistor_def/= ttype [attributes] connect_list ";".
ttype/= \fInenh\fP | \fIpenh\fP | \fIndep\fP.
function_def/= "@" ftype [attributes] connect_list ";".
ftype/= \fIinvert\fP | \fInand\fP | \fInor\fP | \fIand\fP | \fIor\fP | \fIexor\fP.
resistor_def/= \fIres\fP \fIf_float\fP connect_list ";".
capacitor_def/= \fIcap\fP \fIf_float\fP connect_list ";".
call_def/= \fIidentifier\fP connect_list";".
attributes/= attribute {attribute}.
attribute/= attr_label "=" attr_val.
attr_label/= \fIw\fP | \fIl\fP | \fItr\fP | \fItf\fP.
attr_val/= \fIf_float\fP.
connect_list/= "(" connects ")" | "{" connects "}".
connects/= connect {"," connect}.
connect/= internal_ref | node_ref.
internal_ref/= [ "[" index_list "]" ] "." node_ref.
node_ref/= \fIinteger\fP | \fIidentifier\fP [ "[" index_list "]" ].
index_list/= index {"," index}.
index/= \fIinteger\fP | range.
range_list/= range {"," range }.
range/= \fIinteger\fP ".." \fIinteger\fP.
.TE
.DE

.H 3 "instance statement"
The instance statement constitutes the actual place of a device, function
or subnetwork in the network being defined. An instance may optionally
have a structure part and must have a definition part.
The instance name in the structure part is especially convenient
for selecting particular nodes within the instance (as might be necessary in the
simulation command file). The instance may have an array structure, which is
specified by a range list between square brackets after the instance name.
The general format of an instance definition is: 
.TS
center tab(/);
lll.
type/attributes/connect_list
.TE
.SP
In each definition a node connection list is specified. This list
can be interpreted in two ways:
.VL 23
.LI instance#major#order
If the list is placed between parenthesis we define
it to be in instance major order.
Instance major order means that when the instance has array structure,
for instance from 1 to 4,
the connection list will be interpreted as follows,
first all the terminal connections for the first array element,
then that of the second array element, etc..
.LI parameter#major#order
If the list is placed between curly brackets we define 
it to be in parameter major order.
Parameter major order means that if the instance has array structure,
the connection list will consist of
first the first terminals of all array elements, then the second terminals
of all array elements etc..
.LE
.DS
Examples:
.sp 0.5
    { inv[1..3] } inverter ( i1, o1,
                                        i2, o2,
                                        i3, o3 );

    { inv[1..3] } inverter { i1, i2, i3,
                                        o1, o2, o3 };

    { inv[1..3,1..3] } inverter { in[1..9], out[1..9] };
.DE
.SP
When "inverter" has terminals i and o, in the first two examples
i1 will be connected to inv[1].i, o1 to inv[1].o, i2 to inv[2].i, 
o2 to inv[2].o, i3 to inv[3].i and o3 to inv[3].o.
In the third example,
in[1] will be connected to inv[1,1].i, in[2] to inv[1,2].i, 
in[3] to inv[1,3].i, 
in[4] to inv[2,1].i, etc.
out[1] to inv[1,1].o, out[2] to inv[1,2].o, 
out[3] to inv[1,3].o, 
out[4] to inv[2,1].o, etc.
.SP
If the instance is an array of elements one can specify internal connections
in the connection list. With an internal connection one can directly
interconnect terminals of the array elements of the instance.
An internal connection is denoted by selecting a formal terminal of
an instanced (possibly array) element and is put into the right place
in the connection list.
.DS
Example:
.sp 0.5
    { inv[1..3] } inverter { i1, [1..2].o, [2..3].i, o3 };

    where inverter was defined as

    network inverter (terminal i, o)
    {
	.
	.
	.
    }
.DE
The next subsections describe the instance definition parts that can occur.

.H 4 "transistor definition"
A transistor definition contains three arguments.
.VL 12
.LI type
specifies the type of the transistor. The following three types can be
chosen:
.AL 1 4 1
.LI
nenh
.LI
penh
.LI
ndep
.LE
.LI attributes
The second argument is optional and specifies the attribute values
of the device. A transistor can have two attributes for specifying
the length and width of a transistor.
If omitted the default values for length and width of
the transistor are 4 micron for \fIsls\fP.
.LI connection
The third argument consists of three or a multiple of three node references,
depending on the structure of the instance.
The sequence of the nodes is important as far as the gate node is concerned.
The gate node must be specified as the first node,
while the sequence of the source node and drain node is arbitrary.
.LE
.DS
Examples:
.sp 0.5
    nenh w=6u l=8u (g1, d1, s1);
    { n1 } penh w=6u l=8u (g1, d1, s1);
    { ni[1..2] } nenh (g1, d1, s1, g2, d2, s2);
    { np[1..2] } ndep {g1, g2, d1, d2, s1, s2};
.DE
.SP

.H 4 "function definition"
A function definition calls a network element that reads its inputs
and produces logical states at its output(s).
Gate-level functions like nands and nors are available as 
built-in functions
while other, more complex functions
(e.g. adders, multiplexers and roms)
can be specified by the user (see [
.[
Hol
.]]).
.DS 0 1
A function definition contains four arguments:
.VL 12
.LI specifier
A function specifier which consists of the character "@".
.LI type
The type of the function element.
The following function types are available as built-in functions:
.AL 1 4 1
.LI
invert
.LI
nand
.LI
nor
.LI
and
.LI
or
.LI
exor
.LE
.LI attributes
The attributes tr and tf are optional and specify the rise time and fall
time of the output(s) when the logic state of one of the inputs changes.
Default tr=0 and tf=0.
.LI connection
Specifies the connection of the function inputs and outputs
to the network nodes.
The connection list of a built-in function may contain an arbitrary number of
input nodes (except for function "invert" which may contain
only one input node) and one output node. 
The order is first the input nodes and then the output node.
.LE
.P
When two or more function outputs are connected to the same network
node and not all outputs force the same state,
the X state will result.
.DE
.DS
Examples:
.sp 0.5
    @ nand (a, b, c, d, e, out);
    { nands[1..2] } @ nand (a, b, out1, c, d, out2);
    @ nor tr=10n tf=5n (a, b, c, d, out);
.DE

.H 4 "resistor definition"
The resistor definition is specified by the keyword \fIres\fP,
the resistance value in Ohm and two node references.
The resistance value may be a floating point number
and may have a multiplication
factor denoted by a letter as described in section 1.
.DS
Example:
.sp 0.5
    res 2k (a, out);
.DE

.H 4 "capacitor definition"
The capacitor definition is specified by the keyword \fIcap\fP,
a capacitance value in Farad and two node references.
The capacitance value may be a floating point number with a multiplication
factor.
.DS
Example:
.sp 0.5
    cap 8.6f (12, a);
.DE

.H 4 "call definition"
A network call defines the instance of a subnetwork.
The sequence of the connection list
is the same as the sequence of the terminal declaration
of the called network.
.DS
Example:
.sp 0.5
    when network subntw was defined as

    network subntw (terminal in[1..2], out, vdd, vss)
    {
        .
        .
    }

    the instance 

    subntw (a, b, c, vdd, vss);

    will connect a to in[1], b to in[2], c to out, vdd to vdd, and vss to vss.
.DE

.H 3 "net statement"
The net statement gives the opportunity to coalesce, cluster and define
nodes.
This statement is especially convenient if the network description
is extracted from a layout description.
In a layout description terminals often represent the
same electrical node and it would be tedious to specify a stimulus
for each of these terminals.
.VL 12
.LI coalesce
Normally all specified terminals between curly brackets are coalesced
to one electrical node.
.LI cluster
A list of parenthesized lists of terminals means that the the terminals
between the parenthesis will be coalesced element wise. The number of
elements in all parenthesized lists must be equal!
.LI define
If a terminal in the list between curly brackets is not defined it will
be defined and can function as local node.
.LE
.DS
Examples:
.sp 0.5
    net { a[1..8], b, c };   /* connect to one */
    net { (a[1..4]), (b, c, d, e) };   /* connect element wise */
    net { (a[1..4]) };   /* define local array */
.DE

.H 2 "External Network Declaration"
.DS
.TB "Syntax of an extern network declaration."
.TS
center tab(/);
ll.
_
extern_network_decl/= \fIextern network identifier\fP decl_part.
.TE
.DE
Apart from one or more network descriptions, a network file
may also contain one or more external network declarations.
An external network declaration 
begins with the keywords \fIextern\fP and
\fInetwork\fP followed
by the name of the network, and followed by
a declaration part similar to the declaration part of a network description.
An external network declaration is used to
specify a sequence for the terminals of networks that 
are not described in the network file in which they are used as instance.
This sequence is then used to connect the terminals of each instance
of the subnetwork.
An external network declaration may for example 
be useful when describing networks
that contain subnetworks that are generated by an extraction program.
The extraction program will generate the terminals in an arbitrarily
order and in order to appropriately connect these terminals,
an external network declaration
of the subnetwork should be added to each file in which the subnetwork
is used as an instance.
.DS
Example:
.sp 0.5
    extern network subntw (terminal in[1..2], out, vdd, vss)

    network totalntw (terminal x[1..2], vdd, vss)
    {
        .
        .

        subntw (a, b, c, vdd, vss);
    }
.DE

.H 2 "Global Nets"
A net (node) may be defined a "global net"
by specifying it in a file called 'global_nets'.
This file may optionally be present in the current project directory
or, otherwise, in the corresponding process directory.
The file 'global_nets' is read by the program
.I csls,
and for each network that is added to the database,
.I csls
takes care that at least terminals are present that have a name 
equal to the names of all global nets that
are specified in the file 'global_nets'.
Further, 
.I csls 
connects all terminals and nets that have a same name
that is a global net name.
Names of global nets may not have an array form.
.SK

.H 2 "Examples"
.H 3 "latch example"
A latch is a two phase clocked register element.
Figure 2 shows the transistor diagram and the network
description of the latch circuit.
The circuit consists of three inverter stages.
Two of these stages are connected as a flipflop by phi2_l.
The input signal is clocked by phi1,
whereas the output stage is clocked by phi2_r.
.SP
.PS
define enh0 X
	[move to (0 , 1)
	line right 0.5
	move to (0.5 , 1.5)
	line down 1
	move to (1, 0)
	line up 0.5 \
	then left 0.4 \
	then up 1 \
	then right 0.4 \
	then up 0.5]
X
define enh90 X
	[move to (1 , 0)
	line up 0.5
	move to (0.5 , 0.5)
	line right 1
	move to (2, 1)
	line left 0.5 \
	then down 0.4 \
	then left 1 \
	then up 0.4 \
	then left 0.5]
X
define enh180 X
	[move to (1 , 1)
	line left 0.5
	move to (0.5 , 1.5)
	line down 1
	move to (0, 0)
	line up 0.5 \
	then right 0.4 \
	then up 1 \
	then left 0.4 \
	then up 0.5]
X
define enh270 X
	[move to (1 , 1)
	line down 0.5
	move to (0.5 , 0.5)
	line right 1
	move to (0, 0)
	line right 0.5 \
	then up 0.4 \
	then right 1 \
	then down 0.4 \
	then right 0.5]
X
define dep0 X
	[move to (0 , 1)
	line right 0.5
	move to (0.5 , 1.5)
	line down 1
	move to (0.7 , 1.5)
	line down 1
	move to (0.68 , 1.5)
	line down 1
	move to (0.66 , 1.5)
	line down 1
	move to (0.64 , 1.5)
	line down 1
	move to (0.62 , 1.5)
	line down 1
	move to (1, 0)
	line up 0.5 \
	then left 0.4 \
	then up 1 \
	then right 0.4 \
	then up 0.5]
X
define dep90 X
	[move to (1 , 0)
	line up 0.5
	move to (0.5 , 0.5)
	line right 1
	move to (0.5 , 0.7)
	line right 1
	move to (0.5 , 0.68)
	line right 1
	move to (0.5 , 0.66)
	line right 1
	move to (0.5 , 0.64)
	line right 1
	move to (0.5 , 0.62)
	line right 1
	move to (2, 1)
	line left 0.5 \
	then down 0.4 \
	then left 1 \
	then up 0.4 \
	then left 0.5]
X
define dep180 X
	[move to (1 , 1)
	line left 0.5
	move to (0.5 , 1.5)
	line down 1
	move to (0.3 , 1.5)
	line down 1
	move to (0.32 , 1.5)
	line down 1
	move to (0.34 , 1.5)
	line down 1
	move to (0.36 , 1.5)
	line down 1
	move to (0.38 , 1.5)
	line down 1
	move to (0, 0)
	line up 0.5 \
	then right 0.4 \
	then up 1 \
	then left 0.4 \
	then up 0.5]
X
define dep270 X
	[move to (1 , 1)
	line down 0.5
	move to (0.5 , 0.5)
	line right 1
	move to (0.5 , 0.3)
	line right 1
	move to (0.5 , 0.32)
	line right 1
	move to (0.5 , 0.34)
	line right 1
	move to (0.5 , 0.36)
	line right 1
	move to (0.5 , 0.38)
	line right 1
	move to (0, 0)
	line right 0.5 \
	then up 0.4 \
	then right 1 \
	then down 0.4 \
	then right 0.5]
X
define invert X
	[
		TE1 : enh0
		TD1 : [dep0] with .s at TE1.n
		move to TD1.w
		line down 1 then right 1
	]
X
define cinvert X
	[
		TE1 : enh270
		INV: [invert] with .sw at TE1.se - (0, 1) 
	]
X
define latch X
	[
		CINV1	: [cinvert]
		TE1	: [enh270] with .sw at CINV1.ne + (1, -1)
		INV1	: [invert] with .sw at CINV1.se + (4, 0)
		CINV2	: [cinvert] with .sw at INV1.se + (1, 0)
		move to TE1.sw
		line down 2 then left 2
		move to TE1.se
		line right 1
		move to TE1.n
		line up 0.5
		move to CINV1.e
		line right 2 then down 1 then right 4
		move to CINV1.ne
		VDD	: line right 9
		move to CINV1.se
		VSS	: line right 9
		move to CINV2.e
		line right 0.5
		"vdd" at INV1.n above
		"vss" at INV1.s below
		"7" at INV1.w + (0, 1) above
		"out" at CINV2.e + (0.6, 0) ljust 
		"in" at CINV1.w + (-0.1, -1) rjust 
		"6" at CINV1.w + (2, -1) above 
		"9" at INV1.w + (0, -1) above 
		"phi1" at CINV1.w + (1, 0) above 
		"phi2_l" at TE1.n + (0, 0.5) above 
		"phi2_r" at CINV2.w + (1, 0) above 
		"10" at CINV2.w + (2, -1) above 
	]	
X
scale = 2.3
latch
.PE
.SP
.fS I
network latch (terminal vdd, vss, phi1, phi2, out, in)
{
   net {phi2, phi2_r, phi2_l}; /* equivalent nodes */
   nenh w=8u l=4u      (9,      vss,    7);
   nenh w=8u l=4u      (10,     vss,    out);
   nenh w=8u l=4u      (6,      vss,    9);
   nenh w=8u l=4u      (phi1,   in,     6);
   nenh w=8u l=4u      (phi2_l, 6,      7);
   nenh w=8u l=4u      (phi2_r, 9,      10);
   ndep w=6u l=18u     (out,    out,    vdd);
   ndep w=6u l=18u     (9,      vdd,    9);
   ndep w=6u l=18u     (7,      7,      vdd);
}
.fE
.FG "Transistor diagram and network description of the latch circuit."
.SP
Note that the two terminals phi2_l and phi2_r are coalesced
to one terminal named phi2 in the net declaration,
for these terminals represented the same electrical node.
.SK

.H 3 "hierarchical latch example"
The hierarchical network description of the latch is shown in Figure 3.
The latch consists of three inverter networks connected
by pass transistors.
Inside each inverter network
you will find terminals that have equal names.
These terminals represent feed-throughs in the inverter and
are electrically equivalent.
.SP
.PS
define enh0 X
	[move to (0 , 1)
	line right 0.5
	move to (0.5 , 1.5)
	line down 1
	move to (1, 0)
	line up 0.5 \
	then left 0.4 \
	then up 1 \
	then right 0.4 \
	then up 0.5]
X
define enh90 X
	[move to (1 , 0)
	line up 0.5
	move to (0.5 , 0.5)
	line right 1
	move to (2, 1)
	line left 0.5 \
	then down 0.4 \
	then left 1 \
	then up 0.4 \
	then left 0.5]
X
define enh180 X
	[move to (1 , 1)
	line left 0.5
	move to (0.5 , 1.5)
	line down 1
	move to (0, 0)
	line up 0.5 \
	then right 0.4 \
	then up 1 \
	then left 0.4 \
	then up 0.5]
X
define enh270 X
	[move to (1 , 1)
	line down 0.5
	move to (0.5 , 0.5)
	line right 1
	move to (0, 0)
	line right 0.5 \
	then up 0.4 \
	then right 1 \
	then down 0.4 \
	then right 0.5]
X
define dep0 X
	[move to (0 , 1)
	line right 0.5
	move to (0.5 , 1.5)
	line down 1
	move to (0.7 , 1.5)
	line down 1
	move to (0.68 , 1.5)
	line down 1
	move to (0.66 , 1.5)
	line down 1
	move to (0.64 , 1.5)
	line down 1
	move to (0.62 , 1.5)
	line down 1
	move to (1, 0)
	line up 0.5 \
	then left 0.4 \
	then up 1 \
	then right 0.4 \
	then up 0.5]
X
define dep90 X
	[move to (1 , 0)
	line up 0.5
	move to (0.5 , 0.5)
	line right 1
	move to (0.5 , 0.7)
	line right 1
	move to (0.5 , 0.68)
	line right 1
	move to (0.5 , 0.66)
	line right 1
	move to (0.5 , 0.64)
	line right 1
	move to (0.5 , 0.62)
	line right 1
	move to (2, 1)
	line left 0.5 \
	then down 0.4 \
	then left 1 \
	then up 0.4 \
	then left 0.5]
X
define dep180 X
	[move to (1 , 1)
	line left 0.5
	move to (0.5 , 1.5)
	line down 1
	move to (0.3 , 1.5)
	line down 1
	move to (0.32 , 1.5)
	line down 1
	move to (0.34 , 1.5)
	line down 1
	move to (0.36 , 1.5)
	line down 1
	move to (0.38 , 1.5)
	line down 1
	move to (0, 0)
	line up 0.5 \
	then right 0.4 \
	then up 1 \
	then left 0.4 \
	then up 0.5]
X
define dep270 X
	[move to (1 , 1)
	line down 0.5
	move to (0.5 , 0.5)
	line right 1
	move to (0.5 , 0.3)
	line right 1
	move to (0.5 , 0.32)
	line right 1
	move to (0.5 , 0.34)
	line right 1
	move to (0.5 , 0.36)
	line right 1
	move to (0.5 , 0.38)
	line right 1
	move to (0, 0)
	line right 0.5 \
	then up 0.4 \
	then right 1 \
	then down 0.4 \
	then right 0.5]
X
define hinvert X
	[
		Ibox	: box ht 4 wid 2 "invert"
		"i" at Ibox.sw + (0.2, 1) ljust
		"i" at Ibox.se + (-0.1, 1) rjust
		"i" at Ibox.ne + (-0.1, -1) rjust
		"o" at Ibox.nw + (0.2, -1) ljust
		"o" at Ibox.e  + (-0.1, 0) rjust
		"vdd" at Ibox.n below
		"vss" at Ibox.s above
		line from Ibox.n up 0.5
		line from Ibox.s down 0.5
	]
X
define hlatch X
	[
		TE1	: [enh270]
		HINV1	: [hinvert] with .sw at TE1.se + (0, -1.5)
		TE2	: [enh270] with .sw at HINV1.ne + (0, -1.5)
		HINV2	: [hinvert] with .nw at TE2.se + (0, 1.5)
		move to HINV1.e
		line right 1 then down 1 then right 1
		TE3	: [enh270] with .sw at HINV2.se + (0, 1.5)
		HINV3	: [hinvert] with .sw at TE3.se + (0, -1.5)
		move to HINV3.e
		line right 0.5
		move to HINV1.n
		VDD	: line right 8
		move to HINV1.s
		VSS	: line right 8
		"vdd" at HINV2.n above
		"vss" at HINV2.s below
		"7" at TE2.se + (-0.2, 0) below
		"out" at HINV3.e + (0.6, 0) ljust 
		"in" at TE1.sw + (-0.1, 0) rjust
		"6" at TE2.sw + (0.2, 0) below 
		"9" at HINV1.e + (0.2, 0) below 
		"phi1" at TE1.n above 
		"phi2_l" at TE2.n above 
		"phi2_r" at TE3.n above 
		"10" at TE3.se + (-0.2, 0) below 
		"9" at TE3.sw + (0.2, 0) below 
	]	
X
scale = 2.4
hlatch
.PE
.SP
.fS I
network invert (terminal i, o, vdd, gnd)
{
   nenh w=8u l=4u     (i,   o,     gnd);
   ndep w=6u l=18u    (o,   vdd,   o);
}

network latch (terminal vdd, vss, phi1, phi2, out, in)
{
   net {phi2, phi2_r, phi2_l}; /* equivalent nodes */
   {inv[1..3]} invert (6,  9,   vdd, vss,
                       9,  7,   vdd, vss,
                       10, out, vdd, vss);
   nenh w=8u l=4u     (phi1,    in,   6);
   nenh w=8u l=4u     (phi2_l,  6,    7);
   nenh w=8u l=4u     (phi2_r,  9,   10);
}
.fE
.FG "Hierarchical description of the latch circuit."
.SP
.SK

.H 3 "latch with nand example"
The previous latch example only contained transistors.
Besides transistors, resistors and capacitors it is
also possible to use function elements
(ands, ors, nands, nors and
exors) in the network description.
In the following example the network inverter of the hierarchical latch circuit
has been changed.
The n-enhancement and depletion transistor have been substituted
by a nand gate with one input.
The total network description is now.
.SP
.fS I
network invert (terminal i, o)
{
   @ nand tr=5n tf=3n    (i,    o);
}
  
network latch (terminal vdd, vss, phi1, phi2, out, in)
{
   net {phi2, phi2_r, phi2_l}; /* equivalent nodes */
   {inv[1..3]} invert (6,  9, 
                       9,  7,
                       10, out);
   nenh w=8u l=4u     (phi1,    in,   6);
   nenh w=8u l=4u     (phi2_l,  6,    7);
   nenh w=8u l=4u     (phi2_r,  9,   10);
}
.fE
.SP
The last node name of the nand statement gives the output of the nand.
In this example there is only one input node for the nand,
so only one node precedes the output node.
However, it is possible to use an arbitrarily number of input
nodes for a function.
For the nand a rise time of 5 nsec. and a fall time of 3 nsec.
have been specified.
.SK

.H 3 "shift example"
Figure 4 shows the transistor diagram and the
network description of a circuit which we will call shift.
.SP
.PS
define enh0 #
	[move to (0 , 1)
	line right 0.5
	move to (0.5 , 1.5)
	line down 1
	move to (1, 0)
	line up 0.5 \
	then left 0.4 \
	then up 1 \
	then right 0.4 \
	then up 0.5]
#
define enh90 #
	[move to (1 , 0)
	line up 0.5
	move to (0.5 , 0.5)
	line right 1
	move to (2, 1)
	line left 0.5 \
	then down 0.4 \
	then left 1 \
	then up 0.4 \
	then left 0.5]
#
define enh180 #
	[move to (1 , 1)
	line left 0.5
	move to (0.5 , 1.5)
	line down 1
	move to (0, 0)
	line up 0.5 \
	then right 0.4 \
	then up 1 \
	then left 0.4 \
	then up 0.5]
#
define enh270 #
	[move to (1 , 1)
	line down 0.5
	move to (0.5 , 0.5)
	line right 1
	move to (0, 0)
	line right 0.5 \
	then up 0.4 \
	then right 1 \
	then down 0.4 \
	then right 0.5]
#
define dep0 #
	[move to (0 , 1)
	line right 0.5
	move to (0.5 , 1.5)
	line down 1
	move to (0.7 , 1.5)
	line down 1
	move to (0.68 , 1.5)
	line down 1
	move to (0.66 , 1.5)
	line down 1
	move to (0.64 , 1.5)
	line down 1
	move to (0.62 , 1.5)
	line down 1
	move to (1, 0)
	line up 0.5 \
	then left 0.4 \
	then up 1 \
	then right 0.4 \
	then up 0.5]
#
define dep90 #
	[move to (1 , 0)
	line up 0.5
	move to (0.5 , 0.5)
	line right 1
	move to (0.5 , 0.7)
	line right 1
	move to (0.5 , 0.68)
	line right 1
	move to (0.5 , 0.66)
	line right 1
	move to (0.5 , 0.64)
	line right 1
	move to (0.5 , 0.62)
	line right 1
	move to (2, 1)
	line left 0.5 \
	then down 0.4 \
	then left 1 \
	then up 0.4 \
	then left 0.5]
#
define dep180 #
	[move to (1 , 1)
	line left 0.5
	move to (0.5 , 1.5)
	line down 1
	move to (0.3 , 1.5)
	line down 1
	move to (0.32 , 1.5)
	line down 1
	move to (0.34 , 1.5)
	line down 1
	move to (0.36 , 1.5)
	line down 1
	move to (0.38 , 1.5)
	line down 1
	move to (0, 0)
	line up 0.5 \
	then right 0.4 \
	then up 1 \
	then left 0.4 \
	then up 0.5]
#
define dep270 #
	[move to (1 , 1)
	line down 0.5
	move to (0.5 , 0.5)
	line right 1
	move to (0.5 , 0.3)
	line right 1
	move to (0.5 , 0.32)
	line right 1
	move to (0.5 , 0.34)
	line right 1
	move to (0.5 , 0.36)
	line right 1
	move to (0.5 , 0.38)
	line right 1
	move to (0, 0)
	line right 0.5 \
	then up 0.4 \
	then right 1 \
	then down 0.4 \
	then right 0.5]
#
define invert #
	[
		TE1 : enh0
		TD1 : [dep0] with .s at TE1.n
		move to TD1.w
		line down 1 then right 1
	]
#
define satinvert #
	[
		TE1 : [enh0]
		TD1 : [enh0] with .s at TE1.n
		move to TD1.w
		line up 1 then right 1
	]
#
define cap0 #
	[
		Plates	: [
				line right 0.6
				move up 0.2
				line left 0.6
			  ]
		move to Plates.n
		line up 0.9
		move to Plates.s
		line down 0.9
	]
#
define res90 #
	[
		Bx	: [box ht 0.3 wid 1]
		move to Bx.e
		line right 0.5
		move to Bx.w
		line left 0.5
	]
#
define shift #
	[
		EINV1	: [invert]
		move to EINV1.w + (0, -1)
		line left 1
		TE1	: [enh270] with .sw at EINV1.e
		CAP1	: [cap0] with .n at TE1.se
		TE2	: [enh270] with .sw at TE1.se
		move to TE2.se
		line down 1 then right 1
		INV1	: [satinvert] with .w at TE2.se + (1, 0)
		RES1	: [res90] with .w at INV1.e
		CAP2	: [cap0] with .n at RES1.e
		move to CAP2.n
		line right 1
		move to EINV1.sw + (-1, 0)
		line right 11
		move to EINV1.nw + (-1, 0)
		line right 11
		"vdd" at EINV1.nw above
		"vss" at EINV1.sw below
		"in" at EINV1.w + (-1, -1) above
		"out" at RES1.e + (1, 0) above
		"phi1" at TE1.n above
		"phi2" at TE2.n above
		"1" at EINV1.e + (0.2, 0) above
		"2" at TE1.se + (0, 0) above
		"3" at INV1.w + (0, -1) above
		"4" at INV1.e + (-0.3, 0)
	]	
#
scale = 2
shift
.PE
.SP
.fS I
network shift (terminal vdd, vss, phi1, phi2, in, out)
{
   nenh w=6u  l=4u     (in,     vss,    1);
   ndep w=6u  l=10u    (1,      1,      vdd);
   nenh w=8u  l=4u     (phi1,   1,      2);
   cap  400f   (2,    vss);
   nenh w=8u  l=4u     (phi2,   2,      3);
   nenh w=6u  l=4u     (3,      vss,    4);
   nenh w=6u  l=4u     (vdd,    4,      vdd);
   res  20k    (4,    out);
   cap  150f   (out,  vss);
}
.fE
.SP
.FG "Transistor diagram and network description of the shift circuit."
.SP
In the shift circuit, 
the inverted input signal is clocked by phi1 onto a large storage capacitance.
When phi1 becomes low and phi2 becomes high, 
the storage capacitance shares its charge 
with the gate capacitance of the second inverter stage.
Because the gate capacitance is several times smaller then the storage
capacitance, the gate will obtain the same state as the storage capacitance.
The second inverter stage consists of two enhancement transistors
with the upper transistor being saturated and acting as load
for the lower transistor.
The resistor and capacitor on the output represent interconnection
resistance and capacitance.
.SK

.H 3 "flipflop example"
Figure 5 shows the transistor diagram and the
network description of a flipflop circuit.
In this circuit
the two nand circuit stages are cross coupled together
by attaching the output of one nand to an input of the other.
.SP
.PS
define enh0 #
	[move to (0 , 1)
	line right 0.5
	move to (0.5 , 1.5)
	line down 1
	move to (1, 0)
	line up 0.5 \
	then left 0.4 \
	then up 1 \
	then right 0.4 \
	then up 0.5]
#
define enh90 #
	[move to (1 , 0)
	line up 0.5
	move to (0.5 , 0.5)
	line right 1
	move to (2, 1)
	line left 0.5 \
	then down 0.4 \
	then left 1 \
	then up 0.4 \
	then left 0.5]
#
define enh180 #
	[move to (1 , 1)
	line left 0.5
	move to (0.5 , 1.5)
	line down 1
	move to (0, 0)
	line up 0.5 \
	then right 0.4 \
	then up 1 \
	then left 0.4 \
	then up 0.5]
#
define enh270 #
	[move to (1 , 1)
	line down 0.5
	move to (0.5 , 0.5)
	line right 1
	move to (0, 0)
	line right 0.5 \
	then up 0.4 \
	then right 1 \
	then down 0.4 \
	then right 0.5]
#
define dep0 #
	[move to (0 , 1)
	line right 0.5
	move to (0.5 , 1.5)
	line down 1
	move to (0.7 , 1.5)
	line down 1
	move to (0.68 , 1.5)
	line down 1
	move to (0.66 , 1.5)
	line down 1
	move to (0.64 , 1.5)
	line down 1
	move to (0.62 , 1.5)
	line down 1
	move to (1, 0)
	line up 0.5 \
	then left 0.4 \
	then up 1 \
	then right 0.4 \
	then up 0.5]
#
define dep90 #
	[move to (1 , 0)
	line up 0.5
	move to (0.5 , 0.5)
	line right 1
	move to (0.5 , 0.7)
	line right 1
	move to (0.5 , 0.68)
	line right 1
	move to (0.5 , 0.66)
	line right 1
	move to (0.5 , 0.64)
	line right 1
	move to (0.5 , 0.62)
	line right 1
	move to (2, 1)
	line left 0.5 \
	then down 0.4 \
	then left 1 \
	then up 0.4 \
	then left 0.5]
#
define dep180 #
	[move to (1 , 1)
	line left 0.5
	move to (0.5 , 1.5)
	line down 1
	move to (0.3 , 1.5)
	line down 1
	move to (0.32 , 1.5)
	line down 1
	move to (0.34 , 1.5)
	line down 1
	move to (0.36 , 1.5)
	line down 1
	move to (0.38 , 1.5)
	line down 1
	move to (0, 0)
	line up 0.5 \
	then right 0.4 \
	then up 1 \
	then left 0.4 \
	then up 0.5]
#
define dep270 #
	[move to (1 , 1)
	line down 0.5
	move to (0.5 , 0.5)
	line right 1
	move to (0.5 , 0.3)
	line right 1
	move to (0.5 , 0.32)
	line right 1
	move to (0.5 , 0.34)
	line right 1
	move to (0.5 , 0.36)
	line right 1
	move to (0.5 , 0.38)
	line right 1
	move to (0, 0)
	line right 0.5 \
	then up 0.4 \
	then right 1 \
	then down 0.4 \
	then right 0.5]
#
define invert #
	[
		TE1 : enh0
		TD1 : [dep0] with .s at TE1.n
		move to TD1.w
		line down 1 then right 1
	]
#
define einvert #
	[
		TE1 : [enh0]
		TD1 : [dep0] with .s at TE1.n
		move to TD1.w
		line up 1 then right 1
	]
#
define cap0 #
	[
		Plates	: [
				line right 0.6
				move up 0.2
				line left 0.6
			  ]
		move to Plates.n
		line up 0.9
		move to Plates.s
		line down 0.9
	]
#
define res90 #
	[
		Bx	: [box ht 0.3 wid 1]
		move to Bx.e
		line right 0.5
		move to Bx.w
		line left 0.5
	]
#
define flipflop #
	[
		TE1	: [enh0]
		TE2	: [enh180] with .sw at TE1.ne
		TD1	: [dep0] with .se at TE2.nw + (0, 1)
		line from TD1.se to TE2.nw
		line from TD1.w down 1 then right 1
		line from TD1.se right 1 then down 2 right 1 
		line from TE2.nw left 1.5
		CAP1	: [cap0] with .n at TE2.nw + (-1.5, 0)
		line from CAP1.s + (-0.25,0) right 0.5
		TE3	: [enh180] with .sw at TE1.se + (3, 0)
		TE4	: [enh0] with .se at TE3.nw
		TD2	: [dep180] with .sw at TE4.ne + (0, 1)
		line from TD2.sw to TE4.ne
		line from TD2.e down 1 then left 1
		line from TD2.sw left 1 then down 2 left 1 
		line from TE4.ne right 1.5
		CAP2	: [cap0] with .n at TE4.ne + (1.5, 0)
		line from CAP2.s + (-0.25,0) right 0.5
		line from TD1.ne + (-2, 0) right 7
		line from TE1.se + (-2, 0) right 7
		line from TE1.w left 1
		line from TE3.e right 1
		"vdd" at TD1.ne + (1.5, 0) above
		"vss" at TE1.se + (1.5, 0) below
		"in[1]" at TE1.w + (-1.1, 0) rjust
		"out[1]" at CAP1.n + (-0.1, 0) rjust
		"out[2]" at CAP2.n + (0.1, 0) ljust
		"in[2]" at TE3.e + (1.1, 0) ljust
	]	
#
scale = 2
flipflop
.PE
.SP
.fS I
network flipflop (terminal vdd, vss, in[1..2], 
                                     out[1..2]) 
{
   nenh w=6u    l=4u   (in[1],   vss,     1);
   nenh w=6u    l=4u   (out[1],  2,       out[2]);
   nenh w=6u    l=4u   (in[2],   vss,     2);
   nenh w=6u    l=4u   (out[2],  1,       out[1]);
   ndep w=6u    l=20u  (out[1],  out[1],  vdd);
   ndep w=6u    l=20u  (out[2],  out[2],  vdd);
   cap  100f   (out[1], gnd);
   cap  150f   (out[2], gnd);
}
.fE
.SP
.FG "Transistor diagram and network description of the flipflop circuit."
.SK
.H 1 "SIMULATION AND SIMULATION COMMANDS"
.H 2 "Simulation Control Commands"
The simulation control commands must reside in a separate file
(the command file).
This file contains the signal descriptions of the network inputs
and the commands which specify options and output
format.
.DS
.TB "Syntax of the simulation control part."
.TS
center tab(/);
ll.
_
sim_cmd_list/= sim_cmd ( ";" | \fInewline\fP )
/  { sim_cmd ( ";" | \fInewline\fP ) }.
sim_cmd/= set_cmd | fill_cmd | def_cmd | print_cmd | plot_cmd
/| dump_cmd | initialize_cmd | dissip_cmd | option_cmd | \fIempty\fP.
set_cmd/= \fIset\fP node_refs "=" signal_exp
/| \fIset\fP node_refs ":" node_refs \fIfrom\fP \fIstring\fP.
signal_exp/= value_exp { value_exp }.
value_exp/= value [ "*" duration ].
value/= \fIh\fP | \fIl\fP | \fIx\fP | \fIf\fP | "(" signal_exp ")".
duration/= \fIinteger\fP | "~".
fill_cmd/= \fIfill\fP full_node_ref \fIwith\fP fill_vals.
fill_vals/= fill_val { fill_val }.
fill_val/= \fIstring\fP | \fIinteger\fP | \fIf_float\fP.
def_cmd/= \fIdefine\fP node_refs ":" \fIidentifier\fP def_minterms.
def_minterms/= def_minterm { def_minterm }.
def_minterm/= def_in_val { def_in_val } ":" def_out_val.
def_in_val/= \fIh\fP | \fIl\fP | \fIx\fP | \fI-\fP.
def_out_val/= \fIinteger\fP | \fIidentifier\fP | $\fIidentifier\fP.
print_cmd/= \fIprint\fP node_refs.
plot_cmd/= \fIplot\fP node_refs.
dump_cmd/= \fIdump\fP \fIat\fP \fIinteger\fP.
initialize_cmd/= \fIinitialize\fP \fIfrom\fP \fIstring\fP.
dissip_cmd/= \fIdissipation\fP [ node_refs ].
option_cmd/= \fIoption\fP option { option }.
option/= \fIsimperiod\fP "=" \fIinteger\fP
/| \fIsigoffset\fP "=" \fIinteger\fP
/| \fIsigunit\fP "=" \fIf_float\fP
/| \fIoutunit\fP "=" \fIpower_ten\fP
/| \fIoutacc\fP  "=" \fIpower_ten\fP
/| \fIlevel\fP  "=" \fIinteger\fP
/| \fIprocess\fP "=" \fIstring\fP 
/| \fItdevmin\fP "=" \fIf_float\fP
/| \fItdevmax\fP "=" \fIf_float\fP
/| \fIstep\fP  "=" ( \fIon\fP | \fIoff\fP )
/| \fIprint races\fP  "=" ( \fIon\fP | \fIoff\fP )
/| \fImaxldepth\fP "=" \fIinteger\fP
/| \fIonly changes\fP "=" ( \fIon\fP | \fIoff\fP )
/| \fIdisperiod\fP "=" \fIinteger\fP
/| \fIprint devices\fP "=" ( \fIon\fP | \fIoff\fP )
/| \fIprint statistics\fP "=" ( \fIon\fP | \fIoff\fP )
/| \fImaxpagewidth\fP "=" \fIinteger\fP
/| \fIvh\fP "=" \fIf_float\fP
/| \fIvminh\fP "=" \fIf_float\fP
/| \fIvmaxl\fP "=" \fIf_float\fP
/| \fIinitialize random\fP  "=" ( \fIon\fP | \fIoff\fP )
/| \fIinitialize full random\fP  "=" ( \fIon\fP | \fIoff\fP )
/| \fIsta_file\fP  "=" ( \fIon\fP | \fIoff\fP ).
node_refs/= node_ref_item { node_ref_item }.
node_ref_item/= full_node_ref | ",".
full_node_ref/= [ "!" ] { inst_name "." } node_ref.
inst_name/= \fIidentifier\fP [ "[" index_list "]" ].
node_ref/= \fIinteger\fP | \fIidentifier\fP [ "[" index_list "]" ].
index_list/= index { "," index }.
index/= \fIinteger\fP | range.
range/= \fIinteger\fP ".." \fIinteger\fP.
.TE
.DE
Simulation commands are separated by a newline character or by a ";".
So unlike with the network description language, a newline character
can not be used here as an ordinary separator between symbols.
However, a newline can be escaped as a command separator by preceding it
with a "\\".
.P
For an explanation of
the non-literal terminals (e.g \fIpower_ten\fP, \fIf_float\fP, etc.)
the reader is referred to the table "lexical constructions" in the network
syntax description part in this manual
.P
Examples of node references (node_refs) in the commandfile are
.DS
    cin  4,  counter1.latch_a.112
    total.submod[0..7,3].in[4..1]
.DE
with counter1, latch_a, total and submod[0..7,3] being instance names.
.P
The explanation to the simulation commands is given in the next
paragraphs.

.H 3 "set command"
The set command describes the pulse signals that are applied to the network.
A node with a signal attached is by definition an input node.
(In principle each node in the network, including non-terminal nodes,
may be used as an input node).
The signal expression that describes the signal in the set command 
consists of a sequence of value expressions.
A value expression specifies
a logic level (i.e. \fIl\fP = 0, \fIh\fP = 1 and \fIx\fP = undefined),
the absence of a forced signal (i.e. \fIf\fP = free state)
or a nested signal expression (between curly brackets), 
all with an optional duration.
The duration specifies how many times the value expression is added
to the preceding value expressions.
By absence of the duration the default 1 is assumed.
The values \fIl\fP, \fIh\fP, \fIx\fP or \fIf\fP
are pulse functions with width is 1.
So for the following set command:
.DS
    set node1 = l*2 h*~
.DE
the process of generation is shown in Figure 6.
.SP
.DS
.PS
define puls0 #
	[
	   line down 0.5 then right 0.5 * $1\
	   then up 0.5 then right 0.5 * $1
	]
#
define puls1 #
	[
	   line up 0.5 then right 0.5 * $1\
	   then down 0.5 then right 0.5 * $1
	]
#
define phi1 #
	[
	    right
	    puls1(1)
	    puls1(1)
	    puls1(1)
	    puls1(1)
	    puls1(1)
	    puls1(1)
	    puls1(1)
	    puls1(1)
	    puls1(1)
	    puls1(1)
	    puls1(1)
	    puls1(1)
	]
#
define phi2 #
	[
	    right
	    puls0(1)
	    puls0(1)
	    puls0(1)
	    puls0(1)
	    puls0(1)
	    puls0(1)
	    puls0(1)
	    puls0(1)
	    puls0(1)
	    puls0(1)
	    puls0(1)
	    puls0(1)
	]
#
define in #
	[
	    Pulse : [
			right
	    		puls1(4)
	    		puls1(4)
		    ]
	    Pend  : [ 	line right 4] with .w at Pulse.se
	]
#
define latch_sgn #
 	[
		Ver	: [ line down 5.5 ]
		Hor	: [ line right 12 ] with .w at Ver.s
		L1	: [line down 0.5 then right 0.5; line right 11.5 dotted]\
				with .w at Ver.n + (0, -0.5)
		L2	: [line down 0.5; line right 1; line right 11 dotted]\
				with .w at Ver.n + (0, -1.5)
		H1	: [line right 0.5 then down 0.5;line right 11.5 dotted]\
				with .w at Ver.n + (0, -2.5)
		HP	: [line up 0.5 then right 12] with .w at Ver.n + (0, -3.5)
		LH	: [line right 1 then up 0.5 then right 11]\
				with .w at Ver.n + (0, -4.5)
		"l" at L1.w + (-1.5,0) ljust
		"l*2" at L2.w + (-1.5,0) ljust
		"h" at H1.w + (-1.5,0) ljust
		"h*~" at HP.w + (-1.5,0) ljust
		"l*2 h*~" at LH.w + (-1.5,0) ljust
		"0" at Ver.se below
		"2" at Ver.se + (1,0) below
		"4" at Ver.se + (2,0) below
		"6" at Ver.se + (3,0) below
		"8" at Ver.se + (4,0) below
		"10" at Ver.se + (5,0) below
		"12" at Ver.se + (6,0) below
		"14" at Ver.se + (7,0) below
		"16" at Ver.se + (8,0) below
		"18" at Ver.se + (9,0) below
		"20" at Ver.se + (10,0) below
		"22" at Ver.se + (11,0) below
		"24" at Ver.se + (12,0) below
		move to Ver.se + (9,-0.7)
		arrow right 1 "time" below
	]
#
scale = 2.54
latch_sgn
.PE
.SP
.FG "Construction of an input signal."
.DE
.SP
A tilde (~) expresses infinite duration.
When no infinite duration is specified in the signal expression,
after the duration of the signal expression
the signal value of the node remains its last value.
.P
Note that it is possible to initialize a node in the high state by
.DS
    set node1 = h*1 f*~
.DE
.P
The other way to apply a signal to a node is by using a signal
description present in a .res file (which can be the output of 
a previous simulation)
Then the first group of nodes is followed by another group of nodes
and a file specification.
The second group of nodes must be nodes which were printed when
the .res file denoted by \fIstring\fP (which is the file name
without the extension ".res") was made.
The number of nodes in the first group must be equal to the number
of nodes in the second group because the node to node
assignment is done in pairs.
For example, to set the signal descriptions of nodes in[1..8] equal
to the simulation results of nodes out[1..8] which are in file "counter.res":
.DS
    set in[1..8] : out[1..8] from "counter"
.DE

.H 3 "fill command"
With this command, state variables of function blocks
can be initialized.
To select a particular state variable, the variable must be preceded
by the (possibly hierarchical) instance name of the function block.
The type of the variable must correspond to the type
of the value that is assigned to it,
with the convention 
that a string is used to specify one or more character values.
.DS
Example:
.sp 0.5
    fill ram1.mem[2..3, 0..3] with  "OIOI"  "OOII"
.DE

.H 3 "define command"
With this command, one can define new output values for the network based
on certain combinations of normal node values.
For example, one may define the value of a state variable based
on the values of a set of nodes, or one may translate the values
of a bundle of nodes into an integer value.
After the keyword "define", first the nodes are specified
that are used to determine the value of the new variable that is defined.
Then, after a colon, the name of the new variable is specified.
Next, an arbitrary number of minterms is defined.
Each minterm consists of a number of
node values (h, l, x or - for don't care) equal to the number of input
nodes, followed by a colon and followed by the corresponding value for the 
new variable.
The output value may be an integer, an identifier 
or a keyword starting with a "$".
The simulator recognizes the following keywords starting with a "$":
.VL 8 2
.LI $bin
print binair representation.
.LI $dec
print decimal representation.
.LI $oct
print octal representation.
.LI $hex
print hexadecimal representation.
.LI "$sbin, $sdec, $soct, $shex"
.br
idem, use left-most value as sign bit.
.LI "$tbin, $tdec, $toct, $thex"
.br
idem, assume two-complements representation.
.LE
.P
During simulation, the simulator traverses the list of minterms,
starting with the first minterm, to find a matching input combination.
If it finds a matching input combination, 
the output value of that minterm is used.
If no matching combination of input values is found,
an X output value is used.
The output values of the new variables can be printed out by referring to 
them in a print command (see below).
.DS
Examples:
.sp 0.5
.ta 1.4c 2.6c
define a_1 a_2	:  inputstate  \e 
	-   h	:  disabled  \e 
	l   l	:  nothing   \e 
	h   l	:  push  
.sp 0.5
define in[1..8] : count8  \e 
	- - - - - - - - : $dec
.DE

.H 3 "print command"
The print command specifies nodes that must be printed out.
The sequence
of the nodes printed out is the same as specified in the print command.
When a comma is specified in the print command,
an empty column is printed on that place in the output.
When an "!" sign is used before a node specification, the inverse value will
be printed for that node.
The output will be printed in a file named to the network and
tagged with the extension ".out".
If the network name is longer than
ten characters it will be truncated to ten characters.
.DS
Examples:
.sp 0.5
    print in[3..0], out[7..0], !notcarry,
    print inv.i inv.o
.DE
The last print command will print node "i" and node "o" of
the sub-cell with instance name "inv".
Apart from any network node, also the value of a user-defined variable
may be printed.

.H 3 "plot command"
For nodes that are specified in the plot command, a description
of their approximating voltage waveforms will be stored in the file
"\fInetwork\fP.plt".
To visualize these voltage waveforms, an appropriate post-processor
is required (lpsig or simeye).
.DS
Example:
.sp 0.5
    plot cout sout inv1.in
.DE

.H 3 "dump command"
With the dump command, the state of a network can be dumped at any time.
Together with the initialize command this allows next simulations
to start from that particular point.
This can be convenient for example when several simulations have to
be done for a circuit in which first a large number of registers
has to be filled by means of shift operations.
The file in which the state will be dumped will be called
"dump.\fItime\fP",
where \fItime\fP is the simulation time at which the dump is done.
During one simulation more than one dump may be executed.
.DS
Example:
.sp 0.5
    dump at 200
.DE

.H 3 "initialize command"
The initialize command specifies a network state file 
that is used to initialize the network state at the beginning
of the simulation.
The network state file must have been obtained by using the dump
command during a previous simulation.
.DS
Example:
.sp 0.5
    initialize from "dump.200"
.DE

.H 3 "dissipation command"
The dissipation command allows the user to obtain
estimated values for the dynamic dissipation in the circuit.
The information provided
is only valid for networks that are described at the transistor
level.
For the (network input) nodes that are given as an argument
to the dissipation command,
the dynamic dissipation in Watts will printed in a file called 
\fIcell\fP.dis.
The total dissipation for the network can also be found there.
By using the option disperiod, it is possible to obtain an average
dissipation for different time intervals.

.H 3 "option command"
.H 4 "simperiod"
Specifies the end time (expressed in sigunit)
of the simulation time interval.
The start of the simulation is always at t = 0.
When simperiod is not specified,
the simulation will stop when the
network has stabilized after the last input change.
.br
.sp 0.5
(default simperiod = endless)

.H 4 "sigoffset"
Specifies the offset that is used for the signal specifications
in the set command. 
That is, each signal specification f(t) in the set command will be 
used as f(t-sigoffset) during simulation.
.br
.sp 0.5
(default sigoffset = 0)

.H 4 "sigunit"
Specifies (in seconds) the unit of signal duration in the set command,
and the unit of each other variable that denotes a time 
(e.g. the unit of \fIsimperiod\fP).
.br
.sp 0.5
(default sigunit = 1)

.H 4 "outunit"
Specifies the unit of time that will be used
in the simulation output.
.br
.sp 0.5
(default outunit = \fIpower_ten\fP that is closest to sigunit)

.H 4 "outacc"
Specifies the unit of the least significant decimal of the time in
the simulation output.
It must hold that outacc <= outunit.
.br
.sp 0.5
(default outacc = outunit)

.H 4 "level"
Specifies the level of simulation.
There are three simulation levels :
.SP
level 1
.SP 0
The simulator uses abstractions for transistors
and nodes to determine logic states.
The conduction of n-enhancement and p-enhancement type transistors
is always considered then to be equal and much larger than the conduction
of depletion transistors,
and the capacitances of all nodes are considered to be equal.
A logic O or I state is only assigned when the conducting path
to an O or I state input has a much larger conduction than the paths to
other input nodes,
or - with charge sharing - only when all nodes have the same logic state.
In other situations the X state is assigned.
.SP 1
level 2
.SP 0
With level 2,
logic state determination is based
on the actual parameters of transistors (i.e. width and length)
and interconnections (i.e. resistances and capacitances).
In this way the simulator
is able to find the right states for circuits 
where charge sharing effects occur between nodes with different
capacitances and different states,
and for circuits that exploit
a high-impedance/low-impedance resistance division
effect between transistors of the same type.
In addition to the latter, 
at level 2 the simulator will also account for the fact
that n and p-enhancement transistors may be saturated.
During simulation at level 2, first for each node
an analogue voltage is determined by means of 
charge sharing or resistance division, and next
a logic state is derived by means of a minimum voltage
for the high state (vminh) and a maximum voltage for the low state (vmaxl).
When a simulation is done at level 2,
dimensions of transistors must be present in the
network and process information is used.
.SP 1
level 3
.SP 0
The simulator can also be used to simulate the timing of a network.
Based on the parameters of the transistors and the interconnections,
also the delay times for the logic state transitions are determined then.
To find the delay times, the simulator uses approximating
piece-wise-linear voltage waveforms that are found by
performing RC constant calculations.
At the start of a simulation at level 3,
first a steady state
of the network is determined according to level 2.
To simulate with timing,
transistor dimensions must be present
in the network file and process information is used.
Only at level 3 the rise and fall delays of the function elements (nands,
nors etc.) are taken into account.
.br
.sp 0.5
(default level = 1)

.H 4 "process"
Specifies the process file that contains the process information
(e.g. transistor model capacitances and resistances)
which is used when
simulation is done at level 2 or 3.
First the working directory is searched for this file.
When it is not found there,
the process library directory is searched.
In \fIslsmod(4ICD)\fP 
and [
.[
Schooneveld SLS parameters
.]] 
the syntax and semantics of a process file is described.
.br
.sp 0.5
(default process = "slsmod")

.H 4 "tdevmin,tdevmax"
By specifying minimum and maximum timing deviations it is possible
to do a min-max delay simulation at level 3.
The delays of the logic state transitions are multiplied by tdevmin
and by tdevmax,
to obtain a min-delay and a max-delay for each logic state
transition.
For example,
when a node normally changes from O to I after 10 ns.,
a tdevmin = 0.6 and a tdevmax = 1.3
causes that the node will change from the O to the X state after 6 ns.,
and to the I state after 13 ns.
In this way it is possible to determine how the final logical state
of the simulated network depends on timing deviations
due to parameter variations or model accuracy
(see also the flipflop example at the end of this chapter).
Note: strictly speaking tdevmin and tdevmax 
speed up and slow down the minimum and maximum voltage waveforms
that are being used by the simulator,
and delay times are affected only indirectly.
.br
.sp 0.5
(default tdevmin = 1,
tdevmax = 1)

.H 4 "step"
When step is on,
the values of nodes that are specified by the print command,
are printed immediately after a change has occurred in the
value of one of these nodes.
In this way the simulation of the network can be debugged by observing all
logic state transitions occurring on the printed nodes.
It must be realized here that even during simulation without timing
(i.e. at level 1 or 2),
the simulator always performs sequential simulation steps to update 
logic states.
When step is off,
the values of the output nodes are printed only
after each new stabilization of the printed nodes at
a particular time.
.br
.sp 0.5
(default step = off)

.H 4 "print races"
When feedback loops are in the network (e.g. flipflops),
there sometimes may occur zero-delay oscillations during the simulation
because of undecided races.
The simulator will automatically put the relevant nodes in the undefined state.
Print races is on
will cause the simulator to give a list
at the end of the simulation output,
which shows where and when
these races appeared.
.br
.sp 0.5
(default print races = on)

.H 4 "maxldepth"
With the maximum logic depth of the network (maxldepth),
the simulator decides when an oscillation
because of undecided races is occurring (see option \fIprint races\fP).
When maxldepth is specified too small
the simulator will put nodes in the undefined state which shouldn't,
and when it is much too large it will cause unnecessary
long simulation times when oscillations occur.
However, the simulator detects that the maximum logic depth is
always less than the number of nodes in the circuit.
.br
.sp 0.5
(default maxldepth = 100)

.H 4 "only changes"
By using only changes is on, in the output file only the logical values
that have changed will be printed.
For an output signal which didn't change a "." will be printed.
In this way it will be more easy to trace changing
signals.
.br
.sp 0.5
(default only changes = off)

.H 4 "disperiod"
Specifies the length of the time intervals (expressed in sigunit)
for which the average dissipation 
is printed (see the dissipation command).
.br
.sp 0.5
(default disperiod = simperiod)

.H 4 "print devices"
Usually the only extra information that is given in the simulation
output is the number of nodes in the network.
When print devices is on also the numbers of the different kinds
of devices (transistors, resistors and functions)
are printed in the output file.
.br
.sp 0.5
(default print devices = off)

.H 4 "print statistics"
When this option is on, simulation statistics like the
number of simulation time points and the number of logic events will be printed.
.br
.sp 0.5
(default print statistics = off)

.H 4 "maxpagewidth"
Specifies the number of characters on a line in the output file.
However, when maxpagewidth >= 80 and all printed nodes will still fit on one
line of 80 characters,
the number of characters on a line in the output file will be 80.
.br
.sp 0.5
(default maxpagewidth = 132)

.H 4 "vh,vminh,vmaxl"
Simulation with level 2 or 3 requires a minimum stable voltage (vminh)
for the I state,
a maximum stable voltage (vmaxl) for the O state,
and a voltage vh for input nodes with the I state (vl - for
input nodes with the O state - is assumed to be 0 always).
When a stable voltage of a node is calculated to be between vmaxl and vminh,
an X state will be assigned to the node.
Usually the variables vh,
vminh and vmaxl are read from the process file.
But it is also possible to overwrite these values by specifying them
in the command file.
Enlarging vmaxl for example,
causes that nodes will be set to the O state
instead of the X state more often.
It must hold that vmaxl < vh/2 < vminh.
.br
.sp 0.5
(default vh,
vminh,
vmaxl are read from the process file)

.H 4 "initialize (full) random"
Normally, when simulating circuits with flipflops or
memory cells that do not have a reset input,
some nodes in the circuit will remain uninitialized or X.
When using the option "initialize random", 
the simulator will randomly set these nodes at O or I.
Default, this initialization will be the same for different
simulation runs of the same network.
However, when using the option "initialize full random", 
the result will be different for each different simulation run.
In general, the above options allow one to reach some valid initial
state at the beginning of a simulation, without
first loading a state over many clock cycles.
.br
.sp 0.5
(default, initialize random = off and initialize full random = off)

.H 4 "sta_file"
Read additional commands from the file \fIcell\fP.sta if it exists.
Typically, this option is used to read in state variable 
definitions and print commands that are generated by an auxiliary program.
.br
.sp 0.5
(default, sta_file = off)
.SK
.H 2 "Examples"
.H 3 "latch example"
The simulation control file of the latch circuit is as follows:
.fS I
/* latch simulation commands */
	 
set in   = (h*4 l*4)*2   
set phi1 = (h*1 l*1)*~
set phi2 = (l*1 h*1)*~
set vdd  = h*~
set vss  = l*~
option simperiod = 10
print vdd vss phi1 phi2 in out

.fE
The set commands result in the following input pulse sequences:
.SP
.PS
define puls0 #
	[
	   line down 0.5 then right 0.5 * $1\
	   then up 0.5 then right 0.5 * $1
	]
#
define puls1 #
	[
	   line up 0.5 then right 0.5 * $1\
	   then down 0.5 then right 0.5 * $1
	]
#
define phi1 #
	[
	    right
	    puls1(1)
	    puls1(1)
	    puls1(1)
	    puls1(1)
	    puls1(1)
	    puls1(1)
	    puls1(1)
	    puls1(1)
	    puls1(1)
	    puls1(1)
	    puls1(1)
	    puls1(1)
	]
#
define phi2 #
	[
	    right
	    puls0(1)
	    puls0(1)
	    puls0(1)
	    puls0(1)
	    puls0(1)
	    puls0(1)
	    puls0(1)
	    puls0(1)
	    puls0(1)
	    puls0(1)
	    puls0(1)
	    puls0(1)
	]
#
define in #
	[
	    Pulse : [
			right
	    		puls1(4)
	    		puls1(4)
		    ]
	    Pend  : [ 	line right 4] with .w at Pulse.se
	]
#
define latch_sgn #
 	[
		Ver	: [ line down 3.5 ]
		Hor	: [ line right 12 ] with .w at Ver.s
		In	: [in] with .nw at Ver.n + (0, -0.5)
		Phi1	: [phi1] with .nw at Ver.n + (0, -1.5)
		Phi2	: [phi2] with .nw at Ver.n + (0, -2.5)
		"in" at In.w + (-1,0) ljust
		"phi1" at Phi1.w + (-1,0) ljust
		"phi2" at Phi2.w + (-1,0) ljust
		"0" at Ver.se below
		"2" at Ver.se + (1,0) below
		"4" at Ver.se + (2,0) below
		"6" at Ver.se + (3,0) below
		"8" at Ver.se + (4,0) below
		"10" at Ver.se + (5,0) below
		"12" at Ver.se + (6,0) below
		"14" at Ver.se + (7,0) below
		"16" at Ver.se + (8,0) below
		"18" at Ver.se + (9,0) below
		"20" at Ver.se + (10,0) below
		"22" at Ver.se + (11,0) below
		"24" at Ver.se + (12,0) below
		move to Ver.se + (9,-0.7)
		arrow right 1 "time" below
	]
#
scale = 2.54
latch_sgn
.PE
.SP
The signals vdd and vss have a constant value during the simulation.
The simulation period is specified by option \fIsimperiod\fP.
.P
The node values of vdd,
vss,
phi1,
phi2,
in,
out will printed in the file "latch.out" as follows:
.fS I
================================================================
                             S L S
                          version: 3.0
               S I M U L A T I O N   R E S U L T S
================================================================
 time         | v v p p i o
 in 1e+00 sec | d s h h n u
              | d s i i   t
              |     1 2
================================================================
            0 | 1 0 1 0 1 x
            1 | 1 0 0 1 1 1
            2 | 1 0 1 0 1 1
            3 | 1 0 0 1 1 1
            4 | 1 0 1 0 0 1
            5 | 1 0 0 1 0 0
            6 | 1 0 1 0 0 0
            7 | 1 0 0 1 0 0
            8 | 1 0 1 0 1 0
            9 | 1 0 0 1 1 1
           10 | 1 0 1 0 1 1
================================================================
  network : latch                                   nodes : 10  
================================================================
.fE
Remark that for the hierarchical latch example node "out"
could also have been referred to by "inv[3].o".

.SK
.H 3 "shift example"
The following command file has been used to simulate the shift circuit:
.fS I
set vss  = l*~
set vdd  = h*~
set phi1 = (h*200 l*200)*2
set phi2 = (l*200 h*200)*2
set in   = l*400 h*400
set 3    = l*1 f*~
option sigunit = 1n
option outacc  = 10p
option level   = 3
print phi1 phi2 in, 2 3 out
.fE
Because a high-impedance/low-impedance resistance division effect
between two n-enhancement transistors (a saturated one and a
and a non-saturated one) is exploited, 
and because charge sharing is used to force the state of a node
with a large capacitance to the state of a node with a small capacitance,
the shift circuit has to be simulated with level 2 or 3.
To illustrate timing simulation,
level 3 has been chosen here.
We have to choose a proper input unit and output accuracy
with the options \fIsigunit\fP and \fIoutacc\fP
in order to see the delays in the simulation output.
Node 3 is initialized in the low state in this command file.
.P 
The output file "shift.out" will be as follows:
.fS I
================================================================
                             S L S
                          version: 3.0
               S I M U L A T I O N   R E S U L T S
================================================================
 time         | p p i   2 3 o
 in 1e-09 sec | h h n       u
              | i i         t
              | 1 2
================================================================
         0.00 | 1 0 0   1 0 1
       200.00 | 0 1 0   1 0 1
       200.14 | 0 1 0   1 1 1
       205.68 | 0 1 0   1 1 0
       400.00 | 1 0 1   1 1 0
       407.85 | 1 0 1   0 1 0
       600.00 | 0 1 1   0 1 0
       600.07 | 0 1 1   0 0 0
       607.07 | 0 1 1   0 0 1
================================================================
  network : shift                                   nodes : 10  
================================================================
.fE

.SK
.H 3 "flipflop example1"
With the flipflop circuit a race condition is possible.
When in[1] and in[2] both change state from O to I simultaneously,
the voltage of both out[1] and out[2] will start falling.
The transistors with their gate connected to out[1] and out[2]
will start to turn off, and the falling of the voltages of out[1] and out[2]
will stop.
In practice however a race condition exists because
the voltage of one output will fall somewhat
faster than the voltage of the other output, and
the flipflop will be driven in a stable state where the fastest output is
low and the other output is high.
The flipflop circuit is first simulated without timing.
The following command file is used:
.fS I
set vss = l*~
set vdd = h*~
set in[1..2]   = l*100 h*100
option sigunit = 1n
option outacc  = 10p
option level   = 2
option step    = on
print in[1..2], out[1..2]
.fE
The output file "flipflop.out" will be as follows:
.fS I
================================================================
                             S L S
                          version: 3.0
               S I M U L A T I O N   R E S U L T S
================================================================
 time         | i i   o o
 in 1e-09 sec | n n   u u
              | * *   t t
              | 1 2   * *
              | * *   1 2
              |       * *
================================================================
         0.00 | 0 0   x x
         0.00 | 0 0   1 1
       100.00 | 1 1   1 1
       100.00 | 1 1   0 0
       100.00 | 1 1   1 1
       100.00 | 1 1   0 0
       100.00 | 1 1   1 1
       100.00 | 1 1   0 0
       100.00 | 1 1   1 1
       100.00 | 1 1   x x
================================================================
  races occurred during simulation :
         time  nodes
       100.00  out[1] out[2]
================================================================
  network : flipflop                                 nodes : 7  
================================================================
.fE
Because option \fIstep\fP is \fIon\fP has been used in the command file,
we see that the state of each node is printed after each simulation step.
Because simulation is done without timing,
out[1] and out[2] will change state to O at t = 100 simultaneously.
The transistors will be turned off then,
node out[1] and out[2] will become I again,
and the circuit will start oscillating because of this undecided race.
After a number of simulations steps equal to the logic depth of the circuit
(which the simulator assumes
to be equal to the number of nodes in the circuit) has occurred at t = 100,
the simulator automaticly puts the oscillating nodes in the X state.

.H 3 "flipflop example2"
A second simulation of the flipflop circuit is done with timing.
The command file is as follows:
.fS I
set vss = l*~
set vdd = h*~
set in[1..2]   = l*100 h*100
option sigunit = 1n
option outacc  = 10p
option level   = 3
option step    = on
print in[1..2], out[1..2]
.fE
The output file "flipflop.out" will be as follows:
.fS I
================================================================
                             S L S
                          version: 3.0
               S I M U L A T I O N   R E S U L T S
================================================================
 time         | i i   o o
 in 1e-09 sec | n n   u u
              | * *   t t
              | 1 2   * *
              | * *   1 2
              |       * *
================================================================
         0.00 | 0 0   x x
         0.00 | 0 0   1 1
       100.00 | 1 1   1 1
       108.53 | 1 1   0 1
================================================================
  network : flipflop                                 nodes : 7  
================================================================
.fE
Because now capacitances are used to find delay times for the logic state
transitions,
and the capacitance of node out[1] is less than the capacitance of node
out[2],
node out[1] will 'win the race' and become O.

.SK
.H 3 "flipflop example3"
Finally a min-max delay simulation of the flipflop circuit is done.
The min-max delay has been specified by means of tdevmin and tdevmax.
Node out[1] will now change from I to O via the X state.
During the interval that out[1] is X,
out[2] also becomes X,
and the X state will result as the stable state for out[1] and out[2].
In this way the simulator indicates that the final logic states depend
on the timing,
and that wrong logic states can result when circuit parameters have 50 percent
deviations.
.P
The following command file was used for simulation:
.fS I
set vss = l*~
set vdd = h*~
set in[1..2]   = l*100 h*100
option sigunit = 1n
option outacc  = 10p
option level   = 3
option tdevmin = 0.5 tdevmax = 2
option step    = on
print in[1..2], out[1..2]
.fE
The output file "flipflop.out" will be as follows:
.fS I
================================================================
                             S L S
                          version: 3.0
               S I M U L A T I O N   R E S U L T S
================================================================
 time         | i i   o o
 in 1e-09 sec | n n   u u
              | * *   t t
              | 1 2   * *
              | * *   1 2
              |       * *
================================================================
         0.00 | 0 0   x x
         0.00 | 0 0   1 1
       100.00 | 1 1   1 1
       104.26 | 1 1   x 1
       104.75 | 1 1   x x
================================================================
  network : flipflop                                 nodes : 7  
================================================================
.fE
.SK
.H 3 "random counter example"
This example shows the approximating (min-max)
voltage waveforms as used during timing simulation.
The approximating voltage waveforms
can be inspected by using the plot command,
and by running the program
.I simeye
or the program
.I lpsig
afterwards.
The pictures as shown in this section have been obtained by running
the program
.I lpsig.
.P
The circuit "rand_cnt" is an 8 bit random counter which uses
a two-phase clock phi1 and phi2 and which
generates an 8 bit random number at the outputs out[0..7].
The command file that has been used for the first simulation
is as follows:
.fS I
set     in[0..7]                = h*~
set     vss_r1 vss_r3           = l*~
set     vdd_r3 vdd_r2 vdd_r1    = h*~
set     phi1_r                  = (h*1 l*1)* ~
set     phi2_r                  = (l*1 h*1)* ~
set     p_ld_r                  = h*1 l*~
set     run_r                   = l*1 h*~
option  simperiod = 32
option  sigunit = 100n
option  outunit = 1n
option  outacc = 10p
option  level = 3
plot    phi1_r phi2_r out[0..7] fb_in
.fE
The output for this simulation is shown below:
.DS
.PS < ../sls/n3.pic
.PE
.DE
Notice the slopes on the signals
out[0..7] and the spikes on fb_in.
Fb_in is a feedback signal which is being constructed from
the signals out[0..7] to generate the next number.
.P
Next we add the line
.fS I
option  tdevmin = 0.7 tdevmax = 1.5
.fE
to the command file in order to perform a min-max delay
simulation.
The result shows that now,
during signal transitions,
the minimum and maximum voltage waveform do no longer coincide.
Also, at fb_in more spikes become visible.
.DS
.PS < ../sls/n4.pic
.PE
.DE
This way we are able to study the influence
of parameter variations and model accuracy on the
simulation results and
we for example see that spikes are possible on fb_in
at the end of each positive pulse of phi1.
.SK
When we increase the uncertainty in the delay times
and use
.fS I
option  tdevmin = 0.3 tdevmax = 1.9
.fE
the result will be as follows:
.DS
.PS < ../sls/n5.pic
.PE
.DE
Notice that now the timing deviations have become too large
and that the X state results at all the output nodes.
In fact, the simulator shows that when timing deviations may occur
that are within the boundaries as specified by
tdevmin and tdevmax
the circuit will most likely not be working correctly.
.SK
.H 1 "TROUBLESHOOTING"
In this chapter some suggestions are given to solve troubles
which may occur during simulation.
.VL 24
.LI \fITrouble\fP
\fIPossible cause\fP
.LI syntax#error
The statement or command did not obey the corresponding
syntax description as given in this manual.
.LI no#signal#delay
You didn't use simulation level 3.
.LI  # 
The output accuracy (option outacc) is too large.
.LI simulation#never#stops
You used a repetitive input signal and didn't specify a simulation end time
(option simperiod).
.LI races#occurred
To find the cause of races: use option step = on and print all nodes
for which a race occurred.
For more information, see the flip flop simulation example and the
explanation to the option print races.
.LI #
A (inappropriate) warning about the occurrence of races may also be given
if the real logic depth of the circuit is more than the value of maxldepth.
In that case, enlarge the value of maxldepth.
.LI wrong#output#values
You didn't set all supply nodes (e.g. vdd and gnd) in the h or l state.
.LI  # 
You did forget to connect nodes by means of a net statement.
.LI  # 
For the circuit simulated right logic states can only be simulated 
at level 2 or 3.
.LI  # 
Some transistors have a wrong length or width.
.LI  # 
If you can't find it: recursively print the gate nodes of the 
transistors that make
up the wrong signal and/or use option step = on.
.LI  # 
Finally: when you use analog circuits like sense amplifiers
it might be possible that the simulator is not capable of determining
the right logic states.
In that case a solution is to replace those circuit parts by gate level
descriptions.
.SK
.[
$LIST$
.]
.TC
