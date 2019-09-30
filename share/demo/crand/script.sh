#!/bin/sh
# directory: demo/crand

R="[1;31m"
G="[1;32m"
B="[1;34m"
X="[0m"

unset CWD
clear
cat <<!
$G*********************************************************************
** $R Demonstration of (1) extraction of a random counter circuit  $G  **
** $R              and (2) switch-level simulation of the circuit. $G  **
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
exit
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
    We shall use the$R scmos_n$X technology and a lambda of$R 0.2$X micron.
    We change the current working directory $R'.'$X into a project directory
    with the following command:

    % mkpr -p scmos_n -l 0.2 .
$G*********************************************************************$X
!
printf "Shall we do it? (type$R Enter$X)"; read ok
if test "$ok" == n
then
echo "Ok, see you a next time!"
exit
fi

echo "+ mkpr -p scmos_n -l 0.2 ."
mkpr -p scmos_n -l 0.2 .

cat <<!

$G*********************************************************************$B
STEP 2.$X
    The layout of the$R random counter$X is available in a gds2 file.
    We use$R cgi$X (convert-gds-internal) to put the layout into the
    project database with the following command:

    % cgi crand.gds
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ cgi crand.gds"
cgi crand.gds
fi
fi

cat <<!

$G*********************************************************************$B
2b.$X You can list the contents of the project and show the hierarchical
    layout tree with the$R dblist$X command:

    % dblist -h
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ dblist -h"
dblist -h
cat <<!
You see$R 4$X hierarchical levels. The top cell$B crand$X has$R 1$X sub-cell.
This$B counter$X sub-cell has again$R 4$X sub-cells.
!
fi

cat <<!

$G*********************************************************************$B
2c.$X Now, we can inspect the layout with the layout editor/viewer$R dali$X.
    To show the layout, we use the following command:

    % dali crand

    To see more layout levels, use the number keys ($R'2'$X,$R'3'$X or $R'4'$X).
    Use command $R'read_cell'$X in the $B"DB_menu"$X to inspect the sub-cells.
    To exit the program, click on $R'-quit-'$X and $R'yes'$X.
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ dali crand"
dali crand
fi

cat <<!

$G*********************************************************************$B
3.$X  Perform a flat extraction of cell$B crand$X in batch mode. Extract
    also ground capacitances (option $R-c$X).  The other options are
    $R-v$X to use verbose mode and $R-F$X to use flat extraction.
    The command used is:

    % space -vFc crand
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ space -vFc crand"
space -vFc crand
fi

cat <<!

$G*********************************************************************$B
3b.$X We can now inspect the extraction result, the extracted circuit.
    To see a$R SLS$X description (or netlist), we use the$R xsls$X
    command to eXstract SLS from the database:

    % xsls crand                       (see:$R icdman$X xsls)
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ xsls crand"
xsls crand
fi

cat <<!

$G*********************************************************************$B
4.$X  Start the simulation GUI$R simeye$X and run a$R sls$X simulation.

    % simeye
*********************************************************************$B
4b.$X Click on the $B"Simulate" menu$X and choice the $R"Prepare"$X item.
    Select in the $R"Circuit:"$X field cell name $R"crand"$X and
    in the $R"Stimuli:"$X field file name $R"crand.cmd"$X (click on it).
    To inspect or edit the input signals, click on the $R"Edit"$X button.
*********************************************************************$B
5.  To run a$R sls$B simulation:$X
    Go back to the $R"Simulate -> Prepare"$X dialog and choice
    simulation $R"Type: sls-timing"$X and for $R"Read: Analog"$X.
    Perform a switch-level timing simulation by clicking button
    $R"Run"$X and watch the simulation results.
*********************************************************************$B
5b. To exit the$R simeye$B program:$X
    Go to the $B"File" menu$X and click on $R"Exit"$X and $R"Yes"$X.
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
echo "+ rm -f exp_dat crand.out crand.plt crand.res sim.diag"
rm -f exp_dat crand.out crand.plt crand.res sim.diag
fi
