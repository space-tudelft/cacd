.nr Hy 0
.SA 1
.nr Ej 1
.H 1 "Conclusions"
Very important issues in achieving efficient
operations in design rule checking are:
.AL
.LI
The exploitation of the hierarchy.
.LI
The generation of combination masks necessary for
the checking of intermask rules.
.LI
The complexity of the scan itself.
.LE
.P
We believe that we have achieved near optimal
results on the three counts.
Hierarchy is handled by making cells independent (for
checking purposes) from their sub_cells through
the notions of 'augmented instance' and 'checktype'.
The stateruler concept allows for a single pass to
determine all combination masks needed.
The scan itself is linear in the number of edges.
Although at present only implemented for paraxial
geometries the principles are generally applicable.
At the moment we are extending the method to general
geometries.
