.T= "Text Processing Application Note"
.DS 2
.rs
.sp 1i
.B
.S 15 20
SPACE APPLICATION NOTE
ABOUT
TEXT PROCESSING
.S
.sp 1
.I
S. de Graaf
.sp 1
.R
Circuits and Systems Group
Faculty of Electrical Engineering,
Mathematics and Computer Science
Delft University of Technology
The Netherlands
.DE
.sp 2c
.ce
Report EWI-ENS 03-10
.ce
November 28, 2003
.sp |9i
.DS
.in +5
.S 9
Copyright \s+3\(co\s-3 2003 by the author.

Last revision: December 9, 2003.
.S
.in -5
.DE
.SK
.S=
.H 1 "INTRODUCTION"
This application note describes how you can make
.P= space
text documents and which special processing commands you can use.
It tells also something about other important things, i want to remember,
like the
.P= space
web pages.
.H 1 "DOCUMENTS"
.H 2 "How to Make"
The
.P= zmake
program is used to format the document.
You need a "ZMAKE" configuration file,
which describes the source files and dependencies and the roff command and options.
The output result (in PostScript A4 format) is placed in a temporary file directory
and is converted to "Letter" format by
.P= psresize
and placed in the install directory.
To view the A4 formatted file, go to your central CACD directory and type
the following GhostView command:
.fS
    cd /u/52/52/work/simon/CACD
    gv -a4 .@tmp/linux-2.4-i686-europa/develop/tmp-zmake-xxx.ps
.fE
To take over the temporary file path,
double click with the left mouse button and release the selected text with the middle button.
You must self type the first two characters of the path "\fC\s-2.@\s0\fP",
because they are not selected (the at-sign stops the selection).
.H 2 "Processing Options"
The option "-rs2" gives extra vertical white space between sections.
It can give problems with figure numbering.
.P
The option "-rN2" is needed to suppress the page number and heading on the front page.
.H 2 "Groff Text Processor"
The
.P= groff
program is a GNU successor of the UNIX
.P= troff
program, we are using in the Linux environment to roff (run-off) the document.
.H 2 "C Like Preprocessor Statements"
For example the following conditional statements can be used:
.fS
    #ifdef REL4
	...
    #else
	...
    #endif

    #ifndef ESE
	...
    #endif
.fE
To remove conditional parts correctly, the program
.P= unifdef
with option
.O= -t
must be used.
.H 2 "The Front Page"
The official name of the faculty is changed from "ITS" into "EWI".
For other countries "EEMCS", the complete name is:
.fS
    Faculty of Electrical Engineering,
    Mathematics and Computer Science.
.fE
The name of our group is "CAS" (in Dutch "ENS"):
.fS
    Circuit and Systems Group
.fE
.H 2 "Memorandum Macros"
For a complete and detailed description of all macro's,
see the "MM - Memorandum Macros" UNIX document of Bell Labs.
Now follows an overview of the most used MM macro commands.
.br
For section headers are used the following two macro's:
.fS
    .H  .HU
.fE
For alternated text fonts the macro's:
.fS
    .I  .IR  .IB  .B  .BR  .BI  .R  .RI  .RB
.fE
All commands (except .R) can have a maximum of 6 arguments.
.br
To start a new paragraph the macro command:
.fS
    .P
.fE
The default paragraph style is left justified (type 0) and uses a half vertical spacing (amount is 1).
To use an indented paragraph style, set number register "Pt" to 1.
The default indent is 5, to use another indent amount, change number register "Pi".
To use another vertical spacing amount, change number register "Ps".
.br
For a page skip, use the macro command:
.fS
    .SK
.fE
For font size and vertical spacing the macro command:
.fS
    .S size [spacing]
.fE
For a table of content the macro command:
.fS
    .TC
.fE
For displays the macro commands:
.fS
    .DF  .DS  .DE
.fE
For lists the macro commands:
.fS
    .VL  .BL  .DL  .LI  .LE
.fE
.H 2 "Commonly Used Nroff Commands"
We speak about the
.P= nroff
program, but we use the
.P= troff
version (see
.P= groff )
of the program.
Note, each line that starts with a "dot" is not outputted but interpretted
by the used text processor.
.br
Here follows a list of the most commonly used nroff commands.
.br
For a line break, use the command:
.fS
    .br
.fE
For setting the no filling mode for lines the command:
.fS
    .nf
.fE
For setting the filling mode for lines the command:
.fS
    .fi
.fE
For setting a number register "Pt" to 1, use the command:
.fS
    .nr Pt 1
.fE
To increment the number register:
.fS
    .nr Pt +1
.fE
For using this number register, use the sequence '\\n(Pt'.
It can be used in the following "if" statement:
.fS
    .if \\n(Pt > 0 .nr Pt 0
.fE
Another "if" statement example, when using nroff in place of troff:
.fS
    .if n \\{\\
    anything
    anything \\}
.fE
You can define your own macro (for example "T>") with the following commands:
.fS
    .de T>
    anything
    anything
    ..
.fE
For setting string register "sP", for example the command:
.fS
    .ds sP "S\\s-2PACE\\s+2
.fE
Use for inline text point size changes the backslash 's' commands
and for using a defined string the '\\*(xx' command, for example:
.fS
    Hallo, i am using the S\\s-2PACE\\s+2 system.
    Hallo, i am using the \\*(sP system.
.fE
For translating an at-sign into a quote character the command:
.fS
    .tr @'
.fE
For inline font changes of text the commands:
.fS
    \\fI italic-text \\fP previous-font-text
    \\fB bold-text \\fC courier-text \\fR roman-text
.fE
.H 2 "Space Document Macros"
For the definitions of these additional macro's, see the "head.mm" file.
For the effect of these macro's, see the "Space User's Manual".
.br
The following macro is used to change back to the default document font, size and spacing:
.fS
    .S=
.fE
This macro is another name for the macro "sF", which can be found in the "lib/style/setup.mm" file.
The definition of macro "sF" is something like:
.fS
    .de sF
    .S \\\\np \\\\np+3
    .R
    ..
.fE
Note that the number register "p" is default set to font point size 12.
.br
The following macro is used to style (italic) program names:
.fS
    .P= space
.fE
Note that this macro can have a second argument,
which is concatenated in roman font to the first argument.
.br
The following macro is used to style (bold/italic) options:
.fS
    .O= -v -verbose
    .O= -S "" "param=value"
.fE
The 1st argument specifies the option key in short form (is printed in bold),
and the 2nd argument can specify the option in an alternative form (as used
by the historic ESE version of space programs).
The 3th argument can specify the option argument (printed in italic).
Note that this macro can have a 4th argument,
which is concatenated in roman font to the 3th argument.
.br
The following macro's define an "\fBExample:\fP" section:
.fS
    .E(
    .E)
.fE
The following macro's define a "\fBNOTE:\fP" section:
.fS
    .N(
    .N)
.fE
The following macro's define a bullit list:
.fS
    .I(
    .I)
.fE
An list item starts for example with the following macro:
.fS
    .I= "list-item-text"
.fE
When the list item text is specified, it is put in boldface
behind the bullit.
.P
The following macro's define an option list:
.fS
    .M(
    .M)
.fE
An option list item starts for example with the following macro:
.fS
    .ME -v -verbose
.fE
A 3th argument may be given to specify an option argument.
.P
The following macro is used for an appendix start:
.fS
    .aH "appendix-head-text"
.fE
It does initial a skip page instruction.
.P
The "head.mm" file contains also a "delim" instruction for using in-line
.P= eqn
commands.
.EQ
delim off
.EN
.fS
    .EQ
    delim $$
    .EN
.fE
This is default set, but maybe not a good choice for your document!
.H 2 "Space Document Setup Macros"
The setup macro's are defined in the "lib/style/setup.mm" file.
.br
The following macro's define a line printer font (Courier) block:
.fS
    .fS [I]
    .fE
.fE
This block is put in a static display and can be indented.
The point size of the font is default 10 and is controlled by register "r".
There is also a floating display version, which is called "fF".
.P
Use the following macro to define a section reference label:
.fS
    .sY %LABEL_NAME_1% \\n(H1
    .sY %LABEL_NAME_2% \\n(H1.\\n(H2
.fE
Number register "H1" contains the number of the first header level,
"H2" the number of the second header level.
To use the label in the text, do it as follows:
.fS
    Text text ... (see section %LABEL_NAME_2%).
.fE
.H 2 "Using Refer"
.H 2 "Using Figures"
Figures can be made with the program
.P= xfig .
Type the following command:
.fS
    % xfig [fig_name.fig]
.fE
A saved figure can be converted to encapsulated PostScript
with the following command:
.fS
    % fig2dev -L eps fig_name.fig > fig_name.ps
.fE
The figure can be put in the document with the following text processing commands:
.fS
    .F+
    .PSPIC "fig_name.ps" 15c 2.5i
    .F-
.fE
You see that a long path name to the figure is needed.
You can also insert figures of other documents in your document.
The "PSPIC" command is supported by the program
.P= groff .
Behind the path you specify the required size of the picture.
The first value specifies the maximum width (15 cm) and the second (optional)
value specifies the maximum height (in this case 2.5 inch).
.br
For extra space before or after the figure you can use the MM paragraph command ".P".
.H 1 "MAN PAGES"
.H 2 "Man Macros"
.H 1 "WEB PAGES"
.H 2 "Cascading Style Sheets"
In a style sheet you can define the text style (font name, size, slanting, color, etc.) of a part of a web page.
The command ".css" is used for the definition.
A style sheet has a class name.
The hypertext "span" command is used to use a style sheet.
Here follows two examples:
.fS
<span class=faq> text </span>

<span style='color: #ff00ab'> text </span>
.fE
The last example gives some inline text an alternative color.
