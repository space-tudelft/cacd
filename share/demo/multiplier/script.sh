#!/bin/sh
# directory: demo/multiplier

R="[1;31m"
G="[1;32m"
B="[1;34m"
X="[0m"

unset CWD
clear
cat <<!
$G***************************************************************
** $R Demonstration of functional/switch-level simulation.    $G **
** $B See: "Functional Simulation User's Manual" and          $G **
** $B      "SLS: Switch-Level Simulator User's Manual".       $G **
**                                                           **
** $R A network 'total' is simulated that has a ram and a     $G **
** $R multiplier that are described at the functional level.  $G **
** $R Pass transistors are used to demultiplex the output     $G **
** $R signals of the ram and to multiply two subsequent       $G **
** $R words that come out of the ram.                         $G **
***************************************************************$B
Warning: This demo does not always work, due to library linking problems.
When you have these problems and want to use function blocks, contact us.$X

Note: See the $R"crand"$X demo, for another$R sls$X and$R simeye$X example.

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
    We shall use the$R scmos_n$X technology and a default lambda value.
    We change the current working directory $R'.'$X into a project directory
    with the following command:

    % mkpr -p scmos_n .
$G*********************************************************************$X
!
printf "Shall we do it? (type$R Enter$X)"; read ok
if test "$ok" == n
then
echo "Ok, see you a next time!"
exit
fi

echo "+ mkpr -p scmos_n ."
mkpr -p scmos_n .

cat <<!

$G*********************************************************************$B
STEP 2.$X
    Run$R cfun$X to compile the functional description of $R"ram.fun"$X
    and $R"mul8x8.fun"$X, and add them to the database:

    % cfun ram.fun
    % cfun mul8x8.fun
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ cfun ram.fun"
cfun ram.fun
echo "+ cfun mul8x8.fun"
cfun mul8x8.fun
fi

cat <<!

$G*********************************************************************$B
STEP 3.$X
    Run$R csls$X to add the network $R'total'$X to the database:

    % csls total.sls
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ csls total.sls"
csls total.sls
fi
fi

cat <<!

$G*********************************************************************$B
3b.$X You can list the contents of the project and show the hierarchical
    circuit tree with the$R dblist$X command:

    % dblist -h
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ dblist -h"
dblist -h
cat <<!
You see$R 2$X hierarchical levels. The top cell$B total$X has$R 2$X sub-cells.
This sub-cells are the function blocks.
!
fi

cat <<!

$G*********************************************************************$B
4.$X  Start the simulation GUI$R simeye$X and run a$R sls$X simulation.

    % simeye
*********************************************************************$B
5.$X  Click on the $B"Simulate" menu$X and choice the $R"Prepare"$X item.
    Select in the $R"Circuit:"$X field cell name $R"total"$X and
    in the $R"Stimuli:"$X field file name $R"total.cmd"$X (click on it).
    Click on the $R"Run"$X button and watch the simulation results.
*********************************************************************$B
6.$X  Zoom-in onto the lowest 3 signals using $R"View -> ZoomIn"$X (click in
    the results window and move the mouse to specify a zooming area and
    click again). Now, execute $R"View -> Measure"$X, and click on the right
    mouse button to watch the integer values of the multiplied signals.
*********************************************************************$B
7.$X  To exit the$R simeye$X program:
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
echo "+ rm -f sim.diag sls sls.funlist deffunc.c total.out total.plt total.res"
rm -f sim.diag sls sls.funlist deffunc.c total.out total.plt total.res
fi
