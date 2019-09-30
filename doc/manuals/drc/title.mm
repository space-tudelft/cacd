.T= "An Hierarchical DRC"
.DS 2
.rs
.sp 0.6i
.B
.S 15 20
An Hierarchical and Technology Independent Design Rule Checker
.S
.sp 1
.I
T. G. R. van Leuken
.br
J. Liedorp
.sp 1
.R
Circuits and Systems Group
Department of Electrical Engineering
Delft University of Technology
The Netherlands
.DE
.sp 2
.ce
.B Abstract
.DS 0 1
.ll -3
.in +3
This paper describes a uniform and new approach to
a technology independent and hierarchical
artwork verification method.
It is based upon the 'augmented instance' of a cell and
the stateruler scan algorithm.
By making an hierarchical instance of a cell, the cell is
made independent of other cells.
The artwork verification programs based upon the two mentioned
concepts exploit the hierarchy and repetition present in
the layout description of an integrated circuit.
That way the run time and memory requirements are no longer a function
of the number of layout primitives in the fully instanced integrated circuit,
but only of the number of primitives defined in the original
hierarchical layout description.
In the method of artwork verification described in this paper
the design rules that can be tested upon are based upon
the presence of combinations of masks.
All combinations of masks can be tested with respect to each
other, so the programs for the verification of the artwork
are largely technology independent.
Aside from handling the verification in an hierarchical manner,
the main problem addressed by the method is the efficient
handling of the large class of possible design rules.
We describe the concepts and their implementation.
The results are illustrated by some examples.
The techniques presented have been implemented for
paraxial geometrics.
They also are usable in the general context.
.in -3
.ll +3
.DE
.sp |9.1i
.DS
.in +3
.S 9
Copyright \s+3\(co\s-3 1988-2003 by the authors.

Date: October, 1986
Last revision: May, 2003.
.S
.in -3
.DE
.SK 0           \" goto next page
