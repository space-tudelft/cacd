.H 1 "Introduction"
.H 2 "3D Capacitance Extraction"
Parasitic capacitances of interconnects in integrated circuits become
more important as the feature sizes on the circuits are decreased
and the area of the circuit is unchanged or increased.
For submicron integrated circuits - where the vertical 
dimensions of the wires are in the same order of magnitude as their 
minimum horizontal dimensions -
3D numerical techniques are even required to
accurately compute the values of the interconnect capacitances.
.P
This document describes the layout-to-circuit extraction program
.P= space3d,
that is used to accurately and efficiently compute
3D interconnect capacitances of integrated circuits
based upon their mask layout description.
The 3D capacitances are part of an output circuit
together with other circuit components like transistors and resistances.
This circuit can directly be used as input 
for a circuit simulator like SPICE.
.H 2 "Space Characteristics"
To compute 3D interconnect capacitances,
.P= space3d
uses a boundary-element method. 
In the boundary-element method, elements are placed
on the boundaries of the interconnects.
This has as an advantage over the finite-element
and the finite-difference method (where the domain between the 
conductors is discretized) that - especially for 3D situations -
a lower number of discretization elements is used.
However, a disadvantage of the boundary-element method is that in order 
to compute
the capacitance matrix it
requires the inversion of a full matrix of
size $N times N$,
where $N$ is the total number of elements.
This takes $O(N sup 3 )$ time and $O ( N sup 2 )$ memory.
.P
To reduce the complexity of the above problem,
.P= space3d
employs a new matrix inversion technique 
that computes only an approximate inverse.
In practice, this means that only coupling effects
are computed between ``nearby'' elements and that
no coupling capacitances are found between elements that are far apart.
For flat layout descriptions, this method has a computation complexity
that is $O(N)$ and a space complexity that is $O(1)$.
As a result, 
.P= space3d
is capable of quickly extracting relatively large circuits
(> 100 transistors), and memory limitations of the computer
are seldom an insurmountable obstacle in using the program.
.H 2 "Documentation"
Throughout this document it is assumed that the
reader is familiar
with the usage of
.P= space
as a basic layout-to-circuit extractor,
i.e. extraction of transistors and connectivity.
This document only describes the additional information that
is necessary to use
.P= space3d
for 3D capacitance extraction.
The usage of
.P= space
or
.P= space3d
as a basic layout-to-circuit extractor is described
in the following documents:
.tr @'
.I(
.I= "Space User@s Manual"
This document describes all features of
.P= space
except for the 3D capacitance extraction mode.
It is not an introduction to
.P= space
for novice users,
those are referred to the
.I "Space Tutorial" .
.I= "Space Tutorial"
The \fISpace Tutorial\fP
provides a hands-on introduction to using
.P= space
and the auxiliary tools in the system that are used in conjunction with
.P= space .
It contains several examples.
.I= "Space Tutorial - Helios Version"
The same tutorial as above, but now described under the assumption
that the graphical user interface 
.P= helios 
is used to run the extraction tools.
.I= "Manual Pages"
For
.P= space
and
.P= space3d
as well as for other tools that are used in conjunction with
.P= space ,
manual pages are available describing (the usage of)
these programs.
The manual pages are on-line available,
as well as in printed form.
The on-line information can be obtained using the
.P= icdman
program.
.I= "Xspace User@s Manual"
This manual describes the usage of
.P= Xspace ,
a graphical X Windows based
interactive visualization tool of
.P= space3d.
(
.P= Xspace
is also part of
.P= helios
and can best be run from there.)
.I)
Also available:
.I(
.I= "Space Substrate Resistance Extraction User@s Manual"
This manual describes how resistances between
substrate terminals are computed in order
to model substrate coupling effects in analog
and mixed digital/analog circuits.
.I)
.tr @@
.H 2 "On-line Examples"
Two examples are presented in this manual that are also
available on-line.
We will assume that the \fISpace Software\fP
has been installed under the directory \fB/usr/cacd\fP.
The examples are then found in the directories
/usr/cacd/share/demo/poly5 and /usr/cacd/share/demo/sram respectively.
