.T= "Using SubstrateStorm"
.DS 2
.rs
.sp 1i
.B
.S 15 20
USING
SUBSTRATESTORM
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
Report EWI-ENS 05-06
.ce
November 21, 2005
.sp |9i
.DS
.in +5
.S 9
Copyright \s+3\(co\s-3 2005-2006 by the author.

Last revision: May 17, 2006.
.S
.in -5
.DE
.SK
.S=
.H 1 "INTRODUCTION"
To compare the subtrate netlist extraction results of the BEM/FEM
.P= Space
method, we want to use
.P= SubstrateStorm.
.P
.P= SubstrateStorm
is a program from Simplex Solutions, Inc. (see: www.simplex.com).
Simplex Solutions, Inc., a Sunnyvale, California-based company that provides
software and services for the design and verification of integrated circuits (ICs).
Simplex's customers use its products and services prior to manufacture to design and verify ICs
to help ensure that they will perform as intended, taking into account the complex effects of deep-submicron semiconductor physics.
.P
Now Simplex is a part of Cadence Design Systems, Inc. (see: www.cadence.com).

See announcement of January 16, 2002:
.br
\fB"Cadence To Offer Simplex Extraction Technology",
"Exclusive Agreement to Provide World-class Gate-level Extraction Solution".\fP
Cadence and Simplex today announced a technology-licensing agreement
that gives Cadence exclusive distribution rights to the Simplex standalone parasitic extraction technology, Fire & Ice(R) QX.
The agreement addresses customers' requirements for fast,
accurate parasitic extraction by incorporating Simplex extraction technology as part of the Cadence
advanced deep submicron solution-the industry's most complete design flow.

See announcement of April 24, 2002:
.br
\fB"Cadence Expands Technology Leadership At 0.13-Micron-And-Below With Acquisition Of Simplex",
"Cadence Brings Together World-Class Teams and Best-In-Class Technologies".\fP
"Our strategy is to bring the best technology to market for 0.13-micron-and-below design,"
said Ray Bingham, president and CEO of Cadence Design Systems.
"The proposed acquisition of Simplex fuels our efforts to supply our customers
with the world's best technology to ensure 1st Silicon Success(R).
Simplex's world-class technology and management team will be making significant contributions to the combined company."
.P
As semiconductor process geometries move to 0.13-micron and below,
deep-submicron (DSM) physics create new challenges for the semiconductor industry
that require a new generation of design technology solutions.
To be successful at these smaller geometries, designers need tools that can easily interoperate,
support hierarchical design methods and rapid prototyping,
and that take into account critical DSM effects such as signal integrity,
voltage (IR) drop, electromigration and substrate noise.
.P
As a leading innovator in the area of DSM system-on-a-chip (SoC) verification solutions,
Simplex will provide Cadence with complementary best-in-class technology for 3D parasitic extraction and full-chip power-grid planning,
electromigration and signal integrity solutions.
Simplex also brings a fast-growing, leading-edge design services capability that specializes in high-performance,
multi-million-gate digital designs to complement Cadence's strengths in leading-edge analog and mixed-signal design.
Simplex will also provide Cadence with an exciting opportunity to improve supply chain relationships through the X Initiative,
and design performance, power, cost and yield through the X Architecture interconnect technology.

See announcement of June 27, 2002:
.br
\fB"Cadence Completes Acquisition of Simplex Solutions",
"Electronic Design Leader Fields World's Best Technology Line-Up for 0.13-Micron-and-Beyond".\fP
Cadence reached a definitive agreement to acquire Simplex on April 24, 2002.
This acquisition is a key milestone in the Cadence strategy to deliver manufacturing-aware design flows
that enable customers to achieve 1st Silicon Success(R) with the most advanced manufacturing technologies
It also advances the Cadence Design Chain Initiative aimed at strengthening development relationships throughout the electronics industry.
.P
The physics challenges at 0.13-micron and beyond demand a new generation of design technology solutions.
To be successful, designers need tools that can easily interoperate, and support hierarchical design methods and rapid prototyping.
These tools must also take into account critical, deep submicron effects, such as signal integrity,
voltage (IR) drop, electromigration and substrate noise.
.P
Cadence has had exclusive distribution rights for Fire & Ice(R) QX, Simplex's leading 3D interconnect modeling and extraction tool,
since January 2002.
A roadmap for the incorporation of Simplex's other products is currently under development.
This will include:
VoltageStorm(TM) SoC, Simplex's flagship power grid verification product,
ElectronStorm(TM), for electromigration analysis,
SignalStorm(TM) SoC, for digital signal integrity delay calculation and library characterization, and
SubstrateStorm(TM), for substrate noise analysis.
.H 1 "WHAT SIMPLEX DOCUMENTATION SAYS"
.H 2 "The Simplex Solution"
The Simplex Solution is a fully integrated suite of products that perform extremely fast and
accurate post-layout interconnect parasitic extraction, followed by interconnect analysis,
to validate full-chip timing, power distribution, clock performance, and design reliability.
It also includes substrate extraction and analysis.
.H 2 "Extraction"
By providing full-chip capacity combined with three-dimensional accuracy and fast turnaround times,
Simplex' extraction product, Fire & Ice QX, handles interconnect parasitic extraction for even
the largest, most complex designs.
.H 2 "Interconnect Analysis"
As process technologies shrink below 0.25 microns, interconnect plays a more dominant role in
determining a design's performance. Many aspects can limit the potential of a design or even
cause it to fail prematurely. Simplex offers four analysis products to help design teams
validate the interconnect of their designs before tapeout: VoltageStorm SoC, ClockStorm,
ElectronStorm, and SI Report. These products interface to the transistor-level extraction
capability of Fire & Ice QX.
.br
An other tool, SignalStorm, enables you to accurate capture instance and interconnect delay and the noise
information that are used in timing verification.
.H 2 "Substrate Extraction and Analysis"
SubstrateStorm enables you to analyse noise in advanced wireless, radio-frequency(RF),
analog, and mixedsignal SoC designs by building a three-dimensional RC model of the substrate.
.P
SubstrateStorm provides highly accurate substrate modeling across a wide range of technologies,
including CMOS, BiCMOS, SiGe, and pure bipolar processes on lightly doped, heavily doped, or SOI wafers.
It can create models for the most advanced silicon processes, including complex structures like triple wells,
buried layers, multiple well depths, and trenches. To produce a substrate model, SubstrateStorm automatically
generates a three-dimensional mesh constructed of high-density regions separated by coarse meshes,
efficiently focusing accuracy in areas of greatest interest.
.P
SubstrateStorm's easy-to-use interface and full-color visualization provides a powerful tool for
analyzing substrate crosstalk and for interactively exploring solutions to noise problems.
SubstrateStorm can be used throughout the design cycle, from floorplanning to cell design
to final post-layout verification. It produces compact netlists suitable for the most common
electrical simulators, and supports GDSII and SPICE formats.
SubstrateStorm's viewer interface is available as SubstrateStormCDS, a plug-in to the editor
of Cadence Design Systems.
.H 1 "AVAILABLE SUBSTRATESTORM DISTRIBUTIONS"
On our disks i found 3 Simplex SubstrateStorm distributions.
.nf
Note: "/opt/cad/cadence" is linked to "/u/50/50/cadence"
Note: "/u/50/50" is linked to "/u/46/46/warga/50"
Thus: "/opt/cad/cadence" is equal to "/u/46/46/warga/50/cadence"
.P
1) /opt/cad/cadence/simplex (Release 3.5.0 (Limited Release 3.5 (LR3.5)), May 2002 (3th patch))
2) /opt/cad/cadence/SNA3.1  (Release 3.5.1, April 2003)
3) /opt/cad/cadence/SNA3.2  (Release 3.6)
(see files: /opt/cad/cadence/.../SubstrateStorm/VERSION)
.P
2,3) distribution includes rules files and skill hooks for Assura 3.0.x
.P
Supported Cadence Versions:
1) 4.4.2, 4.4.3, 4.4.5, 4.4.6
2) 4.4.6, 5.0.0
3a) 4.4.6, 5.0.0
3b) 4.4.6, 4.4.6-64b, 5.0.0, 5.0.0-64b
(see directories: /opt/cad/cadence/.../SubstrateStorm/cds/context)
.P
Supported Platforms:
1) hpux (HP-UX xx.11*), sun5 (SunOS 5.6, 5.7*, 5.8*)
(install doc: HP-UX 11.0 or later; Sun Solaris 6 or later)
2) hppa (bin/hpux xx.11*), sun4v (bin/sun5, SunOS 5.7*, 5.8*)
3a) tools.lnx86 (Linux 2.xx)
3b) tools.sun4v (SunOS 5.6, 5.7*, 5.8*)
(see files: /opt/cad/cadence/.../SubstrateStorm/bin/.SCstarter)
.P
*) Possible usage of 64-bit program versions (calibre2sna, snasnd, snapp, snaxtr, snarcr).
3a) Programs snaedt, snatct and all 64-bit program versions are not supported under Linux.
.P
The SubstrateStorm distribution contains the following sna tools:
- calibre2sna = interface for Calibre tool users (LVS)
	    (generates a SAV or a backannotated functional netlist)
- snacdsd = cadence deamon (? simplex only)
- snaedt  = editor (graphics user interface (GUI))
- snapp   = computes the perturbing path (path of least impedance
	    between two selected points at the surface)
- snarcr  = RC reduction tool (generates SPICE netlist)
- snasnd  = computes a surface noise distribution of 3D substrate model
- snatct  = technology compiler tool (generates SCtechnology file)
- snaxtr  = extractor (3D substrate calculation)
.P
See: /opt/cad/cadence/SNA3.1/doc/substormcdsman/substormcdsman.pdf

.H 1 "AVAILABLE CADENCE DISTRIBUTIONS"
.nf
1) /opt/cad/cadence/2001 (Version 4.4.5 ), for: hppa, sun4v
2) /opt/cad/cadence/2002 (Version 4.4.6 ), for: hppa, sun4v       (simplex)
3) /opt/cad/cadence/2003 (Version 5.0.0 ), for: hppa, sun4v, lnx86 (SNA3.1)
4) /opt/cad/cadence/2004 (Version 5.0.33), for: hppa, sun4v, lnx86 (SNA3.2)
5) /opt/cad/cadence/2005 (Version 5.1.41), for: hppa, sun4v, lnx86
.fi
.P
I have tried to use version 4.4.6, because Philips (and qiang) was using it with
the SubstrateStorm ``simplex'' version (Release 3.5).
This Cadence version does not yet support a Linux platform.
For the usage of the ``simplex'' version it was needed to use version 4.4.6,
because the ``simplex'' version does not support a cadence 5.0.0 context.
Another problem, to use version 4.4.6, was, that the ``icfb'' program cannot
use the 24-bit TrueColor Visual of my workstation (use xdpyinfo, to see the
available visuals).
Qiang had also this same problem before.
To fix this problem, Antoon has added an 8-plane PseudoColor visual setup to my workstation.
To use this 8-bit display together with my 24-bit display, i must type <Ctrl><Alt><F8>.
I get then a login dialog window for remote host cobalt (Sun Solaris 9).
To switch back to my (default) 24-bit display, i must type <Ctrl><Alt><F7>.
.P
After all, i found out, that the ``simplex'' version can also work with cadence version 5.0.
The only thing you must do, is to install the 5.0.0 context in the ``simplex'' tree.
Take the one from the SNA3.1 distribution.
However, there is no reason to use the ``simplex'' version anymore.
Because you can also directly use the SNA3.1 version.
Use the SNA3.2 version, when you want to use SubstrateStorm on a Linux platform.
The SNA3.2 version, however, does not support HP-UX anymore.
Another problem can be, that tools snaedt and snatct are not supported yet on Linux.
There is also a problem using SNA3.2.
Maybe SNA3.2 is not correct installed!
.H 2 Conclusion
For the usage of the Philips CAD environment (inclusive ``simplex'') it is maybe needed
to use the old cadence version 4.4.6. Because i am not sure that for all tools a cadence
version 5.0.0 context is available. But i have not tried it, because i do not really want
to use the Philips environment.
.P
To forcome a lot of problems, you must at least use cadence version 5.0 or later.
These versions support also Linux platforms (if needed) and 24-bit visuals.
To use the full tool set of SubstrateStorm on HP or Sun platform, use the SNA3.1 distribution.
I have used host "skrins" (HP) and host "adonis" (Sun).
.P
However, to use SubstrateStorm, you don't really need to use the Cadence Design System.
I found out (see snaedt tutorial), that the snaedt GUI is also a good interface for the
usage of SubstrateStorm.
.H 1 "Eelco TEST CELL"
The test cell from Eelco Schrik is saved in directory:
.fS I
~simon/eelco/substratestorm_vs_space
.fE
.nf
See e-mail from Eelco Schrik (d.d. Thu, 22 Sep 2005 16:06).
It contains the tar-file of the project directory and some info.
The layout example (test cell) uses a lambda of 0.03125 um.
With that value is the transistor gate-length exact 250 nm.
.fi
For that gate-length are usable transistor-models available (see
file "spice3f3_new.lib" in subdirectory "proc").
.P
The README file (in the project) contains some extra info, how the standard SubstrateStorm doping-profile
can be adapted, thus that it can be compared with the SPACE substrate calculations (see appendix A).
.br
See: /opt/cad/cadence/SNA3.1/doc/substormtcttut/substormtcttut.pdf
.P
The cell "oscil_nwellguard" is a ring oscillator, which contains 9 invertor sub-cells ("inv").
See the figure below:
.F+
.PSPIC "an0506/fig1.ps" 5.5i
.F-
The cell has 6 terminals. Two power connections ("vss" and "vdd").
The "vss" interconnect is a closed metal1 ring and has a substrate contact.
The "vdd" interconnect has n-well contacts (in each "inv" one).
The "sens" terminal is also a substrate contact.
The n-well guard-ring has a contact with terminal "vdd2".
The total layout is using only 7 different masks (no metal2 is used):
.TS
allbox;
l l.
cmf	metal first
cpg	polysilicon
caa	active area
cwn	n-well region
csn	n-channel implant
cca	contact cmf - caa/cwn/@sub
ccp	contact cmf - cpg
.TE
The "inv" sub-cell contains 4 terminals ("vdd", "vss", "in" and "out").
The "caa" mask is only used in "inv" (2x) for the 2 transistors.
The "ccp" mask is also only used in the sub-cell (1x), this,
to connect the output (cmf) to the input gates (cpg) of a next invertor.
The "csn" mask is used together with the "caa" mask, to define
n-channel implant area for a nmos transistor in the p-substrate.
The mask "csn" is also used to define a n-well contact.
The following figures show a slice for the invertor and
the layout.
The 3th figure shows the total layout of cell "oscil_nwellguard".
.F+
.PSPIC "an0506/fig2.ps" 5i
.F-
.F+
.PSPIC "an0506/inv.ps" 5i
.F-
.F+
.PSPIC "an0506/osc.ps" 5i
.F-
.SK
The
.P= space3d
extractor has the possibility to do a BEM/FEM method substrate res extraction.
With the combined BEM/FEM method, the total substrate area is taken into account.
Because this method sets all masks distributed, you get a maximum number of substrate terminals.
To use this method, you must define wafer configurations and a bem_depth in the technology file.
The width of the substrate bounding box around the cell bounding box must also be chosen.
However, the
.P= space3d
extractor
is default using a substrate extension of be_window width.
Note that by the BEM/FEM method, you get not only horizontal be_window width strips,
but also vertical be_window width tile splitting.
Thus in total 711 substrate tiles/terminals are used with this test cell.
.br
See the figure below.
.F+
.PSPIC "an0506/ext4.ps" 5.5i
.F-
The used wafer configurations for the BEM/FEM method are specified in the technology file.
Note that the conductivity values and the number of layers is hypothetical chosen.
Because a ``pdiff'' layer is missing for the process the test cell is using,
i must use a different definition method for the contacts.
Thus, the pcontact cross-section is specified with the dimension of the via mask "cca".
This is not the case for the ncontact.
The ncontact cross-section is specified with the dimension of the "csn" mask.
.br
See the figure below.
.F+
.PSPIC "an0506/fig4.ps" 5i 1.7i
.F-
.SK
Important is to note, that the "cwn" and "caa" conductors are taken over by the FEM
wafer cross-section.
However, this is not done for the transistor gates (conductor "cpg" must be used).
Therefor, conductor "cpg" may not be specified as 'p' or 'n' type conductor.
.P
Note that these wafer configurations can be compared with the eight doping profiles
cross-sections for a CMOS process which SubstrateStorm is using.
.P
When we look to the numbers, we see that there are 711 subterms (substrate terminals / tiles).
This number is the sum of all not nwell tiles (pdefault, pcontact, nsd, nchannel) and
all nwell tiles (ndefault, ncontact, psd, pchannel).
.TS
box;
c s | r s
l r | l r.
not nwell	nwell (guard ring)
=
pdefault	252	ndefault	121 (14)
pcontact	6	ncontact	48   (3)
nsd	136	psd	126   (0)
nchannel	13	pchannel	9   (0)
_
total	407	total	304 (17)
.TE
When you look more accurate to the above numbers, you see that the nchannel and pchannel numbers are almost exact
the number of gates of the nenh/penh transistors (18).
There are 4 nchannel tiles more, because the top 4 nenh transistors are subdivided by the horizontal be-window line.
However, the source/drain area's (nsd / psd) are subdivided a lot more than needed.
That is, because contacts to first metal are laying on these area's.
Maybe, we can optimize this in the future.
.P
When all substrate nodes (subnodes) are eliminated (elim_sub_term_node=on), then there are 20 subnodes left.
This is of the 9 nenh transistors (9 source pins and 9 bulk pins) and 2 terminals (vss and sens) in the non nwell area.
The total number of nodes (79) is the sum of the total number of transistor pins (18 * 4) and the number of terminals (6)
and the SUBSTR node.
.P
When parameter elim_sub_term_node=off, then the non nwell area is default eliminated (because it is distributed and
is connected to the BEM substrate area).
Only the nwell substrate terminals (304) are not eliminated.
Thus, we get 324 subnodes (20 + 304) and 383 nodes (79 + 304).
.P
When we look to the utime (user time) numbers, we see that substrate node elimination cost a lot of time (about 40% more).
This is not the case for technology file number 5, because by that example there is p-fem under the nwell area.
Thus, all substrate terminals are eliminated by default.
Note that the user time numbers are not so accurate, because these numbers are also depending of other system activities.
But they are more accurate than the real time used (which is depending of the system load).
.H 1 "SNAXTR USAGE"
Typical command line:
.fS I
% snaxtr -O -w dirpath techname cellname
.fE
.VL 10
.LI "-G"
compute only the surface grid (for visualization)
.LI "-h"
help information
.LI "-O"
for reading option file <dirpath>/SClibrary.opt
.LI "-t ext"
using this technology file extension,
.br
composing: <site>/<techname>/SCtechnology<.ext>
.br
(default: no extension or specified in SClibrary.opt)
.LI "-S factor"
geometry scaling factor (default: 1.0 or specified in SClibrary.opt)
.LI "-w dirpath"
working directory path of cell library
.LI "-x -Default"
using this parameter file extension
(no <site>/<techname>/SCparameters.xtr will be loaded)
.LE
.P
Note: <site> is specified with environment variable SUBSTRATESTORMSITE
.P
The "SClibrary.opt" contains SubstrateStorm setup information.
Using the Cadence environment,
this file is copied from ".sna" to each ".sna/cell" directory.
Thus, each cell can have different setup parameters.
The setup parameters can be changed in the option menu dialog windows.
When you start SubstrateStorm (doing Verify->SubstrateStorm) from Virtuoso ``extracted'' window,
the system is reading this file.
When this file is incorrect, the SubstrateStorm interface cannot be started.
.nr Ej 0
.H 1 "SNARCR USAGE"
Typical command line:
.fS I
% snarcr -n -p -f 1.0 -mode RC -z 0 -w dirpath cellname
.fE
.VL 10
.LI "-h"
help information
.LI "-n"
use node names in spice netlist
.LI "-p"
use geometry node names
.LI "-f 1.0"
reduction frequency (GHz) for RC mode
.LI "-mode RC"
reduction mode (RC / R / None)
.LI "-z 0"
report level (0 = no report, 1 = default, 2 = full)
.LI "-w dirpath"
working directory path of cell library
.LE
.nr Ej 1
.H 1 "SNAEDT USAGE"
First,
start with the SubstrateStormEDT Tutorial (for example,
go to directory "/opt/cad/cadence/simplex/docs/3.5.0"
or "/opt/cad/cadence/SNA3.1/doc/substormedttut" and
read the ".pdf" file).
.P
To use
.P= snaedt ,
login on a Sun or HP platform and do first (for SNA3.1) the following source command:
.fS I
% source /opt/cad/cadence/2003/sourceme.sna
.fE
Read the EDT Tutorial, section "Getting Started", create a working directory etc.
and go to directory "~/SubstrateStorm/examples/edt" and start
.P= snaedt .
To use your own SubstrateStorm site data (maybe you want to change it),
do the following setenv:
.fS I
% setenv SUBSTRATESTORMSITE ~/SubstrateStorm/examples/site
.fE
After you have done the "Reset Database" command for library "adc8",
you see which site data you are using.
Each library contains a SubstrateStorm library options file "SClibrary.opt",
which defines the technology parameters.
In this case you are using "cmos1" (this is a sub-directory of the site directory)
and you are using technology extension "ld.10".
This result in the load of "cmos1/SCparameters.edt" and the
load of technology file "cmos1/SCtechnology.ld.10".
The extension means, that a lightly doped substrate profile is used with 10 subdivions
(see SubstrateStormTCT).
Each library contains cells, which are stored in sub-directories with there cell name.
And each cell sub-directory contains views.
For example the AbstractView is stored in text file "SCabstractview" and the
Layout view is stored in the GDSII file "layout.gds".
.P
See the EDT Tutorial, use the library browser and load the layout of the "comp" cell.
The
.P= snaedt
program gives nicely info about what it is doing and tells something about the GDSII file.
Note that you can use NELSIS tool
.P= cga
to show the contents of the GDSII file.
The GDSII file contains libname "comparateur" and only one structure with name "Cell0".
The following units are defined in the GDSII file:
(a) an user unit is equal to 0.5 db_unit, and
(b) a db_unit is 500 nm (5E-07).
The GDSII layer numbers are mapped to Cadence
.P= snaedt
layer names, see file "cmos1/gds.map".
For the used layer colors and fill styles, see file "cmos1/SCparameters.edt".
Note that terminal names are defined with text elements (only for layer 50 and 52).
The used boundary element layer numbers are 44-47, 50-52, 55-56, 58-59 and 61.
.P
The
.P= snaedt
program works with layout coordinates which are specified in microns.
It uses the second value of the GDSII file UNITS record to scale the coordinates correctly.
When reading, the
.P= snaedt
program shows the contents of the UNITS record, but uses too less digits to do this correctly.
See the "snaedt.log" file for the correct values.
It tells also how much nano meter one db_unit is.
Thus, the user unit for the
.P= snaedt
program is always one micron meter.
Also the coordinate values in the generated "SCabstractview" file are in microns (and can have
three digits after the decimal point).
The following figure shows the control flow of the
.P= snaedt
program.
.F+
.PSPIC "an0506/edt.ps" 4.5i
.F-
.P
The file "snaedt.lib" defines which cell libraries there are and what there directory path is.
Each library contains a "SClibrary.opt" file, which defines the technology used and some
other important parameters.
Each cell directory contains a number of views.
The Layout View is stored in the "layout.gds" GDSII binary file.
The generated Substrate Abstract View (SAV) is stored in the "SCabstractview" text file.
The (optional) Ideal Netlist View is stored in the "netlist.ini" SPICE-like text file
(not shown in the above figure).
This file contains geometry (shape) information and
can be used for SAV access port generation,
and is also used to get external netlist node names in the generated substrate netlist.
.P
There are more files in the cell directory, but that are not views,
because the
.P= snaedt
program can not display geometry information of them.
However, the "mesh.sna" file is used to display the generated surface mesh.
The files "grid.sna" and "model.sna" are generated by the
.P= snaxtr
(substrate extractor) program pass.
The file "substrate.subckt" is generated by the
.P= snarcr
(rc reduction) program pass.
This last file,
a HSPICE subcircuit netlist,
is the extraction end result.
.P
The three important
.P= snaedt
site files are listed in appendix E.
A
.P= snaedt
example with input and result files is given in appendix F.
.H 1 "SNATCT USAGE"
Program
.P= snatct
is the SubstrateStorm Technology Characterization Tool.
It creates the technology file, which is used by the
.P= snaxtr
program.
It calculates the vertical and horizontal resistivity for each substrate subdivision.
To calculate this, it uses the doping profile information for that region cross-section.
For each region cross-section used, there must be a doping profile data file.
For a standard CMOS process are there normally two regions, the "default" (p) region and the "nwell" region.
Each region has normally four cross-sections (default, contact, source/drain, channel).
See content of a technology file (see appendix C).
.P
To start the
.P= snatct
program,
login on a Sun or HP platform and do first (for SNA3.1) the following source command:
.fS I
% source /opt/cad/cadence/2003/sourceme.sna
.fE
Note that you can also use the SNA3.2 version, but there are some problems.
First, you must set the environment variable LM_LICENSE_FILE and second, that version has no
vertical scrollbar in the log panel.
.P
Read the TCT Tutorial (SNA3.2/doc/substormtcttut/substormtcttut.pdf)
or the TCT Manual (SNA3.2/doc/substormtctman/substormtctman.pdf), version 3.6 of Nov 2003.
You can also read the SNA3.1 version of April 2003 (there is almost nothing changed).
See the color pictures on page 14, 15 and 16 in the Tutorial.
Note that the names of the doping profiles files "pfield.txt" and "nfield.txt" are incorrect.
I found the names "pdefault.txt" and "ndefault.txt".
The text is a little bit misleading, a doping profile is not for a region, but for a region cross-section.
Note that the cross-sections start on different hights (or depth).
See page 16, the source/drain area's are not modeled as a part of the substrate level,
but are on the device level.
Thus, the profile of that cross-section begins on a higher depth (z-position).
.P
See also the picture on page 30 for the vertical substrate discretization.
Note that the bottom of the substrate is incorrectly specified in the picture.
The top of the substrate does not begin on z=0, but on the minimum-z (minZ) found
in the given profile data.
The depth of the substrate (the bottom) ends on the specified backside depth.
If you press the discretization button, the
.P= snatct
program choices the best position for the number of subdivisions you want.
The number of subdivisions (lines) above critical depth, you choice, is inclusive the line
on critical depth.
The number of subdivisions (lines) below critical depth, you choice, is inclusive the line
on the backside depth.
.P
Thus, for Figure 3-18 on page 31 the chosen critical depth is 12.34 \(*mm and the backside depth
is 375 \(*mm.
In this case is chosen for 7 divisions above critical and 3 below critical depth.
The
.P= snatct
program choices also the best position for the critical depth.
The depth, after which the doping concentration is almost constant.
You can choice another critical depth position by moving the critical depth line.
But in that case, you cannot get it easy on a specific position.
You can also delete the critical depth line and add a new one on a specific position.
For the calculations, the critical depth line is just a line like the others.
Note that there are always two division lines you cannot choice, the minZ line and backside depth line.
When you click on the discretization button again, then all lines are moved to their best position.
It is not possible to fix one line on a specific position.
.P
See also page 14 of the TCT Manual.
A discrete vertical mesh involves a loss of the impedance accurary.
The subdivision lines are placed at boundaries in the profile,
where the concentration greatly changes.
More subdivisions result in a more accurate model,
but costs more memory and processing time.
It is a good practice to use more subdivisions above critical depth.
However, you may not use more than a maximum of 20 subdivisions.
.P
For each profile subdivision a vertical and horizontal resistivity is calculated.
See also Figure 1-7 on page 15 of the TCT Manual.
This is done for each cross-section out of the net carrier doping concentration
(N-net >= 1e11 cm-3 for n-type silicon).
.ta 1c 7c
.P
	N-net = (N-donor - N-acceptor)	[cm-3]
.P
The subdivision resistivity is:
.P
	\(*s = CondFactor * N-net-subdiv	[S/cm]
.br
	\(*r = 1 / \(*s	[\(*W.cm]
.P
The subdivision res-horizontal and res-vertical are:
.P
	dz = subdivision thickness	[\(*mm]
.P
	res-horizontal = \(*r * 1e4 / dz	[\(*W]
.br
	res-vertical = \(*r * 1e4 * dz	[\(*W.\(*mm^2]
.P
The subdivision cell impedances by surface area (dx*dy [\(*mm^2]) are:
.P
	ratio = dy / dx
.br
	R-x = res-horizontal / ratio	[\(*W]
.br
	R-y = res-horizontal * ratio	[\(*W]
.br
	R-z = res-vertical / (dx*dy)	[\(*W]
.P
.F+
.PSPIC "an0506/cell.ps" 3i 2i
.F-
.SK
The profile data can be specified in four different formats
(see page 90 of the TCT Manual).
The first column contains always the depth (in \(*mm) from the substrate surface (>= 0).
The second column in a two columns formatted file can contain the
net carrier concentration (in cm-3) or the resistivity value (in \(*W.\(*mm).
A net carrier concentration must be in the range 1e11 to 1e22.
A resistivity value may not be greater than 1e9 \(*W.\(*mm.
A negative sign is used for p-type cross-sections,
when the holes (acceptors) are the majority carriers.
.TS
allbox;
r r r.
\(*s [S/m]	\(*r [\(*W.cm]	\(*r [k\(*W.\(*mm]
_
10	10.0	100
20	5.0	50
40	2.5	25
50	2.0	20
100	1.0	10
1000	0.1	1
.TE
.P
The profile data can also be specified in a tree columns format.
The second column specifies the electron concentration (n) and the third column the hole concentration (p).
The product of both concentrations must be in the range 1.94e+20 and 2.41e+20.
However, typical this product (cmos/lightlyDoped) is in the range 2.090546e+20 and 2.090817e+20.
When this product is out of range, you get the message "May be this file is an Acceptor and Donors
format?" and sometimes "Value error. Profile ... will not be loaded.".
However, i don't know, how to specify the acceptor/donor (4th) format.
See appendix B for the cmos/lightlyDoped profiles and the discretization.
.P
Appendix B shows the
.P= snatct
program window after the load of the "step8.tct" session file.
The window displays the 8 profile curves (see also the small Legend window).
You see also 10 red subdivision lines above (and on) critical depth.
All profiles end after a certain depth to the same deep-p substrate doping concentration.
The n-type profiles must change into p-type.
This transition point is calculated and is around 6.5 \(*mm.
You see, that the profiles of the different cross-sections have a different critical depth.
For the default region profiles, this critical depth is about 6.25 \(*mm.
For the nwell region profiles, this critical depth is around 10.7 and 12.34 \(*mm.
The program must make a choice and takes 12.34 \(*mm as critical depth position.
For sampling of the profile data, a sampling step is calculated with formula:
.fS I
step = (critical_depth - minZ) / 2000
.fE
For the n-type profiles, you see, that a transition is found.
The average of the calculated transition depths is taken.
When you choice for 2 subdivisions above critical depth, you see, that 3 subdivisions are set.
This, because the program wants to have a subdivision line on the transition depth.
Click also on the discretization button, because there was a change in the settings.
By the choice 10/3 you see, that the subdivision lines are concentrated around positions
where the profile data most change.
Note that the subdivision lines concentration (positions) depends on the "compress" setting.
By a value setting greater than 1, the lines are more concentrated towards the substrate surface.
.P
Looking to the profile data, you see, that each profile begins on a different depth.
For the minZ, the begin depth of profile "nchan.txt" is chosen.
The picture at the end of the appendix shows the different depths.
The 1st, 2nd and 3th subdivision lines are shown dashed.
The nsd and ndefault cross-sections begin below the 2nd subdivision line at 0.744 \(*mm.
The psd cross-section even below the 3th subdivision at 0.945 \(*mm.
This implies, that there are no resistance cell cubes above the line for these cross-sections.
.P
Appendix C shows the generated technology file.
Note that some lines are joined to make the listing a little shorter.
The technology file contents is easy to read.
The 10/3 subdivision choice (13 lines) result in 12 subdivision thickness values.
Before the vertical and horizontal resistivity values for each region cross-section are specified,
you find a list of calculated junction capacitances parameters.
At the end of the technology file you can find the interface and gate parameters.
And the last ascii character is used as a checksum.
.P
Note that,
before you can generate a technology file, some parameters must be set:
.BL
.LI
At least one valid OnField area capacitance value for an interconnect layer.
.LI
At least one valid p-/n-Contact resistance value for an interconnect layer.
.LI
At least one valid area capacitance value for a gate layer.
.LE
.P
Appendix D shows a list of values which
.P= snatct
is using.
The mobility can be calculated with the following formula:
.P
    mobility = 1 / ( 1.6e-19 * resistivity * concentration)
.P
Appendix H to J show the
.P= space3d
substrate extraction results for an one layer substrate of 10 S/m.
The test cell contains two substrate contacts, called "a" and "b".
When the case is symmetric, both resistors from "a" and "b" to the SUBSTR node are equal.
.br
Appendix H shows what happens, when you are using more surface mesh elements per contact area.
How more elements you are using how more accurate the extraction result
(the resistivity values convergent to a certain value).
You see also, that be_mode "0g" (pwc Galerkin) is more accurate than "0c" (pwc collocation).
.br
Appendix I shows what happens, when you change the distance between the two contacts.
The value Rab is almost linear and Rsub convergent to a value of 44 k\(*W.
.br
Appendix J shows what happens, when you change the size of the two contacts
(by a constant distance of 30 \(*mm and constant number of mesh elements).
.H 1 "SUBSTRATESTORM RESISTANCE MEASUREMENTS"
The SubstrateStorm extractor uses the Finite Difference Method (FDM) to calculate the substrate resistances.
It uses a sparse orthogonal (in most cases) surface mesh and uses in the depth substrate subdivisions.
Thus you get substrate cubes and each cube has its own specific substrate resistance values.
And for each cube, six substrate resistances are calculated (see also the previous
.P= snatct
section).
Four resistances are horizontal and two resistances are vertical connected to the central main node of the cube.
You can inspect the resulting substrate resistance netlist, when you choice "None" as reduction mode for the
.P= snarcr
pass.
Note that you default choice for "RC" reduction mode, which reduces the netlist.
The maximum operation frequency is only important when there are capacitances.
Even, when there are no capacitances, you don't choice the "R" reduction mode.
Because this can give an unexpected result.
.P
Depending on the package definition (see the
.P= snaedt
option menu), you can generate three different substrate netlists.
The BACKSIDE connection can be defined: (1) None, (2) Grounded and (3) Connected.
When you choice for "None", you don't have a BACKSIDE connection.
When you choice for "Grounded", you have a BACKSIDE connection with default node id 0.
When you choice for "Connected", you have a BACKSIDE connection with default node name BACKSIDE.
In practice, netlists (2) and (3) are equal, but have a different BACKSIDE node name.
For two external nodes you get the following reduced networks (see figure).
.P
.F+
.PSPIC "an0506/fig5.ps" 4i
.F-
When you reduce network (2) further to network (1), then the resulting resistor Rv2 is not equal to Rv1.
This, because in the unconnected case, a number of vertical resistors is unconnected.
See the SubstrateStorm resistivity results in appendix K.
.P
The figure in appendix K shows that above a certain substrate thickness (\(>= 5 \(*mm) the results
of Rv1 and Rv2 are equal.
When more subdivisions are used, the result is less increasing by a greater substrate thickness.
But we are not using a constant thickness for each subdivision.
When we use a constant subdivision thickness (see appendix L), the Rv value is almost constant for
a substrate thickness of \(>= 5 \(*mm.
Using a smaller surface bounding box result in a lower Rv value.
However, this value is not low enough, when we compare it with the
.P= space3d
results (see appendix H).
We expect a Rv value of about 81 k\(*W.
Thus, the surface grid must be refined a little bit more.
In appendix M are the calculated results of some surface grid refined cases.
When there are used more 1 \(*mm width surface grid elements the results are better.
Case 4-3 and case 4-4 give almost the same result.
Thus, more bounding box surface elements around the contacts is not useful.
And now we come in the direction of the expected 81 k\(*W value for Rv.
.H 2 SubstrateStorm Conclusions
.BL
.LI
It seems to be correct what Eelco Schrik (see appendix A) already said, that
all carrier-concentrations in the SubstrateStorm example profiles
have to be multiplied by 2, to get results which can be compared with
.P= space3d
results.
This is also true, when using resistivity value data for the profiles.
.LI
The resulting SubstrateStorm SPICE netlist can have a BACKSIDE node or not.
To be sure that both netlists are equivalent, you need to use a number of profile subdivisions.
At least 5, but more than 10 is not needed.
.LI
To get good accuracy, the surface mesh around contacts must also be refined with the size
of the contacts (see appendix M) till at least a bounding box of 3 times the contact size.
.LI
I have used 5 to 10 profile subdivisions, each with the same thickness of the contact size.
For a constant substrate profile resistivity this gives good results.
This are the subdivisions you must specify above critical depth.
Below critical depth you can add one subdivision to extend the substrate to each
backside depth you want to use.
This last subdivision has almost no effect on the calculated Rv results.
However, when the netlist contains a BACKSIDE node, the resistors to the BACKSIDE node
and between the contacts
depends on the chosen backside depth.
For example, the last listing in appendix F shows the resulting SPICE netlist for a chosen BACKSIDE depth of 375 \(*mm.
.LE
.H 1 GLOSSARY
.nf
.ta 8
BEM	Boundary Element Mesh / Method
BiCMOS	Bipolar CMOS technology
CDS	Cadence Design Systems
CIW	Command Interpreter Window
CMOS	Complementary Metal Oxide Semiconductor
DFII	Design Framework II (CDS)
DIVA	Layout Verification/Extraction tool (CDS)
DSM	Deep-Submicron
EPI	Epitaxial layer (CMOS process)
FDM	Finite Difference Method
FEM	Finite Element Mesh / Method
GUI	Graphics User Interface
HD	Heavily Doped substrate
icfb	IC front-to-back (CDS start-up program)
ICs	Integrated Circuits
LD	Lightly Doped substrate
LPP	Layer Purpose Pair
LSW	Layer Selection Window
LVS	Layout Versus Schematic
Na	acceptor (hole) doping concentration
Nd	donor (electron) doping concentration
NMOS	N-type MOS transistor (in p-well region)
NSD	NMOS source/drain section ( ,,  ,, )
PGS	Power Grid Sign-off
PMOS	P-type MOS transistor (in n-well region)
PSD	PMOS source/drain section ( ,,  ,, )
RCR	RC Reduction (SNA)
SAV	Substrate Abstract View
SiGe	Silicon-Germanium technology
SiO2	Silicon dioxide
SNA	Substrate Network Analysis
SND	Surface Noise Distribution
SoC	System-on-a-Chip
SOI	Silicon-On-Insulator wafers
STI	Shallow-trench (oxide) isolation
TCT	Technology Characterization Tool (SNA)
TOX	Gate oxide thickness
UDSM	Ultra DSM
XTR	extractor (SNA)
.H 1 "APPENDICES"
.SP
.nr Hu 2
.HU "APPENDIX A -- Eelco README File"
.nf
Cell "oscil_nwellguard" is the basic ring oscillator cell used in the thesis
by Eelco Schrik. The used technology in the thesis is a simple modification
of a SubstrateStorm example doping profile.

The SubstrateStorm example doping profiles can be found in the examples
directory provided with the SubstrateStorm installation:
.fS I
$SUBSTRATESTORMHOME/etc/examples/tct/cmos/lightlyDoped/profiles/
.fE
The files in this directory contain samples of the doping profile in the
following way:
.fS I
<depth>      <n-type concentration>    <p-type concentration>
                (Electrons)               (Holes)
.fE
Concentrations are in cm^-3. Note that 'depth' does not mean 'thickness' and
that the first depth sample may not be at depth zero, because the oxide may
also be taken into account, which typically contains no charge-carriers, but
does have a certain thickness itself. For example, the first 3 lines
from 'pdefault.txt' are as follows:
.fS I
0.70420003         4727.1001               4.4227001e+16
0.72359997         3730.5                  5.6043001e+16
0.74910003         2911.5                  7.1808001e+16
.fE
The modifications used in the thesis by Eelco Schrik are straightforward:
.DL
.LI
all p-type carrier-concentrations in the SubstrateStorm example profiles
have to be multiplied by 2
.LI
all n-type carrier-concentrations in the SubstrateStorm have to be
multiplied by 0.5.
.LE
.P
NOTE: Nd * Na = Ni * Ni   (Ni is constant?)
.P
NOTE: SubstrateStorm uses a FDM method. Each mesh 3D-box has a center node,
which is connected with 6 resistors. Each going to a center node of a face
of the box. Pairs have an equal value. Half of the resistors is eliminated
(or removed). That is maybe the reason of the factor 2.
.SK
.HU "APPENDIX B -- SNATCT Discretization of Profiles"
.F+
.PSPIC "an0506/tct.ps" 6i
.F-
.nf
.S 9
.ft C
see SCtechno.log
================
Critical Depth for Cross-Section default = 6.25
Critical Depth for Cross-Section contact = 6.25
Critical Depth for Cross-Section source/drain = 6.25
Critical Depth for Cross-Section channel = 6.25
Region "default" Critical Depth = 6.25
In region : "default" Nr Profiles = 4

Critical Depth for Cross-Section default = 10.7
Critical Depth for Cross-Section contact = 10.7
Critical Depth for Cross-Section source/drain = 12.34
Critical Depth for Cross-Section channel = 12.34
Region "nwell" Critical Depth = 12.34
In region : "nwell" Nr Profiles = 4

Sampling...   Critical Depth = 12.4461  Step = 0.0061
cmos/lightlyDoped/profiles/pdefault.txt minZ = 0.7042
cmos/lightlyDoped/profiles/pcont.txt    minZ = 0.2933
cmos/lightlyDoped/profiles/pchan.txt    minZ = 0.6218
  transition number 1 value = 6.5  ; transition number calc 1 value = 6.49428
cmos/lightlyDoped/profiles/psd.txt      minZ = 1.08
  transition number 1 value = 6.5  ; transition number calc 1 value = 6.49428
cmos/lightlyDoped/profiles/ndefault.txt minZ = 0.7691
  transition number 1 value = 6.5  ; transition number calc 1 value = 6.50036
cmos/lightlyDoped/profiles/ncont.txt    minZ = 0.3946
  transition number 1 value = 6.55 ; transition number calc 1 value = 6.50645
cmos/lightlyDoped/profiles/nchan.txt    minZ = 0.2747
cmos/lightlyDoped/profiles/nsd.txt      minZ = 0.886
transitionAverageCalcSorted[0] = 6.49884

===== choice 2/1: =========================================
Set Number of Subdivisions above critical depth from 2 to 3
Subdivisions Above Critical Depth (1.2440e+01 [um]):
    2.7470e-01 [um]
    6.4988e+00 [um]
    1.2440e+01 [um]
Subdivisions Below Critical Depth:
    3.7500e+02 [um]

===== choice 10/3: ========================================
Subdivisions Above Critical Depth (1.2440e+01 [um]):
    2.7470e-01 [um]
    7.4406e-01 [um]
    9.4468e-01 [um]
    1.2186e+00 [um]
    3.8392e+00 [um]
    5.9203e+00 [um]
    6.3117e+00 [um]
    6.4988e+00 [um]
    6.9172e+00 [um]
    1.2440e+01 [um]
Subdivisions Below Critical Depth:
    3.2604e+01 [um]
    1.0622e+02 [um]
    3.7500e+02 [um]

cmos/lightlyDoped Profiles:
===========================

.ta 2c 5c 8c
pdefault.txt:
=============
0.7042	4727.1	4.4227e+16	<== 1
0.7491	2911.5	7.1808e+16	<== 2
0.8546	2099.4	9.9585e+16	<== n-min/p-max
0.9584	2195.3	9.5236e+16	<== 3
1.2	3300.3	6.3348e+16	<== 4
2.962	427320	4.8926e+14	<== n-max/p-min
3.855	362310	5.7705e+14	<== 5
5.900	307110	6.8076e+14	<== 6
6.300	304270	6.8711e+14	<== 7
6.5	303210	6.8951e+14	<== 8
6.900	301630	6.9312e+14	<== 9
12.34	298670	6.9999e+14	<== 10
15	298670	7.0000e+14	<== 1
350	298670	7.0000e+14	<== 3

pcont.txt:
==========
0.2933	5.7436	3.6400e+19	<== 1
0.3927	5.1362	4.0705e+19	<== n-min/p-max
0.7491	27.437	7.6200e+18	<== 2
0.8546	3948.6	5.2947e+16
0.9584	34063	6.1377e+15	<== 3
1.2	83191	2.5131e+15	<== 4
2.358	547000	3.8221e+14	<== n-max/p-min
2.962	459350	4.5514e+14
3.855	362520	5.7671e+14	<== 5
5.900	307110	6.8076e+14	<== 6
6.300	304270	6.8712e+14	<== 7
6.5	303210	6.8951e+14	<== 8
6.900	301630	6.9312e+14	<== 9
12.34	298670	6.9999e+14	<== 10
15	298670	7.0000e+14	<== 1
350	298670	7.0000e+14	<== 3

nchan.txt:
==========
0.2747	14167	1.4757e+16	<== 1, minZ
0.3673	9191.9	2.2745e+16	<== n-min/p-max
0.7491	39033	5.3562e+15	<== 2
0.9584	53111	3.9365e+15	<== 3
1.2	86655	2.4127e+15	<== 4
2.358	547850	3.8162e+14	<== n-max/p-min
2.962	459500	4.5499e+14
3.855	362520	5.7670e+14	<== 5
5.9	307110	6.8076e+14	<== 6
6.3	304270	6.8712e+14	<== 7
6.5	303210	6.8951e+14	<== 8
6.9	301630	6.9312e+14	<== 9
12.34	298670	6.9999e+14	<== 10
15	298670	7.0000e+14	<== 1
350	298670	7.0000e+14	<== 3

nsd.txt:
========
0.8860	3.1427e+09	6.6525e+10	<== n-max/p-min
0.9205	3.8967e+08	5.3653e+11
0.9584	55523000	3.7654e+12	<== 3
1.2	201320	1.0385e+15	<== 4
1.321	150990	1.3847e+15	<== n-min/p-max
3.855	362520	5.7670e+14	<== 5
5.9	307110	6.8076e+14	<== 6
6.3	304270	6.8712e+14	<== 7
6.5	303210	6.8951e+14	<== 8
6.9	301630	6.9312e+14	<== 9
12.34	298670	6.9999e+14	<== 10
15	298670	7.0000e+14	<== 1
350	298670	7.0000e+14	<== 3
.SK
ndefault.txt:
=============
0.7691	1.6286e+16	12838	<== n-max/p-min
0.9584	9.0982e+15	22979	<== 3
1.2	5.9306e+15	35252	<== 4
3.855	2.1969e+15	95167	<== 5
5.9	3.0953e+13	6754300	<== 6
6.3	2.3769e+11	8.7959e+08	<== 7
6.5	1.4195e+10	1.4729e+10	<== 8 n/p-transition
6.9	75998000	2.751e+12	<== 9
10	312090	6.6990e+14
12.35	299450	6.9817e+14	<== 10
  ...
350	298670	7.0000e+14	<== n-min/p-max

ncont.txt:
==========
0.3946	2.1230e+20	0.98478	<== n-max/p-min
0.7491	2.6934e+17	776.21997	<== 2
0.9584	5.8886e+15	35504	<== 3
1.2	5.2858e+15	39553	<== 4
3.855	2.1978e+15	95124	<== 5
5.9	3.1524e+13	6632100	<== 6
6.3	2.4438e+11	8.5549e+08	<== 7
6.5	1.4612e+10	1.4308e+10	<== 8
  ...
350	298670	7.0000e+14	<== n-min/p-max

pchan.txt:
==========
0.6218	7.0546e+10	2.9636e+09
  ...

psd.txt:
========
1.08	8.8067e+10	2.3740e+09
  ...
.ft
.S
.F+
.PSPIC "an0506/profile.ps" 6i
.F-
.SK
.HU "APPENDIX C -- SubstrateStorm Technology File"
.nf
.S 9
.ft C
# Work-Station: adonis
# User: simon
# Time: 13:20:59
# Date: 4/6/2006

# version number
VERSION 3.1

# define the cross-sections
CROSS_SECTION ( default contact source/drain channel )

# define the regions and their associated cross-sections
REGION (
  default( default contact source/drain channel )
  nwell ( default contact source/drain channel )
)

# specify the layers for interconnect
INTERCONNECT (metal2 poly metal1 )

# specify the layers for gate
GATE ( poly )

DEVICE ( device )

TECHNOLOGY
#number of subdivision
12
#######################################
# thickness (um) for each subdivision #
#######################################
0.467624
0.200688
0.272898
2.58834
2.11445
0.39247
0.18932
0.41575
5.41776
19.924
73.272
269.464
#############################################
#      Junction Capacitances Parameters     #
#############################################
# For each region
# syntax: <regionID> <regionID> (<nr perimeter values> | <nr area values>)
1 1 1   # nwell/nwell
7 13.6154    13.5549    0.36         #area Capacitance Calculated
1 0 6   # nwell/default
2 4402       13923.4    0.34          #perimeter Capacitance Calculated
3 5510.44    19748.6    0.33          #perimeter Capacitance Calculated
4 5561.87    19743.6    0.33          #perimeter Capacitance Calculated
5 5548.67    19744.4    0.33          #perimeter Capacitance Calculated
6 1472.22    3781.97    0.39          #perimeter Capacitance Calculated
7 3215.58    18900.8    0.5           #perimeter Capacitance Calculated
0 0 0   # default/default
###################################################################
# material characteristics for each cross-section per subdivision #
# sorted by region/cross-section according to the region section  #
###################################################################
# default/default
# device characteristics: <resistive(Ohm.um^2)>
# capacitive ( fF/um^2 )
1.0000e-09   -1
# <material type> and <vertical>, <horizontal> resistive
# characteristics per subdivision
   1     5.9367e+01   3.9457e+04
   1     2.0166e+02   4.9762e+03
   1     3.1014e+02   4.1168e+03
   1     2.0362e+05   4.1296e+03
   1     2.2521e+05   5.0251e+04
   1     3.9084e+04   2.5374e+05
   1     1.8743e+04   5.2293e+05
   1     4.0977e+04   2.3707e+05
   1     5.2837e+05   1.8001e+04
   1     1.9400e+06   4.8871e+03
   1     7.1345e+06   1.3289e+03
   1     2.6238e+07   3.6135e+02
# default/contact
# device characteristics: <resistive(Ohm.um^2)>
# capacitive ( fF/um^2 )
1.0000e-09   -1
# <material type> and <vertical>, <horizontal> resistive
# characteristics per subdivision
   1     8.5948e+00   3.7947e+01
   1     5.4366e+02   1.2580e+03
   1     5.3648e+03   6.6643e+04
   1     3.3695e+05   4.1035e+04
   1     2.2522e+05   5.0253e+04
   1     3.9084e+04   2.5374e+05
   1     1.8743e+04   5.2292e+05
   1     4.0977e+04   2.3707e+05
   1     5.2837e+05   1.8001e+04
   1     1.9400e+06   4.8871e+03
   1     7.1345e+06   1.3289e+03
   1     2.6238e+07   3.6135e+02
# default/source/drain
# device characteristics: <resistive(Ohm.um^2)> # capacitive ( fF/um^2 )
1.0000e-09   -1
# <material type> and <vertical>, <horizontal> resistive # characteristics per subdivision
   2     0.0000e+00   0.0000e+00
   1     1.9394e+07   2.9250e+09
   1     1.1938e+06   7.1368e+05
   1     3.4178e+05   4.4600e+04
   1     2.2522e+05   5.0253e+04
   1     3.9084e+04   2.5374e+05
   1     1.8743e+04   5.2292e+05
   1     4.0977e+04   2.3707e+05
   1     5.2837e+05   1.8001e+04
   1     1.9400e+06   4.8871e+03
   1     7.1345e+06   1.3289e+03
   1     2.6238e+07   3.6135e+02
# default/channel
# device characteristics: <resistive(Ohm.um^2)> # capacitive ( fF/um^2 )
1.0000e-09   -1
# <material type> and <vertical>, <horizontal> resistive # characteristics per subdivision
   1     3.3440e+03   1.2063e+04
   1     2.9846e+03   7.3610e+04
   1     6.1448e+03   8.0495e+04
   1     3.3771e+05   4.1358e+04
   1     2.2522e+05   5.0253e+04
   1     3.9084e+04   2.5374e+05
   1     1.8743e+04   5.2292e+05
   1     4.0977e+04   2.3707e+05
   1     5.2837e+05   1.8001e+04
   1     1.9400e+06   4.8871e+03
   1     7.1345e+06   1.3289e+03
   1     2.6238e+07   3.6135e+02
# nwell/default
# device characteristics: <resistive(Ohm.um^2)> # capacitive ( fF/um^2 )
1.0000e-09   -1
# <material type> and <vertical>, <horizontal> resistive # characteristics per subdivision
   2     0.0000e+00   0.0000e+00
   0     3.9082e+02   1.2941e+04
   0     9.5393e+02   1.2587e+04
   0     1.7440e+04   2.4593e+03
   0     1.4383e+05   1.0201e+04
   0     8.7490e+06   1.0370e+07
   0     9.8621e+07   1.7468e+09
   1     2.0254e+08   2.5129e+08
   1     2.7069e+06   2.2487e+04
   1     1.9400e+06   4.8871e+03
   1     7.1345e+06   1.3289e+03
   1     2.6238e+07   3.6135e+02
# nwell/contact
# device characteristics: <resistive(Ohm.um^2)> # capacitive ( fF/um^2 )
1.0000e-09   -1
# <material type> and <vertical>, <horizontal> resistive # characteristics per subdivision
   0     6.8002e+00   9.6861e+00
   0     4.6760e+02   5.4535e+03
   0     1.2251e+03   1.6432e+04
   0     1.7478e+04   2.4707e+03
   0     1.4217e+05   1.0188e+04
   0     8.5180e+06   1.0150e+07
   0     9.6460e+07   1.7007e+09
   1     2.0497e+08   2.5765e+08
   1     2.7534e+06   2.2499e+04
   1     1.9400e+06   4.8871e+03
   1     7.1345e+06   1.3289e+03
   1     2.6238e+07   3.6135e+02
# nwell/source/drain
# device characteristics: <resistive(Ohm.um^2)> # capacitive ( fF/um^2 )
1.0000e-09   -1
# <material type> and <vertical>, <horizontal> resistive # characteristics per subdivision
   2     0.0000e+00   0.0000e+00
   2     0.0000e+00   0.0000e+00
   0     5.5982e+06   5.7740e+06
   0     2.1455e+04   2.6427e+03
   0     1.4840e+05   1.0215e+04
   0     9.5349e+06   1.1043e+07
   0     1.0625e+08   1.9101e+09
   1     1.9296e+08   2.2594e+08
   1     2.5288e+06   2.2586e+04
   1     1.9400e+06   4.8871e+03
   1     7.1345e+06   1.3289e+03
   1     2.6238e+07   3.6135e+02
# nwell/channel
# device characteristics: <resistive(Ohm.um^2)> # capacitive ( fF/um^2 )
1.0000e-09   -1
# <material type> and <vertical>, <horizontal> resistive # characteristics per subdivision
   0     4.7007e+06   1.0155e+07
   0     6.0666e+03   5.5179e+04
   0     1.2965e+03   1.7389e+04
   0     1.7486e+04   2.4723e+03
   0     1.4840e+05   1.0215e+04
   0     9.5344e+06   1.1042e+07
   0     1.0625e+08   1.9101e+09
   1     1.9297e+08   2.2594e+08
   1     2.5289e+06   2.2586e+04
   1     1.9400e+06   4.8871e+03
   1     7.1345e+06   1.3289e+03
   1     2.6238e+07   3.6135e+02
#############################################
#            Interface Connections          #
#############################################
# allowed connection sorted by cross-section :
#{ default contact source/drain channel }
# Resistive : 0=No 1=Yes
0 1 0 0
# Capacitive : 0=None 1=On-Field 2=On-Active 3=Gate
1 0 0 3
##############################################
#           Interface Parameters
##############################################
#sorted by interconnect layer :
#interconnect { metal2  poly  metal1  }
# RESISTIVE PARAMETERS                   CAPACITIVE PARAMETERS
#   N        P             On Field Oxide                     on Active Oxide
#    (Ohm.um^2)    Perimeter (fF/um)  Area (fF/um^2)  Perimeter (fF/um)  Area (fF/um^2)
   -1       -1      -1                 0.015           -1                 0.023
   -1       -1      -1                 0.038           -1                 -1
   10       10      -1                 0.021           -1                 0.046
##############################################
#                Gate Parameters
##############################################
# sorted by gate layer : gate { poly }
# Gate Capacitance
#   Perimeter (fF/um)    Area(fF/um^2)
        0                        0.84

x
.ft
.S
.SK
.HU "APPENDIX D -- SNATCT Substrate Resistivity Table"
.S 11
.nf
.P
For Silicon by temperature of 300 Kelvin.
.TS
box;
l| c s s| c s s
l| l l l| l l l.
	n-type doping	p-type doping
_
net impurity	resistivity	resistivity	calc.	resistivity	resistivity	calc.
concentration	in techfile	in profile	mobility	in techfile	in profile	mobility
[cm-3]	[\(*W.\(*mm]	[\(*W.cm]	[cm^2/V.s]	[\(*W.\(*mm]	[\(*W.cm]	[cm^2/V.s]
_
1.0e+21	3.5285e-1	7.0570e-5	0.8857e+2	5.7211e-1	1.1442e-4	0.5462e+2
1.0e+20	3.4111e+0	6.8222e-4	0.9161e+2	5.5478e+0	1.1096e-3	0.5633e+2
1.0e+19	2.7353e+1	5.4706e-3	1.1425e+2	4.5411e+1	9.0822e-3	0.6882e+2
1.0e+18	1.1907e+2	2.3814e-2	2.6245e+2	2.1824e+2	4.3648e-2	1.4319e+2
1.0e+17	4.0147e+2	8.0294e-2	7.7839e+2	9.4521e+2	1.8904e-1	3.3061e+2
1.0e+16	2.5617e+3	5.1234e-1	1.2199e+3	7.1497e+3	1.4299e+0	4.3708e+2
1.0e+15	2.3600e+4	4.7200e+0	1.3242e+3	6.8295e+4	1.3659e+1	4.5757e+2
1.0e+14	2.3333e+5	4.6666e+1	1.3393e+3	6.7873e+5	1.3575e+2	4.6042e+2
1.0e+13	2.3299e+6	4.6598e+2	1.3413e+3	6.7818e+6	1.3564e+3	4.6079e+2
1.0e+12*	2.3293e+7	4.6586e+3	1.3416e+3	6.7771e+7	1.3554e+4	4.6111e+2
1.0e+11*	2.3126e+8	4.6252e+4	1.3513e+3	6.3917e+8	1.2783e+5	4.8892e+2
.TE
*) substract 2.09e+8 / 2.09e+9 from this value
.S

.ta 1.2c
Note:	You must use a minus sign before the net concentration or
	resistivity values in a p-type doping profile file.  You must
	specify the resistivity as given in column 3, but in \(*W.\(*mm.
	The values in a technology file are divided by 2.
.P
The following figure shows the resistivity in \(*W.cm versus impurity concentration (x-axis)
using logarithmic scales.
.F+
.PSPIC "an0506/app-F.ps" 5i 3i
.F-
.SK
.HU "APPENDIX E -- SNAEDT Site Files"
.nf
.S 9
.ft C
% cd /u/01/01/simon/SubstrateStorm/examples/site/scmos_n

%%%%%%%%%%%%%%%%%%%%
(1) gds.map
%%%%%%%%%%%%%%%%%%%%
# Layer map table from GDSII to Cadence Design System
# Technology: scmos_n
# ---------------------------------------------------
#       Cadence            |        GDSII layer
# -------------------------|-------------------------
#   name   |    purpose    |    number    |   type
# ---------|---------------|--------------|----------
  CWN           drawing             42        0-63
  CCP           drawing             47        0-63
  CAA           drawing             43        0-63
  CSN           drawing             45        0-63
  COG           drawing             52        0-63
  CPG           drawing             46        0-63
  CCA           drawing             48        0-63
  CMF           drawing             49        0-63
  CMS           drawing             51        0-63
  CVA           drawing             50        0-63

%%%%%%%%%%%%%%%%%%%%
(2) SCparameters.edt
%%%%%%%%%%%%%%%%%%%%
snapAngle          = 45
cifdialect         = cadence
smoothRegion       = -1.000e+00
smoothAccessPort   = -1.000e+00
cursorResolution   = 5.000e-01
snapSize           = 5.000e-01
grid_on_off        = no
gridMinor          = 2
gridMajor          = 10
globalDisplayDepth = -1
displayFastRefresh = yes
displayPrecision   = 99
displayAutoRaise   = yes

color.CWN = VIOLET
filled.CWN = yes
show.CWN = yes
stipple.CWN = 40
color.CAA = GREEN
filled.CAA = yes
show.CAA = yes
stipple.CAA = 12
color.CSN = CYAN
filled.CSN = no
show.CSN = yes
stipple.CSN = 12
color.CCP = WHITE
filled.CCP = yes
show.CCP = yes
stipple.CCP = 7
color.CPG = RED
filled.CPG = yes
show.CPG = yes
stipple.CPG = 7
color.CMF = BLUE
filled.CMF = yes
show.CMF = yes
stipple.CMF = 8
color.CCA = BLACK
filled.CCA = yes
show.CCA = yes
stipple.CCA = 0
color.CMS = YELLOW
filled.CMS = yes
show.CMS = yes
stipple.CMS = 9
color.CVA = WHITE
filled.CVA = no
show.CVA = yes
stipple.CVA = 0
color.COG = lightgrey
filled.COG = yes
show.COG = yes
stipple.COG = 4
color.CEL = WHITE
filled.CEL = yes
show.CEL = no
stipple.CEL = 2
color.XP = WHITE
filled.XP = no
show.XP = no
stipple.XP = 0

SCdefaultRegionBias ()
SCdefaultNodesCharacteristics (
 (  1  Vss  0.0  U  0.0  -1.000e+00  0  )
 (  2  Vdd  0.0  U  0.0  -1.000e+00  0  )
)

SCabstractViewGeneration (
    ShapeExtraction( generation )
  # RegionRecognition( SCwell )
  # AccessPortRecognition( SCwellChannel )
  # AccessPortRecognition( SCwellContact  SCsubsContact )
    AccessPortRecognition( SCsubsContact )
)

SCabstractViewSelection ()

SCshapeExtraction (
   ( generation 
       ( =  CWN  SCWELL )
       ( =  (And CWN (And CAA CPG)) SCWELLCHANNEL )
       ( = ( And ( And      CWN      CSN  ) CCA ) SCWELLCONTACT )
       ( = ( And ( And (Not CWN)(Not CSN )) CCA ) SCSUBSCONTACT )
   )
)

SCaccessPortRecognition (
  # ( SCwellChannel SCWELLCHANNEL nwell   channel 0 device x )
  # ( SCwellContact SCWELLCONTACT nwell   contact 0 CMF vdd )
    ( SCsubsContact SCSUBSCONTACT default contact 0 CMF x )
)

SCregionRecognition (
  # ( SCwell SCWELL nwell ( 5.0 2 ) ) 
)

GeometryFormat = prefix

NetConnectInfo (
   (
     ( component_type  resistor )
     ( componentID  r           )
     ( model_type ((3 subc))    )
     ( bulk_electrode  3        )
     ( region          default  )
     ( cross_section   contact  )
     ( connection_type  0       )
     ( connection_layer  CMF    )
     ( connection_number  i     )
     ( color  WHITE             )
     ( stipple  2               )
     ( show  yes                )
     ( filled  yes              )
   )
)

%%%%%%%%%%%%%%%%%%%%
(3) SCparameters.xtr
%%%%%%%%%%%%%%%%%%%%
localTolerance         = 0.000e+00
globalTolerance        = 1.000e-05
spiceTolerance         = 1.000e-02
localPrecision         = 1.000e-16
globalPrecision        = 1.000e-16
spicePrecision         = 1.000e-16
overGriddingFactor     = 1
overGriddingResolution = 1.000e+02
wellCapacitanceSize    = 1.000e+02
wellCapacitanceDepth   = 0.000e+00
wellCapacitanceThreshold = 0.000e+00
visualConnectImpedance = 1
gridMaxDepth           = 1
gridMaxLocalDivisions  = 15
gridGlobalResolution   = 0.000e+00
gridGlobalDivision0    = 1
gridGlobalDivision1    = 8
gridGlobalDivision2    = 6
gridUseHighPrecision   = 0
.ft
.S
.SK
.HU "APPENDIX F -- SNAEDT Example & Files"
.nf
.S 9
.ft C
.F+
.PSPIC "an0506/snaedt.ps" 5i
.F-
% cd /u/01/01/simon/SubstrateStorm/examples/edt/simon/t2

%%%%%%%%%%%%%%%%%%%%
netlist.ini
%%%%%%%%%%%%%%%%%%%%
* substrate contacts
*
* shape: (( 0.0  0.0) ( 1.0  1.0))
rs1 vss subc1 0.001
* shape: (( 5.0  0.0) ( 6.0  1.0))
rs2 vss subc2 0.001

%%%%%%%%%%%%%%%%%%%%
SCabstractview
%%%%%%%%%%%%%%%%%%%%
# -----------------------
# Substrate Abstract View
# -----------------------
# 
# generated from SubstrateStorm
# 
VERSION 3.5

ABSTRACTVIEW (
    NB_EXTERNAL_NODES  2
    NB_REGIONS         1
    NB_ACCESS_PORTS    8
    NB_MACRO_PORT_MASKS 0
)

EXTERNAL_NODES ( 
  (  4  subc1     2    0.000000e+00  U 1.000000e+04  -1.000000e+00 0 )
  (  5  subc2     2    0.000000e+00  U 1.000000e+04  -1.000000e+00 0 )
)

REGIONS ( 
  ( default [ 0.000  -1] 0 ( -3.000  -3.000   12.000 7.000))
)

ACCESS_PORTS (
  ( default  contact   0          CMF  4 resistor (  0.000  0.000   1.000 1.000 )) 
  ( default  contact   0          CMF  5 resistor (  5.000  0.000   1.000 1.000 )) 
  ( default  default  -1 Disconnected -1   <none> ( -3.000 -3.000   1.000 1.000 )) 
  ( default  default  -1 Disconnected -1   <none> ( -2.000 -2.000   1.000 1.000 )) 
  ( default  default  -1 Disconnected -1   <none> (  2.000  0.000   1.000 1.000 )) 
  ( default  default  -1 Disconnected -1   <none> (  3.000  0.000   1.000 1.000 )) 
  ( default  default  -1 Disconnected -1   <none> (  7.000  2.000   1.000 1.000 )) 
  ( default  default  -1 Disconnected -1   <none> (  8.000  3.000   1.000 1.000 )) 
)

MACRO_PORT_MASKS ( 
)

%%%%%%%%%%%%%%%%%%%%
substrate.subckt
%%%%%%%%%%%%%%%%%%%%
******** HSPICE Subcircuit, by SUBSTRATESTORM  ********
*
*   Fri Apr 28 14:21:45 2006
*
*   Cell name: t2
*   Reduction Mode: RC
*   Reduction Frequency: 5e+09
*
*   Backside Connection: Connected
*   Use Node Names     : yes
*   Backside Node      : BACKSIDE 3
*   Node BACKSIDE     3 capacitance = 0, conductance = 0
*   Node subc1        4 capacitance = 0, conductance = 0.1
*   Node subc2        5 capacitance = 0, conductance = 0.1
*
* Sample instantiation statement:
* X<name> BACKSIDE subc1 subc2 SCt2

\&.SUBCKT SCt2 BACKSIDE subc1 subc2 

R1 subc1 BACKSIDE 43222.5
R2 subc2 BACKSIDE 43222.5
R3 subc2 subc1 1.31276e+06
\&.ENDS SCt2

*SNA_STAMPS03504e5370a4ebb0eda4c78204ca0d66490ecc096ab6fc76c86a2416613
*bf154b6f77854073371400613d06741d7d193d2ef5ec1a2be0c937cfd86ef45d8a89a2
*c8ade988b7cc7a4725f636875ebc84fd1a06950300088c340000000000000000000883
*...
.ft
.S
.SK
.HU "APPENDIX G -- Testcell With Two Substrate Contacts"
.nf
.S 10
.ft C
% cd /u/52/52/work/simon/CACD/demo/demo/suboscil/projectname
% xldm -f "" t2b
ms t2b
term cmf 0 0 0 0 a
term cmf 50 50 0 0 b
box cmf 48 62 -2 12
box cmf -2 12 -2 12
box cca 0 10 0 10
box cca 50 60 0 10
me

% cat param.p
BEGIN sub3d # Data for the boundary-element method
be_mode        0g    # pwc galerkin
max_be_area    1.0   # [um^2] max size of surface mesh interior elements
edge_be_ratio  0.001 # max size of edge elements = edge_be_ratio * max_be_area
edge_be_split  0.5   # edge/interior size ratio by split
be_window      inf   # [um] window width
neumann_simulation_ratio 1000
END sub3d

disp.save_prepass_image  on    # Data for Xspace

% Xspace -B -E tech.t -P param.p t2b -Ssub3d.edge_be_ratio=0.1
.F+
.PSPIC "an0506/xspace.ps" 5i 3i
.F-
% xsls t2b
network t2b (terminal a, b)
{
    res 644.0282k (b, a);
    res 48.6771k (b, SUBSTR);
    res 48.6771k (a, SUBSTR);
}
.ft
.S
.SK
.HU "APPENDIX H -- Testcell Substrate Resistivity vs. Number of Elements"
.nf
.S 10
.ft C
testcell: 2 contacts (a and b) of size 1 x 1 um
          distance between contacts: 4 um
substrate: 1 layer with conductivity of 10 S/m
sub3d.be_window=inf   sub3d.max_be_area=1.0   sub3d.edge_be_split=0.5
.ft
.S
.S 11
.TS
box;
l|r| c s s| c s s
l|r| l l l| l l l
r|r| r r r| r r r.
edge	nr_of	sub3d.be_mode=0g	sub3d.be_mode=0c
ratio	elmts	Rab[k\(*W]	Rsub[k\(*W]	Rv[k\(*W]	Rab[k\(*W]	Rsub[k\(*W]	Rv[k\(*W]
_
1.000	1	699.7749	50.57662	88.3781	984.2432	59.29840	105.8432
0.500	2	698.8330	50.54454	88.3141	878.4212	56.28165	99.7775
0.400	4	698.3104	50.52697	88.2789	790.6651	53.53428	94.2990
0.200	8	669.3293	49.57765	86.3616	734.4101	51.76744	90.7423
0.100	16	644.0282	48.67710	84.5702	685.9581	50.12044	87.4601
0.050	28	622.9055	47.93933	83.0894	648.8552	48.85425	84.9206
0.025	44	616.2622	47.69843	82.6090	635.4859	48.38021	83.9743
0.010	72	603.5721	47.24404	81.6984	614.7498	47.64600	82.5032
0.005	104	601.3967	47.16469	81.5398	610.6423	47.49782	82.2070
0.002	164	594.3540	46.90949	81.0286	599.5495	47.09808	81.4063
0.001	228	593.6187	46.88261	80.9748	598.2321	47.05019	81.3105
.TE
.S
.F+
.PSPIC "an0506/app-G.ps" 5i
.F-
.F+
.PSPIC "an0506/app-G2.ps" 5i
.F-
.SK
.HU "APPENDIX I -- Testcell Substrate Resistivity vs. Contact Distance"
.nf
.S 10
.ft C
sub3d.be_window=inf   sub3d.max_be_area=1.0   sub3d.edge_be_split=0.5
sub3d.be_mode=0g      sub3d.edge_be_ratio=0.001    size=1.0um^2
.ft
.S
.S 11
.TS
box;
l| l l l| l| l l l
r| r r r| r| r r r.
dist	Rab[k\(*W]	Rsub[k\(*W]	Rv[k\(*W]	dist	Rab[k\(*W]	Rsub[k\(*W]	Rv[k\(*W]
_
1	223.783	51.78736	70.8041	16	2036.878	44.62367	85.5010
2	349.572	49.04312	76.5946	17	2156.893	44.57160	85.6052
3	472.155	47.68917	79.3493	18	2276.899	44.52502	85.6984
4	593.619	46.88261	80.9748	19	2396.895	44.48310	85.7822
5	714.552	46.34706	82.0503	20	2516.884	44.44518	85.8581
6	835.188	45.96549	82.8153	21	2636.867	44.41071	85.9270
7	955.642	45.67977	83.3877	22	2756.843	44.37924	85.9900
8	1075.976	45.45779	83.8321	23	2876.816	44.35038	86.0477
9	1196.226	45.28034	84.1873	24	2996.784	44.32384	86.1007
10	1316.415	45.13523	84.4776	25	3116.747	44.29934	86.1497
11	1436.558	45.01439	84.7194	26	3236.709	44.27667	86.1951
12	1556.667	44.91215	84.9239	27	3356.667	44.25560	86.2372
13	1676.749	44.82456	85.0992	28	3476.621	44.23598	86.2764
14	1796.809	44.74866	85.2510	29	3596.574	44.21770	86.3131
15	1916.851	44.68225	85.3839	30	3716.524	44.20058	86.3473
.TE
.S
.F+
.PSPIC "an0506/app-H.ps" 5i 2.1i
.F-
.F+
.PSPIC "an0506/app-H2.ps" 5i 2.1i
.F-
The figures on next page show the effect of using metalization.
.SK
The following two figures show the effect of using metalization.
.F+
.PSPIC "an0506/app-H3.ps" 5i
.F-
.F+
.PSPIC "an0506/app-H4.ps" 5i
.F-
.fi
For "-20d" the default neumann_simulation_ratio of 100 is used.
For the other metalizations neumann_simulation_ratio of 1000 is used.
The numbers behind Rab and Rsub mean the metalization depth in \(*mm.
You see that Rab becomes non-linear for small depths.
The values for Rab/Rsub by a depth of 400 \(*mm is almost the same
with the values without using metalization.
.SK
.HU "APPENDIX J -- Testcell Substrate Resistivity vs. Contact Size"
.nf
.S 10
.ft C
sub3d.be_window=inf   sub3d.max_be_area=w^2   sub3d.edge_be_split=0.5
sub3d.be_mode=0g      sub3d.edge_be_ratio=0.001    distance=30um
.ft
.S
.S 11
.TS
box;
l| l l l| l| l l l
r| r r r| r| r r r.
w	Rab[k\(*W]	Rsub[k\(*W]	Rv[k\(*W]	w	Rab[k\(*W]	Rsub[k\(*W]	Rv[k\(*W]
_
1.0	3716.524	44.20058	86.3473	1	3716.524	44.20058	86.3473
1.1	3081.258	40.22738	78.4075	2	958.421	22.34108	42.6918
1.2	2597.289	36.91612	71.7914	3	438.805	15.04509	28.1592
1.3	2220.032	34.11403	66.1937	4	253.956	11.39056	20.9058
1.4	1920.191	31.71194	61.3960	5	167.038	9.19311	16.5631
1.5	1677.911	29.63002	57.2385	6	119.092	7.72455	13.6751
1.6	1479.311	27.80825	53.6013	7	89.745	6.67269	11.6178
1.7	1314.428	26.20043	50.3919	8	70.415	5.88152	10.0793
1.8	1176.042	24.77120	47.5397	9	56.970	5.26429	8.8863
1.9	1058.736	23.49226	44.9880	10	47.216	4.76893	7.9349
.TE
.S
.F+
.PSPIC "an0506/app-I.ps" 5i
.F-
.F+
.PSPIC "an0506/app-I2.ps" 5i
.F-
.SK
.HU "APPENDIX K -- SubstrateStorm Resistivity Results"
The following figure shows the extracted substrate resistivity versus the substrate thickness in \(*mm.
For the figure the substrate thickness is subdivided by 10, 20 and 39 layers (each of equal thickness).
The two substrate contact example was used (contact size 1 by 1 \(*mm, distance 4 \(*mm).
A profile resistivity of 50 k\(*W.\(*mm is used (the half value for a conductivity of 10 S/m).
In the SubstrateStorm technology file, you can find 25 k\(*W by a subdivision thickness of 1 \(*mm.
A not refined surface grid is used.
The width of the bounding box around the contacts is 20 \(*mm.
Two resistivity values Rv1 and Rv2 are given.
The value of Rv1 is for the none backside case.
Above a substrate thickness of 4 \(*mm the values of Rv1 and Rv2 are almost equal to each other.
Because the thickness of one subdivision is not constant, the resistivity of Rv1 / Rv2 is not constant.
How more layers are used, how slower the slope in the resistivity.
See appendix L for the case when a constant subdivision thickness is used.
.F+
.PSPIC "an0506/app-K.ps" 5i
.F-
.S 11
.TS
box;
r| c s| c s| c s
r| r r| r r| r r.
substrate	10 layers	20 layers	39 layers
thickness	Rv1[k\(*W]	Rv2[k\(*W]	Rv1[k\(*W]	Rv2[k\(*W]	Rv1[k\(*W]	Rv2[k\(*W]
_
1	179.91	82.11	179.77	82.07	179.74	82.06
2	131.37	112.69	131.01	112.40	130.92	112.36
3	124.09	120.13	123.37	119.46	123.19	119.30
4	123.27	122.26	122.03	121.04	121.73	120.74
10	131.03	130.93	123.60	123.56	121.77	121.68
20	157.04	157.02	130.98	130.95	123.74	123.71
40	233.80	233.80	157.03	157.03	131.46	131.46
100	514.36	514.36	277.77	277.77	176.34	176.34
.TE
.S
.SK
.HU "APPENDIX L -- Calculated Resistivity Values"
We can calculate the total resistivity value (Rv) between two substrate contacts
(with distance 4 \(*mm of each other)
for a resistor network of the SubstrateStorm FDM model with the
.P= spice
network simulator.
The bounding box around the two contacts is 20, 10 and 1 \(*mm width.
The surface mesh is not refined.
The table below shows the Rv results in k\(*W for a number of layers for both the none bs and backside(bs) case.
Each layer has a thickness of 1 \(*mm.
.TS
box;
l| c s| c s| c s
l| l l| l l| l l
r| r r| r r| r r.
nr of	bbox width 20 \(*mm	bbox width 10 \(*mm	bbox width 1 \(*mm
layers	w/o bs	with bs	w/o bs	with bs	w/o bs	with bs
_
1	197.8583	 87.2419	161.5917	 83.9961	149.3976	71.5648
2	142.5501	120.7769	122.7939	109.4376	110.4812	85.2653
3	133.4869	128.7973	117.2125	114.1147	101.1845	90.7960
4	131.6635	130.4874	116.1157	114.9328	 97.8733	93.4326
5	131.2541	130.8404	115.8026	115.1039	 96.5455	94.6360
6	131.1421	130.9179	115.6636	115.1619	 95.9902	95.1683
7	131.0990	130.9380	115.5807	115.1963	 95.7541	95.4002
8	131.0754	130.9455	115.5242	115.2230	 95.6529	95.5006
9	131.0593	130.9500	115.4835	115.2451	 95.6095	95.5439
10	131.0472	130.9535	115.4532	115.2637	 95.5908	95.5626
.TE
.F+
.PSPIC "an0506/app-L.ps" 5i
.F-
As you can see, the results convergent to a value of 131 k\(*W for a bbox of 20 \(*mm width,
to a value of 115.4 k\(*W for a bbox of 10 \(*mm width,
and to a value of 95.575 k\(*W for a bbox of 1 \(*mm width.
.SK
.HU "APPENDIX M -- More Calculated Resistivity Values"
More calculations of the total resistivity value (Rv) between two substrate contacts
(with distance 4 \(*mm of each other).
Initial for a bounding box width of 1 \(*mm (see also previous appendix).
I call this case 1-1 (first 1 for 1 box of 4 by 1 between the contacts and second 1 for 1 \(*mm width).
In the second case (2-1), the box between the contacts is split to 2 boxes of 2 by 1.
In the third case (4-1), the surface mesh between the contacts is further refined to 4 boxes of 1 by 1 \(*mm.
In the case 4-2, the area around the contacts is expanded with a new ring of 1 by 1 boxes.
And thus, for case 4-3 and 4-4 the bounding box width becomes 3 and 4 \(*mm.
You see, that case 4-4 gives the best results.
.F+
.PSPIC "an0506/app-M.ps" 5i
.F-
Calculated Rv [k\(*W] values:
.S 11
.TS
box;
l| c s| c s| c s| c s| c s
r| l l| l l| l l| l l| l l.
nr	case 2-1	case 4-1	case 4-2	case 4-3	case 4-4
lay	!bs	bs	!bs	bs	!bs	bs	!bs	bs	!bs	bs
_
1	149.2	70.59	148.0	69.76	122.3	69.22	113.1	69.19	108.8	69.19
2	109.9	83.23	108.9	81.85	95.75	78.33	91.13	77.84	88.98	77.76
3	100.2	88.58	99.05	87.17	89.49	81.17	86.32	79.94	84.88	79.64
4	96.56	91.32	95.37	89.94	87.04	82.71	84.49	80.96	83.37	80.42
5	95.04	92.65	93.81	91.31	85.90	83.60	83.60	81.56	82.65	80.86
6	94.37	93.28	93.11	91.96	85.33	84.10	83.13	81.93	82.26	81.14
7	94.07	93.57	92.80	92.27	85.03	84.37	82.87	82.16	82.03	81.32
8	93.94	93.71	92.66	92.41	84.88	84.52	82.72	82.30	81.89	81.44
9	93.87	93.77	92.59	92.48	84.79	84.60	82.63	82.38	81.81	81.52
10	93.85	93.80	92.56	92.51	84.75	84.65	82.58	82.43	81.75	81.57
.TE
.S
