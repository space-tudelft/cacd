#!/bin/sh
# directory: demo/poly5

R="[1;31m"
G="[1;32m"
B="[1;34m"
X="[0m"

unset CWD
clear
cat <<!
$G*********************************************************************
** $R Demonstration of Space 3D Capacitance Extraction             $G  **
** $R using 5 parallel conductors.  See also section "5.1" in the  $G  **
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
    We shall use the$R scmos_n$X technology and a lambda of$R 0.05$X micron.
    We change the current working directory $R'.'$X into a project directory
    with the following command:

    % mkpr -p scmos_n -l 0.05 .
$G*********************************************************************$X
!
printf "Shall we do it? (type$R Enter$X)"; read ok
if test "$ok" == n
then
echo "Ok, see you a next time!"
exit 1
fi

echo "+ mkpr -p scmos_n -l 0.05 ."
mkpr -p scmos_n -l 0.05 .

cat <<!

$G*********************************************************************$B
STEP 2.$X
    The layout of the$R 5$X conductors is available in a gds2 file.
    We use$R cgi$X (convert-gds-internal) to put the layout into the
    project database with the following command:

    % cgi poly5.gds
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ cgi poly5.gds"
cgi poly5.gds
fi
fi

cat <<!

$G*********************************************************************$B
2b.$X We can inspect the layout with the layout editor$R dali$X.
    The layout is stored in the database using cell name $R"poly5"$X.
    To show the layout, we use the following command:

    % dali poly5

    To exit the program, click on $R'-quit-'$X and click on $R'yes'$X.
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ dali poly5"
dali poly5
fi

cat <<!

$G*********************************************************************$B
3.$X  We use a local technology file for the extraction of the circuit.
    This element definition file needs to be compiled before usage.
    Use the technology compiler$R tecc$X to create the $R"tech.t"$X file.
    Use the following command:

    % tecc tech.s
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ tecc tech.s"
tecc tech.s
fi

cat <<!

$G*********************************************************************$B
4.$X  Perform an extraction of cell$B poly5$X in batch mode.  Extract
    3D capacitances (using option $R-C3$X).  The other options are:
       $R-E$G<file>$X to use an alternate element definition file,
       $R-P$G<file>$X to use an alternate parameter file.
    The command used is:

    % space3d -C3 -E tech.t -P param.p poly5
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ space3d -C3 -E tech.t -P param.p poly5"
space3d -C3 -E tech.t -P param.p poly5
fi

cat <<!

$G*********************************************************************$B
4b.$X We can also perform an interactive extraction of cell$B poly5$X.
    Using the$R Xspace$X program with following command:

    % Xspace -C3 -E tech.t -P param.p poly5
*********************************************************************$B
Note:$X
    You must set some display options (what you want to see), go
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
echo "+ Xspace -C3 -E tech.t -P param.p poly5"
Xspace -C3 -E tech.t -P param.p poly5
fi

cat <<!

$G*********************************************************************$B
5.$X  We can now inspect the extraction result, the extracted circuit.
    To see a$R SPICE$X description (or netlist), we use the$R xspice$X
    command to eXstract SPICE from the database:

    % xspice -a poly5                     (see:$R icdman$X xspice)
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ xspice -a poly5"
xspice -a poly5
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
echo "+ rm -f display.out exp_dat tech.t"
rm -f display.out exp_dat tech.t
fi
