. \"---------------------------------------------
.if !t \{
.tm OEPS! TROFF LIKE TEXT PREPROCESSOR REQUIRED!
.ex \}
. \"---------------------------------------------
.so ../lib/mmt.groff
.so ../lib/tmac.eqn
.so ../lib/mrefs
.so ../lib/tmac.pic
.so ../lib/tmac.psfig
. \".so /usr/lib/tmac/tmac.cw
. \".so /usr/lib/tmac/tmac.qt
.po 3c \" if \*(.T == "psc"
. \".so /usr/lib/tmac/tmac.mi
. \"---------------------------------------------
.de DF
.nr Ds 0
.DS 0 1
.DE
.nr Ds 1
.nr :t 1
.)J "\\$1" "\\$2" "\\$3"
..
. \"---------------------------------------------
.de ix
.if \\n(.$>1 .tm TOO MANY ARGS \\$1 \\$2 ...
.tm \\$1	\\nP
..
. \"---------------------------------------------
.de UL \" underline argument, don't italicize
.if t \\$1\l'|0\(ul'\\$2
.if n .I \\$1 \\$2
..
. \"---------------------------------------------
