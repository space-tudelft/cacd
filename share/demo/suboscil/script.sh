#!/bin/sh
# directory: demo/suboscil

R="[1;31m"
G="[1;32m"
B="[1;34m"
X="[0m"

unset CWD
clear
cat <<!
$G*********************************************************************
** $R Demonstration of Space Substrate Resistance Extraction       $G  **
** $R using a CMOS Ring Oscillator.  See also section "5.5" in the $G  **
** $R  "Space Substrate Resistance Extraction User's Manual".      $G  **
*********************************************************************$X
First using option $R-b$X (the fast$R interpolation method$X)
Second using option $R-B$X (the more$R accurate 3D BE-method$X)

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
    We change the cwd into a project directory with the following command:

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
    The oscillator layout is available in a GDS2 file.
    We use$R cgi$X (convert-gds-internal) to put the layout into the project
    database with the following command:

    % cgi oscil.gds
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ cgi oscil.gds"
cgi oscil.gds
fi
fi

cat <<!

$G*********************************************************************$X
    You can show the hierarchical layout tree with the$R dblist$X command:

    % dblist -h
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ dblist -h"
dblist -h
cat <<!
You see$R 2$X hierarchical levels. The top cell$R 'oscil'$X has$R 9$X sub-cells.
These sub-cells have the name$R 'inv'$X and are invertors.
!
fi

cat <<!
$G*********************************************************************$B
STEP 3.$X
    You can use the layout editor$R dali$X to inspect the layout.
    We do the following command:

    % dali oscil

Note: To see more levels, click on $B'DB_menu'$X, then on $R'all_exp'$X, then
      on $R'maximum'$X (or use a hotkey, for example $R'2'$X for 2 levels).
Note: To exit the program, click on $R'-return-'$X and $R'-quit-'$X (or
      use the hotkey $R'q'$X) and click on $R'yes'$X.
(the hotkeys work only if the mouse cursor is in the dali window)
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ dali oscil"
dali oscil
fi

cat <<!

$G*********************************************************************$B
4.$X  Perform an extraction of cell$B oscil$X in batch mode.  Extract
    substrate resistances (using option $R-b$X) and coupling capacitances
    (using option $R-C$X). The other options are: $R-v$X for verbose, $R-F$X for
    flat extraction and $R-P$G<file>$X to use an alternate parameter file.
    Note that the $R-F$X option is not really needed for substrate res
    extraction, because that is the default way to do it.
    The command used is:

    % space3d -vF -P param.p -bC oscil
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ space3d -vF -P param.p -bC oscil"
space3d -vF -P param.p -bC oscil
fi

cat <<!

$G*********************************************************************$B
4b.$X We can also perform an interactive extraction of cell$B oscil$X.
    Using the$R Xspace$X program with following command:

    % Xspace -P param.p
*********************************************************************
Note: Because the cell name is not specified, we must set the cell first:
    click on button $R'oscil'$X in the $B'Database' menu$X.
    And to set the extraction options, go to the $B'Options' menu$X and
    click button$R <inter. sub. res.>$X and$R <coupling cap.>$X.
    ($R<inter. sub. res.>$X = interpolated substrate resistances)
    You must also set some display options (what you want to see), go to
    the $B'Display' menu$X and click button $R'DrawSubTerm'$X and $R'DrawSubResistor'$X.
    Now, to start the extraction, go to the $B'Extract' menu$X and click $R'extract'$X
    (note that you can also use the hotkey $R'e'$X).  To leave the program,
    click on $R'quit'$X in the $B'Extract' menu$X (or use hotkey $R'q'$X).
*********************************************************************
Note: To show colored substrate terminals, click on $R'FillSubTerm'$X in
the $B'Display' menu$X and use hotkey $R'a'$X to view the result 'again'.
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ Xspace -P param.p"
Xspace -P param.p
fi

cat <<!

$G*********************************************************************$B
5.$X  We can now inspect the extraction result, the extracted circuit.
    To see a$R SPICE$X description (or netlist), we use the$R xspice$X
    command to eXstract SPICE from the database:

    % xspice -a oscil                     (see:$R icdman$X xspice)
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ xspice -a oscil"
xspice -a oscil
fi

cat <<!
$G*********************************************************************$B
6.$X  If you have a$R spice3$X simulator available, you can perform a spice3
    simulation using the simulation GUI$R simeye$X.
Note: Check the shell script$R nspice$X in \$ICDPATH/share/bin to see
    if spice3 is called correctly.
    To start simeye, the following command is used:

    % simeye

Note: Click on the $B"Simulate" menu$X and choice item $R"Prepare"$X.
     Select in the $R"Circuit:"$X field cell name $R"oscil"$X and
     in the $R"Stimuli:"$X field file name $R"oscil.cmd"$X.
     Choice simulation $R"Type: spice"$X and click on button $R"Run"$X.
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
7.$X  Perform an extraction of cell$B oscil$X in batch mode.
    Extract now substrate resistances using the$B BEM method$X.
    This method can only be used with the 3D space extractor.
    Thus, use$R space3d$X and use the option $R-B$X for BEM.

    % space3d -vF -P param.p -BC oscil
 or
    % tecc tech.s ; space3d -vF -P param.p -E tech.t -BC oscil

    (both circuit extractions must be equal)
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ space3d -vF -P param.p -BC oscil"
space3d -vF -P param.p -BC oscil
fi

cat <<!

$G*********************************************************************$B
7b.$X Perform also an interactive substrate BEM extraction:

    % Xspace -P param.p -BC oscil

Note: Click button $R"DrawBEMesh"$X, $R"DrawSubTerm"$X and $R"DrawSubResistor"$X in
    the$B menu "Display"$X, and use hotkey $R'e'$X to $R"extract"$X.
Use hotkey $R'i'$X to zoom-in on cursor position and the arrow-keys to pan around.
Use hotkey $R'o'$X to zoom-out and $R'b'$X to set bounding-box view again.
You see$R 19$X substrate contact terminals, but each has a very
fine boundary element mesh (BE-mesh).
*********************************************************************
Use hotkey $R'q'$X to$R exit$X the Xspace program.
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ Xspace -P param.p -BC oscil"
Xspace -P param.p -BC oscil
fi

cat <<!

$G*********************************************************************$B
8.$X  We do step$B 7$X again, but now using another technology file.
    We use the following command, to look what is different:

    % diff tech.s tech2.s
$G*********************************************************************$X
!
printf "Shall we diff?"; read ok
if test "$ok" != n
then
echo "+ diff tech.s tech2.s"
diff tech.s tech2.s
fi

cat <<!

$G*********************************************************************$B
8.$X  We have changed $R@gnd$X into $R@sub$X. Each @sub generates substrate
    contact terminal areas. Compile this technology file using command:

    % tecc tech2.s
$G*********************************************************************$X
!
printf "Shall we compile?"; read ok
if test "$ok" != n
then
echo "+ tecc tech2.s"
tecc tech2.s
echo "Note that the warnings of swapping pins are ok."
fi

cat <<!

$G*********************************************************************$B
8.$X  We use also another space parameter file $R"param2.p"$X.
    The following command shows what is different:

    % diff param.p param2.p
$G*********************************************************************$X
!
printf "Shall we diff?"; read ok
if test "$ok" != n
then
echo "+ diff param.p param2.p"
diff param.p param2.p
fi

cat <<!

$G*********************************************************************$B
8b.$X We have set parameter $R"sep_sub_term"$X to $R"on"$X, because we want
    separate substrate terminals for the poly and metal areas (else
    everything is merged). Because we are now using a very large
    substrate area, we must reduce the number of BE elements (else
    the calculation costs too much time). Thus, we use a default
    $R"edge_be_ratio"$X and a $R"max_be_area"$X of$R 10$X micron^2.
    We are also using a smaller $R"be_window"$X size of$R 4$X micron.
    Perform an extraction of cell$B oscil$X in batch mode:

    % space3d -vF -P param2.p -E tech2.t -BC oscil
$G*********************************************************************$X
!
printf "Shall we extract?"; read ok
if test "$ok" != n
then
echo "+ space3d -vF -P param2.p -E tech2.t -BC oscil"
space3d -vF -P param2.p -E tech2.t -BC oscil
fi

cat <<!

$G*********************************************************************$B
8c.$X Perform also an interactive extraction:

    % Xspace -P param2.p -E tech2.t -BC oscil

Note:
    Use directly $R'e'$X to $R"extract"$X, the parameter file contains
    the other needed settings to get a nice picture.
    Note that a $R-b$X extraction does not work, because at this moment
    the program makedela cannot handle these substrate areas laying
    against each other.
    Note that you can also make the substrate terminals distributed.
    Then you get more separate substrate terminal areas. But this
    works only for $R-B$X in combination with interconnect res extraction.
    And the causing conductors must have a high-ohmic sheet res value.
    Set parameter $R"low_sheet_res"$X to$R 0$X ohm to make all conductors
    high-ohmic (note that conductor $R"cwn"$X cannot be set high-ohmic,
    because it has a zero sheet res value).
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ Xspace -P param2.p -E tech2.t -BC oscil"
Xspace -P param2.p -E tech2.t -BC oscil
fi

cat <<!

$G*********************************************************************$B
8d.$X Perform the extraction with distributed interconnect res:

    % space3d -vF -P param2.p -E tech2.t -BCr oscil

    You see that there are a lot more substrate terminals.
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ space3d -vF -P param2.p -E tech2.t -BCr oscil"
space3d -vF -P param2.p -E tech2.t -BCr oscil
fi

cat <<!

$G*********************************************************************$B
8e.$X Perform also an interactive extraction:

    % Xspace -P param2.p -E tech2.t -BCr oscil

Note:
    After you have done $R'e'$X to $R"extract"$X, put button $R"DrawBEMesh"$X
    off to see only the substrate terminals with there borders.
    You do not need to extract again. Use hotkey $R'a'$X to view the
    display output file again, but with-out the mesh.
*********************************************************************
Type $R'q'$X to$R exit$X the Xspace program.
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ Xspace -P param2.p -E tech2.t -BCr oscil"
Xspace -P param2.p -E tech2.t -BCr oscil
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
echo "+ rm -f display.out exp_dat oscil.spc tech.t tech2.t"
rm -f display.out exp_dat oscil.spc tech.t tech2.t
echo "+ rm -f sim.diag oscil.ana oscil.out oscil.plt oscil.res"
rm -f sim.diag oscil.ana oscil.out oscil.plt oscil.res
fi
