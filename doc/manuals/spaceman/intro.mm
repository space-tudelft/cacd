.H 1 "Introduction"
.H 2 "What is Space"
.P= Space
is an advanced layout to circuit extractor for analog and digital
integrated circuits.
From a mask-level layout,
.P= space
produces a circuit netlist.
This netlist contains
.I(
.I=
active devices,
.I=
instances of other cells (only in hierarchical mode),
.I=
terminals (if they are defined in the layout).
.I)
Optionally,
this netlist contains
.I(
.I=
wiring capacitances to substrate,
.I=
inter-wire coupling capacitances
(both cross-over coupling capacitances and lateral coupling capacitances),
.I=
wiring resistances,
.I=
substrate resistances and capacitances.
.I)
These parasitics are extracted very accurately,
this is a must for state-of-the-art IC technologies where interconnection
parasitics tend to determine maximum circuit performance.
.P
Besides accuracy,
another key characteristic of
.P= space
is efficiency.
This is mainly due to new algorithms
for handling the IC geometry
and careful implementation of the program.
.P= Space
is intended for on-line, interactive use
but without problems it handles very large designs even on a workstation
or on a minicomputer.
.P
.P= Space
can operate in hierarchical mode
(in which case the extracted circuit possesses the same hierarchical
structure as the layout from which it is derived),
in flat mode (in which case only one circuit cell is generated for
the total layout),
or in mixed flat/hierarchical mode.
.H 2 "Benefits of Using Space"
A layout as extracted by
.P= space
can be simulated while fully taking into account all parasitic elements.
This is important,
since the decrease of feature sizes and the increase of chip dimensions
make the influence of wiring parasitics on the performance
of the chip critical, or even dominant:
Gate delays decrease but wiring delays may actually increase.
.P
Since
.P= space
is an integrated on-line tool, and very fast,
the effects of wiring parasitics can be taken into account in the
.I "design loop" .
This prevents
.I(
.I=
missed performance targets or even malfunction of the silicon produced,
.I=
and/or costly redesigns when the problem is detected late
(when relying on stand-alone verification tools).
.I)
.P= Space
enables meaningful performance/functional
characterization,
even for submicron technologies,
early in the design process.
.H 2 Features
The main features of
.P= space
are summarized as below.
Space is ...
.P
.I(
.I= "hierarchical"
.I= "capable of extracting 45 degree polygonal layout"
.I= "extremely fast"
The running time is linear in the size of the layout.
The extraction speed is depending on the extraction options selected.
.I= "technology independent"
.P= Space
is capable of extracting parasitics
from analog and digital,
MOS and bipolar layouts.
.P= Space
accepts user defined/adjusted element definition files.
.I= "needs little computer memory"
The memory usage is slightly worse than proportional to the square-root
of the size of the layout.
Consequently,
.P= space
can extract huge flat layouts using a minimum amount of memory.
.I= "accurately extracts lateral and crossover coupling capacitances"
The capacitance model employed by
.P= space
includes surface to surface, edge to surface and edge to edge
coupling capacitances.
.I= "extracts bipolar device model parameters"
Bipolar device model parameters are extracted
using layout information like emitter area, emitter
perimeter and base width.
.I= "employs Finite Element methods for accurate resistance extraction"
These are much more accurate than polygon-partitioning based heuristics,
and nearly as efficient.
.I= "produces accurate but simple RC networks"
.P= Space
employs a new, Elmore time constant preserving
algorithm to transform a detailed Finite Element RC mesh
into a low complexity lumped network
that accurately models the distributed
nature of the parasitic resistance and capacitance of IC interconnects.
To model higher order time constants, the number of RC sections
in the output network is a function of the maximum
signal frequency which can be adjusted by the user.
.I= "can extract substrate resistances and capacitances"
In order to model substrate coupling effects in analog and
mixed digital/analog circuits,
.P= space 
can apply an accurate numerical technique or a fast interpolation
technique to extract the substrate resistances.
Additional, substrate capacitances can be extracted using the same
accurate technique or fast by using a rc-ratio.
.I= "can read/write various layout/circuit formats"
For example, 
CIF or 
GDSII input and SPICE, SPF, SPEF, EDIF or VHDL output.
.I= "enables trading of accuracy versus computer time"
Users can select the type of parasitics they want to extract.
.I)
.H 2 "Documentation on Space"
The following documentation on 
.P= space
is available:
.tr @'
.I(
.I= "Space User@s Manual"
This document.
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
This short manual describes the usage of
.P= Xspace, 
a graphical X Windows based
interactive version of
.P= space.
Note, however, that a more general graphical user interface to
.P= space
is provided by the program
.P= helios .
.I= "Space 3D Capacitance Extraction User@s Manual"
The space 3D capacitance extraction user's manual provides
information on how
.P= space
can be used to extract accurate 3D capacitances.
.I= "Space Substrate Resistance Extraction User@s Manual"
This manual describes how resistances between
substrate terminals are computed in order
to model substrate coupling effects in analog
and mixed digital/analog circuits.
.tr @@
.I)
