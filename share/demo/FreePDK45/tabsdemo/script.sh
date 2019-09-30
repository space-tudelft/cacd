#!/bin/sh
# directory: demo/FreePDK45/tabsdemo

R="[1;31m"
G="[1;32m"
B="[1;34m"
X="[0m"

unset CWD
clear
cat <<!
$G*********************************************************************
** $R Demonstration of flat space extractions of a counter cell    $G  **
** $R made with the FreePDK45 process and the use of the tabs tool.$G  **
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

if ! test -f ../NangateOpenCellLibrary/.dmrc
then
echo "sorry, the NangateOpenCellLibrary is not yet installed"
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
    We shall use the$R FreePDK45$X technology and the lambda value of the
    NangateOpenCellLibrary.  We assume that the$B NangateOpenCellLibrary$X
    project is installed.  We change the current working directory $R'.'$X
    into a project directory with the following command:

    % mkpr -u ../NangateOpenCellLibrary .
$G*********************************************************************$X
!
printf "Shall we do it? (type$R Enter$X)"; read ok
if test "$ok" == n
then
echo "Ok, see you a next time!"
exit
fi

echo "+ mkpr -u ../NangateOpenCellLibrary ."
mkpr -u ../NangateOpenCellLibrary .

cat <<!

$G*********************************************************************$B
STEP 2.$X
    To use the$B NangateOpenCellLibrary$X project, it must be added
    to the local project list.  Use the following command:

    % addproj ../NangateOpenCellLibrary
$G*********************************************************************$X
!
printf "Shall we do it? (type$R Enter$X)"; read ok
if test "$ok" != n
then
echo "+ addproj ../NangateOpenCellLibrary"
addproj ../NangateOpenCellLibrary
fi

cat <<!

$G*********************************************************************$B
STEP 3.$X
    To be able to use$B the basic cells$X of the NangateOpenCellLibrary we
    must import them in our project.  Use the following command:

    % impcell -lc -a ../NangateOpenCellLibrary
$G*********************************************************************$X
!
printf "Shall we do it? (type$R Enter$X)"; read ok
if test "$ok" != n
then
echo "+ impcell -lc -a ../NangateOpenCellLibrary"
impcell -lc -a ../NangateOpenCellLibrary
fi

cat <<!

$G*********************************************************************$B
STEP 4.$X
    The$B counter$X layout is available in a gds2 file.  We use$R cgi$X
    (convert-gds-internal) to put the layout into the project database
    and make a hierarchical listing.  We use the following commands:

    % cgi c23.gds
    % dblist -h c23
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ cgi c23.gds"
cgi c23.gds
echo "+ dblist -h c23"
dblist -h c23
fi
fi

cat <<!

$G*********************************************************************$B
STEP 5.$X
    Now, we can inspect the$R c23$B counter$X layout with the layout editor$R
    dali$X.  Note that it contains 5 sub cells with the name$B cnt1b$X.
    Cell$B cnt1b$X itself contains 4 basic cells (1 AND2, 1 DFF and 2 MUX2).
    Start$R dali$X with the following command:

    % dali c23

    To see an extra hierarchical level, type $R'2'$X. Use the $R'i'$X key
    to zoom-in and the arrow keys. Use $R'b'$X to get the bbox-window.
    To exit the program, click on $R'-quit-'$X and click on $R'yes'$X.
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ dali c23"
dali c23
fi

cat <<!

$G*********************************************************************$B
STEP 6.$X
    We use a local space$B technology file$R space.def.s$X which first must
    be compiled, before we can use it for the space extractor.  Type:

    % tecc space.def.s

    The output file gets the name$R space.def.t$X and shall be specified
    on the space command line with option$R -E$X.
$G*********************************************************************$X
!
printf "Shall we compile?"; read ok
if test "$ok" != n
then
echo "+ tecc space.def.s"
tecc space.def.s
fi

cat <<!

$G*********************************************************************$B
STEP 7.$X
    Now, perform a$B flat netlist extraction$X of the$R c23$X layout. Note
    that the basic cells are not extracted, because they are$B library$X
    cells.  Type the following command to used the space extractor:

    % space -Fv -E space.def.t -P space.def.p c23

    Note that we specify also a local space$B parameter file$R space.def.p$X
    on the command line with option$R -P$X.  The$R -F$X option is for$B
    flat$X and the$R -v$X option is for$B verbose$X mode.
$G*********************************************************************$X
!
printf "Shall we extract?"; read ok
if test "$ok" != n
then
echo "+ space -Fv -E space.def.t -P space.def.p c23"
space -Fv -E space.def.t -P space.def.p c23
fi

cat <<!

$G*********************************************************************$B
STEP 8.$X
    We can now inspect the extraction result, the extracted circuit.
    To see a$R SLS$X description (or netlist), we use the$R xsls$X
    command to eXstract SLS from the database:

    % xsls c23                  (see:$R icdman$X xsls)
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ xsls c23"
xsls c23
fi

cat <<!

$G*********************************************************************$B
STEP 9.$X
    We can perform a logic$R sls$X simulation on this netlist.
    We use the$R simeye$X program to run the simulator and to show
    the resulting waveforms.  Type:

    % simeye

    Note: Click on the $B"Simulate" menu$X and choice item $R"Prepare"$X.
    Select in the $R"Circuit:"$X field cell name $R"c23"$X and in the
    $R"Stimuli:"$X field file $R"c23.cmd"$X and click the $R"Run"$X button.
    To measure the logic values, open the $B"View" menu$X and click on
    $R"Measure"$X. Now you can move a ruler in the window. If you click
    on the right mouse button, you get logic values on the right side.
    To leave the program, choice item $R"Exit"$X in the $B"File" menu$X.
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ simeye"
simeye
fi

cat <<!

$G*********************************************************************$B
STEP 10.$X
    If we want to get a flat netlist to the$B transistor level$X, we must
    overrule the library status of the used basic cells.
    To do so, give the following command:

    % xcontrol -regular AND2_X1 AND3_X1 DFF_X1 MUX2_X1 OR2_X1
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ xcontrol -regular AND2_X1 AND3_X1 DFF_X1 MUX2_X1 OR2_X1"
xcontrol -regular AND2_X1 AND3_X1 DFF_X1 MUX2_X1 OR2_X1
fi

cat <<!

$G*********************************************************************$B
STEP 11.$X
    Now we can perform a complete flat netlist extraction of the$R c23$X
    layout.  Run space with the following command:

    % space -Fv -E space.def.t -P space.def.p c23
$G*********************************************************************$X
!
printf "Shall we extract?"; read ok
if test "$ok" != n
then
echo "+ space -Fv -E space.def.t -P space.def.p c23"
space -Fv -E space.def.t -P space.def.p c23
fi

cat <<!

$G*********************************************************************$B
STEP 12.$X
    To generate a SPICE netlist, we use the following command:

    % xspice -auo c23                (see:$R icdman$X xspice)

    The$R -u$X option omits addition of transistor bulk connections.
    The$R -o$X option omits the transistor model parameters.
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ xspice -auo c23"
xspice -auo c23
fi

cat <<!

$G*********************************************************************$B
STEP 13.$X
    We can also perform a cap2D extraction. Before we can do that we
    must have a 2D capacitances section in the space technology file.
    To generate these 2D capacitances, you can use the$R tabs$X tool.
    We use the following command:

    % tabs -s space.def.s

    Note that we do not generate 2D capacitances for all metal layers,
    because this is very time consuming. We use in this example only
    the first two metal layers (metal1 and metal2).
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ tabs -s space.def.s"
tabs -s space.def.s
fi

cat <<!

$G*********************************************************************$B
STEP 14.$X
    Now we can perform a complete flat netlist extraction of the$R c23$X
    layout with 2D caps.  Run space with the following command:

    % space -Fv -Cl -E space.def.t -P space.def.p c23
$G*********************************************************************$X
!
printf "Shall we extract?"; read ok
if test "$ok" != n
then
echo "+ space -Fv -Cl -E space.def.t -P space.def.p c23"
space -Fv -Cl -E space.def.t -P space.def.p c23
fi
printf "Shall we netlist?"; read ok
if test "$ok" != n
then
echo "+ xspice -auo c23"
xspice -auo c23
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
echo "+ rm -f exp_dat space.def.t sim.diag c23.res c23.out"
rm -f exp_dat space.def.t sim.diag c23.res c23.out
fi
