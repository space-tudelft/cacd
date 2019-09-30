#!/bin/sh
# directory: demo/sram

R="[1;31m"
G="[1;32m"
B="[1;34m"
X="[0m"

unset CWD
clear
cat <<!
$G*********************************************************************
** $R Demonstration of Space 3D Capacitance Extraction             $G  **
** $R using a CMOS static RAM cell.  See also section "5.2" in the $G  **
** $R  "Space 3D Capacitance Extraction User's Manual".            $G  **
*********************************************************************$X

Note: Type$R Enter$X on the keyboard, each time, to do a step.
      If you want to skip a step, type first a $R'n'$X.
      If you want to stop the demo, type$R Ctrl-C$X.
*********************************************************************
This script first tries to remove old project data.
*********************************************************************
!
printf "Shall we start this demo? (type$R Enter$X)"; read ok
if test "$ok" == n
then
echo "Ok, see you another time!"
exit 1
fi

if test -f .dmrc
then
printf "Shall we remove existing project directory? (type$R Enter$X)"; read ok
    if test "$ok" != n
    then
	echo "+ rmpr -fs ."
	rmpr -fs .
    fi
fi

if test "$ok" != n
then
cat <<!

$G*********************************************************************$B
STEP 1.$X
    First, we need a new project directory to work with.
    We shall use the$R scmos_n$X technology and a lambda of$R 0.25$X micron.
    We change the current working directory $R'.'$X into a project directory
    with the following command:

    % mkpr -p scmos_n -l 0.25 .
$G*********************************************************************$X
!
printf "Shall we do it? (type$R Enter$X)"; read ok
if test "$ok" == n
then
echo "Ok, see you a next time!"
exit 1
fi

echo "+ mkpr -p scmos_n -l 0.25 ."
mkpr -p scmos_n -l 0.25 .

cat <<!

$G*********************************************************************$B
STEP 2.$X
    The$B sram$X layout is available in a gds2 file.
    We use$R cgi$X (convert-gds-internal) to put the layout into the
    project database with the following command:

    % cgi sram.gds
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ cgi sram.gds"
cgi sram.gds
fi
fi

cat <<!

$G*********************************************************************$X
    We can inspect the$B sram$X layout with the layout editor$R dali$X.
    We use the following command:

    % dali sram

    To exit the program, click on $R'-quit-'$X and click on $R'yes'$X.
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ dali sram"
dali sram
fi

cat <<!

$G*********************************************************************$B
STEP 3.$X
    We use a local technology file for the extraction of the circuit.
    This element definition file needs to be compiled before usage.
    Use the technology compiler$R tecc$X to create the $R"sram.t"$X file.
    Use the following command:

    % tecc sram.s
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ tecc sram.s"
tecc sram.s
fi

cat <<!
$G*********************************************************************$B
4.$X  Perform an extraction of cell$B sram$X in batch mode.  Extract
    3D capacitances (using option $R-C3$X).  The other options are:
       $R-E$G<file>$X to use an alternate element definition file,
       $R-P$G<file>$X to use an alternate parameter file.
    The command used is:

    % space3d -C3 -E sram.t -P sram.p sram
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ space3d -C3 -E sram.t -P sram.p sram"
space3d -C3 -E sram.t -P sram.p sram
fi

cat <<!

$G*********************************************************************$B
4b.$X We can also perform an interactive extraction of cell$B sram$X.
    Using the$R Xspace$X program with following command:

    % Xspace -E sram.t -P sram.p sram
*********************************************************************
    To set the extraction options, go to the $B'Options' menu$X and
    click button$R <coupling cap.>$X and$R <3D capacitance>$X.
    You must also set some display options (what you want to see), go
    to the $B'Display' menu$X and click button $R'DrawBEMesh'$X, $R'DrawGreen'$X
    and $R'3 dimensional'$X.  Now, to start the extraction,
    go to the $B'Extract' menu$X and click $R'extract'$X (note that you
    can also use the hotkey $R'e'$X).  To$R exit$X the program,
    click on $R'quit'$X in the $B'Extract' menu$X (or use hotkey $R'q'$X).
*********************************************************************
    You can also rotate the 3D mesh with the keypad arrow keys.
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ Xspace -E sram.t -P sram.p sram"
Xspace -E sram.t -P sram.p sram
fi

cat <<!

$G*********************************************************************$B
5.$X  We can now inspect the extraction result, the extracted circuit.
    To see a$R SPICE$X description (or netlist), we use the$R xspice$X
    command to eXstract SPICE from the database:

    % xspice -a sram                      (see:$R icdman$X xspice)
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ xspice -a sram"
xspice -a sram
fi

cat <<!

$G*********************************************************************$B
6.$X  Of coarse, you can also perform a circuit simulation.
    There is already a stimuli file supplied ($R"sram.cmd"$X), to do a
    SLS or SPICE simulation using the simulation GUI$R simeye$X.
    To start simeye, the following command is used:

    % simeye
*********************************************************************
    Click on the $B"Simulate" menu$X and choice item $R"Prepare"$X.
    Select in the $R"Circuit:"$X field cell name $R"sram"$X and in the
    $R"Stimuli:"$X field file name $R"sram.cmd"$X and click on $R"Run"$X.
    See:$R icdman$X sls, and see also:$R icdman$X simeye.
    You can also choice another simulation type in the $R"Type:"$X field.
    To leave the program, choice item $R"Exit"$X in the $B"File" menu$X.
*********************************************************************$B
Note:$X
    To perform a SPICE simulation, you need to have a$R spice3$X simulator
    available.  Check the shell script$R nspice$X in \$ICDPATH/share/bin
    to see if spice3 is called correctly.
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ simeye"
simeye
fi

cat <<!

$G*********************************************************************$R
    Congratulations, you have now finished this demo!
$G*********************************************************************$X
!
printf "Shall we clean the demo directory and exit?"; read ok
if test "$ok" != n
then
echo "+ rmpr -fs ."
rmpr -fs .
echo "+ rm -f display.out exp_dat sram.spc sram.t"
rm -f display.out exp_dat sram.spc sram.t
echo "+ rm -f sim.diag sram.ana sram.out sram.plt sram.res"
rm -f sim.diag sram.ana sram.out sram.plt sram.res
fi
