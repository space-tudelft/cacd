.H 2 "Examples of function block descriptions"
In this section a few examples will be shown of descriptions
of function blocks. These function blocks will be used in
examples of networks as will be shown and explained in section 4.
.P
The first example is a functional description of a multiplexer.
The block diagram of the multiplexer is shown in figure 4.
.DS
.S -3

.PS < ../funman/mfig4.pic

.S
.FG "Block diagram of multiplexer"
.DE

.fS I
.PS < ../funman/mux.fun
.fE
.FG "Functional description of multiplexer"

The functional description of this block is very simple,
as can be seen in figure 5.
The multiplexer has no state variables and no load or initial body.
Both \fIin\fP and \fIselect\fP terminals are input terminals
since a change of value of one of those terminals could cause a change
in the \fIout\fP terminal. The one line in the behavior body
defines the logic behavior of the multiplexer:
the bit string \fIselect\fP (2 bits) is converted to
an integer and this integer determines which of
the \fIin\fP terminals will give its data to the \fIout\fP terminal.
This is only executed if the integer is smaller then 4.
This test is necessary because when one of the \fIselect\fP bits
will have the value BTUNDEF (which is 'X'), the integer that
will be returned by the routine BSTOI will be MAXINT (which is
the maximum integer on the computer one is working on).
When the MAXINT's bit in the string \fIin\fP is asked,
this will probably cause a 'segmentation violation'.
.P
In the second example, a description of a shiftregister will
be shown. The design of the shiftregister function block
is shown in figure 6.
.DS
.S -3

.PS < ../funman/mfig6.pic

.S
.FG "Block diagram of shiftregister"
.DE


The block has two control variables
of which the coding is:
.DS
.TS
l l l.
_
Control	c0	c1
_
hold	O	O
left_shift	O	I
right_shift     	I     	O
load	I	I
_
.TE
.DE

The functional description of the shiftregister can be found
in figure 7.

.fS I
.S -1
.PS < ../funman/shiftr.fun
.S
.fE
.FG "Functional description of shiftregister"
.P
Several remarks can be made about this description:
.DL
.LI
Because of the use of macro definitions the number of
shift-elements in the shiftregister can be changed easily:
only the integer 4 in the definition needs to be changed.
The same can be said about the name of the function block.
.LI
The routine BSCOPY() is used for assigning values to
the bit strings mem and out.
.LI
It is easy to understand what the function of the block is;
just by reading the simple code the actions are clear.
.LI
The terminal bit-string in_out[4] is bidirectional.
The value of the bit-string can be used as input data
but values can be assigned to the bit-string as well.
Notice that the bits are set to free (character 'F')
when no data is to be outputted.
.LI
No timing properties are given to the function block:
the input, inread and inout terminals cause no load,
and the delay of the inout terminals is zero.
.LE
.P
In the third example, a description of a counter will
be shown. In this description much attention is given
to the timing properties of the function block.
The designed counter is a normal counter which can load
new data and count up or down. The two-phase non-overlapping
clocks phi1 and phi2 are used: during phi1 new data is loaded
or the counter counts, during phi2 the data is sent to the output terminals.
The termcount terminal gets high during phi2
when all the bits in the counter are high.
The block diagram of the counter is shown in figure 8.

.DS
.S -3

.PS < ../funman/mfig8.pic

.S
.FG "Block diagram of counter"
.DE

The functional description of the counter can be found
in figure 9.

.fS I
.S -1
.PS < ../funman/count.fun
.S
.fE
.FG "Functional description of a counter"
.P
Several remarks can be made about this description as well:
.DL
.LI
In the initial part the counter is set to zero.
.LI
The behavior of the counter is easy to understand when reading
the code in the behavior part. The only new aspect is
.fS I
1<<NMBEL
.fE
which means $ 2 sup NMBEL $.
.LI
For output, curses routines have been used. Because of the auxiliary
routine  single_curs_step()  the simulation of the function block
can be done step-wise. As can be concluded from the used curses routines,
on line number 4, on the 8th column, the bits of the counter followed
by the decimal representation of the bits will
be shown on the terminal screen during simulation.
.LI
Notice that the state variables are integers, the counting is
done in integers.
The decimal value of the bit-string that is loaded
into the counter is found using the conversion routine BSTOI();
the bit-values of the counter are found
using the conversion routine ITOBS().
.LI
The timing properties of the counter function block are defined with
the routines:
.fS I
    delay ('b', 1000 * cap_val(VICIN,MAX,out[0]), out[0]);
    delay ('b', 1000 * cap_val(VICIN,MIN,out[1]), out[1]);
    delay ('b', 1000 * cap_val(VICIN,MAX,out[2]), out[2]);
    delay ('b', 1000 * cap_val(NODE, MAX,out[3]), out[3]);
.fE
With delay() the delay times of the output
terminals out[0] to out[3] are defined.
For the several rise and fall delay times, real values are
calculated, using the dynamic capacitance of the nodes in the
network that are connected with the output terminals of the function
block. The capacitances of the nodes themselves as well as the
capacitance of the node together
with its vicinity are used for this purpose.
The value 1000 could be seen as output resistance;
in that way RC-times are found for delay times.
Notice that the delay times are defined in the behavior block.
This is due to the fact that the delay of the output terminals
depend on the structure of the network the
function block is part of. At every new evaluation of the function
block the vicinities of the nodes may differ. The delay times of
the three output terminals depend of the capacitance of the nodes
with their vicinities and therefore these times have to be
calculated again at every function block evaluation.
.br
The input load of the function block is defined with the routine:
.fS I
    cap_add (100e-15, in[0]);
    cap_add (150e-15, in[1]);
    cap_add (200e-15, in[2]);
    cap_add (250e-15, in[3]);
.fE
With cap_add() the load of the inread terminals in[0] to in[3] is defined:
the capacitance of the node in the network
that is connected with the inread terminal in[0]
is increased with 100 femto-farad,
the capacitance of the node in the network
that is connected with the inread terminal in[1]
is increased with 150 femto-farad, et cetera.
.LE
.P
In the last example, a description of a function block
with combinational logic will
be shown. In the file that contains this functional
description, other C-code has been placed as well.
The contents of the file containing the
functional description of this function block can be found
in figure 10.

.fS I
.S -1
.PS < ../funman/combi.fun
.S
.fE
.FG "Contents of file containing a functional description of block: combinator"
.P
About the data in this file
several remarks can be made:
.DL
.LI
In the file two normal C-routines can be found and the
functional description of the function block. The two
routines are called in the behavior block of the functional
description. The routine letsee() outputs the values of
three of the four characters in its argument
list (bits in this case!) together with the
result of an EXOR operation to stderr. Stderr
normally is the terminal screen.
After that,
again the values of all
the four characters are printed together with a string.
The routine four_of_four() returns the character 'I' (which
is BTTRUE) when the four characters in its argument list do
have the values 'O', 'I', 'O' and 'O' and the
character 'O' (which is BTFALSE) else.
.LI
In the
behavior block, a question is send to stderr
and four characters must be typed on the keyboard.
These four characters are placed in the local
variable (character string) named \fIin\fP. Notice
that this array of characters must have 5 elements:
four elements for characters and one element
for the terminating '\\0'. The typed characters must
have values 'O', 'X' or 'I' or else an error will occur.
.LE
.SK
