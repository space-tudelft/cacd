'	\" Page setup definitions.
'	\"
'	\" register s  -> document style (1 or 2)
'	\" register x  -> use as temporary register.
'	\" register a  -> appendix counter
'	\" register c  -> appendix page counter
'	\" register p  -> body point size
'	\" register q  -> footer point size
'	\" string   q= -> footer font
'	\" register r  -> lineprinter point size
'	\" string   r= -> lineprinter font
'	\" register v  -> vertical spacing
'	\" string   t= -> title string
'	\"
'	\" default style is 1, this can be changed by -rs#.
.if \ns=0 .nr s 1
.ds .T psc
.ie !\*(.Tpsc\{\
.nr p 11
.nr v 14
.nr r 9
.ds r= FX
.nr q 9
.ds q= (SR\}
.el \{\
.nr p 12
.nr v 15
.nr r 10
.ds r= C
.nr q 10
.ds q= H\}
.de TP
.ps \np
.vs \nv
.sp 2
.ps
.if \\nP&\w@\\*(}t@ \{\
.tl %\\*(t=%%\s\np\\nP\s0%
.tl %\D'l \\nWu 0u'%%%\}
'sp 2
.if \\nP=1 \{\
.PF "%%\s\nq\f\*(q=The Nelsis IC Design System\fP\s0%"\}
.vs
..
' \" right margin justification
.SA 1
.nr P -1
.nr Si 4        \" standard indent for displays
.nr Hb 4        \" break after each heading at level <= 4
' \".nr Cl 3	\" table of contents level
.BS
.SP 1
.BE
.de HX
.if \\$1=1 .ne 6
.if \ns=2 .if \\$1=2 .ne 4
.if \ns=2 .if \\$1=2 .SP 2
..
.de HZ
.if \ns=2 .if \\$1=1 .nr Fg 1      \"reset figure number at new chapter
.if \ns=2 .if \\$1=1 .nr Tb 1      \"reset table number at new chapter
..
.de sF		\" Change to default size and font.
.S \\np \\np+3
.R
..
.sF
'		\" End of page setup.
'		\" Below are macros to be used in the documents.
'		\" .sF can also be used.
.de T=		\" Define title string.
.ds t= \s\np\fI\\$1\fP\s0
..
.de L{		\" Change to lineprinter font.
.S \\nr
.ft \*(r=
..
.de L} 		\" Change back from lineprinter font.
.sF
..
.de fS		\" Start lineprinter font static display
.DS \\$1
.L{
..
.de fF		\" Start lineprinter font floating display
.DF \\$1
.L{
..
.de fE		\" End lineprinter font display
.L}
.DE
..
.de sY		\" Symbolic reference to something
.if !'\\$1'' .if !'\\$2'' .tm 1,$s#\\$1#\\$2#g
..
.if \ns=1 \{\
.de fG		\" Figure title with symbolic name
.sY \\$2 \\n(Fg
.FG "\\$1"
..
.de tB		\" Table title with symbolic name.
.sY \\$2 \\n(Tb
.TB "\\$1"
.. \}
.if \ns=2 \{\
.de fG		\" Figure title with symbolic name
.sY \\$2 \\n(H1.\\n(Fg
.FG "\\$1" \\n(H1.\\n(Fg 2        \"override figure number
..
.de tB		\" Table title with symbolic name.
.sY \\$2 \\n(H1.\\n(Tb
.TB "\\$1" \\n(H1.\\n(Tb 2        \"override table number
.. \}
