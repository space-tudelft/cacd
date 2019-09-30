#!/usr/bin/env tclsh
# directory: demo/crand

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
**  Demonstration of (1) extraction of a random counter circuit    **
**               and (2) switch-level simulation of the circuit.   **
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
    We shall use the scmos_n technology and a lambda of 0.2 micron.
    We change the current working directory '.' into a project directory
    with the following command:

    % mkpr -p scmos_n -l 0.2 .
*********************************************************************}
set ok [ask "Shall we do it? (type Enter)"]
if {!$ok} { echo "Ok, see you a next time!\n"; exit 1 }

echo "+ mkpr -p scmos_n -l 0.2 ."
exec mkpr -p scmos_n -l 0.2 . >& tmp.log
set fp [open tmp.log r]
while {[gets $fp line] >= 0} { echo $line }
close $fp
file delete tmp.log

puts $f1 "STEP 2."
puts {
*********************************************************************
STEP 2.
    The layout of the random counter is available in a gds2 file.
    We use cgi (convert-gds-internal) to put the layout into the
    project database with the following command:

    % cgi crand.gds
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ cgi crand.gds"
    exec cgi crand.gds >& tmp.log
    set fp [open tmp.log r]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
    file delete tmp.log
}
}

puts $f1 "STEP 2b."
puts {
*********************************************************************
2b. You can list the contents of the project and show the hierarchical
    layout tree with the dblist command:

    % dblist -h
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ dblist -h"
    exec dblist -h >& tmp.log
    set fp [open tmp.log r]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
    file delete tmp.log
puts {
You see 4 hierarchical levels. The top cell crand has 1 sub-cell.
This counter sub-cell has again 4 sub-cells.}
}

puts $f1 "STEP 2c."
puts {
*********************************************************************
2c. Now, we can inspect the layout with the layout editor/viewer dali.
    To show the layout, we use the following command:

    % dali crand

    To see more layout levels, use the number keys ('2','3' or '4').
    Use command 'read_cell' in the "DB_menu" to inspect the sub-cells.
    To exit the program, click on '-quit-' and 'yes'.
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ dali crand"
    catch {exec dali crand}
}

puts $f1 "STEP 3."
puts {
*********************************************************************
3.  Perform a flat extraction of cell crand in batch mode. Extract
    also ground capacitances (option -c).  The other options are
    -v to use verbose mode and -F to use flat extraction.
    The command used is:

    % space -vFc crand
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ space -vFc crand"
    catch {exec space -vFc crand >& tmp.log}
    set fp [open tmp.log r]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
    file delete tmp.log
}

puts $f1 "STEP 3b."
puts {
*********************************************************************
3b. We can now inspect the extraction result, the extracted circuit.
    To see a SLS description (or netlist), we use the xsls
    command to eXstract SLS from the database:

    % xsls crand                       (see: icdman xsls)
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ xsls crand"
    catch {exec xsls crand >& tmp.log}
    set fp [open tmp.log r]
    while {[gets $fp line] >= 0} { puts $line }
    close $fp
    file delete tmp.log
}

puts $f1 "STEP 4."
puts {
*********************************************************************
4.  Start the simulation GUI simeye and run a sls simulation.

    % simeye
*********************************************************************
4b. Click on the "Simulate" menu and choice the "Prepare" item.
    Select in the "Circuit:" field cell name "crand" and
    in the "Stimuli:" field file name "crand.cmd" (click on it).
    To inspect or edit the input signals, click on the "Edit" button.
*********************************************************************
5.  To run a sls simulation:
    Go back to the "Simulate -> Prepare" dialog and choice
    simulation "Type: sls-timing" and for "Read: Analog".
    Perform a switch-level timing simulation by clicking button
    "Run" and watch the simulation results.
*********************************************************************
5b. To exit the simeye program:
    Go to the "File" menu and click on "Exit" and "Yes".
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ simeye"
    catch {exec simeye}
}

puts {
*********************************************************************
    Congratulations, you have now finished this demo!
*********************************************************************}
set ok [ask "Shall we clean the demo directory and exit?"]
if {$ok} {
    echo "+ rmpr -fs ."
    catch {exec rmpr -fs .}
    file delete exp_dat crand.out crand.plt crand.res sim.diag
}
