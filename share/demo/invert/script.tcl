#!/usr/bin/env tclsh
# directory: demo/invert

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

proc copy {from to} {
    set in  [open $from]
    set out [open $to w]
    puts -nonewline $out [read $in]
    close $out
    close $in
}

proc edit {file} {
    if {[string equal $::tcl_platform(platform) windows]} {
	set editor notepad
    } else {
	set editor gedit
    }
    if {[catch {exec $editor $file} result]} { echo $result }
}

puts {
***********************************************************************
** Demonstration of extraction of an inverter with transistor bulk   **
** connections and drain/source regions. The latter are extracted as **
** either non-linear junction capacitances, or drain/source area and **
** perimeter information attached to the MOS transistors.            **
** See the "Space Tutorial" sections "3.8", "4" and "5".             **
***********************************************************************

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
    We shall use the scmos_n technology and a lambda of 0.2 micron.
    We change the current working directory '.' into a project directory
    with the following command:

    % mkpr -p scmos_n -l 0.2 .
*********************************************************************}
set ok [ask "Shall we do it? (type Enter)"]
if {!$ok} { puts "Ok, see you a next time!"; exit 1 }

echo "+ mkpr -p scmos_n -l 0.2 ."
catch {exec mkpr -p scmos_n -l 0.2 . >& tmp.log}
set fp [open tmp.log]
while {[gets $fp line] >= 0} { echo $line }
close $fp

puts $f1 "STEP 2."
puts {
*********************************************************************
STEP 2.
    The layout of the inverter is available in a gds2 file.
    We use cgi (convert-gds-internal) to put the layout into the
    project database with the following command:

    % cgi invert.gds
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ cgi invert.gds"
    catch {exec cgi invert.gds >& tmp.log}
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
}
}

puts $f1 "STEP 2b."
puts {
*********************************************************************
2b. Now, we can inspect the layout with the layout editor/viewer dali.
    Because the inverter has cell name 'invert', we use command:

    % dali invert

    To exit dali, click on '-quit-' and 'yes'.
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ dali invert"
    if {[catch {exec dali invert} result]} { echo $result }
}

puts $f1 "STEP 3."
puts {
*********************************************************************
3.  Perform an extraction of the invert cell. Use option -C to
    include coupling capacitances. The drain/source regions are extracted
    as non-linear junction capacitances that have parameters 'area' and
    'perimeter' (using option -S). The following command is used:

    % space -C -Sjun_caps=area-perimeter invert
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ space -C -Sjun_caps=area-perimeter invert"
    catch {exec space -C -Sjun_caps=area-perimeter invert >& tmp.log}
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
puts {
*********************************************************************}
if {![string equal $tcl_platform(platform) windows]} {
puts {  If you see a warning message of the "makegln" program, then this is
  no problem. In that case, no "compress" tools are installed.}
}
puts {  Note that "space" gives a warning about the join of two different
  active areas (p+/n+). In this layout, an active area contact with
  first metal and a substrate contact lay close to each other.
  Accidental active area mask "caa" is used for the substrate contact.
*********************************************************************}
    set ok [ask "Shall we go on?"]
    if {!$ok} {
	puts "Ok, then we stop!";
	file delete exp_dat tmp.log
	exit 1
    }
}

puts $f1 "STEP 4."
puts {
*********************************************************************
4.  Look in the file "xspicerc" to see how the junction capacitances are
    printed in the netlist output as diodes with parameters 'area' and 'pj':

    % type xspicerc
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ type xspicerc"
    set fp [open xspicerc]
    puts -nonewline [read $fp]
    close $fp
}

puts $f1 "STEP 4b."
puts {
*********************************************************************
4b. And next get the SPICE circuit description. We use the xspice
    command with options -a and -u to eXstract SPICE from the database:

    % xspice -au invert                    (see: icdman xspice)
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ xspice -au invert"
    catch {exec xspice -au invert >& tmp.log}
    set fp [open tmp.log]
    puts -nonewline [read $fp]
    close $fp
}

puts $f1 "STEP 5."
puts {
*********************************************************************
5.  An alternative is to extract the drain/source regions as 'area' and
    'perimeter' information that is attached to the MOS transistors.
    This is achieved by modifying the transistor definitions in the element
    definition file.  Therefore, first copy the element definition file
    from the process directory to the local file "elem.s":

    % copy $ICDPATH/share/lib/process/scmos_n/space.def.s elem.s
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    if {![info exists env(ICDPATH)]} {
	set path [pwd]
	set i [string first "/cacd/" $path]
	if {$i < 0} { set path "" } else { set path [string replace $path $i+5 end] }
    } else {
	set path $env(ICDPATH)
    }
    if {[string len $path] == 0} {
	echo "Please, set environment variable ICDPATH first!"
	exit 1
    }
    if {![file exists $path/share/lib/process/scmos_n/space.def.s]} {
	echo "Sorry, cannot find the element definition file:"
	echo "\t$path/share/lib/process/scmos_n/space.def.s"
	exit 1
    }
    echo {+ copy $ICDPATH/share/lib/process/scmos_n/space.def.s elem.s}
    copy $path/share/lib/process/scmos_n/space.def.s elem.s
}

puts $f1 "STEP 5b."
puts {
*********************************************************************
5b. To change the lines, edit the "elem.s" file:

    % edit elem.s
*********************************************************************
(1) Change the lines (in conductors definitions)
       cond_pa : caa !cpg !csn : caa  : 70  : p    # p+ active area
       cond_na : caa !cpg  csn : caa  : 50  : n    # n+ active area
    into
       cond_pa : caa !cpg !csn : caa  : 0   : p    # p+ active area
       cond_na : caa !cpg  csn : caa  : 0   : n    # n+ active area
    to avoid complaints by tecc about the fact the resistance for drain/
    source regions should be zero (that resistance is already modeled in
    the transistor model).

(2) Change the lines (in fets definitions)
       nenh : cpg caa  csn : cpg caa : @sub  # nenh MOS
       penh : cpg caa !csn : cpg caa : cwn   # penh MOS
    into
       nenh : cpg caa  csn : cpg caa (!cpg caa csn)  : @sub # nenh MOS
       penh : cpg caa !csn : cpg caa (!cpg caa !csn) : cwn  # penh MOS
    to describe the drain/source regions.

(3) And remove the 'ndif' and 'pdif' junction capacitance lists.
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ edit elem.s"
    edit elem.s
}

puts $f1 "STEP 5c."
puts {
*********************************************************************
5c. Next, run tecc to compile the new element definition file:

    % tecc elem.s
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ tecc elem.s"
    catch {exec tecc elem.s >& tmp.log}
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
}

puts $f1 "STEP 5d."
puts {
*********************************************************************
5d. Now, run space again with the new element definition file:

    % space -C -Sjun_caps=area-perimeter -E elem.t invert
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ space -C -Sjun_caps=area-perimeter -E elem.t invert"
    catch {exec space -C -Sjun_caps=area-perimeter -E elem.t invert >& tmp.log}
    set fp [open tmp.log]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
}

puts $f1 "STEP 6."
puts {
*********************************************************************
6.  And watch the SPICE output again:

    % xspice -au invert
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ xspice -au invert"
    catch {exec xspice -au invert >& tmp.log}
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
    file delete exp_dat elem.s elem.t tmp.log
}
