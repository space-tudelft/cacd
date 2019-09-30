.nr Hy 0
.SA 1
.nr Ej 1
.nr Fg 0
.nr Tb 0
.H 1 "Results"
The programs described in the previous segments
have been written in the program language C and
are running under the UNIX operating system on
a HP9000 series 500 machine.
All programs in the design rule check
system are based upon the stateruler concept.
As shown in 
.[
newell fitzpatrick
.]
this algorithm is linear with
respect to the number of edges in the design
of an integrated circuit.
So one might expect the checker system also to be
linear in this aspect.
To investigate this the checker has been applied to a
cell containing a random counter with about 1500 edges.
Thereafter the checker has been applied to arrays of this
cell of increasing size.
The time needed for the checking is recorded.
This gives the following results:
.DS
.TS
center box tab(~);
c | c | c | c.
array~number~convert~check
size~of edges~(hh:mm:ss)~(hh:mm:ss)
_
 1 x 1 ~  1476 ~    38 ~    1:50
 2 x 2 ~  5904 ~  2:01 ~    6:57
 3 x 3 ~ 13284 ~  4:39 ~   16:33
 4 x 4 ~ 23616 ~  9:59 ~   33:34
 5 x 5 ~ 36900 ~ 15:17 ~   57:15
 6 x 6 ~ 53136 ~ 24:39 ~ 1:14:10
 7 x 7 ~ 72324 ~ 28:29 ~ 1:49:56
 8 x 8 ~ 94464 ~ 43:40 ~ 2:27:32
.TE
.TB "Checker cpu times" \n(H1.
.DE
In this table under convert the cpu time to
make the line segment files is recorded and under checker the
cpu time needed to perform the boolean operations
and do the actual checks.
From this one may conclude that the check time indeed is
about linear with the number of edges.
The results have been obtained making no use of the hierarchy
of the cell.
The time saved by making use of the hierarchy of course
is very much dependent of the number of repetitions
of subcells in the cell.
The cell rand_cnt mentioned above,
is hierarchically build up in the following way.
.DS
.PS < ../drc/fig3.pic
.PE
.FG "Cell Hierarchy" \n(H1.
.DE
In the next table the results of the checking of this cell
in an hierarchical way and linear are compared.
.DS
.TS
center box tab(~);
c | c s | c s
^ | c | c | c | c
^ | c | c | c | c
c | c | c | c | c.
cell~linear~hierarchical
_
~convert~checker~convert~checker
~(seconds)~(seconds)~(seconds)~(seconds)
_
mod2_fb~9~26~9~26
latch~10~24~10~24
select~8~20~8~20
feedback~13~28~13~23
sel_reg8~32~95~29~56
rand_cnt~38~110~20~34
.TE
.TB "Comparison between hierarchical and linear expanded cells" \n(H1.
.DE
We see that the time needed to check the cells in a
hierarchical way is much less 
for top level cells in this case,
even though some overlaps are present.
Even more dramatic changes in time may be expected if
the repetitions of cells is greater.
