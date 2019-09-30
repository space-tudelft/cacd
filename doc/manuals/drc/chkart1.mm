.nr Hy 0
.SA 1
.nr Ej 1
.H 1 "Introduction"
The growing complexity of the artwork of
integrated circuits necessitates the use
of structured design methods.
An important issue in this respect is
the possibility to use hierarchy in the
description of an integrated circuit, i.e. a cell in a
description may call another cell etc.
In most artwork description languages this concept
of hierarchy is present.
For efficiency reasons it is very desirable that
programs that are involved with the design and
verification of an integrated circuit can exploit this hierarchy.
The following demands are made on our system:
.AL a
.LI
The freedom of the designer's methodology should in
no way be impaired by the tools (for that reason we will not
use the notion 'hierarchical protection frame').
.LI
Minimal complexity both in the scanning and in the handling
of the multitude of design rules must be achieved.
.LI
The hierarchy must be exploited as much as possible.
.LE
.P
In this paper we will show a method for artwork
verification that optimally
exploits this.
We will describe the underlaying principles and describe
the way they are used to form an efficient way for
artwork verification.
.P
In section 2 we will define the 'augmented instance' of
a cell.
In doing so we will be able to see an hierarchical cell description
as the composition of a number of independent
augmented cell instances.
.SP 0
The augmented instances are made independent by
requiring that cells are interconnected by means of 'terminals'.
We will define a terminal as an area in a cell on a certain mask
where primitives of other cells, defined in the same  mask,
may overlap.
A difference between our approach and that of Newell and
Fitzpatric 
.[
newell fitzpatrick
.]
is that our approach preserves the cell
hierarchy in its original form.
.P
In section 3 we will discuss the conversion of the augmented
instances to line segments.
The algorithm for doing so will be based upon the stateruler scan
algorithm.
This algorithm also forms the basis for the artwork
verification programs discussed in the next sections.
.P
In section 4 we will discuss the artwork verification
programs, using the line segments as input.
.P
In section 5 some results of tests with the programs
mentioned will be given.
It turns out that the algorithm indeed is linear with
respect to the number of primitives in the cell under test,
as was already stated in.
.[
leuken fokkema iccd efficient vlsi artwork verification 1983
.]
.P
Section 6 at last will give some conclusions.
