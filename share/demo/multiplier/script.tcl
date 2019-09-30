#!/usr/bin/env tclsh
# directory: demo/multiplier

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
***************************************************************
**  Demonstration of functional/switch-level simulation.     **
**  See: "Functional Simulation User's Manual" and           **
**       "SLS: Switch-Level Simulator User's Manual".        **
**                                                           **
**  A network 'total' is simulated that has a ram and a      **
**  multiplier that are described at the functional level.   **
**  Pass transistors are used to demultiplex the output      **
**  signals of the ram and to multiply two subsequent        **
**  words that come out of the ram.                          **
***************************************************************
Warning: This demo does not always work, due to library linking problems.
When you have these problems and want to use function blocks, contact us.

Note: See the "crand" demo, for another sls and simeye example.

Note: Type Enter on the keyboard, each time, to do a step.
      If you want to skip a step, type first a 'n'.
      If you want to stop the demo, type Ctrl-C.
*********************************************************************
This script first tries to remove old project data.
*********************************************************************}
set ok [ask "Shall we start this demo? (type Enter)"]
if {!$ok} { puts "Ok, see you another time!"; exit 1 }

set f1 [open script.log w]

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
    We shall use the scmos_n technology and a default lambda value.
    We change the current working directory '.' into a project directory
    with the following command:

    % mkpr -p scmos_n .
*********************************************************************}
set ok [ask "Shall we do it? (type Enter)"]
if {!$ok} { puts "Ok, see you a next time!"; exit 1 }

echo "+ mkpr -p scmos_n ."
catch {exec mkpr -p scmos_n . >& tmp.log}
set fp [open tmp.log]
while {[gets $fp line] >= 0} { echo $line }
close $fp

puts $f1 "STEP 2."
puts {
*********************************************************************
STEP 2.
    Run cfun to compile the functional description of "ram.fun"
    and "mul8x8.fun", and add them to the database:

    % cfun ram.fun
    % cfun mul8x8.fun
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ cfun ram.fun"
    if {[catch {exec cfun ram.fun >& tmp.log} result]} { echo $result }
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
    echo "+ cfun mul8x8.fun"
    if {[catch {exec cfun mul8x8.fun >& tmp.log} result]} { echo $result }
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
}

puts $f1 "STEP 3."
puts {
*********************************************************************
STEP 3.
    Run csls to add the network 'total' to the database:

    % csls total.sls
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ csls total.sls"
    catch {exec csls total.sls >& tmp.log}
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
}
}

puts $f1 "STEP 3b."
puts {
*********************************************************************
3b. You can list the contents of the project and show the hierarchical
    circuit tree with the dblist command:

    % dblist -h
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ dblist -h"
    catch {exec dblist -h >& tmp.log}
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
puts {
You see 2 hierarchical levels. The top cell total has 2 sub-cells.
This sub-cells are the function blocks.}
}

puts $f1 "STEP 4."
puts {
*********************************************************************
4.  Start the simulation GUI simeye and run a sls simulation.

    % simeye
*********************************************************************
5.  Click on the "Simulate" menu and choice the "Prepare" item.
    Select in the "Circuit:" field cell name "total" and
    in the "Stimuli:" field file name "total.cmd" (click on it).
    Click on the "Run" button and watch the simulation results.
*********************************************************************
6.  Zoom-in onto the lowest 3 signals using "View -> ZoomIn" (click in
    the results window and move the mouse to specify a zooming area and
    click again). Now, execute "View -> Measure", and click on the right
    mouse button to watch the integer values of the multiplied signals.
*********************************************************************
7.  To exit the simeye program:
    Go to the "File" menu and click on "Exit" and "Yes".
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ simeye"
    if {[catch {exec simeye} result]} { echo $result }
}

puts {
*********************************************************************
    Congratulations, you have now finished this demo!
*********************************************************************}
set ok [ask "Shall we clean the demo directory and exit?"]
if {$ok} {
    echo "+ rmpr -fs ."
    catch {exec rmpr -fs .}
    file delete sim.diag sls sls.funlist deffunc.c total.out total.plt total.res tmp.log
}
