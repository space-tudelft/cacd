.tr ~

.ne 3
\fBThe C preprocessor\fP

The C compiler contains a preprocessor capable of macro substitution,
conditional compilation, and inclusion of named files. Lines beginning
with # communicate with this preprocessor. A compiler-control line of
the form
.fS I
~#define  \fIidentifier  token-string\fP
.fE
causes the preprocessor to replace subsequent instances of the identifier
with the given string of tokens. For example
.fS I
~#define    NAME    counter
~#define    nmbel   8
.fE
will replace NAME with counter and nmbel with 8, everywhere
one of these two identifiers appears in the source text.
.br
The control-line
.fS I
~#include "\fIfilename\fP"
.fE
causes that line to be replaced by the entire contents of the file
\fIfilename\fP.
.br
A compiler control-line of the form
.fS I
~#if \fIconstant-expression\fP
.fE
checks whether the constant expression evaluates to non-zero.
For example:
.fS I
~#if (CURS==1)
    move (3,0);
    printw("%s", in);
    move (23,0);
    refresh ();
~#endif
.fE
Only when CURS==1, the part between #if and #endif will be included in
the compiled object code. The control lines
.fS I
~#ifdef \fIidentifier\fP
~#ifndef \fIidentifier\fP
~#else
.fE
can be used as well.


.ne 3
\fBThe C-code in the function block description\fP

The C-code in the load, initial and behavior part
is completely similar to the C-code
of function body of any C function without declaration list. However
some remarks have to be made. First of all it is not allowed to use the
.fS I
return();
.fE
statement in the C-code of the load, initial or behavior part. This is
because the C-code of the those parts is supplemented with
C-code when it is converted by the parser into genuine C functions.
When the return() statement is used, the necessary supplementary C-code
at the end of the function can not be executed.
.P
Second it must be known that a file may contain only one function
block description but, that any other correct C-code may be added to that
description in the file. This means that a file can be any file with
C-code as long as only one function block description exists within the file.
The program \fIcfun\fP will convert this file into a file with
normal C-code, which is compiled and placed into the database.
(See section 2).

.ne 3
\fBThe auxiliary routines and defines\fP

In the C-code of the load, initial or behavior part several auxiliary routines
and defines can be used. The defines that can be used are:
.fS I
#define BTTRUE  'I'
#define BTFALSE 'O'
#define BTUNDEF 'X'
#define BTFREE  'F'
#define BSCOPY strcpy
#define BSCMP  strcmp
#define BSNCMP strncmp
.fE
The first four defines have been discussed previously, of the others,
examples are shown here:
.TS
l l.
BSCOPY(mem,in)	copies the contents of (bit-)string in into (bit-)string mem.
int~BSCMP(mem1,mem2)	compares the (bit-) strings mem1 and mem2; 0 is returned
	when the strings are identical, non-zero else.
int~BSNCMP(mem1,mem2,n)	compares the first n characters of the (bit-)strings
	mem1 and mem2.
.TE
It should be understood by now that a bit-string is in fact an ordinary
string of characters in C, so for example strcpy and BSCOPY can
be (and are) exactly the same routine.
.P
It is very important to notice that bit-strings must
\fBnever\fP be used in assignment statements:
.SK
When \fIin\fP and \fImem\fP both are bit-strings, \fBnot correct\fP is
.fS I
in = mem;
.fE
while
.fS I
BSCOPY (in, mem);
.fE
can very well be used.

The auxiliary routines that can be used in the C-code are:
.BL
.LI
routines for operations on bit variables (e.g. zero dimensional terminals):
.TS
l l.
char BTINVERT (bt) 	returns inverted value of bt
char BTAND (bt1, bt2,...) 	returns logical-AND value of bt1, bt2,...
char BTNAND (bt1, bt2,...) 	returns logical-NAND value of bt1, bt2,...
char BTOR (bt1, bt2,...) 	returns logical-OR value of bt1, bt2,...
char BTNOR (bt1, bt2,...) 	returns logical-NOR value of bt1, bt2,...
char BTEXOR (bt1, bt2,...) 	returns logical-EXOR value of bt1, bt2,...
char BTEXNOR (bt1, bt2,...) 	returns logical-EXNOR value of bt1, bt2,...
.TE
Warning: when a variable argument list is used and the routine call
is not in a file that is parsed by \fIcfun\fP,
the argument list should be terminated by a '@' character.
.LI
routines for operations on bit string variables (e.g. one dimensional
terminals):
.TS
l l.
char * BWINVERT (bs)	returns bit-string which is the bitwise
	complement of bs
char * BWAND (bs1, bs2,...)	returns bit-string which is the result of a
	bitwise logical-AND of bs1, bs2,...
char * BWNAND (bs1, bs2,...)	returns bit-string which is the result of a
	bitwise logical-NAND of bs1, bs2,...
char * BWOR (bs1, bs2,...)	returns bit-string which is the result of a
	bitwise logical-OR of bs1, bs2,...
char * BWNOR (bs1, bs2,...)	returns bit-string which is the result of a
	bitwise logical-NOR of bs1, bs2,...
char * BWEXOR (bs1, bs2,...)	returns bit-string which is the result of a
	bitwise logical-EXOR of bs1, bs2,...
char * BWEXNOR (bs1, bs2,...)	returns bit-string which is the result of a
	bitwise logical-EXNOR of bs1, bs2,...
BSSET(bs)	sets all bits (characters) of bit-string bs to
	value BTTRUE (which means character: 'I')
BSRESET(bs)	sets all bits (characters) of bit-string bs to
	value BTFALSE (which means character: 'O')
BSUNDEF(bs)	sets all bits (characters) of bit-string bs to
	value BTUNDEF (which means character: 'X')
BSFREE(bs)	sets all bits (characters) of bit-string bs to
	value BTFREE (which means character: 'F')
char * BSROTATE (bs, direction)	returns bit-string that is the round rotated
	bit-string bs; direction can be 'l' (left) or
	'r' (right)
int BSTOI(bs)	returns integer with converted value of
	bit-string bs, using the radix-2 representation;
	MAXINT is returned when the bit string
	contains a bit with value BTUNDEF ('X')
int TCTOI(bs)	returns integer with converted value of
	bit-string bs, using the two's complement
	representation; MAXINT is returned when
	the bit xstring contains a bit with value
	BTUNDEF ('X')
.TE
Warning: when a variable argument list is used and the routine call
is not in a file that is parsed by \fIcfun\fP,
the argument list should be terminated by the string "ENDVAR".
.LI
other routines
.TS
l l.
char * ITOBS(intgr, nmbbt)	returns a bit-string that is the converted value
	of the integer intgr, using the radix-2
	representation; nmbbt is the number of bits of
	the returned bit-string.
char * ITOTC(intgr, nmbbt)	returns a bit-string that is the converted value
	of the integer intgr, using the two's complement
	representation; nmbbt is the number of bits of
	the returned bit-string.
single_step()	simulation is stopped until a arbitrary character
	is typed: 'q' stops the simulation immediately;
	'x' stops the simulation after the current
	simulation step; 'c' continues simulation without
	stopping; any other character continues simulation
	until a next call to single_step.(do not use this
	routine when using curses routines).
func_error()	error message is given, question is asked whether
	the simulation must continue; simulation continues
	if 'y' is typed. (do not use this routine when
	using curses routines).
single_curs_step()	same as routine single_step(); should be used
	when curses routines are used.
curs_error()	same as routine func_error(); should be used
	when curses routines are used.
.TE
.LI
In section 3.1. it was stated that in a function block the
load of input, inread and
inout terminals can be defined, and that
the delay of output and inout terminals can be specified.
The routines needed for that purpose are discussed here. First of all a routine
exists for obtaining the value of the capacitance of the node a function block
terminal is connected with:
.fS I
cap_val ( \fIenvironment\fP, \fImode\fP, \fIterminal\fP )
.fE
The arguments of the routine are:
.TS
l l l.
\fIenvironment\fP: 	NODE	the capacitance of the node is given
                  	VICIN	the capacitance of the node plus the
                 		capacitances of the nodes that are
                 		connected with the investigated node
                 		through undefined or closed transistors
                 		is given
\fImode\fP:	MIN	connected through \fBonly\fP
         		closed transistors
          	MAX	connected through closed or undefined
          		transistors
\fIterminal\fP		any terminal of the function block.
.TE
Notice that with 'node' is meant: the node that is connected with the
terminal that is used in the argument-list.
.P
The routine for defining the load of input, inread or inout terminals of a
function block is:
.fS I
cap_add ( \fIexpression\fP, \fIin_terminal\fP )
.fE
In this routine \fIexpression\fP can be any expression that results in a
real value (float), \fIin-terminal\fP is an input, inread or inout terminal
of the function block.
The effect of this routine is that the value of \fIexpression\fP is added
to the value of the capacitances of the node the relevant terminal is
connected with (both static and dynamic capacitance).
In that way the load of this terminal is accounted
for by the capacitance of the node it is connected with.
.P
When the load capacitance of a terminal is constant for the
whole simulation interval, the capacitance for that terminal
should preferably be specified in the load part of the function description.
Since the load part of a function is executed before the initial part,
all total node capacitances are known then when calls to
cap_val() occur in the initial parts of the function blocks.
.P
Finally the routine that is used for defining the delay times
of an output or inout terminal of a function block is:
.fS I
delay ( \fImode\fP, \fIexpression\fP, \fIout-terminal\fP)
.fE
The arguments of the routine can be:
.TS
l l l.
\fImode\fP	'r'	rise delay time
          	'f'	fall delay time
         	'b'	both rise- and fall delay time
\fIexpression\fP		any expression resulting in a real
                  		value (float)
\fIout-terminal\fP		any output or inout terminal of the
                  		function block
.TE
.LE
In general, string that are returned by auxiliary routines
remain until the end of the evaluation
of the function block.
After that, their memory space will be disposed.
.P
Furthermore, be sure to use a value
like 2.0 instead of 2 when a float value is required.
