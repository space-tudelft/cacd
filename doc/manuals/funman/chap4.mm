.H 1 "THE USAGE OF THE MIXED-LEVEL SIMULATOR"
.H 2 "Examples of simulations of networks containing function blocks"
In this section several examples will be shown of the usage of the
Mixed Level Simulator. In the networks that are described the
function blocks that have been shown in section 3.4. will
be used.
In the examples simulations will be done on functional level
and mixed-level.
.P
In the first example a network is simulated with the function
block \fImultiplexer\fP in it. The description of this function block
can be found in figure 5 in section 3.4. The SLS network description of the
network is shown in figure 11. In this description can be seen
that the network consists of only one subblock: the function block.
.fS I
.PS < ../funman/mux.sls
.fE
.FG "SLS description of network \fImux_netw\fP"
.P
In the SLS description the way of calling a function block in a network
is shown: the character @ is used just as is done when a built-in
function is called in a network.
The simulation of this network can be considered as a purely
functional simulation, since the only element in the network
is a function block. In the simulation the logic behavior of the
function block \fImultiplexer\fP can be verified.
.br
The function block has six \fBinput\fP terminals (see figure 5):
\fIin[0]\fP to \fIin[3]\fP and \fIselect[0]\fP and \fIselect[1]\fP.
In the network \fImux_netw\fP these terminals are connected with the
nodes \fImux_in[1]\fP to \fImux_in[4]\fP and \fImux_select[1]\fP
and \fImux_select[2]\fP.
Whenever at least one of those nodes
changes logic state, the function block will be evaluated.
In the simulation these nodes will change logic state as a
result of a 'set' command in the command file.
The function block has one output terminal: \fIout\fP;
This terminal is connected with the node \fImux_out\fP.
This node can be used in
a 'print' command in the command file.
The results of a simulation of the network \fImux_netw\fP is
shown in figure 12.
.fS I
.S 8
.PS < ../funman/mux_netw.out
.S
.fE
.FG "Results of a simulation of network \fImux_netw\fP"
.P
In the second example the function block \fIshiftregister\fP is used.
The description of this block can be found in figure 7.
Now the network \fIdoub_shft\fP
does not consist of one instance of the function block
but of two instances of the same function block plus some
inverters. The network is shown in figure 13.
.DS
.S 8

.PS < ../funman/mfig13.pic

.S 
.FG "Network diagram of network \fIdoub_shft\fP"

.DE
The SLS description of the subnetwork \fIinverter\fP and
the SLS description of the network \fIdoub_shft\fP are
shown in figure 14.
.fS I
.S -1
.PS < ../funman/d_shft.sls
.S
.fE
.FG "SLS description of networks \fIinverter\fP and \fIdoub_shft\fP"

In the network \fIdoub_shft\fP the function
block \fIshiftregister\fP is called twice: two different
instances exist of the function block. For each instance of
the function block, the state variables are different.
Only then it is possible for the two registers to contain
different data. 
.br
The subblock \fIshiftregister\fP is described on functional level and
the subblock \fIinverter\fP is described on transistor level.
Therefore the simulations of this network are real mixed-level simulations.
.br
In the network a bus exists: both registers and the
inverters are connected with the bus. It is not
possible to set data on the bus with the 'set' command
in the command file, because \fBa 'set' input node may
never be connected with a function block inout or output terminal\fP.
During the simulation a register puts data on the bus
only if its \fIo_enable\fP signal is high (see functional description
in figure 7). The inverters however, put data on the bus continuously.
Function block output terminals force their logic value on
the nodes they are connected with, unless their value is BTFREE.
Therefore, the bus will only get its data from the inverters when the value of
both the function block output terminals is BTFREE.
The results of a simulation of the network \fIdoub_shft\fP are
shown in figure 15.
.fS I
.S 8
.PS < ../funman/d_shft.out
.S
.fE
.FG "Results of a simulation of network \fIdoub_shft\fP"

In the functional description of \fIshiftregister\fP the terminals
\fIin_out\fP are set on BTFREE when no new values are assigned
to these terminals during the evaluation of the function block.
When this would not be done, the logic values that were inputed
from the bus and assigned to the terminals \fIin_out\fP,
would be outputted again. This would give rise to a 'pass-through' function
in the behavior of the function block that was not intended.
.P
The network of the third example is shown in figure 16.
In this network the function block \fIcount1\fP is called as
a subblock. The description of this function block can be found
in figure 9.
The SLS description of the network \fIcount_netw\fP is shown in figure 17.
.DS
.S 8

.PS < ../funman/mfig16.pic

.S
.FG "Network diagram of network \fIcount_netw\fP"
.DE


.fS I
.S -1
.PS < ../funman/cnt_ntw.sls
.S
.fE
.FG "Network description of network \fIcount_netw\fP"
.P
A simulation of this network will again be a mixed-level simulation.
The function block \fIcount1\fP is used in a network full of transistors,
resistors and capacitances. As can be seen in figure 9,
the input load as well
as the timing properties of the function block 
are defined in the functional description of the function block.
So in this example simulations will be shown on level 3:
timing simulations.
.fS I
.S 8
.PS < ../funman/cnt_ntw.out
.S
.fE
.FG "Results of a simulation of network \fIcount_netw\fP"
.P
In figure 18 results are shown of a simulation of the network
\fIcount_netw\fP. The simulation begins with loading the counter.
For that purpose data is set on the nodes \fIin[1]\fP to \fIin[4]\fP in
the network. This data is inverted and placed on the nodes
\fIin_in[1]\fP to \fIin_in[4]\fP which are connected with
the inread terminals
\fIin[0]\fP to \fIin[3]\fP of the function block.
The load of the inread terminals has been defined
in the description of the function block.
The effect of this load can be seen in the times it takes
to charge or discharge the nodes that are connected with the
terminals.
.br
When the counter is loaded, the counter can start counting;
in this case the counter will count 'up'.
When \fIphi2\fP is high the output terminals can change.
For the output terminals the rise and fall delay times are
defined (see figure 9). The delay time is the time between the
moment the
value of the output terminal changes and
the moment this change will influence the logic state
of the node that is connected with the output terminal.
As can be seen in the functional description of the
block \fIcount1\fP, the delay times of the output terminals
depend of the load of the node the terminals are connected with.
This load depends on the state of the transistors in the network.
In the results of the simulation is shown that the delay
times of the various output terminals differ and that they change
depending on the states of the pass-transistors
\fIp[1]\fP to \fIp[5]\fP.
.P
The last example describes the simulation of the function
block \fIcombinator\fP, of which the functional description
can be found in figure 10. The network \fIcomb_netw\fP is
created that contains only the function block.
The description of the network is shown in figure 19.
.fS I
.PS < ../funman/comb_ntw.sls
.fE
.FG "Network description of network \fIcomb_netw\fP"
.P
When this simulation is executed, input is asked to the
user by the function block (see figure 10). Therefore the results of the
simulation depend on the data given by the user.
The command file for a simulation is shown
in figure 20.
.fS I
.PS < ../funman/comb_ntw.cmd
.fE
.FG "Simulation input for simulation of network \fIcomb_netw\fP"
.P
The node \fIphi\fP is set to O and I for sixteen times. Each
time the node \fIphi\fP changes logic state, the function block
is evaluated. When the logic state is high, the value of the
input terminal \fIphi\fP of the function block
is BTTRUE and the user is asked to typ in
four characters. These characters must be 'O' or 'X' or 'I',so
the value of the output terminals can be determined. The nodes
that are connected with the output terminals are printed in the
simulation output file, since the 'print' command in the
simulation input file is stated that way.
.fS I
.S 8
.PS < ../funman/comb_ntw.out
.S
.fE
.FG "Simulation results of simulation of network \fIcomb_netw\fP"
.P
In figure 21 the output of a simulation is shown. The logic
states of the nodes \fIus_in[1]\fP to \fIus_in[4]\fP are
the characters typed by the user.

.H 2 "Mixed Level Simulations in the NELSIS-VLSI-design system"
In this section a sequence of actions and commands is
shown that can be used when using the Mixed Level
Simulation package.
The example in the previous section in which the
simulation of the network \fIdoub_shft\fP 
was discussed, will be used here. To be able to
perform simulations first the several descriptions
have to be created. Suppose the functional description
of the block \fIshiftregister\fP has been placed in the
file called 'shift.fun' and the network description of the
network \fIdoub_shft\fP has been placed in the
file 'doub_shft.sls'.
Then the following actions can be executed in an nMOS project directory:
.VL  33 2
.LI cfun~shift.fun
information about
the function block described in the file 'shift.fun' will
be placed in the database. The functional description is
converted to C functions which are compiled and placed in
the database.
.LI csls~doub_shft.sls
the network described in the file 'doub_shft.sls' is placed
in the database.
.LI sls_exp~doub_shft
a binary description of the cell \fIdoub_shft\fP is created
and placed in the database. An executable program 
\fBsls\fP 
is created by linking the object files describing the function
block(s) that are used in the network and the object files
describing the Switch Level Simulator. This will only be done
when no executable program 
\fBsls\fP 
exist, in which the function
block(s) already are incorporated.
The files 'deffunc.c' and 'sls.funlist' are created as well.
.LI sls~doub_shft~doub_shft.cmd
the cell \fIdoub_shft\fP is simulated using the commands
contained by the command file 'doub_shft.cmd'.
.LE
