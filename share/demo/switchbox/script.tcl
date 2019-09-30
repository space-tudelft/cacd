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
**  Demonstration of the switchbox example from the                **
**       "Space Tutorial".                                         **
*********************************************************************

*********************************************************************
Note: Type Enter on the keyboard, each time, to do a step.
      If you want to skip a step, type first a 'n'.
      If you want to stop the demo, type Ctrl-C.
*********************************************************************
}
set ok [ask "Shall we start this demo? (type Enter for yes or n for no)"]
if {!$ok} { puts "Ok, see you another time!"; exit 1; }

set f1 [open script.log w]

if [file exist .dmrc] {
puts "The current working directory seems to be a project directory."
puts "This script first tries to remove the old project data."
set ok [ask "Shall we restart this demo and do it? (type Enter or n)"]
    if {$ok} {
	echo "+ rmpr -fs ."
	catch {exec rmpr -fs .}
	file delete exp_dat dblist.log dali.log
    }
}

if {$ok} {
puts $f1 "STEP A."
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
if {!$ok} { puts "Bye."; exit 1; }
echo "+ mkpr -p scmos_n -l 1 ."
catch {exec mkpr -p scmos_n -l 1 . >& tmp.log}
set fp [open tmp.log]
while {[gets $fp line] >= 0} { echo $line }
close $fp
}

if ![file exist layout/switchbox4] {
puts $f1 "STEP B."
puts {
*********************************************************************
STEP B.
    The layout of the switchbox4 is available in a gds2 file.
    We use cgi (convert-gds-internal) to put the layout into the
    project database with the following command:

    % cgi switchbox4.gds
*********************************************************************}
set ok [ask "Shall we do it?"]
if {!$ok} { puts "Bye."; exit 1; }
echo "+ cgi switchbox4.gds"
catch {exec cgi switchbox4.gds >& tmp.log}
set fp [open tmp.log]
while {[gets $fp line] >= 0} { echo $line }
close $fp
}

if ![file exist dblist.log] {
puts $f1 "STEP C."
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
    catch {exec dblist -h >& dblist.log}
    set fp [open dblist.log]
    while {[gets $fp line] >= 0} { puts $line }
    close $fp
puts {
You see 3 hierarchical levels. The top cell switchbox4 has 4 sub-cells.
This dec1of4 cell has also a number of sub-cells (leaf cells).}
}
}

if ![file exist dali.log] {
puts $f1 "STEP D."
puts {
*********************************************************************
STEP D.
    Now, we can inspect the layout with the layout editor/viewer dali.

    % dali

    Use the command 'read_cell' in the "DB_menu" to read the
    different cells.  To see more levels, use hotkey '2' or '3'.
    To exit the program, use hotkey 'q' and click on 'yes'.
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ dali"
    if [catch {exec dali >& dali.log} result] { echo $result }
}
}

puts $f1 "STEP 1."
puts {
*********************************************************************
1.  Perform a hierarchical extraction of the layout of cell switchbox4.
    We use the verbose mode (option -v) to see what the program is doing.
    The following command shall be used:

    % space -v switchbox4
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ space -v switchbox4"
    catch {exec space -v switchbox4 >& tmp.log}
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp

puts $f1 "STEP 1b."
puts {
*********************************************************************
1b. We shall list the contents of the project again, using:

    % dblist -h
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ dblist -h"
    catch {exec dblist -h >& tmp.log}
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { puts $line }
    close $fp
}

puts $f1 "STEP 1c."
puts {
*********************************************************************
1c. We shall now inspect the extracted circuits. To see a hierarchical
    SPICE description (or netlist), we use the xspice command with
    option -h to eXstract SPICE from the database:

    % xspice -ah switchbox4               (see: icdman xspice)
*********************************************************************
    Note: With option -a you get node names and terminal names.
    Note: Use option -u, if you do not want automatically addition
    of terminals for bulk connections (with names: pbulk and nbulk).

    Note that hierarchical extraction works fine here because
    the terminals are rectangles that abut.  When the terminals
    are just points (e.g. when they originate from gds text
    structures and not from gds boundary elements with a property),
    they must have an overlap at a higher cell level in order to
    connect them to other parts of the circuit.
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ xspice -ah switchbox4"
    catch {exec xspice -ah switchbox4 >& tmp.log}
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { puts $line }
    close $fp
}
}

puts $f1 "STEP 2."
puts {
*********************************************************************
2.  Perform a flat extraction of the layout of cell switchbox4.
    Use option -F to enable the flat mode and type:

    % space -vF switchbox4
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ space -vF switchbox4"
    catch {exec space -vF switchbox4 >& tmp.log}
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp

puts $f1 "STEP 2b."
puts {
*********************************************************************
2b. List the contents of the project database again.
    You can use option -d to list the separate device calls.

    % dblist -hd
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ dblist -hd"
    catch {exec dblist -hd >& tmp.log}
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { puts $line }
    close $fp
}

puts $f1 "STEP 2c."
puts {
*********************************************************************
2c. To retrieve the switchbox4 circuit SPICE netlist, type:

    % xspice -aou switchbox4
*********************************************************************
    Note that option -o is used to omit model definitions for devices
    and that option -u is used to omit the bulk terminals.
    Option -h is not more needed, because there are no sub-cells.
    However, there are 144 devices (mos transistors).
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ xspice -aou switchbox4"
    catch {exec xspice -aou switchbox4 >& tmp.log}
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { puts $line }
    close $fp
}
}

puts $f1 "STEP 3."
puts {
*********************************************************************
3.  Perform flat extractions of the layout of cell switchbox4 with
    a capacitance or a resistance extraction option.
    Try out:
    - option -c: capacitance extraction to GND
    - option -C: couple capacitance extraction
    - option -r: resistance extraction
    - option -z: resistance extraction with mesh refinement

    % space -vFc switchbox4
    % xspice -aou switchbox4
    ...
*********************************************************************}
set ok [ask "Shall we do these try outs?"]
if {$ok} {
    set ok [ask "Shall we do -c?"]
    if {$ok} {
	echo "+ space -vFc switchbox4"
	catch {exec space -vFc switchbox4 >& tmp.log}
	set fp [open tmp.log]
	while {[gets $fp line] >= 0} { echo $line }
	close $fp
	ask "Ok?"
	echo "+ xspice -aou switchbox4"
	catch {exec xspice -aou switchbox4 >& tmp.log}
	set fp [open tmp.log]
	while {[gets $fp line] >= 0} { puts $line }
	close $fp
    }
    set ok [ask "Shall we do -C?"]
    if {$ok} {
	echo "+ space -vFC switchbox4"
	catch {exec space -vFC switchbox4 >& tmp.log}
	set fp [open tmp.log]
	while {[gets $fp line] >= 0} { echo $line }
	close $fp
	ask "Ok?"
	echo "+ xspice -aou switchbox4"
	catch {exec xspice -aou switchbox4 >& tmp.log}
	set fp [open tmp.log]
	while {[gets $fp line] >= 0} { puts $line }
	close $fp
    }
    set ok [ask "Shall we do -r?"]
    if {$ok} {
	echo "+ space -vFr switchbox4"
	catch {exec space -vFr switchbox4 >& tmp.log}
	set fp [open tmp.log]
	while {[gets $fp line] >= 0} { echo $line }
	close $fp
	ask "Ok?"
	echo "+ xspice -aou switchbox4"
	catch {exec xspice -aou switchbox4 >& tmp.log}
	set fp [open tmp.log]
	while {[gets $fp line] >= 0} { puts $line }
	close $fp
    }
    set ok [ask "Shall we do -z?"]
    if {$ok} {
	echo "+ space -vFz switchbox4"
	catch {exec space -vFz switchbox4 >& tmp.log}
	set fp [open tmp.log]
	while {[gets $fp line] >= 0} { echo $line }
	close $fp
	ask "Ok?"
	echo "+ xspice -aou switchbox4"
	catch {exec xspice -aou switchbox4 >& tmp.log}
	set fp [open tmp.log]
	while {[gets $fp line] >= 0} { puts $line }
	close $fp
    }
}

puts $f1 "STEP 4."
puts {
*********************************************************************
4.  Note that the layout of cell dec1of4 contains a label test.
    Labels of the top cell are always used as net names, unless
    space parameter no_labels is specified.

    % space -v dec1of4
    % xspice -a dec1of4
*********************************************************************
    Note: Terminal names have precedence in SPICE listings. Thus, when
    a terminal is in the same net as the label, the terminal is shown.
    This is not the case in SLS listings (net statements are used).
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ space -v dec1of4"
    catch {exec space -v dec1of4 >& tmp.log}
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
    echo "+ xspice -a dec1of4"
    catch {exec xspice -a dec1of4 >& tmp.log}
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { puts $line }
    close $fp
}

puts $f1 "STEP 5."
puts {
*********************************************************************
5.  When performing a flat extraction for switchbox4, the label
    'test' may be inherited from each sub-cell dec1of4 when setting
    the parameter hier_labels.   Note that the inherited label names
    use the sub-cell name, the original label name and the coordinates
    of the labels (because there are no cell instance names).
    Note that you can force this notation with parameter cell_pos_name.

    % space -vF -Shier_labels switchbox4
    % xspice -aou switchbox4
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ space -vF -Shier_labels switchbox4"
    catch {exec space -vF -Shier_labels switchbox4 >& tmp.log}
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
    echo "+ xspice -aou switchbox4"
    catch {exec xspice -aou switchbox4 >& tmp.log}
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { puts $line }
    close $fp
}

puts $f1 "STEP 6."
puts {
*********************************************************************
6.  In place of hier_labels you can also use hier_terminals.
    When both are used, the labels of terminals has precedence.
    Because the sub-cells have no instance names, the notation of
    space parameter cell_pos_name is used.

    % space -F -Shier_terminals switchbox4
    % xspice -aou switchbox4
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ space -F -Shier_terminals switchbox4"
    catch {exec space -F -Shier_terminals switchbox4 >& tmp.log}
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
    echo "+ xspice -aou switchbox4"
    catch {exec xspice -aou switchbox4 >& tmp.log}
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { puts $line }
    close $fp
}

puts $f1 "STEP 7."
puts {
*********************************************************************
7.  Note that you can also make a dump of the net file of the database.
    For more information, see the manual page with: icdman dbcat.

    % dbcat -cs net switchbox4

    Note: We list only the first 40 lines.
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ dbcat -cs net switchbox4"
    catch {exec dbcat -cs net switchbox4 >& tmp.log}
    set fp [open tmp.log]
    set n 0
    while {[gets $fp line] >= 0 && $n < 40} { puts $line; incr n }
    close $fp
}

puts $f1 "STEP 8."
puts {
*********************************************************************
8.  When you want to use space parameter leaf_terminals, you need to
    have leaf cells.  By a flat extraction that is normally not the
    case.  However, by a hierarchical extraction the sub-cells are
    also the leaf cells.  We shall see what happens with dec1of4.

    % space -Sleaf_terminals dec1of4
    % xspice -a dec1of4
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ space -Sleaf_terminals dec1of4"
    catch {exec space -Sleaf_terminals dec1of4 >& tmp.log}
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
    echo "+ xspice -a dec1of4"
    catch {exec xspice -a dec1of4 >& tmp.log}
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { puts $line }
    close $fp
}

puts $f1 "STEP 9."
puts {
*********************************************************************
9.  You can give the leaf cells instance names.  After that
    you can see the leaf_terminal names in another format.
    To add instance names, start dali and click on the inst_menu.
    Click on menu item name_inst and click in the bounding box of
    the sub-cell dubinv and type an instance name (for example di)
    and finish your input with the Enter key.
    Repeat this for the four nan3 sub-cells (for example n0 - n3)
    and for the nan4rout sub-cell (for example n4).
    When you are ready, write the modified cell back to the database.
    Click on -return-, DB_menu, write_cell and dec1of4.
    To leave the program, click on -return-, -quit- and yes.

    % dali dec1of4
    % space -Sleaf_terminals dec1of4
    % xspice -a dec1of4
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ dali dec1of4"
    if [catch {exec dali dec1of4} result] { echo $result }
    echo "+ space -Sleaf_terminals dec1of4"
    catch {exec space -Sleaf_terminals dec1of4 >& tmp.log}
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
    echo "+ xspice -a dec1of4"
    catch {exec xspice -a dec1of4 >& tmp.log}
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { puts $line }
    close $fp
}

puts $f1 "STEP 10."
puts {
*********************************************************************
10. To get leaf_terminals for a flat extraction.  You must specify
    which cells are leaf cells (often done in standard cell approach).
    Use the xcontrol program to declare that a number of sub-cells
    must be taken as device instances (and do a flat extraction).

    % xcontrol -device dubinv nan3 nan4rout
    % space -F -Sleaf_terminals dec1of4
    % xspice -a dec1of4
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ xcontrol -device dubinv nan3 nan4rout"
    catch {exec xcontrol -device dubinv nan3 nan4rout >& tmp.log}
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
    echo "+ space -F -Sleaf_terminals dec1of4"
    catch {exec space -F -Sleaf_terminals dec1of4 >& tmp.log}
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
    echo "+ xspice -a dec1of4"
    catch {exec xspice -a dec1of4 >& tmp.log}
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { puts $line }
    close $fp
}

puts $f1 "STEP 11."
puts {
*********************************************************************
11. You can also use the xcontrol program to get a list of the status
    of each cell (for other options, see icdman xcontrol).  Type:

    % xcontrol -list
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ xcontrol -list"
    catch {exec xcontrol -list >& tmp.log}
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { puts $line }
    close $fp
}

puts {
*********************************************************************
    The following demonstrates the use of
    the circuit comparison program match.
*********************************************************************}
set ok [ask "Shall we go on?"]
if {$ok} {
puts $f1 "STEP 12."
puts {
*********************************************************************
12. Add a reference circuit description for the switchbox4 circuit
    to the database using the program cspice.

    % cspice swbox_ref.spc
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ cspice swbox_ref.spc"
    catch {exec cspice swbox_ref.spc >& tmp.log}
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
}

puts $f1 "STEP 13."
puts {
*********************************************************************
13. Use cgi to put a flat version of the switchbox4 layout into the
    database and extract the circuit.

    % cgi switchbox4_f.gds
    % space switchbox4_f
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ cgi switchbox4_f.gds"
    catch {exec cgi switchbox4_f.gds >& tmp.log}
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
    echo "+ space switchbox4_f"
    catch {exec space switchbox4_f >& tmp.log}
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
}

puts $f1 "STEP 14."
puts {
*********************************************************************
14. Use the program match to compare the extracted circuit against the
    reference circuit.

    % match swbox_ref switchbox4_f

    The result shows that the circuits are identical.
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ match swbox_ref switchbox4_f"
    catch {exec match swbox_ref switchbox4_f >& tmp.log}
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { puts $line }
    close $fp
}

puts $f1 "STEP 15."
puts {
*********************************************************************
15. Make an error in the layout of switchbox4_f using dali (e.g. remove
    some metal) and run the programs space and match again.

    % dali switchbox4_f
*********************************************************************
    Click "box_menu", click on the "cmf" mask, click 'del_box' and
    click in the layout nearby a "cmf" part that you want to remove.
    (Note: To zoom-in, you can use hotkey 'i'.)
    Move the pointer and click again. To write the cell, click '-return-'
    and click on "DB_menu" and 'write_cell' and "switchbox4_f".
    To quit the program use hotkey 'q' and click 'yes'.
*********************************************************************
    % space switchbox4_f
    % match swbox_ref switchbox4_f

    The result shows that the circuits are not identical.
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ dali switchbox4_f"
    if [catch {exec dali switchbox4_f} result] { echo $result }
    echo "+ space switchbox4_f"
    catch {exec space switchbox4_f >& tmp.log}
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
    echo "+ match swbox_ref switchbox4_f"
    catch {exec match swbox_ref switchbox4_f >& tmp.log}
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { puts $line }
    close $fp
}

puts $f1 "STEP 16."
puts {
*********************************************************************
16. Use the option -fullbindings to see which network parts are matched.

    % match -fullbindings swbox_ref switchbox4_f
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ match -fullbindings swbox_ref switchbox4_f"
    catch {exec match -fullbindings swbox_ref switchbox4_f >& tmp.log}
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { puts $line }
    close $fp
}

puts {
*********************************************************************
The following demonstrates the use of the back-annotation program
highlay in combination with the circuit comparison program match.
Assume that the reference circuit "swbox_ref" has been added to the
database and assume that that an error is present in the layout of
"switchbox4_f" (see the steps above).
*********************************************************************}
set ok [ask "Shall we go on?"]

puts $f1 "STEP 17."
puts {
*********************************************************************
17. Run the program space with parameter -Sbackannotation=2 and
    run the program match with the options -edif and -fullbindings.
    The last two options cause match to write a binding table into
    the database that can be used as input for highlay.

    % space -Sbackannotation=2 switchbox4_f
    % match -edif -fullbindings swbox_ref switchbox4_f
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ space -Sbackannotation=2 switchbox4_f"
    catch {exec space -Sbackannotation=2 switchbox4_f >& tmp.log}
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
    echo "+ match -edif -fullbindings swbox_ref switchbox4_f"
    catch {exec match -edif -fullbindings swbox_ref switchbox4_f >& tmp.log}
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { puts $line }
    close $fp
}

puts $f1 "STEP 18."
puts {
*********************************************************************
18. The nets that have not been matched (the deficient nets) are selected
    for high-lighting by running the program highlay as follows:

    % highlay -d -n -v switchbox4_f
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ highlay -d -n -v switchbox4_f"
    catch {exec highlay -d -n -v switchbox4_f >& tmp.log}
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
}

puts $f1 "STEP 19."
puts {
*********************************************************************
19. Next, use dali to read in the cell HIGH_OUT that has been
    generated by highlay and inspect the unmatched conductors.

    % dali HIGH_OUT

    In the "settings" menu, set 'hash mode' to 'hashed' (you can also
    use the mode toggle key 'd').  To see the sub-cells, higher the
    expand level with key '2'.  To quit use 'q' and click 'yes'.
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ dali HIGH_OUT"
    if [catch {exec dali HIGH_OUT} result] { echo $result }
}
}

puts {
*********************************************************************
    Congratulations, you have now finished this demo!
*********************************************************************}
set ok [ask "Shall we clean the demo directory and exit?"]
if {$ok} {
    echo "+ rmpr -fs ."
    catch {exec rmpr -fs .}
    file delete exp_dat dblist.log dali.log tmp.log
}
