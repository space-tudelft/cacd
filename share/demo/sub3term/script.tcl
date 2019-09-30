#!/usr/bin/env tclsh
# directory: demo/sub3term

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
**  Demonstration of Space Substrate Resistance Extraction         **
**  using 3 substrate terminals.  See also section "4.7" in the    **
**   "Space Substrate Resistance Extraction User's Manual".        **
*********************************************************************
Using option -B (the more accurate 3D BE-method)

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
    We shall use the scmos_n technology and a lambda of 0.1 micron.
    We change the current working directory '.' into a project directory
    with the following command:

    % mkpr -p scmos_n -l 0.1 .
*********************************************************************}
set ok [ask "Shall we do it? (type Enter)"]
if {!$ok} { puts "Ok, see you a next time!"; exit 1 }

echo "+ mkpr -p scmos_n -l 0.1 ."
catch {exec mkpr -p scmos_n -l 0.1 . >& tmp.log}
set fp [open tmp.log]
while {[gets $fp line] >= 0} { echo $line }
close $fp

puts $f1 "STEP 2."
puts {
*********************************************************************
STEP 2.
    The layout of the 3 terminals is available in a gds2 file.
    We use cgi (convert-gds-internal) to put the layout into the
    project database with the following command:

    % cgi sub3term.gds
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ cgi sub3term.gds"
    catch {exec cgi sub3term.gds >& tmp.log}
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
}
}

puts $f1 "STEP 3."
puts {
*********************************************************************
STEP 3.
    Now, we can inspect the layout with the layout editor dali.
    The layout is stored in the database using cell name "sub3term".
    To show the layout, we use the following command:

    % dali sub3term

    To exit the program, click on '-quit-' and click on 'yes'.
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ dali sub3term"
    if [catch {exec dali sub3term} result] { echo $result }
}

puts $f1 "STEP 4."
puts {
*********************************************************************
STEP 4.
    We use a local technology file for the extraction of the circuit.
    This element definition file needs to be compiled before usage.
    Use the technology compiler tecc to create the "elem.t" file.
    Use the following command:

    % tecc elem.s

    Note: The warning about a missing via mask is ok.
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ tecc elem.s"
    catch {exec tecc elem.s >& tmp.log}
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
}

puts $f1 "STEP 5."
puts {
*********************************************************************
5.  Perform an extraction of cell sub3term in batch mode.
    Extract substrate resistances using option -B.
    The other options are: -v for verbose, -E<file>
    to use an alternate element definition file,
    and -P<file> to use an alternate parameter file.
    The command used is:

    % space3d -v -E elem.t -P param.p -B sub3term
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ space3d -v -E elem.t -P param.p -B sub3term"
    catch {exec space3d -v -E elem.t -P param.p -B sub3term >& tmp.log}
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
}

puts $f1 "STEP 5b."
puts {
*********************************************************************
5b. We can also perform an interactive extraction of cell sub3term.
    Using the Xspace program with following command:

    % Xspace -E elem.t -P param.p
*********************************************************************
Note: Because the cell name is not specified, we must set the cell first:
    click on button 'sub3term' in the 'Database' menu.
    And to set the extraction options, go to the 'Options' menu and
    click button <3D sub. res.>.
    You must also set some display options (what you want to see), go to
    the 'Display' menu and click button 'DrawBEMesh' and 'DrawGreen'.
    Now, to start the extraction, go to the 'Extract' menu and click 'extract'
    (note that you can also use the hotkey 'e').  To leave the program,
    click on 'quit' in the 'Extract' menu (or use hotkey 'q').
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ Xspace -E elem.t -P param.p"
    if [catch {exec Xspace -E elem.t -P param.p} result] { echo $result }
}

puts $f1 "STEP 6."
puts {
*********************************************************************
6.  We can now inspect the extraction result, the extracted circuit.
    To see a SPICE description (or netlist), we use the xspice
    command to eXstract SPICE from the database:

    % xspice -a sub3term                  (see: icdman xspice)
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ xspice -a sub3term"
    catch {exec xspice -a sub3term >& tmp.log}
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { puts $line }
    close $fp
}

puts $f1 "STEP 7."
puts {
*********************************************************************
7.  Optionally, we can use Xspace in special prepass mode to view the
    substrate boundary element mesh and the Green functions calculated
    between the points in the mesh (not doing an extraction).
    Using the following command:

    % Xspace -%Z -B -E elem.t -P param.p sub3term
*********************************************************************
    Set the display options, go to the 'Display' menu and click
    button "DrawBEMesh" and "DrawGreen". Go to the 'Extract' menu
    and click "extract" (or hotkey 'e'). Type 'q' to exit.
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ Xspace -%Z -B -E elem.t -P param.p sub3term"
    if [catch {exec Xspace -%Z -B -E elem.t -P param.p sub3term} result] { echo $result }
}

puts {
*********************************************************************
    Congratulations, you have now finished this demo!
*********************************************************************}
set ok [ask "Shall we clean the demo directory and exit?"]
if {$ok} {
    echo "+ rmpr -fs ."
    catch {exec rmpr -fs .}
    file delete display.out exp_dat elem.t tmp.log
}
