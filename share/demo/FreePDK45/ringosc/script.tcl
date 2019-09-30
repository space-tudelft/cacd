#!/usr/bin/env tclsh
# directory: demo/FreePDK45/ringosc

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

proc diff {file1 file2} {
    set fp1 [open $file1 r]
    set fp2 [open $file2 r]
    set n1 0
    set n2 0

    while {[gets $fp1 l1] >= 0 && [gets $fp2 l2] >= 0} {
	incr n1
	incr n2
	if {![string equal $l1 $l2]} {
	    set m1 $n1
	    while {[gets $fp1 ln] > 0} { incr n1; append l1 \n $ln }
	    set m2 $n2
	    while {[gets $fp2 ln] > 0} { incr n2; append l2 \n $ln }
	    puts "$m1,${n1}c$m2,$n2"
	    foreach ln [split $l1 \n] { puts "< $ln" }
	    puts "---"
	    foreach ln [split $l2 \n] { puts "> $ln" }
	    incr n1
	    incr n2
	}
    }
}

proc copy {from to} {
    set in  [open $from]
    set out [open $to w]
    puts -nonewline $out [read $in]
    close $out
    close $in
}

puts {
*********************************************************************
**  Demonstration of a flat Space Extraction                       **
**  of a ring-oscillator made with the FreePDK45 process.          **
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
    To be able to use the basic cells of the NangateOpenCellLibrary we
    must import them in our project.  Use the following command:

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
    The ring-oscillator layout is available in a gds2 file.
    We use cgi (convert-gds-internal) to put the layout into the
    project database.  Use the following command:

    % cgi ringosc.gds
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ cgi ringosc.gds"
    catch {exec cgi ringosc.gds >& tmp.log}
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
    We can inspect the oscillator layout with the layout editor dali.
    Note that it contains 9 imported basic cells with the name INV_X2.
    We start dali with the following command:

    % dali ringosc

    To see an extra hierarchical level, type '2'. Use the 'i' key
    to zoom-in and the arrow keys. Use 'b' to get the bbox-window.
    To exit the program, click on '-quit-' and click on 'yes'.
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ dali ringosc"
    catch {exec dali ringosc}
}

puts $f1 "STEP 6."
puts {
*********************************************************************
STEP 6.
    We use a local space technology file space.def.s which first must
    be compiled, before we can use it for the space extractor.  Type:

    % tecc space.def.s

    The output file gets the name space.def.t and shall be specified
    on the space command line with option -E.
*********************************************************************}
set ok [ask "Shall we compile?"]
if {$ok} {
    echo "+ tecc space.def.s"
    catch {exec tecc space.def.s >& tmp.log}
    set fp [open tmp.log r]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
    file delete tmp.log
}

puts $f1 "STEP 7."
puts {
*********************************************************************
STEP 7.
    Now, perform a flat netlist extraction of the oscillator layout.  Note
    that the basic cells are not extracted, because they are library cells.
    Type the following command to used the space extractor:

    % space -Fv -E space.def.t -P space.def.p ringosc

    Note that we specify also a local space parameter file space.def.p
    on the command line with option -P.  The -F option is for
    flat and the -v option is for verbose mode.
*********************************************************************}
set ok [ask "Shall we extract?"]
if {$ok} {
    echo "+ space -Fv -E space.def.t -P space.def.p ringosc"
    catch {exec space -Fv -E space.def.t -P space.def.p ringosc >& tmp.log}
    set fp [open tmp.log r]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
    file delete tmp.log
}

puts $f1 "STEP 8."
puts {
*********************************************************************
STEP 8.
    We can now inspect the extraction result, the extracted circuit.
    To see a SPICE description (or netlist), we use the xspice
    command to eXstract SPICE from the database:

    % xspice -a ringosc                  (see: icdman xspice)

    The -a option gives node names instead of node numbers.
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ xspice -a ringosc"
    catch {exec xspice -a ringosc >& tmp.log}
    set fp [open tmp.log r]
    puts -nonewline [read $fp]
    close $fp
    file delete tmp.log
}

puts $f1 "STEP 9."
puts {
*********************************************************************
STEP 9.
    To get a flat netlist extraction to the transistor level, we must
    overrule the library status of the INV_X2 basic cell.
    To do so, give the following command:

    % xcontrol -regular INV_X2

    Note: If there is no local cell status, the remote cell status
          is used.
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ xcontrol -regular INV_X2"
    exec xcontrol -regular INV_X2
}

puts $f1 "STEP 10."
puts {
*********************************************************************
STEP 10.
    Now we can perform a complete flat netlist extraction of the ring-
    oscillator layout.  Run space with the following command:

    % space -Fv -E space.def.t -P space.def.p ringosc
*********************************************************************}
set ok [ask "Shall we extract?"]
if {$ok} {
    echo "+ space -Fv -E space.def.t -P space.def.p ringosc"
    catch {exec space -Fv -E space.def.t -P space.def.p ringosc >& tmp.log}
    set fp [open tmp.log r]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
    file delete tmp.log
}

puts $f1 "STEP 11."
puts {
*********************************************************************
STEP 11.
    To generate a SPICE netlist give the following command:

    % xspice -au ringosc                (see: icdman xspice)

    The -u option omits addition of transistor bulk connections.
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ xspice -au ringosc"
    catch {exec xspice -au ringosc >& tmp.log}
    set fp [open tmp.log r]
    puts -nonewline [read $fp]
    close $fp
    file delete tmp.log
}

puts $f1 "STEP 12."
puts {
*********************************************************************
STEP 12.
    We can also extract the netlist with 2D capacitances, type:

    % space -Fv -Cl -E space.def.t -P space.def.p ringosc

    Note that the option -C is used for couple cap extraction
    and the option -l for adding also the lateral couple caps.
*********************************************************************}
set ok [ask "Shall we extract?"]
if {$ok} {
    echo "+ space -Fv -Cl -E space.def.t -P space.def.p ringosc"
    catch {exec space -Fv -Cl -E space.def.t -P space.def.p ringosc >& tmp.log}
    set fp [open tmp.log r]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
    file delete tmp.log
}
set ok [ask "Shall we netlist?"]
if {$ok} {
    echo "+ xspice -au ringosc"
    catch {exec xspice -au ringosc >& tmp.log}
    set fp [open tmp.log r]
    puts -nonewline [read $fp]
    close $fp
    file delete tmp.log
}

puts $f1 "STEP 13."
puts {
*********************************************************************
STEP 13.
    We can perform an analog spice simulation on this netlist.
    We use the simeye program to run the simulator and to show
    the resulting waveforms (note that the spice3 simulator must be
    available on your system).  Type:

    % simeye

Note: Click on the "Simulate" menu and choice item "Prepare".
     Select in the "Circuit:" field cell name "ringosc" and
     in the "Stimuli:" field file name "ringosc.cmd".
     Choice simulation "Type: spice" and click on button "Run".
     To leave the program, choice item "Exit" in the "File" menu.
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ simeye"
    catch {exec simeye}
}

puts $f1 "STEP 14."
puts {
*********************************************************************
STEP 14.
    We can also extract the netlist with 3D capacitances and simulate
    with spice again.  We use the following commands:

    % space3d -Fv -C3 -E space.def.t -P space.def.p ringosc
    % simeye

    Note that option -3 is used in place of the -l option.
    In 3D mode all kind of couple capacitances are extracted.
    Note that the 3D version of space must be used.
*********************************************************************}
set ok [ask "Shall we extract?"]
if {$ok} {
    echo "+ space3d -Fv -C3 -E space.def.t -P space.def.p ringosc"
    catch {exec space3d -Fv -C3 -E space.def.t -P space.def.p ringosc >& tmp.log}
    set fp [open tmp.log r]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
    file delete tmp.log
}
set ok [ask "Shall we simulate again?"]
if {$ok} {
    echo "+ simeye"
    catch {exec simeye}
}

puts $f1 "STEP 15."
puts {
*********************************************************************
STEP 15.
    You must know, that for 3D capacitance extraction a 3D Boundary
    Element Mesh (BEM) is used. It is nice to show how this mesh looks.
    With the Xspace tool you can show what space3d is doing.
    The Xspace tool runs the space3d extractor for you in flat mode.
    In that case space3d knows that it must make a display.out file.
    And Xspace reads this display.out file, while space3d is extracting.
    Use the following command:

    % Xspace -C3 -E space.def.t -P space.def.p ringosc

    To start the extraction, go with the mouse to the Extract menu and
    click on the extract button (you can also use 'e' hotkey instead).
    If you like, you can direct switch the view from 2D into 3D (and back).
    This is only possible with the '*' hotkey. Note that the 3D mesh picture
    can be rotated with the arrow-keys of the small keypad (with the other
    arrow-keys you can shift). You can also zoom-in with 'i' and zoom-out
    with 'o'. And with 'b' you can go back to the bbox view.
   (Note that with extract again or 'a' no extraction is done, but only
    the display.out file is read again.)
    Use the 'q' hotkey to quit the Xspace program.
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ Xspace -C3 -E space.def.t -P space.def.p ringosc"
    catch {exec Xspace -C3 -E space.def.t -P space.def.p ringosc}
}

puts $f1 "STEP 16."
puts {
*********************************************************************
STEP 16.
    To show the substrate noise on the sens pin, we can also
    extract substrate resistances.  We use the following commands:

    % space3d -Fv -Cl -B -E space.def.t -P space.def.p ringosc
    % simeye

    Note that the option -B is used for extraction of accurate
    substrate resistances.  It is using a 3D BEM method, therefor
    the space3d version of the extractor must be used in this case.
*********************************************************************}
set ok [ask "Shall we extract?"]
if {$ok} {
    echo "+ space3d -Fv -Cl -B -E space.def.t -P space.def.p ringosc"
    catch {exec space3d -Fv -Cl -B -E space.def.t -P space.def.p ringosc >& tmp.log}
    set fp [open tmp.log r]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
    file delete tmp.log
}
set ok [ask "Shall we simulate again?"]
if {$ok} {
    echo "+ simeye"
    catch {exec simeye}
}

puts $f1 "STEP 17."
puts {
*********************************************************************
STEP 17.
    To show the substrate noise on the sens pin for the same
    layout with a ground shield.  We extract ringosc2 and simulate
    again.  We use the following commands:

    % cgi ringosc2.gds
    % dali ringosc2
    % space3d -Fv -Cl -B -E space.def.t -P space.def.p ringosc2
    % simeye
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    echo "+ cgi ringosc2.gds"
    catch {exec cgi ringosc2.gds >& tmp.log}
    set fp [open tmp.log r]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
    file delete tmp.log
    set ok [ask "View the layout?"]
    if {$ok} {
	echo "+ dali ringosc2"
	catch {exec dali ringosc2}
    }
    set ok [ask "Shall we extract?"]
    if {$ok} {
	echo "+ space3d -Fv -Cl -B -E space.def.t -P space.def.p ringosc2"
	catch {exec space3d -Fv -Cl -B -E space.def.t -P space.def.p ringosc2 >& tmp.log}
	set fp [open tmp.log r]
	while {[gets $fp line] >= 0} { echo $line }
	close $fp
	file delete tmp.log
    }
    set ok [ask "Shall we simulate ringosc2?"]
    if {$ok} {
	echo "+ simeye"
	catch {exec simeye}
    }
}

puts $f1 "STEP 18."
puts {
*********************************************************************
STEP 18.
    You can also use a 2D method to extract substrate resistances.
    Before you can do that, there must be added 2D substrate data
    to the technology file. This data can be generated with the
    subresgen program.  This program runs the space3d extractor
    a number of times to generate this 2D substrate res data. Type:

    % subresmkdir sub2d 10
    % copy space.def.s sub2d
    % cd sub2d
    % subresgen
    % cd ..
    % diff space.def.s sub2d/space.def.s
    % copy sub2d/space.def.s space.sub2d.s
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
    file delete -force sub2d
    echo "+ subresmkdir sub2d 10"
    catch {exec subresmkdir sub2d 10 >& tmp.log}
    set fp [open tmp.log r]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
    echo "+ copy space.def.s sub2d"
    copy space.def.s sub2d/space.def.s
    echo "+ cd sub2d"
    cd sub2d
    echo "+ subresgen"
    catch {exec subresgen >& tmp.log}
    set fp [open tmp.log r]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
    file delete tmp.log
    echo "+ cd .."
    cd ..
    echo "+ diff space.def.s sub2d/space.def.s"
    diff space.def.s sub2d/space.def.s
    echo "+ copy sub2d/space.def.s space.sub2d.s"
    copy sub2d/space.def.s space.sub2d.s
}

puts $f1 "STEP 19."
puts {
*********************************************************************
STEP 19.
    Now we go to extract simple substrate resistances using a 2D method.
    And after that we go to simulate again.  But first we must compile
    the new technology file.  We do the following commands:

    % tecc space.sub2d.s
    % space -Fv -Cl -b -E space.sub2d.t -P space.def.p ringosc
    % simeye
*********************************************************************}
set ok [ask "Shall we compile?"]
if {$ok} {
    echo "+ tecc space.sub2d.s"
    catch {exec tecc space.sub2d.s >& tmp.log}
    set fp [open tmp.log r]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
}
set ok [ask "Shall we extract?"]
if {$ok} {
    echo "+ space -Fv -Cl -b -E space.sub2d.t -P space.def.p ringosc"
    catch {exec space -Fv -Cl -b -E space.sub2d.t -P space.def.p ringosc >& tmp.log}
    set fp [open tmp.log r]
    while {[gets $fp line] >= 0} { echo $line }
    close $fp
}
file delete tmp.log
set ok [ask "Shall we simulate ringosc?"]
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
file delete display.out exp_dat ringosc.spc space.def.t
file delete sim.diag sim.diag2 ringosc.ana ringosc.out ringosc2.ana ringosc2.spc
file delete -force sub2d
}
