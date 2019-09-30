.T= "Space Resistance Extraction Application Note"
.DS 2
.rs
.sp 1i
.B
.S 15 20
SPACE RESISTANCE EXTRACTION
APPLICATION NOTE
.S
.sp 1
.I
S. de Graaf
.sp 1
.R
Circuits and Systems Group
Faculty of Electrical Engineering,
Mathematics and Computer Science
Delft University of Technology
The Netherlands
.DE
.sp 2c
.ce
Report EWI-ENS 03-01
.ce
January 10, 2003
.sp |9i
.DS
.in +5
.S 9
Copyright \s+3\(co\s-3 2003 by the author.

Last revision: December 9, 2003.
.S
.in -5
.DE
.SK
.S=
.H 1 "INTRODUCTION"
To understand how resistance extraction works
and what is changed after adding a new substrate contact resistance feature,
i have written this note.
See also the "Space User's Manual"
.[
Gender Meijs Beeftink Elias
.]
and the
"Space Tutorial".
.[
Space Tutorial
.]
.H 2 "Substrate Contacts"
Interconnect nodes can be connected with the substrate by means of substrate contacts.
When no (interconnect) resistance extraction is done,
these interconnect nodes become the substrate node SUBSTR in case of
no substrate resistance extraction or
one of the substrate terminal nodes in case of substrate resistance extraction.
.P
The substrate resistance extraction mode (option
.B -b
or
.B -B )
is a separate
extraction mode,
it extracts substrate resistors between the substrate terminal nodes and
between these nodes and the SUBSTR node.
.P
In the (interconnect) resistance extraction mode (option
.B -r
or
.B -z )
however,
there are extracted resistors between high resistive interconnect nodes.
The interconnect is high resistive if the resistance value is greater than
parameter low_sheet_res (default 1 ohm).
The high resistive interconnect parts are node groups,
whereby the nodes of each group are connected with each other by resistors.
.P
Different interconnect layers can be connected with each other by
means of via's (special contact layers).
Because nodes of different groups are connected by a contact resistor,
the groups are joined together and become one big group.
Note that a contact always connects.
However, there are only contact resistors calculated for specified
technology contact resistance values greater than 0.1 ohm per square micron.
Note that there is no low_contact_res parameter.
In other cases the nodes are joined together.
.P
The substrate shall never join two interconnect groups together.
The substrate terminal nodes are also in separate groups,
the substrate resistors between the substrate nodes are flagged as special
and shall not join the groups together.
However, a group can contain more substrate terminal nodes,
because interconnect groups can be joined together.
.H 2 "Dummy Contact Resistors"
By resistance extraction without substrate resistance extraction, however,
the SUBSTR node may not be joined with interconnect nodes.
Because the SUBSTR node shall join all these interconnect groups together.
In that case we must always add a contact resistor to SUBSTR.
When in the technology file no contact resistance value (0 ohm) is specified,
than the program adds dummy contact resistors (value 1e-300).
These dummy contact resistors are normally automatically eliminated,
because parameter "min_res" is set to 1e-299.
.H 2 "New Substrate Contact Resistance Feature"
When in the technology file no contact resistance value (0 ohm) is specified
for contacts with substrate and only (interconnect) resistance extraction
is done,
than shall all interconnect contact node points be joined together.
Note that this feature is not used for contacts between interconnect,
but also for normal contacts is it recommended.
Note that you can force these node point joins,
specify a zero sheet resistance (or a low_sheet_res) value for
the interconnect mask(s) and via mask combination in the technology file
and it must also work.
Thus, after all, it was maybe not needed to add the feature for substrate contacts.
.H 2 "Conclusions"
After running the resistance extraction examples the following conclusions
can be made:
.AL
.LI
The choice of the terminal position is very important.
Normally, extra mesh is generated for option
.B -z .
A different mesh gives different resistance end values.
.LI
Fine tuning of the interconnect contact resistance values in the technology file
to low_sheet_res values gives also a different mesh for option
.B -z .
.LE
.H 1 "RESISTANCE EXTRACTION EXAMPLE 1"
.H 2 "Test2 Layout"
We use the following test example to test the use of substrate contacts.
See the figure below (lambda=0.01 micron).
.F+
.PSPIC an0301/test2lay.ps 5i 3i
.F-
.H 2 "Test2 Initial Mesh by Option -r"
.F+
.PSPIC an0301/test2mr.ps 5i 3i
.F-
Use options -%f (fine network),
.B -n
(no reduction)
and
.B -x
(coordinates) to get this result.
.SK
.H 2 "Test2 Resistance Mesh by Option -r"
.F+
.PSPIC an0301/test2mr2.ps 5i 3.5i
.F-
The tiles are all split in triangles, because all tiles contain extra node points.
The substrate contact area (node points 1,6,10,11) is split into two tiles by
terminal "c" (tile 1,6,9,5 and 5,9,10,11).
Only the corner points of these two tiles have contact resistances to the SUBSTR node.
The resistance line between node points 11,12 and 2,6 is left out,
because the conductance value of the element between these points is zero
and is not outputted.
Note that the above result is different for a contact resistances of zero ohm,
see the figure below:
.F+
.PSPIC an0301/test2mr3.ps 5i 2.5i
.F-
.H 2 "Test2 Resistance Mesh by Option -z"
.F+
.PSPIC an0301/test2mz.ps 5i 3.5i
.F-
The above figure shows also the initial mesh.
Note that the above result is different for a contact resistances of zero ohm,
see the figure below:
.F+
.PSPIC an0301/test2mz2.ps 5i 3.5i
.F-
Note that also the node numbering is different.
.H 2 "Test2 Reduced Network by Option -r"
.fS
network test2 (terminal a, b, c) /* contact res = 1e-6 ohm */
{
    res 24.97386k (a, b);
    res 195.5036 (a, c);
    res 129.9503M (a, SUBSTR);
    res 329.6425 (c, b);
    res 6.716953M (c, SUBSTR);
    res 23.02741M (b, SUBSTR);
}

network test2 (terminal a, b, c) /* contact res = 0 ohm */
{
    net {SUBSTR, c};
    res -67.47437k (a, b);
    res 156.0465 (a, SUBSTR);
    res 171.8272 (b, SUBSTR);
}
.fE
.H 2 "Test2 Reduced Network by Option -z"
.fS
network test2 (terminal a, b, c) /* contact res = 1e-6 ohm */
{
    res 4.592818k (a, b);
    res 324.0175 (a, c);
    res 104.0895M (a, SUBSTR);
    res 444.1103 (c, b);
    res 6.413887M (c, SUBSTR);
    res 29.00236M (b, SUBSTR);
}

network test2 (terminal a, b, c) /* contact res = 0 ohm */
{
    net {SUBSTR, c};
    res 158.596k (a, b);
    res 275.6303 (a, SUBSTR);
    res 273.1351 (b, SUBSTR);
}
.fE
.H 2 "Test2 Technology File"
.fS
conductors:
    condIN   : cmf     : cmf  : 200

contacts:
#   cont_sub : cmf cca : cmf @sub  : 1e-6
    cont_sub : cmf cca : cmf @sub  : 0
.fE
.H 1 "EXAMPLE 1 RESULTS FOR 32,32 32,32 C"
.H 2 "Test2 Reduced Network by Option -r"
.fS
network test2b (terminal a, b, c) /* contact res = 1e-6 ohm */
{
    res 325.0935 (c, b);
    res 154.9491 (c, a);
    res 6.418619M (c, SUBSTR);
    res -7.418598k (b, a);
    res 20.70967M (b, SUBSTR);
    res -244.8220M (a, SUBSTR);
}

network test2b (terminal a, b, c) /* contact res = 0 ohm */
{
    net {SUBSTR, c};
    res -53.87113k (a, b);
    res 155.4959 (a, SUBSTR);
    res 171.3351 (b, SUBSTR);
}
.fE
.H 2 "Test2 Reduced Network by Option -z"
.fS
network test2b (terminal a, b, c) /* contact res = 1e-6 ohm */
{
    res 282.176 (a, c);
    res 6.130878k (a, b);
    res 142.1892M (a, SUBSTR);
    res 447.0263 (c, b);
    res 6.588469M (c, SUBSTR);
    res 24.28055M (b, SUBSTR);
}

network test2b (terminal a, b, c) /* contact res = 0 ohm */
{
    net {SUBSTR, c};
    res 139.4461k (a, b);
    res 267.6901 (a, SUBSTR);
    res 270.4175 (b, SUBSTR);
}
.fE
.H 1 "EXAMPLE 1 RESULTS FOR 32,32 82,72 C"
Note, a point terminal "c" at position 57,52 57,52 gives the same result.
.br
Conclusion:
Every terminal can be considered as a point terminal.
.H 2 "Test2 Reduced Network by Option -r"
.fS
network test2c (terminal a, b, c) /* contact res = 1e-6 ohm */
{
    res 2.99776k (a, b);
    res 307.2914 (a, c);
    res 30.33919M (a, SUBSTR);
    res 314.8159 (c, b);
    res 7.414814M (c, SUBSTR);
    res 31.0821M (b, SUBSTR);
}

network test2c (terminal a, b, c) /* contact res = 0 ohm */
{
    net {SUBSTR, c};
    res -115.2133k (a, b);
    res 156.9577 (a, SUBSTR);
    res 172.6412 (b, SUBSTR);
}
.fE
.H 2 "Test2 Reduced Network by Option -z"
.fS
network test2c (terminal a, b, c) /* contact res = 1e-6 ohm */
{
    res 3.509828k (a, b);
    res 409.4207 (a, c);
    res 50.09233M (a, SUBSTR);
    res 408.413 (c, b);
    res 6.250147M (c, SUBSTR);
    res 49.90071M (b, SUBSTR);
}

network test2c (terminal a, b, c) /* contact res = 0 ohm */
{
    net {SUBSTR, c};
    res 186.309k (a, b);
    res 277.986 (a, SUBSTR);
    res 279.2133 (b, SUBSTR);
}
.fE
.H 1 "EXAMPLE 1 RESULTS FOR 82,72 82,72 C"
.H 2 "Test2 Reduced Network by Option -r"
.fS
network test2d (terminal a, b, c) /* contact res = 1e-6 ohm */
{
    res 327.8002 (a, c);
    res -17.85996k (a, b);
    res 20.23829M (a, SUBSTR);
    res 171.1162 (b, c);
    res -942.7892M (b, SUBSTR);
    res 6.594209M (c, SUBSTR);
}

network test2d (terminal a, b, c) /* contact res = 0 ohm */
{
    net {SUBSTR, c};
    res -53.87113k (a, b);
    res 155.4959 (a, SUBSTR);
    res 171.3351 (b, SUBSTR);
}
.fE
.H 2 "Test2 Reduced Network by Option -z"
.fS
network test2d (terminal a, b, c) /* contact res = 1e-6 ohm */
{
    res 446.1399 (a, c);
    res 5.905819k (a, b);
    res 24.57639M (a, SUBSTR);
    res 286.3825 (c, b);
    res 6.582918M (c, SUBSTR);
    res 135.1225M (b, SUBSTR);
}

network test2d (terminal a, b, c) /* contact res = 0 ohm */
{
    net {SUBSTR, c};
    res 139.4461k (a, b);
    res 267.6901 (a, SUBSTR);
    res 270.4175 (b, SUBSTR);
}
.fE
.H 1 "RESISTANCE EXTRACTION EXAMPLE 2"
As explained before, by zero contact resistance,
a contact between interconnect and substrate
is handled different than a contact between two interconnect layers.
Because the substrate is considered as a zero ohm layer.
In this example, we try to do it also for two interconnect layers.
.H 2 "Test2s Layout"
.F+
.PSPIC an0301/test2slay.ps 5i 3i
.F-
.H 2 "Test2s LDM File"
.fS
:: lambda = 0.01 micron
ms test2s
term cmf 0 0 0 0 a
term cms 0 0 0 0 a2
term cmf 120 120 104 104 b
term cms 120 120 104 104 b2
term cmf 32 82 32 72 c
term cms 32 82 32 72 c2
box cmf 0 120 0 104
box cms 0 120 0 104
box cca 32 82 32 72
me
.fE
.SK
.H 2 "Test2s Technology File"
.fS
conductors:
    condIN  : cmf    !cca : cmf  : 200
    condINc : cmf     cca : cmf  : 0

    condIN2 : cms         : cms  : 200    # try1
  # condIN2 : cms    !cca : cms  : 200    # try2
  # condIN2c: cms     cca : cms  : 0      # try2

contacts:
    cont_IN : cmf cms cca : cmf cms : 0
.fE
With try1, we do not use zero ohm for the second metal layer.
.H 2 "Test2s Reduced Network by Option -r"
.fS
network test2s (terminal a, a2, b, b2, c, c2) /* try1 */
{
    res 48.78049 (c2, c);
    res 172.6412 (c, b2);
    res 172.6412 (c, b);
    res 156.9577 (c, a2);
    res 156.9577 (c, a);
    res -115.2133k (b2, a2);
    res -115.2133k (a, b);
}
.fE
Because there are only contact resistances in the corners of the tiles
and not in every triangle contact area point, therefor terminal "c2" is
not joined with "c".
By option
.B -z
is the initial tile mesh different and is "c2" joined with "c".
By try2 however, terminal "c2" is joined with "c".
.fS
network test2s (terminal a, a2, b, b2, c, c2) /* try2 */
{
    net {c, c2};
    res -115.2133k (a2, b2);
    res 156.9577 (a2, c);
    res 172.6412 (c, b2);
    res 172.6412 (c, b);
    res 156.9577 (c, a);
    res -115.2133k (a, b);
}
.fE
.SK
.H 2 "Test2s Reduced Network by Option -z"
.fS
network test2s (terminal a, a2, b, b2, c, c2) /* try1 */
{
    net {c, c2};
    res 186.309k (a2, b2);
    res 277.986 (a2, c);
    res 186.309k (a, b);
    res 277.986 (a, c);
    res 279.2133 (c, b2);
    res 279.2133 (c, b);
}

network test2s (terminal a, a2, b, b2, c, c2) /* try2 (1) */
{
    net {c, c2};
    res 169.7879k (a2, b2);
    res 273.5306 (a2, c);
    res 169.7879k (a, b);
    res 273.5306 (a, c);
    res 274.5206 (c, b2);
    res 274.5206 (c, b);
}

network test2s (terminal a, a2, b, b2, c, c2) /* try2 (2) */
{
    net {c, c2};   /* point terminal at position 32,32 */
    res 139.4461k (a2, b2);
    res 267.6901 (a2, c);
    res 139.4461k (a, b);
    res 267.6901 (a, c);
    res 270.4175 (c, b2);
    res 270.4175 (c, b);
}
.fE
.P
Note that the above try1 result (for point terminal "c" and "c2"
at position 57,52) is not equal to the above try2 (1) result.
Because the horizontal split is missing by try2.
.P
Note that the above try2 (1) result is equal to a try1 result
for point terminal "c" and "c2" at position 57,32 or 57,72.
.P
Note that the above try2 (1) result is equal to a try2 result
for point terminal "c" and "c2" at position 57,52 or any
other y-position (y >= 32 and y <= 72) for x-position 57.
.P
Note that the above try2 (2) result is equal to a try2 result
for a point terminal "c" and "c2" at all contact corner positions.
Because no extra mesh need to be generated.
For other y-positions for x-position 32 (82) is this not true,
because the old (new) tile on the left (right) side
is horizontal split by the terminal, because resistance must be
extracted for that tile.
The result is conform substrate example 1 network test2b/2d.
.P
Note that a terminal always generate a vertical split.
.H 2 "Problem with two terminals at x-position 82"
During testing i encountered a bug in the
.P= space
program.
When we run the test for try2 with option
.B -z
for point terminals on x-position 82 and
for y-positions between y > 32 and y <= 72 the problem arise.
.P
I give the mesh and the result for point terminals "c" and "c2" at
position 82,52:
.F+
.PSPIC an0301/test2sm.ps 5i 2.5i
.F-
.fS
network test2s (terminal a, a2, b, b2, c, c2)
{
    net {c, c2};
    res 146.3964k (a2, b2);
    res 267.6677 (a2, c);
    res 146.3964k (a, b);
    res 267.6677 (a, c);
    res 274.9844 (c, b2);
    res 274.9844 (c, b);
}
.fE
The bug caused a loop in function nodeJoin() in the
.P= space
program.
The loop occurs when the tile contact area t5 only contains low_sheet_res
conductors.
In that case, tile t5 has only one node point at position 32,32.
It only happens, when there are more terminals at the same position at
the left edge of the tile t5.
.br
In source file "extract/enumpair.c" function connectPoints() shall do
a call to subnodeCopy() twice for the same subnode.
It is fixed in function resEnumPair() as follows:
.fS
817,818c817,818
<     placePoint (newerTile, 1, 'v', term -> x, term -> y, 0, 0);
<     connectPoints (tile, newerTile, 'v', 1, newerTile -> known);
---
>     if (placePoint (newerTile, 1, 'v', term -> x, term -> y, 0, 0))
>         connectPoints (tile, newerTile, 'v', 1, newerTile -> known);
.fE
.SK
.[
$LIST$
.]
.TC
