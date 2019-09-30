.H 1 "DESCRIPTION OF THE FUNCTION BLOCKS"
.H 2 "The function block model"
In figure 2, the model of the function block is shown.
.DS
.S -2
.PS < ../funman/mfig2.pic
.S

.FG "Model of the function block"
.DE

The function block itself can be used in only one way: it gets data
as input and produces data as output similar to a software routine.
This routine is executed \fBonly when one or more of its input terminals
change state\fP. The function block must be placed as a subblock in a
network. This network contains nodes that are connected with the terminals
of the function block. The logic state of these nodes determine the actions
of the function block. Nodes in the network that are connected with
a terminal of a function block serve as input or output (or both)
data for the function block. The nodes that are connected with an
input terminal of a function block determine the timing of the executions
of the function block in the network.
In section 4, more will be explained about
the function blocks in an SLS network, in
this section the function block itself will be regarded.
.SK
Looking at the model, four kinds of terminals can be distinguished.
Apart from the
terminals, a function block can contain state variables as well. The several
types of terminals and the state variables of the function block must be used
in the following sense:
.DL
.LI
input terminals: these terminals are used for two purposes. First
these terminals act as 'triggers' for the function block; whenever
an input terminal changes state, the function block will be evaluated.
Second the data on these
terminals is used as input data during the evaluation of the function block.
.LI
inread terminals: these terminals only serve as input data during the
evaluation of the function block.
.LI
output terminals: these terminals are calculated during an
evaluation of the function block.
.LI
inout terminals: these terminals are bidirectional terminals. At the
beginning of an evaluation of the function block these terminals get
the value (logic state) of the nodes in the network they are
connected with. This data can be used in the evaluation of the function.
Then, the values of these terminals can be changed by the function block
and so they serve as output terminals.
.LI
state variables: these variables are local to every instance of a
function block and they are used to store the state of that instance.
The variables can be of several types and their
values can only be changed during an evaluation
of the instance of the function block.
.LE

The terminals can have a zero-, one- or two-dimensional array structure.
Because the terminals of a function block are connected with nodes in a
network, the values these terminals can take are exactly the values the nodes
can take: logic states. The logic state of a node in a Switch-Level network
can be O, X or I. Therefore the value of a terminal of a function block
can also be O, X, or I. In the description of a function block, which is made
in the C programming language, the implementation of a terminal is a
character ('char'). This character can take the
value 'O' (capital o), 'I' (capital i) or 'X' (capital x).
With these values as input data, the
software routine that forms a function block can produce output data
of the same type. Moreover, an output
or inout terminal can be
given the value 'F' (capital f) which means the 'free' value. This value
is the so called third state providing a high impedance
mode, in which
the output terminal appears as if it was disconnected from the
function block.
This value will have as result that the output terminal will \fB not have
any influence on the logic state of the node it is connected with\fP.
.P
In the picture of the function block model, inside the
block capacitances and 'slopes' can be found.
The 'slopes' in the picture indicate that
the delay times of an inout or output terminal can be defined inside the
function block. Doing so, the timing properties of the function block
are defined. The delay times are the times between the moment of change
of the inout or output terminal of the function block and the moment this change
will have effect on the logic state of the node in the network that is connected
with such a terminal.
The value of an inout or output terminal changes immediately when
the function block is evaluated, that is when the value of at least one of the
\fBinput\fP terminals changes.
In figure 3 this mechanism is further illustrated.

.DS
.PS < ../funman/mfig3.pic

.S -2
.TS
l l.
t0	function evaluated as a result of the change of
  	logic state of one its input terminals
t1	node changes logic state O -> I as a result
  	of function evaluation
t2	function evaluated
t3	node changes logic state I -> O
.TE
.S

.FG "State of a network node connected with one inout or output terminal"
.DE

The capacitances that are shown in the function block model
indicate that it is possible to define the input load of the function block.
This input load has influence on the rest of the network the function block is
part of. Within the function block description this input load can
be defined by assigning capacitances to input, inread and inout terminals.
These capacitances will be added to the capacitances of the nodes in the
network that are connected with such terminals.
In that way the capacitances of these nodes will account for the
input load of the function block.
.SK
.H 2 "The syntax of the function model description"
The meta language as proposed by Wirth, and the general conventions as they
are defined in section 3.1. of the SLS User's Manual,
are used to describe the syntax of a function block description.

The reserved keywords are:

.DS 2
.TB "Reserved keywords in function block descriptions"

.TS
l l l.
_
behavior   	function	initial
inout   	input   	inread
load    	output  	state
.TE
.DE

.H 3 "Function Block Description"

.DS 2
.TB "Syntax of the function block part"

.TS
l l.
_
function_decl	= \fIfunction  identifier\fP   decl_part   funct_body
.TE
.DE

The function block part begins with the keyword \fIfunction\fP
followed by the name of the function block, a declaration part describing
the interface to the outside world and the local state variables, and a body
specifying the initial and general behavior of the function block.

.H 3 "Declarations"

.DS 2
.TB "Syntax of the declaration part"

.TS
l l.
_
decl_part	= term_decls  [state_decls]
term_decls	= "(" term_decl {";" term_decl} ")"
term_decl	= term_type  term {"," term }
term_type	= \fIinput\fP  | \fIinread\fP  | \fIinout\fP  | \fIoutput\fP
state_decls	= \fIstate\fP "{" state_term {";" state_term } "}"
state_term	=  state_type  term {"," term }
state_type	= \fIchar\fP  | \fIint\fP  | \fIfloat\fP | \fIdouble\fP
term	= \fIidentifier\fP [ "[" \fIinteger\fP "]" [ "[" \fIinteger\fP "]" ] ]
.TE
.DE
.SK
The terminal declaration part is bracketed by a left and right parenthesis.
It must contain at least one terminal declaration. A terminal declaration
begins with one of the
keywords \fIinput\fP, \fIinread\fP, \fIinout\fP or \fIoutput\fP followed
by a list of terminals. A list of terminals consists of one or
more terminals separated by commas. A terminal can
be zero-, one- or two-dimensional.
.P
Examples:
.fS I
function counter ( input phi, phi2;
                   inread in[8], load, up;
                   inout out[8], tc )
.fE
.fS I
function registers ( output out[4][4];
                     inread reg, cntrl[2][2];
                     input phi, phi2;
                     inread in[2][4] )
state {
    char  reg_p1[2][8];
    char  reg_p2[2][8];
    int   ch_p1;
}
.fE

.H 3 "Function block behavior"

.DS
.TB "Syntax of the function block body"

.TS
l l.
_
funct_body	= [load_body] [init_body]  behav_body
load_body	= \fIload\fP "{" C_code "}"
init_body	= \fIinitial\fP "{" C_code "}"
behav_body	= \fIbehavior\fP "{" C_code "}"
C_code   	= any C_code as may exist in the function body of a
           	   C function without declaration list
.TE
.DE

The body of the function block consists of three parts:
an optional load part, an optional initial part
and an obligatory behavior part.
All parts contain normal C-code; the load part
and the initial part
are executed once at the start of the simulation
(first the load part of all functions, next the initial
part of all functions), the behavior
part is executed every time the value of an input terminal changes.
The load part is normally used to specify the terminal capacitances
of the function block.
The initial part is often used to specify the delay times of the function
block and to initialize state variables.
In both parts the setting of the output terminals has no effect on the rest
of the network.
In the behavior part of the function block,
the values of the state variables and the values of the output terminals
are specified, as well as the capacitances and delay times may be changed.
.P
Examples:
.fS I
load {
    int i;
    for (i=0; i<4; i++)
        cap_add (50e-15, in[i]);
}
.fE
.fS I
initial {
    int i;
    for (i=0; i<4; i++)
        reg[i] = BTTRUE;
}
.fE
.fS I
behavior {
    if (phi1 == BTTRUE) {
	if (load == BTTRUE)
	    BSCOPY (reg, in);
	else
	    BSCOPY (reg, BSROTATE (reg));
    }
    if (phi2 == BTTRUE)
	BSCOPY (out, reg);
}
.fE
In the following sections the C-code, auxiliary routines and other relevant
subjects will be discussed.
