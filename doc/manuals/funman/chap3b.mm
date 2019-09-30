.H 2 "The C programming language"
In this manual only a brief introduction to the C programming
language is given. For a more comprehensive
explanation, see the book "The C Programming Language" by
B.W. Kernighan and D.M. Ritchie. Here, by describing some
basic ideas of the C language together with the discussion of several examples,
a first start can be made with the use of the C language.
Some familiarity is assumed with basic programming
concepts like variables, assignment statements, loops and functions.

.ne 3
\fBThe C-code\fP

In the previous section the syntax of the
function block body has been described.
It was stated that in the load, initial and behavior blocks
C-code has to be used,
completely similar to any C-code as may exist in the function body of a
C function, without declaration-list. This has also been shown in the example.
Moreover, because the function block description is to be inputed to a parser,
some extra possibilities have been created in the use of
the C language. These possibilities will be
discussed in the next section. Here we discuss one particular aid
that must be known for a better understanding of the several
examples. This aid is the use of the
values BTTRUE, BTFALSE, BTUNDEF and BTFREE which already appeared in the
example. The definition of these values is as follows:
.fS I
#define BTTRUE  'I'
#define BTFALSE 'O'
#define BTUNDEF 'X'
#define BTFREE  'F'
.fE
Since the terminals of the function block are of type character
the value of a terminal can be defined
using these 'defines'. Moreover it is much more clear for reading to use
the line:
.fS I
if (phi1 == BTTRUE)
.fE
then the line
.fS I
if (phi1 == 'I')
.fE
Therefore, in the rest of this manual these 'defines' will always be used.

.ne 3
\fBVariables and types\fP

The terminals that have been declared for a function block can be used
as variables in the C-code. These variables are of
the type character (char),
or they are a one- or two-dimensional array of type char.
A one-dimensional array of characters is a string; a two-dimensional array
of characters is an array of strings.
So, if a terminal declaration looks like:
.fS I
function shift ( input  phi;
                 inread select, in[2][5];
                 output out[5] )
.fE
then the terminals can for example be used in the following way:
.fS I
{
    int i,j;
    if (phi == BTTRUE) {
        if (select == BTFALSE)
            j = 0;
        else
            j = 1;
        for (i=4; i>0; i--)
            out[i] = in[j][i-1];
        out[0] = in[j][4];
    }
}
.fE
This piece of C-code means: if the input terminal \fIphi\fP has the value
value 'I' (a character), then the following will be executed:
if the inread terminal \fIselect\fP has the
value 'O' (a character), then the first string of the two-dimensional
character array \fIin\fP
will be used (local integer variable \fIj\fP will get the value 0),
else the second string will be used; then
the for-loop and the assignment
statement will be executed. In the for-loop variable \fIi\fP starts
with value 4 and is decremented until it is 0, which
is not >0. The string \fIout\fP (one-dimensional array of characters) is
filled with the characters of the \fIj'th\fP string of array \fIin\fP.
Notice that a one-dimensional array with 5 elements has indices:
0,1,2,3,4: generally an array with n elements has indices:
0,1,...,n-1. Furthermore, notice that
the variables \fIi\fP and \fIj\fP that are used
have to be declared, like in a normal C function body. The scope of
these variables is the behavior block they are used in. Other
variables of all kind of types can be used in the C-code of the load
as well as the initial and behavior block whenever they have
been declared locally like variables \fIi\fP and \fIj\fP.
.P
The state variables that have been declared for a function block can
also be used as variables in the C-code of a load, initial or behavior block.
These variables can be of types char, int, float or double.
For every instance of the function block the state
variables are local to that instance; for every instance apart, the
variables may change value within the load, initial or behavior block and keep
these values until they are changed again.
State variables are used in a function
block similar to the way internal static variables are used in
a C function, except that their contents is unique for
every instance of the function block.

.ne 3
\fBOperators\fP

Some of the operators that can be used in the C programming language are:
.BL
.LI
Binary operators: =, -, *, / and the modulus operator %.
The % operator can be used for integers only: x % y
produces the remainder when x is divided by y. Integer division
(operator /) truncates any fractional part.
.LI
Unary operators: -, ! . The result of the unary - operator is the negative
of its operand. The result of the logical negotiation operator  ! is 1 if the
value of its operand is 0. 0 if the value of its operand is non-zero.
.LI
Relational operators: >, >=, <, <=, ==, !=.
Notice that the following is \fBnot correct\fP:
.fS I
if (phi1 = BTTRUE)
.fE
while
.fS I
if (phi1 == BTTRUE)
.fE
is correct.
.P
Much used are the logical connectives && ( which means AND) and ||
(which means OR).
For example:
.fS I
if (c[0] == BTTRUE && c[1] == BTFALSE)
     reg = in;
.fE
and
.fS I
if ( in[0][0] == BTFALSE || in[0][1] == BTFALSE &&
    ( in[1][0] != BTTRUE || in[1][1] != BTTRUE ) )
    out = BTFALSE;
else
    out = BTTRUE;
.fE
.LI
C provides two unusual operators for incrementing and decrementing variables.
The increment operator ++ adds 1 to its operand, the decrement operator \-\|\-
subtracts 1. The unusual aspect is that ++ and \-\|\- may be used either as prefix
operators (before the variable, as in ++n), or postfix (after the variable:
n++). In both cases, the effect is to increment n. But the expression
++n increments n \fIbefore\fP using its value, while n++ increments n
\fIafter\fP its value has been used.
.LE

.ne 3
\fBControl flow\fP

The control flow statements of a language specify the order in which
computations are done.
.BL
.LI
Statements and Blocks:
.br
An \fIexpression\fP such as x=0
becomes a \fIstatement\fP when it is
followed by a semicolon, as in: x=0;.
In C, the semicolon is a statement terminator, rather than a separator
as it is in Algol-like languages.
.br
The braces { and } are used to group statements together into a
\fIcompound statement\fP or \fIblock\fP so that they are syntactically
equivalent to a single statement.
.LI
If-else:
.br
The if-else statement is used to make decisions.
Formally the syntax is:
.fS I
if ( \fIexpression\fP )
    \fIstatement-1\fP
else
    \fIstatement-2\fP
.fE
where the else part is optional.
As examples:
.fS I
if (load == BTTRUE)     and         if (phi1 == BTTRUE){
    count = in_val;                     if (reset == BTFALSE)
else                                         count++;
    count++;                            else {
                                            count = 0;
                                            tc = 0;
                                        }
                                        done = 1;
                                    }
.fE
.LI
Else-If:
.br
The following sequence of if's is the most general way of writing a multi-way
decision:
.fS I
if ( \fIexpression_1\fP )
    \fIstatement-1\fP
else if ( \fIexpression_2\fP )
    \fIstatement-2\fP
else if ( \fIexpression_3\fP )
    \fIstatement-3\fP
else
    \fIstatement-4\fP
.fE
.LI
Loops - While and For:
.br
In:
.fS I
while ( \fIexpression\fP )
    \fIstatement\fP
.fE
the \fIexpression\fP is evaluated. If it is true (which is non-zero)
\fIstatement\fP is executed and \fIexpression\fP is re-evaluated. This
cycle continues until \fIexpression\fP becomes false (which is zero), at
which point execution resumes after \fIstatement\fP.
.br
The for statement:
.fS I
for ( \fIexpression_1\fP; \fIexpression_2\fP; \fIexpression_3\fP )
    \fIstatement\fP
.fE
is equivalent to
.fS I
\fIexpression_1\fP;
while ( \fIexpression_2\fP ){
    \fIstatement
    expression_3\fP;
}
.fE
Grammatically, the three components of a for-loop are expressions. Most
commonly, \fIexpression_1\fP and \fIexpression_3\fP are assignments
or similar and \fIexpression_2\fP
is a relational expression. Any of the three parts can be omitted, although the
semicolon must retain.
Examples:
.fS I
i=0;                             and      for (i=0; i<7; i++)
c=in[i];                                      reg[i]=reg[i+1];
while (c == BTTRUE && i<8) {              reg[7]=in;
    i++;
    c=in[i];
}
.fE
.LE

.ne 3
\fBInput and Output\fP

In SLS the input of data into a network is normally done with the set command
in the command file. The output in a network consists of a sequence
of the logic states of those terminals that have been placed in a
print or a plot command in the command file.
.P
The function blocks written in the C language provide more
possibilities. Input and output routines as
exist in C can be used in the function blocks
to define a specific input and output behavior.
The two most common ways to use C input routines seem to be:
get data from the user's keyboard or from a certain file into a function block;
for output the reverse: place data on the user's terminal
screen or into a certain file. Several C routines are available for these I/O
matters.

.ne 3
\fBFormatted output - printf; formatted input - scanf\fP

The two routines printf for output and scanf for input permit translation
to and from character representation of numerical quantities. The routine
printf is used:
.fS I
printf (control, arg1, arg2, ...)
.fE
The first part, control, is a string that may contain ordinary characters and
conversion specifications, each of which causes conversion and printing of the
next successive argument to printf. Each conversion specification
is introduced by the character % and ended by a conversion character.
For example:
.fS I
printf ("phi1=%c; \etin=%8s\encount=%d\en", phi1, in, count);
.fE
might print something like
.fS I
phi1=O; in=OIIOIO
count=26
.fE
In the control string (between double quotes) %c converts the
character value of phi1 to O or I and %s converts the string value
of in to a sequence of O's and I's; %d converts the integer of count to
decimal notation.
The character \\t means a tab, \\n means a new line.
The digit 8 between % and conversion character s specifies minimum field
width. Some conversion character are:
.tr ~
.VL 16 4
.LI c
The argument is taken to be a single character
.LI s
The argument is a string
.LI d
The argument is converted to decimal notation
.LI f~or~e~or~g
the argument is taken to be a float or a double
.LE
.P
The function scanf is the input analog of printf, providing
many of the same conversion facilities in the opposite direction.
.fS I
scanf (control, arg1, arg2, ...)
.fE
Scanf reads characters from the standard input, interprets them according
to the format specifies in control and stores the results in the remaining
arguments. For example:
.fS I
scanf ("%d %f %s %c", &intgr, &rl, in, &phi);
.fE
with the input line
.fS I
3    3.2e-9    OIOI    I
.fE
will assign the value 3 to \fIintgr\fP, the value 3.2e-9 to \fIrl\fP,
the string "OIOI" to \fIin\fP and the character 'I' to \fIphi\fP.
The arguments of scanf must be pointers;
this has a result that the sign & must be placed just before the arguments
unless the variable is a pointer. A string variable is a pointer, like any
array, therefore in the example the sign & is not placed before in.
Some conversion character are:
.VL 9 4
.LI d
a decimal integer is expected in the input; the corresponding argument
must be an integer pointer (e.g. integer array or &\fIvariable\fP).
.LI c
a single character is expected; the corresponding argument
should be a character pointer.
The normal skip over white space characters is
suppressed; to read the next non-while space character, use %1s
.LI s
a character string is expected; the corresponding argument should be a
character pointer pointing to an array of characters large enough to
accept the string.
.LI f
a floating number is expected; the corresponding argument should be
a pointer to a float.
.LI lf
a floating number is expected and the corresponding argument should be
a pointer to a double.
.LE
More detailed information about printf and scanf can be found in the
book of Kernighan and Ritchie.

.ne 3
\fBFile access\fP

To read from or write into a file, several rules must be followed.
First the declaration:
.fS I
#include <stdio.h>
.fE
must be placed at the top of the file.
Second the declaration:
.fS I
FILE *fp;
.fE
must be placed in the load, initial or behavior part,
depending on in which part a file is accessed.
Then a file has to be opened with:
.fS I
fopen (name, mode)
.fE
with name being the string indicating the name of the file and mode
being the string
indicating the mode for which the file is opened.
For example:
.fS I
fp = fopen ("ram.cont", "r");
.fE
opens the file ram.cont for reading and
.fS I
fp2 = fopen ("netw.rslt", "w");
.fE
opens the file netw.rslt for writing. The two variables fp
and fp2 are file pointers; notice that
the declaration: FILE *fp2; was needed!
.br
The next thing needed is a way to read or write the file once it is open.
There are several possibilities of which getc and putc are the simplest.
Getc returns the next character from a file: it needs the file pointer
to tell it what file. Thus
.fS I
c = getc (fp);
.fE
places in c the next character from the file referred to by fp.
Putc is the reverse of getc:
.fS I
putc (c, fp);
.fE
It is also possible to use the functions fscanf and fprintf for
formatted input and output:
.fS I
fscanf (fp, control, arg1, arg2, ...)
fprintf (fp, control, arg1, arg2, ...)
.fE
The language C offers far more possibilities for I/O handling but the
discussion of those do not fall within the scope of this manual.

.ne 3
\fBCurses\fP

A short introduction of the curses package will be given here. Using
a small subset of the curses package, it is possible to move the
cursor to any point in the screen and to place data anywhere on the
screen. When curses is used, in the initial part of the function block
the initialization of curses must be done. When this is done, in the
behavior part of the function block all the curses routines can be
used. As an example a function block description using curses:
.fS I
#include <curses.h>

function regc (inread in[4], load;
               input phi, phi2;
               output out[4])

state {
    char mem[4];
}

initial {
    initscr ();
    clear ();
    refresh ();
}

behavior {
    int i;

    if (phi == BTTRUE && load == BTTRUE)
        BSCOPY (mem, in);
    if (phi2 == BTTRUE)
        BSCOPY (out, mem);
    move (3,3);
    printw ("phi=%c phi2=%c", phi, phi2);
    move (4,3);
    for (i=0; i<4; i++)
        addch (mem[i]);
    move (5,3);
    printw ("out=%s", out);
    move (23,0);
    refresh ();
}
.fE
The curses routines used in this example are all that are needed for many
possibilities. The routines are:
.VL 15
.LI initscr()
initialization: initscr() must \fBalways\fP be called before
any curses routines are used.
.LI clear()
clears the screen.
.LI refresh()
the effect of used curses routines will be placed on the user's
screen at the moment refresh() is called; before that, nothing will be seen
on the screen.
.LI move(x,y)
move cursor to line x and column y.
.LI printw()
a formatted output routine similar to printf but the output is placed on the
point where the cursor is at the moment.
.LI addch (ch)
outputs character ch on the place where the cursor is.
.LE

Since the use of curses requires the linking command to have the options:
.fS I
    -lcurses -ltermlib
.fE
included, the file sls.ld_arg (see section 2) must contain that line.
.P
Furthermore, notice that the statement #include <curses.h>
has to be included in the function block description file
in order to use the curses routines.
