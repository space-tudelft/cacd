.T= "Path Preserving Demo Instructions"
.DS 2
.rs
.sp 1i
.B
.S 15 20
Path Preserving
Demo Instructions
.S
.sp 2
.I
Simon de Graaf
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
Report EWI-ENS 15-01
.ce
April 21, 2015
.sp |9i
.DS
.in +5
.S 9
Copyright \s+3\(co\s-3 2015 by the author.

Last revision: April 22, 2015.
.S
.in -5
.DE
.SK
.S=
.H 1 "INTRODUCTION"
Path preserving can be enabled when a resistance extraction is done.
To preserve corner nodes, set parameter "use_corner_nodes" to "on".
Thus, around the corners of high resistive conductor paths, the
.P= space
extractor shall retain special line nodes.
At this way the topology of paths in the layout can be found back in
the resulting extracted resistance network.
(Note that parameter "equi_line_width" must also be set.)
.br
For example:
.fS
% space -Suse_corner_nodes=on -Sequi_line_width=2 ...
.fE
Or place these parameters in your "space.def.p" file.
.P
For more information about path preserving resistance extraction,
see also the report EWI-ENS 14-05 of December 15, 2014.
.fS
file: $ICDPATH/share/doc/notes/an1405.pdf
.fE
.P
Note that this document is a handout for demostrating some
layout examples with path preserving.
And note that you need to have an open terminal window for the demo instructions,
to give commands.
.P
Note that we are using the
.P= Xspace
program to start the
.P= space3d
extractor.
However, we don't demostrate any 3D extraction.
Note that all cell extractions are in flat mode.
For all
.P= Xspace
features consult the "Xspace User's Manual" (Xspaceman.pdf).
For the
.P= dali
program consult the "Dali User's Manual" (dali.pdf).
.P
Path preserving works only with Space Release 5.4.7 and higher.
.fS
Released at February 19, 2015.
.fE
.P
First i shall give instructions how to download the Space distro.
.P
After that i shall give general demo instructions.
.P
There after i shall give instructions for the path preserving demos.
.br
Only for the most suitable demo projects.
.P
For support, you can email me on the following address:
.fS
 space-support-ewi@tudelft.nl
.fE
.F+
.PSPIC "an1501/m0802272.ps" 1i
.F-

.H 1 "DOWNLOAD AND INSTALL INSTRUCTIONS"
First download the latest (5.4.7) Space distro from the Space website:
.fS
 www.space.tudelft.nl
.fE
Click on the "Download" menu item and click on the latest Space distro archive
item ( cacd-5.4.7-release-linux... ) to start downloading the archive file.
.P
Choice a Space distro install directory (for example $HOME).
Read also the install instructions on the Space website (under the "Download" menu).
In the install directory a directory "cacd" is created as starting point of the distro.
Later referenced as ICDPATH, which environment variable can be set for usage if needed.
Internally, when running a Space program, a wrapper sets by itself this ICDPATH.
Thus, you only need to set the execution path for the programs of the installed distro.
Therefor the shell PATH environment variable is used.
.P
But first unpack the downloaded Space distro.
Go to the install directory (for example $HOME) with the change directory command.
.fS
% cd $HOME
.fE
Now, unpack the Space distro, which is maybe downloaded in the "Downloads" directory.
.fS
% gunzip .../Downloads/cacd-5.4.7-release....tar.gz
% tar xf .../Downloads/cacd-5.4.7-release....tar
.fE
Now, add the install path to the PATH environment variable:
.fS
% setenv PATH $HOME/cacd/share/bin:$PATH
.fE
And test the execution path, run
.P= space
with the help option:
.fS
% space -h
.fE
The
.P= space
program is running, if the PATH is correctly set.
But there is a license error.
Thus you need a license.
To obtain a license, type
.P= lictool
with the request option:
.fS
% lictool -r
.fE
Fill in your name and company and your email address to receive the license.
(Note that an educational license is only valid for 3 months.)
Now, install the license file.
Open your email reader and look if you have received an email with subject "Your CACD license file".
If found, save the license file (license.txt) and use
.P= lictool
again to install it.
.fS
% lictool -f .../license.txt
.fE
Now, check if the license is working:
.fS
% space -h
.fE
If not, for support, you can email me on the following address:
.fS
 space-support-ewi@tudelft.nl
.fE

.H 1 "GENERAL DEMO INSTRUCTIONS"
Go to the Space demo directory.
Now it is handy to set the ICDPATH environment variable.
Therefor check your install path, type:
.fS
% echo $PATH
/home/simon/cacd/share/bin:.:/usr/bin:/bin:/opt/kde3/bin
.fE
Now, set the ICDPATH:
.fS
% setenv ICDPATH /home/simon/cacd
.fE
Ok, go to the demo directory and make a listing, type:
.fS
% cd $ICDPATH/share/demo
% ls
attenua  crand   FreePDK45  invert    multiplier  nodenames  ny9t
poly5    README  sram       sub3term  suboscil    switchbox
.fE
You can read the README file:
.fS
% cat README
.fE
You can also read the "examples.pdf" file for more information about the examples.
Best you can use a "File Manager" to go to directory $ICDPATH/share/doc.
.P
The first time you are using a demo, you are chosing a demo directory,
for example "switchbox":
.fS
% cd switchbox
% ls
README script.sh script.tcl swbox_ref.spc switchbox4_f.gds switchbox4.gds
.fE
You see again a README file, which you can read.
And two "script" files and a GDS layout file.
But this project directory is not yet changed into a project,
because there are no project database files.
One of the "script" files can be used to run the demo.
It takes care of the initialisation of the project directory.
One script is for the Bourne shell (sh) and the other can be used with tclsh.
If the script is executable (the default) and your current path is in PATH,
then you can easyly run the script by typing:
.fS
% script.sh
.fE
If not, you can always invoke one of the scripts by typing:
.fS
% sh script.sh
% tclsh script.tcl
.fE
Start the demo by Enter or y or yes.
You can always stop the demo after some steps.
Type Ctrl-C to abort the script.
For example, you want only to init the database
and put the layout in it.
You can also skip some steps by typing n or no.
Note that some steps depends on previous taken steps.
At last the script shall clean-up the project.
.P
Note that for the Path Preserving Demo's you need to init the database.
.P
Note that you can better use a very width monitor screen for the demo's.
If you don't have such a width monitor screen, you can better use two monitor screens.

.H 1 "THE SUBOSCIL DEMO"
Go to the "suboscil" demo project directory and init the database.
.fS
% cd $ICDPATH/share/demo/suboscil
% script.sh
.fE
Do at least the
.P= mkpr
and
.P= cgi
steps.
Stop by the
.P= dali
step, giving a Ctrl-C.
We later-on shall start the
.P= dali
layout editor.
.P
Start now the
.P= Xspace
program twice (in the background).
Left on the screen with-out using path preserving (i.e. corner nodes), type:
.fS
% Xspace -geometry 800x600+0+0 &
.fE
And right on the screen with using path preserving:
.fS
% Xspace -geometry 800x600+800+0 -Suse_corner_nodes=on -Sequi_line_width=2 &
.fE
Be sure that you still can see the lowest part of the terminal window (for messages).
.P
First we are looking to the "oscil" basic invertor cell "inv".
Now, choice in both Xspace windows from the "Database" menu the "inv" layout cell.
Choice from the "Options" menu the "resistance" extraction mode and
choice from the "Display" menu the "DrawTile" and "DrawFEMesh" items.
Now, choice from the "Extract" menu the "extract" command.
.P
Note that sometimes the "extract" command fails,
in that case click on the "extract" command again.
Note that you can also use the "e" hotkey.
Note that both
.P= Xspace
programs use the same display file.
Don't extract in both windows at the same time.
Don't use the "extract again" command (or "a") in one window after "extract" of the other window,
because then you display the wrong output.
Note that a hotkey only works for the active window and also can better not be used in a demo.
.P
Ok, now we are looking to both windows and we see that the finite element resistance mesh of both
windows is different.
Because for corner node (or path) preserving another mesh strategy is used.
In this case only the green (active area) and the red (poly) masks are high resistive.
What you see is the initial resistor mesh.
What is outputted (after node reduction and some heuristics) can you display with "DrawOutResistor".
Put off "DrawFEMesh" and on "DrawOutResistor" in the "Display" menu and do "extract".
.P
You see that in the right window three nodes are preserved around the poly corners.
Actually we can replace this triangle in the future into a star configuration.
In this basic cell, we also see that for each transistor three nodes are retained.
In the upper-right corner of the gate a gate node and at the lower-left and -right
side of the gate a source and drain node.
Futher more there are four terminal nodes and a nwell area node.
And one low res area node of the metal1 "inv" output area.
.P
You should have noticed, that there is everytime a warning message comming from the
.P= space3d
extractor.
That some subnode conductors are not joined because the conductor-type is different.
Such a message is only ones given, there may be more not joined subnodes.
This happens because two different types active areas are touching each other.
See the technology file, the "caa" conductor mask is defined with the help of the "csn" mask.
Use
.P= space
with the verbose option to find which technology file is used, type:
.fS
% space -v inv
.fE
You see the line:
.fS
technology file: $ICDPATH/share/lib/process/scmos_n/space.def.t
.fE
Now you can display the source of this file, type:
.fS
% more $ICDPATH/share/lib/process/scmos_n/space.def.s
.fE
You can also start the layout editor
.P= dali
to display the "inv" cell and show all layout masks, type:
.fS
% dali =400x600 inv
.fE
Now it is clear that this happens by the power rails.
On the "vdd" rail is a nwell contact and on the "vss" rail is a substrate contact,
which use both mask "caa".
.P
We let the dali window open and shift it the right-lower corner of the screen.
Now we gone to extract also resistance for the metal1 interconnect.
Put on in the "Options" menu the "metal resistance" extract option.
And extract again by chosing "extract" in the "Extract" menu.
Note that we are still using the "DrawOutResistor" display.
.P
First of all you see that metal1 (terminal) nodes are laying on another position.
Now the center position of the terminal boxes is retained.
And in the metal1 output area there is one extra "corner" node retained.
Note that you can see these metal1 terminal boxes in the layout editor.
Chose the "visible" menu and click on the right side on the "cmf" mask to put
the visibility of all general metal1 "off".
.P
Now we gone to extract the layout of the complete "oscil" cell
and show how it looks like.
We chose "oscil" in the "Database" menu of both Xspace windows.
And extract again in both windows by chosing "extract" in the "Extract" menu.
You see that you also have extracted the "metal resistance".
If you don't want this, you can put off "metal resistance" in the "Options" menu
and extract again.
You see that now only the poly path (red) is retained.
.P
Congratulations, you have now finished this demo!
You see that it is not difficult to use the Xspace program for the demo's.
Don't forget to close the Xspace windows, you can't use them in another project.
Go to the "Extract" menu and chose "quit".

.H 1 "THE SWITCHBOX DEMO"
Go to the "switchbox" demo project directory.
Comming from "suboscil" project, type:
.fS
% cd ../switchbox
.fE
Or else type:
.fS
% cd $ICDPATH/share/demo/switchbox
.fE
And init the database by running the shell script, type:
.fS
% script.sh
.fE
Do at least the
.P= mkpr
and
.P= cgi
steps.
Show the layout hierarchy of cell "switchbox4" with the
.P= dblist
step.
Don't do the
.P= dali
step.
But abort the script by giving Ctrl-C.
Start
.P= dali
yourself on the command line (in the background) with basic cell "dubinv", type:
.fS
% dali dubinv &
.fE
Shift this window to the lower-right side of your screen to make room for the Xspace windows.
Now start the two Xspace windows.
Left on your screen with-out path preserving, type:
.fS
% Xspace -geometry 800x600+0+0 &
.fE
And right on your screen with path preserving:
.fS
% Xspace -geometry 800x600+800+0 -Suse_corner_nodes=on -Sequi_line_width=2 &
.fE
First we are looking to the "switchbox4" basic double invertor cell "dubinv".
Select in both Xspace windows from the "Database" menu the "dubinv" cell.
Choice from the "Options" menu the "resistance" extraction mode and
choice from the "Display" menu the "DrawTile" and "DrawFEMesh" items.
Now, choice from the "Extract" menu the "extract" command.
.P
If you want to display the output resistors, select "DrawOutResistor" from the "Display" menu
(and put off "DrawFEMesh").
And extract again to show new results (choice "extract" from the "Extract" menu).
.P
The same you can do with basic cells "nan3", "nan4rout"
and higher hierarchy cells "dec1of4" and "switchbox4".
For "switchbox4" you can better first resize both Xspace windows to the bottom of your screen.
You can resize the window also after "extract", but you see that the output is not automatically resized.
For that you can give the Xspace resize to bounding box command (hotkey "b").
Note that it must be done directly after "extract", else you possibly do it for the wrong display file.
.P
You can also make your own demo by making a new layout cell in this demo project.
You do that with the
.P= dali
layout editor.
I explain this in the next section.
Note that you must "quit" both Xspace windows and restart them after you have saved the new layout cell.
Else the new cell can't be found in the "Database" menu.

.H 1 "MAKING YOUR OWN DEMO CELL"
As explained in the previous section,
you can make your own demo by making a new layout cell in a project directory.
And that you can do this with the
.P= dali
layout editor.
.P
If you have still open the
.P= dali
layout editor in the "switchbox" project, you can go to the "DB_menu" and erase the workspace
with the "erase_all" command
to start editing a new layout cell.
To go back to the "Main" menu click on "return".
From the "Main" menu select the "box_menu", where you can edit a layout.
First put "on" one layout mask.
You can select the (blue) "cmf" (first metal) mask, but know that this mask is normally low resistive.
If you extract, you must not forget to choice "metal resistance" in the "Options" menu of Xspace.
You can also select the (red) "cpg" (poly) mask, which is normally high resistive.
.P
Note, if you start
.P= dali
without a cellname, you start up in the "Main" menu with an empty layout area.
Select the "box_menu" to start editting.
If you don't see the grid, click on "grid".
To see the grid origin, use the left and bottom arrow-keys.
To zoom out/in and center the layout area, use hotkeys "o", "i" and "c".
.P
Now start editting a path, select "add_wire" and select a wire width (for example 8).
In the layout area, click to set the starting point of the wire.
And click on another position to set the next point of the wire.
And so on.
After the last point, to finish the wire click on "ready".
Note that you can also use "add_box" to make or edit a wire.
Now click on "return" to leave the "box_menu".
.P
The only thing we must not forget, is,
that we must have two nodes which must be retained.
We can choice for labels or terminals.
Go to the "annotate" menu to lay down two labels.
Click on "label" and click somewhere on the begin (end) of the wire to lay down a label.
Note that a label has only one point and you must give it a name.
Type the name on your keyboard and finish input with the Enter-key.
If you want to lay down a terminal box (or point), go to the "term_menu" and click on "add_term".
And click in the layout area to lay down a terminal box.
To finish the terminal box, click a second time.
And you must give the terminal a name.
.P
Now the new layout cell is ready, it can be saved.
Click on "return" to go back to the "Main" menu and go to the "DB_menu" and click on "write_cell".
And click on "new name" and enter on the keyboard a new cellname (for example "tst1").
.P
Now we want to know what the extraction result of this new cell shall be.
We start twice the Xspace extraction display program again (see previous section).
.P
Select in the "Database" menu the new cellname (for example "tst1") and set in
the "Options" menu the extraction mode and choice in the "Display" menu the
display options "DrawTile" and "DrawFEMesh" and "DrawOutResistor".
Go to the "Extract" menu and click on "extract" and show what the result is.
.br
Please experiment, change the new layout cell again and "extract" again.
