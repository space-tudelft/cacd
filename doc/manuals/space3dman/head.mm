.EQ
delim off
.EN
'	\" .if \ns=2 .nr Ej 1
.nr Ej 1
.de M(
.VL 16n 2n
..
.de M)
.LE
.P
..
.de S=
.sF
..
.de ME
.lg 0
.ds i= "\f3\\$1
.if !''\\$3' .as i= "\ \f2\\$3
.LI "\\*(i="
.S=
.nr z 14n
.if \w'\\*(i='>\\nz .br
.lg
..
.ds sP "S\s-2PACE\s+2
.ds sQ "\s-2SPACE\s+2
.de I=
.LI
.if \\n(.$=1 \{\
\f3\\$1\f1
.br \}
..
. \".ds b= \s8\(bx\s0
.de I(
. \".LB 5n 2n 0n 0 \*(b=
.LB 5n 2n 0n 0 \(bu
.S=
..
.de I)
.LE
.P
..
.\" option
.de O=
.if !'\\$3'' \f3\\$1 \f2\\$3\f1\\$4
.if '\\$3'' \f3\\$1\f1\\$4
..
.\" progname
.de P=
\f2\\$1\f1\\$2
..
.de xx
.P
\&... \\$1 \\$2 \\$3 \\$4 \\$5 \\$6 \\$7 \\$8 \\$9
.P
..
.\" start and end of example
.de E(
.P
.ne 5
.in +5n
.B \s-1Example:
.S=
.br
..
.de E)
.S=
.in -5n
.P
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
.\" Appendix headings lettered alphabetically !
.nr Hu 1
.nr aH 0 1
.af aH A
.de aH
.if !\\n(aH \{\
.nr H1 0
.\" .nr Hc 1
.HM A 1 1 1 1 1 1 \}
.PH "''- \\\\\\\\nP -'Appendix \\n+(aH'"
.SK
.HU "Appendix \\n(aH: \\$1"
.SP 2
..
.ds sN ICD
.nr e 1
.de D{
.nr f \\ne
.DS I
..
.de D}
.nr f 0
.DE
..
.de E{
.if \\n(.$ .sY \\$1 \\n(H1.\\ne
.nr f \\ne
.DS I
..
.de E}
.nr e \\ne+1
.nr f 0
.DE
..
.rn EQ eQ
.de EQ
.ie \\nf\{\
.       ie '\\$1'L' \{\
.               ie '\\$2'-' .eQ L
.               el .eQ L (\\n(H1.\\ne\\$2)
.       \}
.       el\{\
.               ie '\\$1'-' .eQ I
.               el .eQ I (\\n(H1.\\ne\\$1)
.       \}
.\}
.el .eQ I \\$1
..
.de gS          \" Start lineprinter font static (dividable) display 
.sp 0.5
.in +0.8c
.nf
.L{
..
.de gE          \" End lineprinter font display
.L}
.fi
.sp 0.5
.in -0.8c
..
.EQ
delim $$
.EN
