.aH "3D Capacitance Model"
.sY %AX_3DCAPMODEL% \n(H1
.H 2 "Introduction"
.P= Space3d
uses a boundary-element method to compute 3D capacitances.
A brief description of this method
is given in Section %SE_BOUNDARY%.
For the solution of the boundary-element equations,
a large matrix needs to be inverted.
The approximate matrix inversion technique that is used
for this is described in Section %SE_APPINV%.
.H 2 "The Boundary-Element Method"
.sY %SE_BOUNDARY% \n(H1.\n(H2
Consider a domain $V$ that contains $M$ conductors.
Our purpose is to find the short-circuit capacitance matrix $C sub s$
that gives the relation between the conductor charges
$Q sup T ~=~ [ Q sub 1 , ~ Q sub 2 , ~ ... ~ Q sub M ] $
and the conductor potentials
$PHI sup T ~=~ [  PHI sub 1 , ~ PHI sub 2 , ~ ... ~ PHI sub M ] $
as
.E{
.EQ
Q ~=~ C sub s ~ PHI .
.EN
.E}
.P
The potential $phi ( p )$ at a point $p$ in $V$ can be expressed as
.[
Weber
.]
.[
Ning
Dewilde
Kluwer
.]
.E{  %EQ_GREENINTVOL%
.EQ
phi ( p ) ~=~ int from V ~ G ( p, ~ q ) ~ rho ( q ) ~ d q ,
.EN
.E}
where $rho ( q )$ is the charge distribution in $V$ and
$G ( p , ~ q )$ is the Green's function for $V$.
In order to solve %EQ_GREENINTVOL%,
the boundary-element method subdivides the surfaces of the conductors
in elements $S sub 1 , ~ S sub 2 , ~ ... ~ S sub N$
(the elements may partly be overlapping),
and approximates the charge distribution $rho ( q )$ by
.E{  %EQ_SHAPECOL%
.EQ
rho ( q ) ~ approx ~ rho tilde ( q ) ~=~ sum from { i=1 } to { N } ~
alpha sub i ~ f sub i ( q ) ,
.EN
.E}
where $alpha sub 1 , ~ alpha sub 2 , ~ ... ~ alpha sub N$
are unknown variables to be determined
and $f sub 1, ~ f sub 2 , ~ ... ~ f sub N$ are N independent shape
functions (also called basis functions).
The $f sub i$'s have the property that
.E{
.EQ
int from { S sub j } ~ f sub i ( q ) ~ d  q  ~=~ ~~ left "{"  ~~
matrix {
lcol { { 1 ~~~~ if ~ i ~=~ j }
above
{ 0 ~~~~ if ~ i ~ != ~ j }
} }
.EN
.E}
Some examples of shape functions are given
in Figure %FG_SHAPEF%.
.DF
.PS 5.0i
copy "../space3dman/shapes.pic"
.PE
.fG "Different types of shape or basis functions that can be used to model the surface charge density on the conductors (a) Dirac, (b) constant: $f$ is described by the top of the wedge, (c) linear: $f$ is described by the 4 slanting planes of the pyramid." %FG_SHAPEF%

.DE
.P
An approximation for the potential distribution is then obtained by
insertion of %EQ_SHAPECOL% into %EQ_GREENINTVOL%:
.E{ %EQ_GREENEQDIS%
.EQ
phi tilde ( p ) ~=~ sum from i=1 to N ~ alpha sub i
int from { S sub i } ~
G ( p, ~ q ) ~ f sub j ( q ) ~ d q .
.EN
.E}
Next, $N$ independent linear equations are obtained by introducing
a set of $N$ independent weight functions
$w sub 1 , ~ w sub 2 , ~ ... ~ w sub N$ that are
defined on the sub-areas $S sub 1 , ~ S sub 2 , ~...~ S sub N$ and
that are used to "average out" the error in $phi tilde ( p )$:
.E{
.EQ
int from { S sub i }
 w sub i  ( p ) ~ left "[" ~ phi tilde ( p ) ~-~ phi ( p )
 right "]"  d p
  ~~=~~ 0  ~~~~~~~~~~
 ( i ~=~ 1 ~...~ N ) .
.EN
.E}
By insertion of %EQ_GREENEQDIS%,
the above set of equation may be rewritten as
.E{  %EQ_TOTALEQS%
.EQ  -
 sum from j=1 to N ~ alpha sub j ~
int from { S sub i } ~
int from { S sub j } ~
G ( p, ~ q ) ~ f sub j ( q ) ~ w sub i  ( p mark ) ~
d q ~ d  p
~~ = ~~
int from { S sub i }
 w sub i  ( p ) ~ phi ( p ) ~
 ~ d  p
.EN
.EQ
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 ( i ~=~ 1 ~...~ N ) .
.EN
.E}
Now, let $F$ be an $N times M$ incidence matrix
in which
.E{
.EQ
F sub ij ~=~ left "{" ~ matrix {
ccol { 1 above 0 }
lcol { { ~~~ "if" ~ S sub i ~ is ~ on ~ conductor ~ j } above { ~~~ otherwise .} }
}
.EN
.E}
Then, Equation %EQ_TOTALEQS% may be written as
a set of $N times N$ equations,
.E{ %EQ_RELGAFP%
.EQ
G ~ alpha ~=~ W ~ F ~ PHI ,
.EN
.E}
where
$G$ is an $N times N$ matrix that has entries
.E{  %EQ_GMEM%
.EQ
G sub ij ~=~
int from { S sub i } ~ int from { S sub j } ~ G ( p, ~ q ) ~ f sub j ( q ) ~ w sub i  ( p ) ~ d q  ~ d p ,
.EN
.E}
$alpha sup T$ = $[ alpha sub 1 , ~ alpha sub 2 , ~...,~ alpha sub N ]$,
and $W$ is an $N times N$ matrix that has entries
.E{  %EQ_WMEM%
.EQ a
W sub ij ~=~ 0, ~~~~~i != j,
.EN
.sp 0.5
.EQ b
W sub ii ~=~
int from { S sub i } ~ w sub i  ( p ) ~ d p .
.EN
.E}
.P
The conductor charges
are found from %EQ_RELGAFP% as
.E{  %EQ_CSPRE%
.EQ
Q ~=~ F sup T ~ alpha ~=~ F sup T ~ G sup -1 ~ W ~ F ~ PHI .
.EN
.E}
Thus, the short-circuit capacitance matrix $C sub s$
is obtained from %EQ_CSPRE% as
.E{   %EQ_CS%
.EQ
C sub s ~=~ F sup T ~ G sup -1 ~ W ~ F .
.EN
.E}
.P
In the Galerkin boundary-element method
.[
Ning Dewilde
Capacitance coefficients
1987
Electron Devices
.]
the weight functions $w sub i$
are chosen equal to the shape functions.
This way, the evaluation of $G$ requires the computation
of a double surface integral, but
$G$ becomes symmetrical, which
is advantageous for computing the inverse
of the elastance matrix.
.P
In the collocation boundary-element method,
.[
Ning SPIDER
IEEE Computer-Aided Design
1988
.]
the weight functions $w sub i$
are chosen equal to Dirac functions.
In this case the computation of $G$ requires
the evaluation of only single surface integrals.
$G$ is artificially made symmetrical
by using the average of the two entries that are at a symmetrical position.
.H 2 "Approximate Matrix Inversion"
.sY %SE_APPINV% \n(H1.\n(H2
Normally, the inversion of the elastance matrix $G$ in %EQ_CS%
requires $O ( N sup 3 )$ time and $O ( N sup 2 )$ space.
To allow fast extraction times, also for large circuits,
.P= space
is capable of computing an approximate inverse for $G$.
Therefore, it utilizes a matrix inversion technique
that takes as input a matrix that is specified on a stair-case
band around the main diagonal and produces
as output a matrix in which only non-zero entries occur for
the positions that correspond to positions in the stair-case band.
The basic idea is illustrated in Figure %FG_4NETW%.
In Figure %FG_4NETW%, different approximations are computed for a simple
boundary-element mesh that consists of 4 elements and that is
described by the following elastance matrix:
.E{
.EQ
left "[" matrix {
ccol { 1.0 above 0.4 above 0.2 above 0.1 }
ccol { 0.4 above 1.0 above 0.4 above 0.2 }
ccol { 0.2 above 0.4 above 1.0 above 0.4 }
ccol { 0.1 above 0.2 above 0.4 above 1.0 } } right "]"
.EN
.E}
.DF
.S -2
.PS 3.95i
copy "../space3dman/4ntw4.pic"
.PE
.S=
.ce 1
(a)
.S -2
.PS 3.95i
copy "../space3dman/4ntw3.pic"
.PE
.S=
.ce 1
(b)
.S -2
.PS 3.95i
copy "../space3dman/4ntw2.pic"
.PE
.S=
.ce 1
(c)
.S -2
.PS 3.95i
copy "../space3dman/4ntw1.pic"
.PE
.S=
.ce 1
(d)
.sp 0.3
.fG "(a) Exact solution, (b) only diagonals 1-3 are computed, (c) only diagonals 1-2 are computed, (d) only the main diagonal is computed." %FG_4NETW%
.DE
.P
For practical layouts the method proceeds as follows.
First, the layout is subdivided into strips
of width $w$
(see Figure %FG_STRIPS%).
All influences between elements that are within
a distance $w$ will be taken into account,
and all influences between elements that are more than
a distance $2w$ apart will not be taken into account.
Next, a banded approximation according to Figure %FG_4NETW%
is computed - whereby only influences are taken into account
between elements that are in the y direction within a distance $w$ -
for (1) each pair of adjacent strips and (2)
each single strip except for the first and last strip.
The results that are obtained for the pairs of strips
are added to the total result and the results that are obtained
for the single strips are subtracted from the total result.
.[
Design Automation
Genderen
1989
.]
.[
Genderen
Ph.D. Thesis
.]
.[
Meijs
Ph.D. Thesis
.]
.P
By executing all steps of the extraction method
as a scanline is swept over the layout from left to right,
the extraction method can be implemented to have
a computation complexity that is
$O ( N w sup 4 )$ and
a memory usage that is $O ( w sup 4 )$.
So, when $w$ is kept constant, which
is reasonable if one type of technology is used,
the computation complexity of the method
is linear with the size of the circuit and the space complexity is constant.
.DS
.PS 2.0i
copy "../space3dman/strips.pic"
.PE
.fG "A layout subdivided into strips of width $w$." %FG_STRIPS%
.DE
.SP
Note that the calculated Green values of a single strip are written to
temporary files (space1.xxxxxx and space2.xxxxxx).
The program is using one of the directories of environment variable SPACE_TMPDIR.
If you want to check these Green buffers you can use parameter:
.DS I
debug.check_green  \fIboolean\fP    (default: off)
.DE
If "on" the Green values are recalculated and checked against the
values read from the Green buffer.
Thus you can check the Green buffers used.
