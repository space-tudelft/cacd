#!/usr/bin/env tclsh
# directory: demo/poly5

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
**  using 5 parallel conductors.  See also section "5.1" in the    **
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
    We shall use the scmos_n technology and a lambda of 0.05 micron.
    We change the current working directory '.' into a project directory
    with the following command:

    % mkpr -p scmos_n -l 0.05 .
*********************************************************************}
set ok [ask "Shall we do it? (type Enter)"]
if {!$ok} { puts "Ok, see you a next time!"; exit 1 }

echo "+ mkpr -p scmos_n -l 0.05 ."
catch {exec mkpr -p scmos_n -l 0.05 . >& tmp.log}
set fp [open tmp.log]
while {[gets $fp line] >= 0} { echo $line }
close $fp

puts $f1 "STEP 2."
puts {
*********************************************************************
STEP 2.
    The layout of the 5 conductors is available in a gds2 file.
    We use cgi (convert-gds-internal) to put the layout into the
    project database with the following command:

    % cgi poly5.gds
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ cgi poly5.gds"
    catch {exec cgi poly5.gds >& tmp.log}
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
}
}

puts $f1 "STEP 2b."
puts {
*********************************************************************
2b. We can inspect the layout with the layout editor dali.
    The layout is stored in the database using cell name "poly5".
    To show the layout, we use the following command:

    % dali poly5

    To exit the program, click on '-quit-' and click on 'yes'.
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ dali poly5"
    if [catch {exec dali poly5} result] { echo $result }
}

puts $f1 "STEP 3."
puts {
*********************************************************************
3.  We use a local technology file for the extraction of the circuit.
    This element definition file needs to be compiled before usage.
    Use the technology compiler tecc to create the "tech.t" file.
    Use the following command:

    % tecc tech.s
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ tecc tech.s"
    catch {exec tecc tech.s >& tmp.log}
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
}

puts $f1 "STEP 4."
puts {
*********************************************************************
4.  Perform an extraction of cell poly5 in batch mode.  Extract
    3D capacitances (using option -C3).  The other options are:
       -E<file> to use an alternate element definition file,
       -P<file> to use an alternate parameter file.
    The command used is:

    % space3d -C3 -E tech.t -P param.p poly5
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ space3d -C3 -E tech.t -P param.p poly5"
    catch {exec space3d -C3 -E tech.t -P param.p poly5 >& tmp.log}
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
}

puts $f1 "STEP 4b."
puts {
*********************************************************************
4b. We can also perform an interactive extraction of cell poly5.
    Using the Xspace program with following command:

    % Xspace -C3 -E tech.t -P param.p poly5
*********************************************************************
Note:
    You must set some display options (what you want to see), go
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
    echo "+ Xspace -C3 -E tech.t -P param.p poly5"
    if [catch {exec Xspace -C3 -E tech.t -P param.p poly5} result] { echo $result }
}

puts $f1 "STEP 5."
puts {
*********************************************************************
5.  We can now inspect the extraction result, the extracted circuit.
    To see a SPICE description (or netlist), we use the xspice
    command to eXstract SPICE from the database:

    % xspice -a poly5                     (see: icdman xspice)
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ xspice -a poly5"
    catch {exec xspice -a poly5 >& tmp.log}
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { puts $line }
    close $fp
}

puts {
*********************************************************************
    Congratulations, you have now finished this demo!
*********************************************************************}
set ok [ask "Shall we clean the demo directory and exit?"]
if {$ok} {
    echo "+ rmpr -fs ."
    catch {exec rmpr -fs .}
    file delete display.out exp_dat tech.t tmp.log
}