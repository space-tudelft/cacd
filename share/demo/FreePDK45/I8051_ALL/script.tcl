#!/usr/bin/env tclsh
# directory: demo/FreePDK45/I8051_ALL

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
**  Demonstration of a flat Space Extraction to basic cell nivo    **
**  of a 8051 processor made with the FreePDK45 process.           **
*********************************************************************

Note: Type Enter on the keyboard, each time, to do a step.
      If you want to skip a step, type first a 'n'.
      If you want to stop the demo, type Ctrl-C.
*********************************************************************
This script first tries to remove old project data.
*********************************************************************}

set ok [ask "Shall we start this demo? (type Enter)"]
if {!$ok} { puts "Ok, see you a next time!\n"; exit 1 }

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
    To use all basic cells of the NangateOpenCellLibrary we must import
    them in our project.  Use the following command:

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
    The 8051 processor layout is available in a gds2 file.
    We use cgi (convert-gds-internal) to put the layout into the
    project database.  We use a non-default basic mask list "bmlist3.gds"
    for the layer numbers.  Thus, we use the following command:

    % cgi I8051_ALL.gds -m bmlist3.gds
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ cgi I8051_ALL.gds -m bmlist3.gds"
    catch {exec cgi I8051_ALL.gds -m bmlist3.gds >& tmp.log}
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
    We can inspect the processor layout with the layout editor dali.
    We shall use the following command:

    % dali I8051_ALL

    To see an extra hierarchical level, type '2'. Use the 'i' key
    to zoom-in and the arrow keys. Use 'b' to get the bbox-window.
    To exit the program, click on '-quit-' and click on 'yes'.
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ dali I8051_ALL"
    catch {exec dali I8051_ALL}
}

puts $f1 "STEP 6."
puts {
*********************************************************************
STEP 6.
    Now, perform a flat netlist extraction of the I8051 layout.  Note
    that the basic cells are not extracted, because they are library cells.
    Type the following command to used the space extractor:

    % space -Fv I8051_ALL
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ space -Fv I8051_ALL"
    catch {exec space -Fv I8051_ALL >& tmp.log}
    set fp [open tmp.log r]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
    file delete tmp.log
}

puts {
*********************************************************************
NOTE:
    When you see the message "makegln: Temp file compression turned off"
    then this is ok, because not always the compression tools are installed.
    In the statis report you see that there are 606 d/s capacitances
    extracted. This is strange, because there are no transistors extracted.
    We can fix this problem by changing the space technology file.
    Change in the fets section the conditions for the dsarea caps.
    Change the following 2 lines:
	nenh:( ... ):poly active ( !nwell nimplant active !poly ):@sub
	penh:( ... ):poly active ( nwell pimplant active !poly ):nwell
    Into:
	nenh:( ... ):poly active ( !nwell nimplant !pimplant active !poly ):@sub
	penh:( ... ):poly active ( nwell !nimplant pimplant active !poly ):nwell
*********************************************************************}
puts $f1 "STEP 7."
puts {
*********************************************************************
STEP 7.
    We can now inspect the extraction result, the extracted circuit.
    To see a VHDL description (or netlist), we use the xvhdl
    command to eXstract VHDL from the database:

    % xvhdl -f I8051_ALL                     (see: icdman xvhdl)
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ xvhdl -f I8051_ALL"
    exec xvhdl -f I8051_ALL
    set fp [open I8051_ALL.vhd r]
    set n 0
    while {[gets $fp line] >= 0 && $n < 40} { puts $line; incr n }
    close $fp
    puts "(for the complete listing see I8051_ALL.vhd)"
}

puts $f1 "STEP 8."
puts {
*********************************************************************
STEP 8.
    When we want to perform a flat netlist extraction of the I8051 layout
    to the transistor level.  We need to change the status of the basic
    library cells into regular with the xcontrol program.

    % xcontrol -regular `cat libcells`

    Note that the libcells file contains a list of all library cells
    that must be changed (it contains not the FILLCELLs). The output of
    the UNIX cat command puts each cell name on the command line.
   (If you want to go back to basic cells level, use option -library.)
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ xcontrol -regular `cat libcells`"
    set fp [open libcells r]
    while {[gets $fp cell] >= 0} { exec xcontrol -regular $cell }
    close $fp
}

puts $f1 "STEP 9."
puts {
*********************************************************************
STEP 9.
    Now we can perform a flat netlist extraction of the I8051 layout to
    the transistor level.  We use the following command:

    % space -Fv -t I8051_ALL

    Note: We add the option -t to show the device positions.
          Don't use option -x, because it generates backannotation
          data for another program. This cost a lot of time for
          a large layout such as the I8051.
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ space -Fv -t I8051_ALL"
    catch {exec space -Fv -t I8051_ALL >& tmp.log}
    set fp [open tmp.log r]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
    file delete tmp.log
}

puts {
*********************************************************************
NOTE:
    You see that space has detected a big number of problem transistors.
    This happens, because the FILLCELLs make contacts on the power lines
    to the substrate and to the nwell area. There the active layer is
    used and this layer is sometimes touched by the poly layer of
    a basic cell.
    Note that you can instruct space to skip these transistors by
    specifying the parameter "omit_incomplete_tors".
    We can also fix this problem by changing the space technology file.
    Change in the fets section the conditions for the gate area.
    Change the following 2 lines:
	nenh:( !nwell pwell nimplant active poly ):poly active (..):@sub
	penh:( nwell !pwell pimplant active poly ):poly active (..):nwell
    Into:
	nenh:( !nwell pwell nimplant !pimplant active poly ):poly active (..):@sub
	penh:( nwell !pwell pimplant !nimplant active poly ):poly active (..):nwell
*********************************************************************}
set ok [ask "Shall we go on?"]
if {!$ok} { exit 1 }

puts $f1 "STEP 10."
puts {
*********************************************************************
STEP 10.
    Now we go to inspect the result, the extracted circuit.  We can
    generate for example a SLS or SPICE netlist description.  We choice
    in this case for a SLS netlist, because it is good readable.

    % xsls -f I8051_ALL

    Note: Output goes to "I8051_ALL.sls".
          We list only the first 40 lines.
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ xsls -f I8051_ALL"
    exec xsls -f I8051_ALL
    set fp [open I8051_ALL.sls r]
    set n 0
    while {[gets $fp line] >= 0 && $n < 40} { puts $line; incr n }
    close $fp
}

puts $f1 "STEP 11."
puts {
*********************************************************************
STEP 11.
    We shall also list a part of the SPICE netlist description.

    % xspice -f -au I8051_ALL

    Note: Output goes to "I8051_ALL.spc".
          We list only the first 40 lines.
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ xspice -f -au I8051_ALL"
    exec xspice -f -au I8051_ALL
    set fp [open I8051_ALL.spc r]
    set n 0
    while {[gets $fp line] >= 0 && $n < 40} { puts $line; incr n }
    close $fp
}

puts {
*********************************************************************
NOTE:
    You see that each transistor has a list of drain/source parameters.
    For example 'ad' is area-drain and 'pd' is perimeter-drain.
*********************************************************************

*********************************************************************
    Congratulations, you have now finished this demo!
*********************************************************************}

set ok [ask "Shall we clean the demo directory and exit?"]
if {$ok} {
    echo "+ rmpr -fs ."
    catch {exec rmpr -fs .}
    file delete exp_dat
}
