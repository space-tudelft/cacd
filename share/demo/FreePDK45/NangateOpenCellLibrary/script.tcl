#!/usr/bin/env tclsh
# directory: demo/FreePDK45/NangateOpenCellLibrary

proc ask {txt} {
	puts -nonewline $txt
	flush stdout
	set ok 1
	if {[string match n* [gets stdin]]} {set ok 0}
	return $ok
}

puts {
*********************************************************************
******      Welcome to the Nangate Open Cell Library!          ******
**  The Nangate 45nm Open Cell Library is an open-source,          **
**  standard-cell library provided for the purposes of testing     **
**  and exploring EDA flows.  The library was generated using      **
**  Nangate's Library Creator(TM) and the 45nm FreePDK Base Kit    **
**  from North Carolina State University (NCSU).                   **
*********************************************************************

Note: Type Enter on the keyboard, each time, to do a step.
      If you want to skip a step, type first a 'n'.
      If you want to stop this visiting tour, type Ctrl-C.
*********************************************************************}
set ok [ask "Shall we start the visiting tour? (type Enter)"]
if {!$ok} { puts "Ok, see you another time!\n"; exit 1 }

if ![file exists .dmrc] {
	puts {sorry, the NangateOpenCellLibrary is not yet installed}
	puts {type "./install.sh" or "sh install.sh" to install the library}
	exit 1
}

puts {
*********************************************************************
STEP 1.
    We can inspect the layout of a basic cell with the layout editor dali.
    Start the layout editor with the following command:

    % dali                         (see: icdman dali)

    Click on the "DB_menu" and click on "read_cell" and click on a cell
    name.  Click a second time, if you are sure to read this basic cell.

    To exit the program, click on '-return-' and on '-quit-'
    and click on 'yes'.
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
	puts "+ dali"
	catch {exec dali}
}

puts {
*********************************************************************
STEP 2.
    We can inspect the extraction result (the extracted circuit netlist)
    of a basic cell.  To see a SPICE netlist of the INV_X1 cell, we
    use the xspice command to eXstract SPICE from the database:

    % xspice -aou INV_X1           (see: icdman xspice)
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
	puts "+ xspice -aou INV_X1"
	exec xspice -aou INV_X1 >& xspice.out
	set fp [open xspice.out r]
	puts -nonewline [read $fp]
}

puts {
*********************************************************************
STEP 3.
    We can also generate a SLS netlist with the following command:

    % xsls INV_X1                  (see: icdman xsls)
*********************************************************************}
set ok [ask "Shall we do it?"]
if {$ok} {
	puts "+ xsls INV_X1"
	exec xsls INV_X1 >& xsls.out
	set fp [open xsls.out r]
	puts -nonewline [read $fp]
}

puts {
*********************************************************************
    Thanks for visiting this short tour, see you another time!
*********************************************************************}
