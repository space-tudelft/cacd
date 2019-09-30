#!/usr/bin/env tclsh
# directory: demo/FreePDK45/tabsdemo

proc ask {txt} {
    puts -nonewline $txt
    flush stdout
    set ok 1
    if {[string match n* [gets stdin]]} {set ok 0}
    return $ok
}

proc echo {txt} {
    global f1
    puts $txt
    puts $f1 $txt
}

puts {
*********************************************************************
**  Demonstration of flat space extractions of a counter cell      **
**  made with the FreePDK45 process and the use of the tabs tool.  **
*********************************************************************

Note: Type Enter on the keyboard, each time, to do a step.
      If you want to skip a step, type first a 'n'.
      If you want to stop the demo, type Ctrl-C.
*********************************************************************
This script first tries to remove old project data.
*********************************************************************}
set ok [ask "Shall we start this demo? (type Enter)"]
if {!$ok} { puts "Ok, see you another time!\n"; exit 1 }

set f1 [open script.log w]

if ![file exists "../NangateOpenCellLibrary/.dmrc"] {
    echo "sorry, the NangateOpenCellLibrary is not yet installed"
    exit 1
}

if {[file exists .dmrc]} {
    set ok [ask "Shall we remove existing project directory? (type Enter)"]
    if {$ok} {
	echo "+ rmpr -fs ."
	catch {exec rmpr -fs .}
    }
}

if {$ok} {
puts $f1 "STEP 1."
puts {
*********************************************************************
STEP 1.
    First, we need a new project directory to work with.
    We shall use the FreePDK45 technology and the lambda value of the
    NangateOpenCellLibrary.  We assume that the NangateOpenCellLibrary
    project is installed.  We change the current working directory '.'
    into a project directory with the following command:

    % mkpr -u ../NangateOpenCellLibrary .
*********************************************************************}
set ok [ask "Shall we do it? (type Enter)"]
if {!$ok} { puts "Ok, see you a next time!\n"; exit 1 }

echo "+ mkpr -u ../NangateOpenCellLibrary ."
catch {exec mkpr -u ../NangateOpenCellLibrary . >& tmp.log}
set fp [open tmp.log r]
while {[gets $fp line] >= 0} { echo $line }
close $fp
file delete tmp.log

puts $f1 "STEP 2."
puts {
*********************************************************************
STEP 2.
    To use the NangateOpenCellLibrary project, it must be added
    to the local project list.  Use the following command:

    % addproj ../NangateOpenCellLibrary
*********************************************************************}
set ok [ask "Shall we do it? (type Enter)"]
if {$ok} {
    echo "+ addproj ../NangateOpenCellLibrary"
    exec addproj ../NangateOpenCellLibrary
}

puts $f1 "STEP 3."
puts {
*********************************************************************
STEP 3.
    To be able to use the basic cells of the NangateOpenCellLibrary we
    must import them in our project.  Use the following command:

    % impcell -lc -a ../NangateOpenCellLibrary
*********************************************************************}
set ok [ask "Shall we do it? (type Enter)"]
if {$ok} {
    echo "+ impcell -lc -a ../NangateOpenCellLibrary"
    exec impcell -lc -a ../NangateOpenCellLibrary
}

puts $f1 "STEP 4."
puts {
*********************************************************************
STEP 4.
    The counter layout is available in a gds2 file.  We use cgi
    (convert-gds-internal) to put the layout into the project database
    and make a hierarchical listing.  We use the following commands:

    % cgi c23.gds
    % dblist -h c23
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ cgi c23.gds"
    catch {exec cgi c23.gds >& tmp.log}
    set fp [open tmp.log r]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp

    echo "+ dblist -h c23"
    catch {exec dblist -h c23 >& tmp.log}
    set fp [open tmp.log r]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
    file delete tmp.log
}
}

puts $f1 "STEP 5."
puts {
*********************************************************************
STEP 5.
    Now, we can inspect the c23 counter layout with the layout editor
    dali.  Note that it contains 5 sub cells with the name cnt1b.
    Cell cnt1b itself contains 4 basic cells (1 AND2, 1 DFF and 2 MUX2).
    Start dali with the following command:

    % dali c23

    To see an extra hierarchical level, type '2'. Use the 'i' key
    to zoom-in and the arrow keys. Use 'b' to get the bbox-window.
    To exit the program, click on '-quit-' and click on 'yes'.
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ dali c23"
    catch {exec dali c23}
}

puts $f1 "STEP 6."
puts {
*********************************************************************
STEP 6.
    We use a local space technology file space.def.s which first must
    be compiled, before we can use it for the space extractor.  Type:

    % tecc space.def.s

    The output file gets the name space.def.t and shall be specified
    on the space command line with option -E.
*********************************************************************}
set ok [ask "Shall we compile?"]
if {$ok} {
    echo "+ tecc space.def.s"
    catch {exec tecc space.def.s >& tmp.log}
    set fp [open tmp.log r]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
    file delete tmp.log
}

puts $f1 "STEP 7."
puts {
*********************************************************************
STEP 7.
    Now, perform a flat netlist extraction of the c23 layout. Note
    that the basic cells are not extracted, because they are library
    cells.  Type the following command to used the space extractor:

    % space -Fv -E space.def.t -P space.def.p c23

    Note that we specify also a local space parameter file space.def.p
    on the command line with option -P.  The -F option is for
    flat and the -v option is for verbose mode.
*********************************************************************}
set ok [ask "Shall we extract?"]
if {$ok} {
    echo "+ space -Fv -E space.def.t -P space.def.p c23"
    catch {exec space -Fv -E space.def.t -P space.def.p c23 >& tmp.log}
    set fp [open tmp.log r]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
    file delete tmp.log
}

puts $f1 "STEP 8."
puts {
*********************************************************************
STEP 8.
    We can now inspect the extraction result, the extracted circuit.
    To see a SLS description (or netlist), we use the xsls
    command to eXstract SLS from the database:

    % xsls c23                  (see: icdman xsls)
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ xsls c23"
    catch {exec xsls c23 >& tmp.log}
    set fp [open tmp.log r]
    while {[gets $fp line] >= 0} { puts $line }
    close $fp
    file delete tmp.log
}

puts $f1 "STEP 9."
puts {
*********************************************************************
STEP 9.
    We can perform a logic sls simulation on this netlist.
    We use the simeye program to run the simulator and to show
    the resulting waveforms.  Type:

    % simeye

    Note: Click on the "Simulate" menu and choice item "Prepare".
    Select in the "Circuit:" field cell name "c23" and in the
    "Stimuli:" field file "c23.cmd" and click the "Run" button.
    To measure the logic values, open the "View" menu and click on
    "Measure". Now you can move a ruler in the window. If you click
    on the right mouse button, you get logic values on the right side.
    To leave the program, choice item "Exit" in the "File" menu.
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ simeye"
    catch {exec simeye}
}

puts $f1 "STEP 10."
puts {
*********************************************************************
STEP 10.
    If we want to get a flat netlist to the transistor level, we must
    overrule the library status of the used basic cells.
    To do so, give the following command:

    % xcontrol -regular AND2_X1 AND3_X1 DFF_X1 MUX2_X1 OR2_X1
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ xcontrol -regular AND2_X1 AND3_X1 DFF_X1 MUX2_X1 OR2_X1"
    exec xcontrol -regular AND2_X1 AND3_X1 DFF_X1 MUX2_X1 OR2_X1
}

puts $f1 "STEP 11."
puts {
*********************************************************************
STEP 11.
    Now we can perform a complete flat netlist extraction of the c23
    layout.  Run space with the following command:

    % space -Fv -E space.def.t -P space.def.p c23
*********************************************************************}
set ok [ask "Shall we extract?"]
if {$ok} {
    echo "+ space -Fv -E space.def.t -P space.def.p c23"
    catch {exec space -Fv -E space.def.t -P space.def.p c23 >& tmp.log}
    set fp [open tmp.log r]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
    file delete tmp.log
}

puts $f1 "STEP 12."
puts {
*********************************************************************
STEP 12.
    To generate a SPICE netlist, we use the following command:

    % xspice -auo c23                (see: icdman xspice)

    The -u option omits addition of transistor bulk connections.
    The -o option omits the transistor model parameters.
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ xspice -auo c23"
    catch {exec xspice -auo c23 >& tmp.log}
    set fp [open tmp.log r]
    while {[gets $fp line] >= 0} { puts $line }
    close $fp
    file delete tmp.log
}

puts $f1 "STEP 13."
puts {
*********************************************************************
STEP 13.
    We can also perform a cap2D extraction. Before we can do that we
    must have a 2D capacitances section in the space technology file.
    To generate these 2D capacitances, you can use the tabs tool.
    We use the following command:

    % tabs -s space.def.s

    Note that we do not generate 2D capacitances for all metal layers,
    because this is very time consuming. We use in this example only
    the first two metal layers (metal1 and metal2).
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ tabs -s space.def.s"
    catch {exec tabs -s space.def.s >& tmp.log}
    set fp [open tmp.log r]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
    file delete tmp.log
}

puts $f1 "STEP 14."
puts {
*********************************************************************
STEP 14.
    Now we can perform a complete flat netlist extraction of the c23
    layout with 2D caps.  Run space with the following command:

    % space -Fv -Cl -E space.def.t -P space.def.p c23
*********************************************************************}
set ok [ask "Shall we extract?"]
if {$ok} {
    echo "+ space -Fv -Cl -E space.def.t -P space.def.p c23"
    catch {exec space -Fv -Cl -E space.def.t -P space.def.p c23 >& tmp.log}
    set fp [open tmp.log r]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
    file delete tmp.log
}
set ok [ask "Shall we netlist?"]
if {$ok} {
    echo "+ xspice -auo c23"
    catch {exec xspice -auo c23 >& tmp.log}
    set fp [open tmp.log r]
    while {[gets $fp line] >= 0} { puts $line }
    close $fp
    file delete tmp.log
}

puts {
*********************************************************************
    Congratulations, you have now finished this demo!
*********************************************************************}
set ok [ask "Shall we clean the demo directory and exit?"]
if {$ok} {
    echo "+ rmpr -fs ."
    catch {exec rmpr -fs .}
    file delete exp_dat space.def.t sim.diag c23.res c23.out
}
