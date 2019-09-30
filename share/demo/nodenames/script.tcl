#!/usr/bin/env tclsh
# directory: demo/switchbox

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
**    Demonstration of the switchbox nan3 example.                 **
**    Which demonstrates net naming using terminals.               **
*********************************************************************

*********************************************************************
Note: Type Enter on the keyboard, each time, to do a step.
      If you want to skip a step, type first a 'n'.
      If you want to stop the demo, type Ctrl-C.
*********************************************************************
}

set f1 [open script.log w]

if {[file exists .dmrc]} {
  echo "The current working directory seems to be a project directory."
  echo "This script first tries to remove the old project data."
  set ok [ask "Shall we restart this demo and do it? (type Enter or n)"]
  if {$ok} {
    echo "+ rmpr -fs ."
    catch {exec rmpr -fs .}
    file delete exp_dat .dblist_done
  }
} else {
  set ok [ask "Shall we start this demo? (type Enter for yes or n for no)"]
  if {!$ok} { puts "Bye."; exit 1 }
}

if {$ok} {
puts {
*********************************************************************
STEP A.
    First, we need a new project directory to work with.
    We shall use the scmos_n technology and a lambda of 1 micron.
    We change the current working directory '.' into a project directory
    with the following command (see icdman mkpr):

    % mkpr -p scmos_n -l 1 .
*********************************************************************}
set ok [ask "Shall we do it? (type Enter)"]
if {!$ok} { puts "Bye."; exit 1 }
echo "+ mkpr -p scmos_n -l 1 ."
catch {exec mkpr -p scmos_n -l 1 . >& tmp.log}
set fp [open tmp.log]
while {[gets $fp line] >= 0} { echo $line }
close $fp
}

if {![file exist layout/switchbox4]} {
puts {
*********************************************************************
STEP B.
    The layout of cell nan3 is available in a GDS2 file.
    We use cgi (convert-gds-internal) to put the layout into the
    project database with the following command:

    % cgi ../switchbox/switchbox4.gds
*********************************************************************}
set ok [ask "Shall we do it?"]
if {!$ok} { puts "Bye."; exit 1 }
echo "+ cgi ../switchbox/switchbox4.gds"
catch {exec cgi ../switchbox/switchbox4.gds >& tmp.log}
set fp [open tmp.log]
while {[gets $fp line] >= 0} { echo $line }
close $fp
}

if {![file exist .dblist_done]} {
puts {
*********************************************************************
STEP C.
    You can list the contents of the project and show the hierarchical
    layout tree with the dblist command:

    % dblist -h
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ dblist -h"
    catch {exec dblist -h >& .dblist_done}
    set fp [open .dblist_done]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
puts {
You see 3 hierarchical levels. The top cell switchbox4 has 4 sub-cells.
This dec1of4 cell has also a number of sub-cells (leaf cells).}
}
}

puts {
*********************************************************************
1.  Do you want to show the layout of the leaf cell nan3 with the
    layout editor/viewer dali?  To do so, type:

    % dali nan3

    To exit the program, click on '-quit-' and click on 'yes'.
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ dali nan3"
    if {[catch {exec dali nan3} result]} { echo $result }
}

puts {
*********************************************************************
2.  Perform an extraction of the layout of cell nan3.
    We use the verbose mode (option -v) to see what the program is doing.
    We use the following command to extract the citcuit from the layout:

    % space -v nan3
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
echo "+ space -v nan3"
catch {exec space -v nan3 >& tmp.log}
set fp [open tmp.log]
while {[gets $fp line] >= 0} { echo $line }
close $fp

puts {
*********************************************************************
    You see that the extraction result contains 8 nodes and 6 mos
    transistors.
    There are 3 p-enhancement (penh) and 3 n-enhancement (nenh) tors.
*********************************************************************
2b. We shall list the contents of the project database, using:

    % dblist -dh
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
echo "+ dblist -dh"
catch {exec dblist -dh >& tmp.log}
set fp [open tmp.log]
while {[gets $fp line] >= 0} { echo $line }
close $fp
}

puts {
*********************************************************************
2c. We shall now inspect the extracted circuit. To see a 
    SPICE description (or netlist), we use the xspice command with
    the following options to eXstract SPICE from the database:

    % xspice -aou nan3               (see: icdman xspice)
*********************************************************************
    Note: With option -a you get node names and terminal names.
    With option -u you do not get automatically addition
    of terminals for bulk connections (with names: pbulk and nbulk).
    With option -o you suppress the output of SPICE models.
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
echo "+ xspice -aou nan3"
catch {exec xspice -aou nan3 >& tmp.log}
set fp [open tmp.log]
while {[gets $fp line] >= 0} { puts $line }
close $fp
}

puts {
*********************************************************************
2d. You can also eXstract the netlist as a SLS description, type:

    % xsls nan3
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
echo "+ xsls nan3"
catch {exec xsls nan3 >& tmp.log}
set fp [open tmp.log]
while {[gets $fp line] >= 0} { puts $line }
close $fp
}

puts {
*********************************************************************
2e. If you wish, you can also list the internal database streams
    of the nan3 circuit view, type:

    % dbcat -c nan3
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
echo "+ dbcat -c nan3"
catch {exec dbcat -c nan3 >& tmp.log}
set fp [open tmp.log]
while {[gets $fp line] >= 0} { puts $line }
close $fp
}
}

puts {
*********************************************************************
3.  We can also extract a circuit with resistances, type:

    % space -rv nan3
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
echo "+ space -rv nan3"
catch {exec space -rv nan3 >& tmp.log}
set fp [open tmp.log]
while {[gets $fp line] >= 0} { echo $line }
close $fp

puts {
*********************************************************************
    You see that the extraction result contains 14 resistances and
    19 nodes and 6 mos transistors.  You see, because of the
    resistances, that there are now more local nodes.
*********************************************************************
3b. To see the SLS description of the result, type:

    % xsls nan3
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
echo "+ xsls nan3"
catch {exec xsls nan3 >& tmp.log}
set fp [open tmp.log]
while {[gets $fp line] >= 0} { puts $line }
close $fp
}

puts {
*********************************************************************
    You see that the local nodes do not have names, but node numbers.
*********************************************************************
3c. You can also inspect the internal net stream, type:

    % dbcat -cs net nan3
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
echo "+ dbcat -cs net nan3"
catch {exec dbcat -cs net nan3 >& tmp.log}
set fp [open tmp.log]
while {[gets $fp line] >= 0} { puts $line }
close $fp
}
}

puts {
*********************************************************************
4.  If you are not satisfied with local node numbers, when extracting
    resistances with the space layout to circuit extractor. Then you
    must know that space has the possibility to extract node names
    based on terminals in the same conductor group. To do so, you
    must use the parameter "term_is_netname", type:

    % space -rv -Sterm_is_netname nan3
*********************************************************************
    Note: You can also specify option -S as follows:
    % space -rv -Sterm_is_netname=on nan3
    But you can left out '=on', because it is the default.
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
echo "+ space -rv -Sterm_is_netname nan3"
catch {exec space -rv -Sterm_is_netname nan3 >& tmp.log}
set fp [open tmp.log]
while {[gets $fp line] >= 0} { echo $line }
close $fp
puts {
*********************************************************************
    Note that in the result, terminal t_in_0 is chosen in stead of
    terminal b_in_0. This because of the x-position of the terminals.
    Note also, that not all local nets get net names, this because
    two nets are only between the nenh transistors.
*********************************************************************
4b. To see the resulting SLS description, type:

    % xsls nan3
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
echo "+ xsls nan3"
catch {exec xsls nan3 >& tmp.log}
set fp [open tmp.log]
while {[gets $fp line] >= 0} { puts $line }
close $fp
}
puts {
*********************************************************************
4c. To see the internal net stream, type:

    % dbcat -cs net nan3
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ dbcat -cs net nan3"
    catch {exec dbcat -cs net nan3 >& tmp.log}
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { puts $line }
    close $fp
}
}

puts {
*********************************************************************
 GENERAL REMARK: Actually, default space is using labels for the
 net naming. And because there are no labels in the layout, you must
 use parameter "term_is_netname", to make them equally to labels
 to be used for the net naming.  However, note that always the first
 label/terminal of the cell found (x,y-position) is used.
 Note: If you do not want to use the labels, you can specify
       parameter "no_labels".
*********************************************************************
    Congratulations, you have now finished this demo!
*********************************************************************}
set ok [ask "Shall we clean the demo directory and exit?"]
if {$ok} {
    echo "+ rmpr -fs ."
    catch {exec rmpr -fs .}
    file delete exp_dat .dblist_done tmp.log
}
