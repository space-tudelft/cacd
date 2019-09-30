#!/bin/sh
# directory: demo/FreePDK45/I8051_ALL

R="[1;31m"
G="[1;32m"
B="[1;34m"
X="[0m"

unset CWD
clear
cat <<!
$G*********************************************************************
** $R Demonstration of a flat Space Extraction to basic cell nivo  $G  **
** $R of a 8051 processor made with the FreePDK45 process.         $G  **
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
echo "Ok, see you a next time!"
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
    NangateOpenCellLibrary.  We assume that the NangateOpenCellLibrary
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
    To use the NangateOpenCellLibrary project, it must be added
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
    To use all basic cells of the NangateOpenCellLibrary we must import
    them in our project.  Use the following command:

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
    The$B 8051 processor$X layout is available in a gds2 file.
    We use$R cgi$X (convert-gds-internal) to put the layout into the
    project database.  We use a non-default basic mask list "bmlist3.gds"
    for the layer numbers.  Thus, we use the following command:

    % cgi I8051_ALL.gds -m bmlist3.gds
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ cgi I8051_ALL.gds -m bmlist3.gds"
cgi I8051_ALL.gds -m bmlist3.gds
fi
fi

cat <<!

$G*********************************************************************$B
STEP 5.$X
    Now, we can inspect the$B processor$X layout with the layout editor$R dali$X.
    We use the following command:

    % dali I8051_ALL

    To see an extra hierarchical level, type $R'2'$X. Use the $R'i'$X key
    to zoom-in and the arrow keys. Use $R'b'$X to get the bbox-window.
    To exit the program, click on $R'-quit-'$X and click on $R'yes'$X.
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ dali I8051_ALL"
dali I8051_ALL
fi

cat <<!

$G*********************************************************************$B
STEP 6.$X
    Now, perform a flat netlist extraction of the I8051 layout.  Note
    that the basic cells are not extracted, because they are library cells.
    Type the following command to used the space extractor:

    % space -Fv I8051_ALL
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ space -Fv I8051_ALL"
space -Fv I8051_ALL
fi

cat <<!

$G*********************************************************************$B
NOTE:$X
    When you see the message$R "makegln: Temp file compression turned off"$X
    then this is ok, because not always the compression tools are installed.
    In the statis report you see that there are 606$R d/s capacitances$X
    extracted. This is strange, because there are no transistors extracted.
    We can fix this problem by changing the space technology file.
    Change in the$B fets$X section the conditions for the$B dsarea$X caps.
    Change the following 2 lines:
	nenh:( ... ):poly active ( !nwell nimplant active !poly ):@sub
	penh:( ... ):poly active ( nwell pimplant active !poly ):nwell
    Into:
	nenh:( ... ):poly active ( !nwell nimplant !pimplant active !poly ):@sub
	penh:( ... ):poly active ( nwell !nimplant pimplant active !poly ):nwell
$G*********************************************************************$X

$G*********************************************************************$B
STEP 7.$X
    We can now inspect the extraction result, the extracted circuit.
    To see a$R VHDL$X description (or netlist), we use the$R xvhdl$X
    command to eXstract VHDL from the database:

    % xvhdl -f I8051_ALL                     (see:$R icdman$X xvhdl)
    % more I8051_ALL.vhd                     (use 'q' to quit more)
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ xvhdl -f I8051_ALL"
xvhdl -f I8051_ALL
echo "+ more I8051_ALL.vhd"
more I8051_ALL.vhd
fi

cat <<!

$G*********************************************************************$B
STEP 8.$X
    When we want to perform a flat netlist extraction of the I8051 layout
    to the transistor level.  We need to change the status of the basic
    library cells into$B regular$X with the$R xcontrol$X program.

    % xcontrol -regular \`cat libcells\`

    Note that the$B libcells$X file contains a list of all library cells
    that must be changed (it contains not the FILLCELLs). The output of
    the UNIX$R cat$X command puts each cell name on the command line.
   (If you want to go back to basic cells level, use option$B -library$X.)
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo '+ xcontrol -regular `cat libcells`'
xcontrol -regular `cat libcells`
fi

cat <<!

$G*********************************************************************$B
STEP 9.$X
    Now we can perform a flat netlist extraction of the I8051 layout to
    the transistor level.  We use the following command:

    % space -Fv -t I8051_ALL

    Note: We add the option$R -t$X to show the device positions.
          Don't use option$R -x$X, because it generates backannotation
          data for another program. This cost a lot of time for
          a large layout such as the I8051.
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ space -Fv -t I8051_ALL"
space -Fv -t I8051_ALL
fi

cat <<!

$G*********************************************************************$B
NOTE:$X
    You see that space has detected a big number of problem transistors.
    This happens, because the FILLCELLs make contacts on the power lines
    to the substrate and to the nwell area. There the$B active$X layer is
    used and this layer is sometimes touched by the$B poly$X layer of
    a basic cell.
    Note that you can instruct$R space$X to skip these transistors by
    specifying the parameter$R omit_incomplete_tors$X.
    We can also fix this problem by changing the space technology file.
    Change in the$B fets$X section the conditions for the$B gate$X area.
    Change the following 2 lines:
	nenh:( !nwell pwell nimplant active poly ):poly active (..):@sub
	penh:( nwell !pwell pimplant active poly ):poly active (..):nwell
    Into:
	nenh:( !nwell pwell nimplant !pimplant active poly ):poly active (..):@sub
	penh:( nwell !pwell pimplant !nimplant active poly ):poly active (..):nwell
$G*********************************************************************$X
!
printf "Shall we go on?"; read ok
if test "$ok" == n
then
exit
fi

cat <<!

$G*********************************************************************$B
STEP 10.$X
    Now we go to inspect the result, the extracted circuit.  We can
    generate for example a SLS or SPICE netlist description.  We choice
    in this case for a SLS netlist, because it is good readable.

    % xsls I8051_ALL | head -40

    Note that we list only the first 40 lines.
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ xsls I8051_ALL | head -40"
xsls I8051_ALL | head -40
fi

cat <<!

$G*********************************************************************$B
STEP 11.$X
    We shall also list a part of the SPICE netlist description.
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ xspice -au I8051_ALL | head -40"
xspice -au I8051_ALL | head -40
fi

cat <<!

$G*********************************************************************$B
NOTE:$X
    You see that each transistor has a list of drain/source parameters.
    For example$R ad$X is$B area-drain$X and$R pd$X is$B perimeter-drain$X.
$G*********************************************************************$X

$G*********************************************************************$R
    Congratulations, you have now finished this demo!
$G*********************************************************************$X
!
printf "Shall we clean the demo directory and exit?"; read ok
if test "$ok" != n
then
echo "+ rmpr -fs ."
rmpr -fs .
echo "+ rm -f exp_dat"
rm -f exp_dat
fi
