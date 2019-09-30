#!/usr/bin/env tclsh
# directory: demo/sram

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
**  Demonstration of Space 3D Capacitance Extraction               **
**  using a CMOS static RAM cell.  See also section "5.2" in the   **
**   "Space 3D Capacitance Extraction User's Manual".              **
*********************************************************************

Note: Type Enter on the keyboard, each time, to do a step.
      If you want to skip a step, type first a 'n'.
      If you want to stop the demo, type Ctrl-C.
*********************************************************************
This script first tries to remove old project data.
*********************************************************************}
set ok [ask "Shall we start this demo? (type Enter)"]
if {!$ok} { puts "Ok, see you another time!"; exit 1 }

set f1 [open script.log w]

if [file exist .dmrc] {
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
    We shall use the scmos_n technology and a lambda of 0.25 micron.
    We change the current working directory '.' into a project directory
    with the following command:

    % mkpr -p scmos_n -l 0.25 .
*********************************************************************}
set ok [ask "Shall we do it? (type Enter)"]
if {!$ok} { puts "Ok, see you a next time!"; exit 1 }

echo "+ mkpr -p scmos_n -l 0.25 ."
catch {exec mkpr -p scmos_n -l 0.25 . >& tmp.log}
set fp [open tmp.log]
while {[gets $fp line] >= 0} { echo $line }
close $fp

puts $f1 "STEP 2."
puts {
*********************************************************************
STEP 2.
    The sram layout is available in a gds2 file.
    We use cgi (convert-gds-internal) to put the layout into the
    project database with the following command:

    % cgi sram.gds
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ cgi sram.gds"
    catch {exec cgi sram.gds >& tmp.log}
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
}
}

puts {
*********************************************************************
    We can inspect the sram layout with the layout editor dali.
    We use the following command:

    % dali sram

    To exit the program, click on '-quit-' and click on 'yes'.
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
echo "+ dali sram"
if [catch {exec dali sram} result] { echo $result }
}

puts $f1 "STEP 3."
puts {
*********************************************************************
STEP 3.
    We use a local technology file for the extraction of the circuit.
    This element definition file needs to be compiled before usage.
    Use the technology compiler tecc to create the "sram.t" file.
    Use the following command:

    % tecc sram.s
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ tecc sram.s"
    catch {exec tecc sram.s >& tmp.log}
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
}

puts $f1 "STEP 4."
puts {
*********************************************************************
4.  Perform an extraction of cell sram in batch mode.  Extract
    3D capacitances (using option -C3).  The other options are:
       -E<file> to use an alternate element definition file,
       -P<file> to use an alternate parameter file.
       -v       to use the verbose mode.
    The command used is:

    % space3d -v -C3 -E sram.t -P sram.p sram
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ space3d -v -C3 -E sram.t -P sram.p sram"
    catch {exec space3d -v -C3 -E sram.t -P sram.p sram >& tmp.log}
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
}

puts $f1 "STEP 4b."
puts {
*********************************************************************
4b. We can also perform an interactive extraction of cell sram.
    Using the Xspace program with following command:

    % Xspace -E sram.t -P sram.p sram
*********************************************************************
    To set the extraction options, go to the 'Options' menu and
    click button <coupling cap.> and <3D capacitance>.
    You must also set some display options (what you want to see), go
    to the 'Display' menu and click button 'DrawBEMesh', 'DrawGreen'
    and '3 dimensional'.  Now, to start the extraction,
    go to the 'Extract' menu and click 'extract' (note that you
    can also use the hotkey 'e').  To exit the program,
    click on 'quit' in the 'Extract' menu (or use hotkey 'q').
*********************************************************************
    You can also rotate the 3D mesh with the keypad arrow keys.
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
echo "+ Xspace -E sram.t -P sram.p sram"
if [catch {exec Xspace -E sram.t -P sram.p sram} result] { echo $result }
}

puts $f1 "STEP 5."
puts {
*********************************************************************
5.  We can now inspect the extraction result, the extracted circuit.
    To see a SPICE description (or netlist), we use the xspice
    command to eXstract SPICE from the database:

    % xspice -a sram                      (see: icdman xspice)
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ xspice -a sram"
    catch {exec xspice -a sram >& tmp.log}
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { puts $line }
    close $fp
}

puts $f1 "STEP 6."
puts {
*********************************************************************
6.  Of coarse, you can also perform a circuit simulation.
    There is already a stimuli file supplied ("sram.cmd"), to do a
    SLS or SPICE simulation using the simulation GUI simeye.
    To start simeye, the following command is used:

    % simeye
*********************************************************************
    Click on the "Simulate" menu and choice item "Prepare".
    Select in the "Circuit:" field cell name "sram" and in the
    "Stimuli:" field file name "sram.cmd" and click on "Run".
    See: icdman sls, and see also: icdman simeye.
    You can also choice another simulation type in the "Type:" field.
    To leave the program, choice item "Exit" in the "File" menu.
*********************************************************************
Note:
    To perform a SPICE simulation, you need to have a spice3 simulator
    available.  Check the shell script nspice in $ICDPATH/share/bin
    to see if spice3 is called correctly.
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ simeye"
    if [catch {exec simeye} result] { echo $result }
}

puts {
*********************************************************************
    Congratulations, you have now finished this demo!
*********************************************************************}
set ok [ask "Shall we clean the demo directory and exit?"]
if {$ok} {
    echo "+ rmpr -fs ."
    catch {exec rmpr -fs .}
    file delete display.out exp_dat sram.spc sram.t tmp.log
    file delete sim.diag sram.ana sram.out sram.plt sram.res
}
