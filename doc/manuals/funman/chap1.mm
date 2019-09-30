.H 1 "INTRODUCTION"
In this manual it is explained how function blocks can be used in the
Switch-Level Simulator SLS. Before a user can do this, he or she should be
familiar with the SLS simulator. Therefore in
this manual the SLS User's Manual [
.[
SLS Switch-Level User's Manual Genderen
.]] 
is assumed to be generally known.
.P
The simulator SLS has been extended to a Mixed Level Simulator that
can be used for simulations on functional level down to switch-level.
This extension has been created by allowing the use of function blocks
in networks. In SLS (as Switch-Level Simulator) a
network can be described hierarchically: subnetworks can be called
in networks. Now in the Mixed-Level Simulator SLS, a subblock
can also be a function block, instead of a network.
When a network consists of only function blocks,
a functional simulation can be performed. When both function blocks and
transistor-level parts or switch-level subnetworks exist in a network,
mixed-level simulations result. And of course the possibility still
exists to use SLS purely as a Switch-Level Simulator.
.P
This new feature of SLS, function blocks, very well supports the top-down
as well as bottom-up design methods. In the top-down method a design
first is partitioned in a few general functional blocks. These blocks are
divided into subblocks that can be described on several different levels.
These subblocks can also be refined until the lowest level is reached.
In the bottom-up approach one or more subblocks are replaced by a block
or a higher level, obtaining a more abstract description.
.P
A function block is a software model of a digital design. It manipulates
its inputs and produces outputs. In the simulator the model of a function block
is such that a good connection exists with the switch-level parts
in a network. This
is the case in logic as well as timing simulations. The language in
which the function blocks is described is mainly the C programming
language. For most of the function blocks the language only needs to be
used in a simple manner. This manual can be used as a guide in doing so,
even when the designer is not familiar with the C language.
Section 2 in this manual gives an overview of the extended SLS package
of programs. Section 3 describes the model of the function block and
the syntax of the description.
Section 4 explains some ways to use function blocks in simulations.
.SK
