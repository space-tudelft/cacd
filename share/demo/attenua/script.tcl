#!/usr/bin/env tclsh
# directory: demo/attenua

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
**********************************************************************
** Demonstration of attenuator circuit in bipolar DIMES-01 process. **
** See also,                                                        **
**   "Low-voltage Low-power Controlled Attenuator for Hearing Aids" **
**   A. van Staveren and A.H.M. van Roermund, Electronic Letters,   **
**   22nd. July 1993, Vol. 29, No. 15, pp. 1355-1356.               **
**********************************************************************
After extraction, the circuit can be simulated using PSPICE.

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
    We shall use the dimes01 technology and a lambda of 0.1 micron.
    We change the current working directory '.' into a project directory
    with the following command:

    % mkpr -p dimes01 -l 0.1 .
*********************************************************************}
set ok [ask "Shall we do it? (type Enter)"]
if {!$ok} { puts "Ok, see you a next time!\n"; exit 1 }

echo "+ mkpr -p dimes01 -l 0.1 ."
exec mkpr -p dimes01 -l 0.1 . >& tmp.log
set fp [open tmp.log r]
while {[gets $fp line] >= 0} { echo $line }
close $fp
file delete tmp.log

puts $f1 "STEP 2."
puts {
*********************************************************************
STEP 2.
    The layout of the attenuator is available in a gds2 file.
    We use cgi (convert-gds-internal) to put the layout into the
    project database with the following command:

    % cgi attenua.gds
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ cgi attenua.gds"
    exec cgi attenua.gds >& tmp.log
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
    while {[gets $fp line] >= 0} { puts $line }
    close $fp
    file delete tmp.log
puts {
You see 3 hierarchical levels. The top cell attenua has 84 sub-cells.
The pnp33 sub-cell has 3 sub-cells.}
}

puts $f1 "STEP 2c."
puts {
*********************************************************************
2c. Now, we can inspect the layout with the layout editor/viewer dali.

    % dali

    Use the command 'read_cell' in the "DB_menu" to read the
    different cells.  To see more levels, use hotkey '2' or '3'.
    To exit the program, use hotkey 'q' and click on 'yes'.
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ dali"
    catch {exec dali}
}

puts $f1 "STEP 3."
puts {
*********************************************************************
3.  Perform now a flat extraction of cell attenua.
    We use verbose mode (option -v) to see what the program is
    doing and option -F for flat extraction.  The command is:

    % space -vF attenua
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ space -vF attenua"
    catch {exec space -vF attenua >& tmp.log}
    set fp [open tmp.log r]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
    file delete tmp.log
}

puts $f1 "STEP 3b."
puts {
*********************************************************************
3b. Shall we list the contents of the project again, using:

    % dblist -h
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ dblist -h"
    exec dblist -h >& tmp.log
    set fp [open tmp.log r]
    while {[gets $fp line] >= 0} { puts $line }
    close $fp
    file delete tmp.log
}

puts $f1 "STEP 4."
puts {
*********************************************************************
4.  Now, inspect the extracted attenua circuit. To see
    a SPICE description (or netlist), the xspice command with
    option -a is used to eXstract SPICE from the database:

    % xspice -a attenua                   (see: icdman xspice)
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ xspice -a attenua"
    catch {exec xspice -a attenua >& tmp.log}
    set fp [open tmp.log r]
    while {[gets $fp line] >= 0} { puts $line }
    close $fp
    file delete tmp.log
}

puts $f1 "STEP 4b."
puts {
*********************************************************************
4b. Try out the options -c, -C, -r and -z for capacitance and
    resistance extraction.

    % space -vFc attenua
    % xspice -a attenua
    ...
*********************************************************************}
set ok [ask "Shall we do -c?"]
if {$ok} {
    echo "+ space -vFc attenua"
    catch {exec space -vFc attenua >& tmp.log}
    set fp [open tmp.log r]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
    ask "Inspect result?"
    echo "+ xspice -a attenua"
    catch {exec xspice -a attenua >& tmp.log}
    set fp [open tmp.log r]
    while {[gets $fp line] >= 0} { puts $line }
    close $fp
    file delete tmp.log
}

set ok [ask "Shall we do -C?"]
if {$ok} {
    echo "+ space -vFC attenua"
    catch {exec space -vFC attenua >& tmp.log}
    set fp [open tmp.log r]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
    ask "Inspect result?"
    echo "+ xspice -a attenua"
    catch {exec xspice -a attenua >& tmp.log}
    set fp [open tmp.log r]
    while {[gets $fp line] >= 0} { puts $line }
    close $fp
    file delete tmp.log
}

set ok [ask "Shall we do -r?"]
if {$ok} {
    echo "+ space -vFr attenua"
    catch {exec space -vFr attenua >& tmp.log}
    set fp [open tmp.log r]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
    ask "Inspect result?"
    echo "+ xspice -a attenua"
    catch {exec xspice -a attenua >& tmp.log}
    set fp [open tmp.log r]
    while {[gets $fp line] >= 0} { puts $line }
    close $fp
    file delete tmp.log
}

set ok [ask "Shall we do -z?"]
if {$ok} {
    echo "+ space -vFz attenua"
    catch {exec space -vFz attenua >& tmp.log}
    set fp [open tmp.log r]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
    ask "Inspect result?"
    echo "+ xspice -a attenua"
    catch {exec xspice -a attenua >& tmp.log}
    set fp [open tmp.log r]
    while {[gets $fp line] >= 0} { puts $line }
    close $fp
    file delete tmp.log
}

puts $f1 "STEP 5."
puts {
*********************************************************************
5.  If you have PSPICE, you can perform a circuit simulation (after
    customizing the script nspice) as follows:

    % nspice attenua attenua.cmd
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ nspice attenua attenua.cmd"
    catch {exec nspice attenua attenua.cmd >& tmp.log}
    set fp [open tmp.log r]
    while {[gets $fp line] >= 0} { puts $line }
    close $fp
    file delete tmp.log
}

puts {
*********************************************************************
* You can compare the extracted circuit against a reference circuit *
* with the circuit comparison program match.                        *
*********************************************************************}
ask "Shall we go on?"

puts $f1 "STEP 6."
puts {
*********************************************************************
6.  First add device descriptions for the bipolar transistors to the
    database so that the reference circuit can be stored into the
    database using the program cspice.

    % putdevmod pnpWP.dev npnBW.dev
    % xcontrol -device pnpWP npnBW
    % cspice att_ref.spc
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ putdevmod pnpWP.dev npnBW.dev"
    catch {exec putdevmod pnpWP.dev npnBW.dev}
    echo "+ xcontrol -device pnpWP npnBW"
    exec xcontrol -device pnpWP npnBW
    echo "+ cspice att_ref.spc"
    catch {exec cspice att_ref.spc >& tmp.log}
    set fp [open tmp.log r]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
    file delete tmp.log
}

puts $f1 "STEP 7."
puts {
*********************************************************************
7.  Extract cell attenua, possibly after an error has been added to
    the layout.

    % space -vF attenua
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ space -vF attenua"
    catch {exec space -vF attenua >& tmp.log}
    set fp [open tmp.log r]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
    file delete tmp.log
}

puts $f1 "STEP 8."
puts {
*********************************************************************
8.  Use the program match to compare the extracted circuit against
    the reference circuit.

    % match -bindings att_ref attenua
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ match -bindings att_ref attenua"
    catch {exec match -bindings att_ref attenua >& tmp.log}
    set fp [open tmp.log r]
    while {[gets $fp line] >= 0} { echo $line }
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
    file delete exp_dat attenua.ana attenua.axa attenua.spc sim.diag
}
