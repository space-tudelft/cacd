#!/bin/sh
# directory: demo/sub3term

R="[1;31m"
G="[1;32m"
B="[1;34m"
X="[0m"

unset CWD
clear
cat <<!
$G*********************************************************************
** $R Demonstration of Space Substrate Resistance Extraction       $G  **
** $R using 3 substrate terminals.  See also section "4.7" in the  $G  **
** $R  "Space Substrate Resistance Extraction User's Manual".      $G  **
*********************************************************************$X
Using option $R-B$X (the more$R accurate 3D BE-method$X)

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
    We shall use the$R scmos_n$X technology and a lambda of$R 0.1$X micron.
    We change the current working directory $R'.'$X into a project directory
    with the following command:

    % mkpr -p scmos_n -l 0.1 .
$G*********************************************************************$X
!
printf "Shall we do it? (type$R Enter$X)"; read ok
if test "$ok" == n
then
echo "Ok, see you a next time!"
exit
fi

echo "+ mkpr -p scmos_n -l 0.1 ."
mkpr -p scmos_n -l 0.1 .

cat <<!

$G*********************************************************************$B
STEP 2.$X
    The layout of the$R 3$X terminals is available in a gds2 file.
    We use$R cgi$X (convert-gds-internal) to put the layout into the
    project database with the following command:

    % cgi sub3term.gds
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ cgi sub3term.gds"
cgi sub3term.gds
fi
fi

cat <<!

$G*********************************************************************$B
STEP 3.$X
    Now, we can inspect the layout with the layout editor$R dali$X.
    The layout is stored in the database using cell name $R"sub3term"$X.
    To show the layout, we use the following command:

    % dali sub3term

    To exit the program, click on $R'-quit-'$X and click on $R'yes'$X.
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ dali sub3term"
dali sub3term
fi

cat <<!

$G*********************************************************************$B
STEP 4.$X
    We use a local technology file for the extraction of the circuit.
    This element definition file needs to be compiled before usage.
    Use the technology compiler$R tecc$X to create the $R"elem.t"$X file.
    Use the following command:

    % tecc elem.s

    Note: The warning about a missing via mask is ok.
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ tecc elem.s"
tecc elem.s
fi

cat <<!

$G*********************************************************************$B
5.$X  Perform an extraction of cell sub3term in batch mode.
    Extract substrate resistances using option $R-B$X.
    The other options are: $R-v$X for verbose, $R-E$G<file>$X
    to use an alternate element definition file,
    and $R-P$G<file>$X to use an alternate parameter file.
    The command used is:

    % space3d -v -E elem.t -P param.p -B sub3term
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ space3d -v -E elem.t -P param.p -B sub3term"
space3d -v -E elem.t -P param.p -B sub3term
fi

cat <<!

$G*********************************************************************$B
5b.$X We can also perform an interactive extraction of cell sub3term.
    Using the$R Xspace$X program with following command:

    % Xspace -E elem.t -P param.p
*********************************************************************
Note: Because the cell name is not specified, we must set the cell first:
    click on button $R'sub3term'$X in the $B'Database' menu$X.
    And to set the extraction options, go to the $B'Options' menu$X and
    click button$R <3D sub. res.>$X.
    You must also set some display options (what you want to see), go to
    the $B'Display' menu$X and click button $R'DrawBEMesh'$X and $R'DrawGreen'$X.
    Now, to start the extraction, go to the $B'Extract' menu$X and click $R'extract'$X
    (note that you can also use the hotkey $R'e'$X).  To leave the program,
    click on $R'quit'$X in the $B'Extract' menu$X (or use hotkey $R'q'$X).
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ Xspace -E elem.t -P param.p"
Xspace -E elem.t -P param.p
fi

cat <<!

$G*********************************************************************$B
6.$X  We can now inspect the extraction result, the extracted circuit.
    To see a$R SPICE$X description (or netlist), we use the$R xspice$X
    command to eXstract SPICE from the database:

    % xspice -a sub3term                  (see:$R icdman$X xspice)
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ xspice -a sub3term"
xspice -a sub3term
fi

cat <<!

$G*********************************************************************$B
7.$X  Optionally, we can use Xspace in special prepass mode to view the
    substrate boundary element mesh and the Green functions calculated
    between the points in the mesh (not doing an extraction).
    Using the following command:

    % Xspace -%Z -B -E elem.t -P param.p sub3term
*********************************************************************
    Set the display options, go to the $B'Display' menu$X and click
    button $R"DrawBEMesh"$X and $R"DrawGreen"$X. Go to the $B'Extract' menu$X
    and click $R"extract"$X (or hotkey $R'e'$X). Type $R'q'$X to$R exit$X.
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ Xspace -%Z -B -E elem.t -P param.p sub3term"
Xspace -%Z -B -E elem.t -P param.p sub3term
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
echo "+ rm -f display.out exp_dat elem.t"
rm -f display.out exp_dat elem.t
fi
