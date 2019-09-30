#!/usr/bin/env tclsh
# directory: demo/suboscil

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
**  using a CMOS Ring Oscillator.  See also section "5.5" in the   **
**   "Space Substrate Resistance Extraction User's Manual".        **
*********************************************************************
First using option -b (the fast interpolation method)
Second using option -B (the more accurate 3D BE-method)

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
    We change the cwd into a project directory with the following command:

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
    The oscillator layout is available in a GDS2 file.
    We use cgi (convert-gds-internal) to put the layout into the project
    database with the following command:

    % cgi oscil.gds
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ cgi oscil.gds"
    catch {exec cgi oscil.gds >& tmp.log}
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
}
}

puts {
*********************************************************************
    You can show the hierarchical layout tree with the dblist command:

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
You see 2 hierarchical levels. The top cell 'oscil' has 9 sub-cells.
These sub-cells have the name 'inv' and are invertors.}
}

puts $f1 "STEP 3."
puts {
*********************************************************************
STEP 3.
    You can use the layout editor dali to inspect the layout.
    We do the following command:

    % dali oscil

Note: To see more levels, click on 'DB_menu', then on 'all_exp', then
      on 'maximum' (or use a hotkey, for example '2' for 2 levels).
Note: To exit the program, click on '-return-' and '-quit-' (or
      use the hotkey 'q') and click on 'yes'.
(the hotkeys work only if the mouse cursor is in the dali window)
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ dali oscil"
    if [catch {exec dali oscil} result] { echo $result }
}

puts $f1 "STEP 4."
puts {
*********************************************************************
4.  Perform an extraction of cell oscil in batch mode.  Extract
    substrate resistances (using option -b) and coupling capacitances
    (using option -C). The other options are: -v for verbose, -F for
    flat extraction and -P<file> to use an alternate parameter file.
    Note that the -F option is not really needed for substrate res
    extraction, because that is the default way to do it.
    The command used is:

    % space3d -vF -P param.p -bC oscil
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ space3d -vF -P param.p -bC oscil"
    catch {exec space3d -vF -P param.p -bC oscil >& tmp.log}
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
}

puts $f1 "STEP 4b."
puts {
*********************************************************************
4b. We can also perform an interactive extraction of cell oscil.
    Using the Xspace program with following command:

    % Xspace -P param.p
*********************************************************************
Note: Because the cell name is not specified, we must set the cell first:
    click on button 'oscil' in the 'Database' menu.
    And to set the extraction options, go to the 'Options' menu and
    click button <inter. sub. res.> and <coupling cap.>.
    (<inter. sub. res.> = interpolated substrate resistances)
    You must also set some display options (what you want to see), go to
    the 'Display' menu and click button 'DrawSubTerm' and 'DrawSubResistor'.
    Now, to start the extraction, go to the 'Extract' menu and click 'extract'
    (note that you can also use the hotkey 'e').  To leave the program,
    click on 'quit' in the 'Extract' menu (or use hotkey 'q').
*********************************************************************
Note: To show colored substrate terminals, click on 'FillSubTerm' in
the 'Display' menu and use hotkey 'a' to view the result 'again'.
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ Xspace -P param.p"
    if [catch {exec Xspace -P param.p} result] { echo $result }
}

puts $f1 "STEP 5."
puts {
*********************************************************************
5.  We can now inspect the extraction result, the extracted circuit.
    To see a SPICE description (or netlist), we use the xspice
    command to eXstract SPICE from the database:

    % xspice -a oscil                     (see: icdman xspice)
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ xspice -a oscil"
    catch {exec xspice -a oscil >& tmp.log}
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { puts $line }
    close $fp
}

puts $f1 "STEP 6."
puts {
*********************************************************************
6.  If you have a spice3 simulator available, you can perform a spice3
    simulation using the simulation GUI simeye.
Note: Check the shell script nspice in \$ICDPATH/share/bin to see
    if spice3 is called correctly.
    To start simeye, the following command is used:

    % simeye

Note: Click on the "Simulate" menu and choice item "Prepare".
     Select in the "Circuit:" field cell name "oscil" and
     in the "Stimuli:" field file name "oscil.cmd".
     Choice simulation "Type: spice" and click on button "Run".
     To leave the program, choice item "Exit" in the "File" menu.
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ simeye"
    if [catch {exec simeye} result] { echo $result }
}

puts $f1 "STEP 7."
puts {
*********************************************************************
7.  Perform an extraction of cell oscil in batch mode.
    Extract now substrate resistances using the BEM method.
    This method can only be used with the 3D space extractor.
    Thus, use space3d and use the option -B for BEM.

    % space3d -vF -P param.p -BC oscil
 or
    % tecc tech.s ; space3d -vF -P param.p -E tech.t -BC oscil

    (both circuit extractions must be equal)
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ space3d -vF -P param.p -BC oscil"
    catch {exec space3d -vF -P param.p -BC oscil >& tmp.log}
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
}

puts $f1 "STEP 7b."
puts {
*********************************************************************
7b. Perform also an interactive substrate BEM extraction:

    % Xspace -P param.p -BC oscil

Note: Click button "DrawBEMesh", "DrawSubTerm" and "DrawSubResistor" in
    the menu "Display", and use hotkey 'e' to "extract".
Use hotkey 'i' to zoom-in on cursor position and the arrow-keys to pan around.
Use hotkey 'o' to zoom-out and 'b' to set bounding-box view again.
You see 19 substrate contact terminals, but each has a very
fine boundary element mesh (BE-mesh).
*********************************************************************
Use hotkey 'q' to exit the Xspace program.
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ Xspace -P param.p -BC oscil"
    if [catch {exec Xspace -P param.p -BC oscil} result] { echo $result }
}

puts $f1 "STEP 8."
puts {
*********************************************************************
8.  We do step 7 again, but now using another technology file.
    We use the following command, to look what is different:

    % diff tech.s tech2.s
*********************************************************************}
set ok [ask "Shall we diff?"]
if {$ok} {
    echo "+ diff tech.s tech2.s"
    set fp [open t1t2.dif]
    while {[gets $fp line] >= 0} { puts $line }
    close $fp
}

puts $f1 "STEP 8."
puts {
*********************************************************************
8.  We have changed @gnd into @sub. Each @sub generates substrate
    contact terminal areas. Compile this technology file using command:

    % tecc tech2.s
*********************************************************************}
set ok [ask "Shall we compile?"]
if {$ok} {
echo "+ tecc tech2.s"
    catch {exec tecc tech2.s >& tmp.log}
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
    puts "Note that the warnings of swapping pins are ok."
}

puts $f1 "STEP 8."
puts {
*********************************************************************
8.  We use also another space parameter file "param2.p".
    The following command shows what is different:

    % diff param.p param2.p
*********************************************************************}
set ok [ask "Shall we diff?"]
if {$ok} {
    echo "+ diff param.p param2.p"
    set fp [open p1p2.dif]
    while {[gets $fp line] >= 0} { puts $line }
    close $fp
}

puts $f1 "STEP 8b."
puts {
*********************************************************************
8b. We have set parameter "sep_sub_term" to "on", because we want
    separate substrate terminals for the poly and metal areas (else
    everything is merged). Because we are now using a very large
    substrate area, we must reduce the number of BE elements (else
    the calculation costs too much time). Thus, we use a default
    "edge_be_ratio" and a "max_be_area" of 10 micron^2.
    We are also using a smaller "be_window" size of 4 micron.
    Perform an extraction of cell oscil in batch mode:

    % space3d -vF -P param2.p -E tech2.t -BC oscil
*********************************************************************}
set ok [ask "Shall we extract?"]
if {$ok} {
    echo "+ space3d -vF -P param2.p -E tech2.t -BC oscil"
    catch {exec space3d -vF -P param2.p -E tech2.t -BC oscil >& tmp.log}
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
}

puts $f1 "STEP 8c."
puts {
*********************************************************************
8c. Perform also an interactive extraction:

    % Xspace -P param2.p -E tech2.t -BC oscil

Note:
    Use directly 'e' to "extract", the parameter file contains
    the other needed settings to get a nice picture.
    Note that a -b extraction does not work, because at this moment
    the program makedela cannot handle these substrate areas laying
    against each other.
    Note that you can also make the substrate terminals distributed.
    Then you get more separate substrate terminal areas. But this
    works only for -B in combination with interconnect res extraction.
    And the causing conductors must have a high-ohmic sheet res value.
    Set parameter "low_sheet_res" to 0 ohm to make all conductors
    high-ohmic (note that conductor "cwn" cannot be set high-ohmic,
    because it has a zero sheet res value).
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ Xspace -P param2.p -E tech2.t -BC oscil"
    if [catch {exec Xspace -P param2.p -E tech2.t -BC oscil} result] { echo $result }
}

puts $f1 "STEP 8d."
puts {
*********************************************************************
8d. Perform the extraction with distributed interconnect res:

    % space3d -vF -P param2.p -E tech2.t -BCr oscil

    You see that there are a lot more substrate terminals.
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ space3d -vF -P param2.p -E tech2.t -BCr oscil"
    catch {exec space3d -vF -P param2.p -E tech2.t -BCr oscil >& tmp.log}
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
}

puts $f1 "STEP 8e."
puts {
*********************************************************************
8e. Perform also an interactive extraction:

    % Xspace -P param2.p -E tech2.t -BCr oscil

Note:
    After you have done 'e' to "extract", put button "DrawBEMesh"
    off to see only the substrate terminals with there borders.
    You do not need to extract again. Use hotkey 'a' to view the
    display output file again, but with-out the mesh.
*********************************************************************
Type 'q' to exit the Xspace program.
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ Xspace -P param2.p -E tech2.t -BCr oscil"
    if [catch {exec Xspace -P param2.p -E tech2.t -BCr oscil} result] { echo $result }
}

puts {
*********************************************************************
    Congratulations, you have now finished this demo!
*********************************************************************}
set ok [ask "Shall we clean the demo directory and exit?"]
if {$ok} {
    echo "+ rmpr -fs ."
    catch {exec rmpr -fs .}
    file delete display.out exp_dat oscil.spc tech.t tech2.t
    file delete sim.diag oscil.ana oscil.out oscil.plt oscil.res tmp.log
}
