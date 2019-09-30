.H 1 "Introduction"
.H 2 "Substrate Resistance Extraction"
In modern analog circuits and
mixed digital/analog circuits,
coupling effects via the substrate can
be an important cause of malfunctioning of the circuit.
This problem becomes more prominent as (1) there is
a trend to integrate more and more different components
on a chip, (2) the decrease of
wire width and increase of wire length causes the interconnect
parasitics and hence the level of noise on the chip to increase,
and (3)
the use of lower supply voltages
makes the circuits more sensitive to internal potential
variations.
.P
An example of a substrate coupling problem is given below.
The figure below shows
mixed-signal integrated circuit. The switching in the digital
part induces potential spikes on the supply lines. These spikes
are then coupled into the substrate, where they propagate to the analogue
part of the circuit. There they are picked up, e.g. by the depletion
capacitance of a diffused resistor or the bulk contact of a
transistor. Thus, the disturbances may appear at the output of the
circuit, degrading the performance or even causing malfunctioning of
the circuit.
.F+
.PSPIC "../spacesubman/problem.eps" 5.3i
.F-
.P
The substrate coupling effects in
integrated circuits can be verified by computing the substrate
resistances between all circuits parts that inject noise into
the substrate and/or that are sensitive to it.
The noise injectors are mainly the contacts that connect the substrate
and the wells to the supply voltages.
The current variations in the supply lines cause fluctuating
potentials over their resistances and inductances, that are
injected into the substrate via the substrate contacts and the well contacts.
Other parts that may generate noise and/or that are sensitive to it
are (1) 
the bulk connections of the transistors.
(2) drain/source areas of transistors,
(3) on-chip resistors and capacitors, and (4) interconnect wires that
are coupled to the substrate via a (large) substrate capacitance.
.P
This document describes how the layout-to-circuit 
extraction program
.P= space
is used to extract substrate resistances
of integrated circuits
based upon the mask layout description of these circuits.
The substrate resistances are part of an output circuit
together with the other extracted circuit components 
like transistors and interconnect parasitics.
This circuit can then directly be used as input 
for a circuit simulator like SPICE in order to 
verify the substrate coupling effects in the circuit.
.H 2 "Space Characteristics"
To compute substrate resistances,
.P= space
uses one of the following methods:
.BL
.LI
Boundary-Element Method
.br
This is an numerical method that provides
accurate results.
However, for large circuits, this method requires
a relatively large amount of memory and is not very fast.
.LI 
Interpolation Method
.br
This method uses interpolation formulas to compute
the substrate resistances.
This method is not as accurate as the boundary-element method,
but it requires less memory and is much faster.
To find the parameters for the interpolation formulas,
this method uses the boundary-element method for some (small)
standard terminal configuration or it uses measurement results
for standard configurations.
.LE
.P
Additional, to compute substrate capacitances,
.P= space
uses one of the following methods:
.BL
.LI
Boundary-Element Method (see above)
.LI
RC-ratio Method
.br
The capacitances are found by multiplying the found substrate resistances
with a constant factor (parameter "sub_rc_const").
.LE
.H 2 "Documentation"
Throughout this document it is assumed that the
reader is familiar
with the usage of
.P= space
as a basic layout-to-circuit extractor,
i.e. extraction of transistors, connectivity and interconnect
parasitics.
This document only describes the additional information that
is necessary to use
.P= space
for substrate resistance extraction.
The usage of
.P= space
as a basic layout-to-circuit extractor is described
in the following documents:
.tr @'
.I(
.I= "Space User@s Manual"
This document describes all basic features of
.P= space.
It is not an introduction to
.P= space
for novice users,
those are referred to the
.I "Space Tutorial" .
.I= "Space Tutorial / Space Tutorial Helios Version"
The space tutorial
provides a hands-on introduction to using
.P= space
and the auxiliary tools in the system that are used in conjunction with
.P= space .
It contains several examples.
The space tutorial helios version provides a similar
hands-on introduction, but now from a point of view
where the graphical user interface
.P= helios
is used to invoke
.P= space .
.I= "Manual Pages"
For
.P= space
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
.P= Xspace, 
a graphical X Windows based
interactive visualization tool of
.P= space.
Note, however, that a more general graphical user interface to
.P= space
is provided by the program
.P= helios .
.I)
Also available:
.I(
.I= "Space 3D Capacitance Extraction User@s Manual"
The space 3D capacitance extraction user's manual provides
information on how
.P= space
can be used to extract accurate 3D capacitances.
.I)
.tr @@
.H 2 "On-line Examples"
Two examples are presented in this manual that are also
available on-line.
We will assume that the
.P= space
software has been installed under the directory \fB/usr/cacd\fP.
The examples are then found in the directories
/usr/cacd/share/demo/sub3term and /usr/cacd/share/demo/suboscil.
.SP 2.5
.N(
The current version of
.P= space
can only compute substrate resistance for
orthogonal substrate terminals.
.N)
