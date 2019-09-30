.T= "Space Conductor Type Warnings"
.DS 2
.rs
.sp 1i
.B
.S 15 20
Space Conductor Type Warnings
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
Report EWI-ENS 10-01
.ce
May 31, 2010
.sp |9i
.DS
.in +5
.S 9
Copyright \s+3\(co\s-3 2010 by the author.

Last revision: May 31, 2010.
.S
.in -5
.DE
.SK
.S=
.H 1 "INTRODUCTION"
For the new
.P= space
version 5.4.2
there is added a polarity check for subnode copy and join.
A subnode is a part of a node which is found in a layout tile.
A subnode is created for every conductor found in a layout tile.
When a neighbor tile already contains the same conductor subnode,
then a subnodeCopy can be done.
In some situations, when subnodes becomes connected, then
a subnodeJoin operation can be done.
.P
However,
in the technology file,
you can specify the same conductor twice.
This is done for the active area conductor,
one has a positive and one has a negative polarity type.
Only one of the two can exist in a layout tile.
.P
However,
when two tiles of the same conductor touch each other, they are connected.
Because of the same conductor number, the polarity is not taken into account.
Therefor a warning message is given when this happens (but only ones).
For example:
.fS
space: warning: node join of subnodes with different conductor-type
    for conductor mask 'caa' at position (20, 28).
.fE
For example the following techfile is used:
.fS
new : PD | ND : caa

conductors :
  # name    : condition  : mask : res : type
    cond_mf : M1         : M1   : 0.2 : m  # conductor metal1
    cond_pg : PO         : PO   : 10  : m  # poly interconnect
    cond_pa : caa PD !PO : caa  : 0   : p  # p+ active area 
    cond_na : caa ND !PO : caa  : 0   : n  # n+ active area 
    cond_wn : NW         : NW   : 0   : n  # n well  
 
fets :
  # name : condition    : gate d/s : bulk
    nenh : PO caa ND    : PO  caa  : @sub
    penh : PO caa PD    : PO  caa  : NW

contacts :
  # name   : condition     : lay1 lay2 : res
    cont_p : CT M1 PO      : M1  PO    : 15  # metal1 to poly
    cont_a : CT M1 caa !PO : M1  caa   : 20  # metal1 to active
    cont_w : CT M1 ND NW   : M1  NW    : 20  # metal1 to well 
    cont_s : CT M1 PD !NW  : M1  @sub  : 20  # metal1 to subs
.fE
Some comments about the techfile:
.P
The new-statement defines a new mask "caa".
This new name can be used in a condition, but is replaced
by its condition (PD | ND).
When it is used as conductor mask, then it gets a conductor number
and the new name is added to the list of standard mask names.
However, the new mask is not a real physical mask, but it exist
in a tile when the PD or ND mask exists in the tile.
.P
The condition of conductor "cond_pa" can be rewritten as:
.fS
    cond_pa : caa PD !PO
    cond_pa : (PD | ND) PD !PO
    cond_pa : PD !PO | ND PD !PO
    cond_pa : (1 | ND) PD !PO
    cond_pa : PD !PO
.fE
The condition of conductor "cond_na" can be rewritten as:
.fS
    cond_na : caa ND !PO
    cond_na : ND !PO
.fE
Note that conductor "cond_na" also exists in the tile where
a nwell contact is done.
And also the "cont_a" contact exists!
This happens also for "cond_pa" and "cont_a" by a substrate contact.
.P
To eliminate this problem "NW" must be specified in the condition
for "cond_pa" and "!NW" in the condition for "cond_na".
.fS
    cond_pa : caa PD !PO  NW : caa  : 0   : p  # p+ active area 
    cond_na : caa ND !PO !NW : caa  : 0   : n  # n+ active area 




.fE
For an example of a problem layout, see the layout in next section.
.br
The following warning happens by extraction:
.fS
space: warning: node join of subnodes with different conductor-type
    for conductor mask 'caa' at position (20, 28).
.fE

.H 1 "PROBLEM LAYOUT"
The following layout gives the warning message.
This because for both transistors the drain/source area is extended
to connect it directly to positive/negative power supply rails.
Thereby PD and ND conductors touch each other.
This happens, because on the power rails substrate and nwell contacts are made.
See next section how to fix it.
.P
.F+
.PSPIC "an1001/fig1.ps" 5i
.F-
.fS
.P
% space -F SAMPLE_INV
space: warning: node join of subnodes with different conductor-type
    for conductor mask 'caa' at position (20, 28).
.fE

.H 1 "FIXED PROBLEM LAYOUT"
The following layout fixes the warning message.
Use always metal1 to connect a drain/source area.
.P
.F+
.PSPIC "an1001/fig2.ps" 5i
.F-
