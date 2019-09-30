#!/bin/sh
# directory: demo/FreePDK45/ringosc

R="[1;31m"
G="[1;32m"
B="[1;34m"
X="[0m"

unset CWD
clear
cat <<!
$G*********************************************************************
** $R Demonstration of a flat Space Extraction                     $G  **
** $R of a ring-oscillator made with the FreePDK45 process.        $G  **
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
    The$B ring-oscillator$X layout is available in a gds2 file.
    We use$R cgi$X (convert-gds-internal) to put the layout into the
    project database.  Use the following command:

    % cgi ringosc.gds
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ cgi ringosc.gds"
cgi ringosc.gds
fi
fi

cat <<!

$G*********************************************************************$B
STEP 5.$X
    We can inspect the$B oscillator$X layout with the layout editor$R dali$X.
    Note that it contains 9 imported basic cells with the name INV_X2.
    We start$R dali$X with the following command:

    % dali ringosc

    To see an extra hierarchical level, type $R'2'$X. Use the $R'i'$X key
    to zoom-in and the arrow keys. Use $R'b'$X to get the bbox-window.
    To exit the program, click on $R'-quit-'$X and click on $R'yes'$X.
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ dali ringosc"
dali ringosc
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
    Now, perform a$B flat netlist extraction$X of the oscillator layout.  Note
    that the basic cells are not extracted, because they are library cells.
    Type the following command to used the space extractor:

    % space -Fv -E space.def.t -P space.def.p ringosc

    Note that we specify also a local space$B parameter file$R space.def.p$X
    on the command line with option$R -P$X.  The$R -F$X option is for$B
    flat$X and the$R -v$X option is for$B verbose$X mode.
$G*********************************************************************$X
!
printf "Shall we extract?"; read ok
if test "$ok" != n
then
echo "+ space -Fv -E space.def.t -P space.def.p ringosc"
space -Fv -E space.def.t -P space.def.p ringosc
fi

cat <<!

$G*********************************************************************$B
STEP 8.$X
    We can now inspect the extraction result, the extracted circuit.
    To see a$R SPICE$X description (or netlist), we use the$R xspice$X
    command to eXstract SPICE from the database:

    % xspice -a ringosc                  (see:$R icdman$X xspice)

    The$R -a$X option gives node names instead of node numbers.
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ xspice -a ringosc"
xspice -a ringosc
fi

cat <<!

$G*********************************************************************$B
STEP 9.$X
    To get a flat netlist extraction to the$B transistor level$X, we must
    overrule the library status of the INV_X2 basic cell.
    To do so, give the following command:

    % xcontrol -regular INV_X2

    Note: If there is no local cell status, the remote cell status
          is used.
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ xcontrol -regular INV_X2"
xcontrol -regular INV_X2
fi

cat <<!

$G*********************************************************************$B
STEP 10.$X
    Now we can perform a complete flat netlist extraction of the ring-
    oscillator layout.  Run space with the following command:

    % space -Fv -E space.def.t -P space.def.p ringosc
$G*********************************************************************$X
!
printf "Shall we extract?"; read ok
if test "$ok" != n
then
echo "+ space -Fv -E space.def.t -P space.def.p ringosc"
space -Fv -E space.def.t -P space.def.p ringosc
fi

cat <<!

$G*********************************************************************$B
STEP 11.$X
    To generate a SPICE netlist give the following command:

    % xspice -au ringosc                (see:$R icdman$X xspice)

    The$R -u$X option omits addition of transistor bulk connections.
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ xspice -au ringosc"
xspice -au ringosc
fi

cat <<!

$G*********************************************************************$B
STEP 12.$X
    We can also extract the netlist with 2D capacitances, type:

    % space -Fv -Cl -E space.def.t -P space.def.p ringosc

    Note that the option$R -C$X is used for couple cap extraction
    and the option$R -l$X for adding also the lateral couple caps.
$G*********************************************************************$X
!
printf "Shall we extract?"; read ok
if test "$ok" != n
then
echo "+ space -Fv -Cl -E space.def.t -P space.def.p ringosc"
space -Fv -Cl -E space.def.t -P space.def.p ringosc
fi
printf "Shall we netlist?"; read ok
if test "$ok" != n
then
echo "+ xspice -au ringosc"
xspice -au ringosc
fi

cat <<!

$G*********************************************************************$B
STEP 13.$X
    We can perform an analog spice simulation on this netlist.
    We use the$R simeye$X program to run the simulator and to show
    the resulting waveforms (note that the spice3 simulator must be
    available on your system).  Type:

    % simeye

Note: Click on the $B"Simulate" menu$X and choice item $R"Prepare"$X.
     Select in the $R"Circuit:"$X field cell name $R"ringosc"$X and
     in the $R"Stimuli:"$X field file name $R"ringosc.cmd"$X.
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
STEP 14.$X
    We can also extract the netlist with 3D capacitances and simulate
    with$B spice$X again.  We use the following commands:

    % space3d -Fv -C3 -E space.def.t -P space.def.p ringosc
    % simeye

    Note that option$R -3$X is used in place of the$R -l$X option.
    In 3D mode all kind of couple capacitances are extracted.
    Note that the 3D version of space must be used.
$G*********************************************************************$X
!
printf "Shall we extract?"; read ok
if test "$ok" != n
then
echo "+ space3d -Fv -C3 -E space.def.t -P space.def.p ringosc"
space3d -Fv -C3 -E space.def.t -P space.def.p ringosc
fi
printf "Shall we simulate again?"; read ok
if test "$ok" != n
then
echo "+ simeye"
simeye
fi

cat <<!

$G*********************************************************************$B
STEP 15.$X
    You must know, that for 3D capacitance extraction a 3D Boundary
    Element Mesh (BEM) is used. It is nice to show how this mesh looks.
    With the$R Xspace$X tool you can show what$R space3d$X is doing.
    The Xspace tool runs the space3d extractor for you in flat mode.
    In that case space3d knows that it must make a$B display.out$X file.
    And Xspace reads this$B display.out$X file, while space3d is extracting.
    Use the following command:

    % Xspace -C3 -E space.def.t -P space.def.p ringosc

    To start the extraction, go with the mouse to the$B Extract$X menu and
    click on the$B extract$X button (you can also use$R 'e'$X hotkey instead).
    If you like, you can direct switch the view from 2D into 3D (and back).
    This is only possible with the$R '*'$X hotkey. Note that the 3D mesh picture
    can be rotated with the arrow-keys of the small keypad (with the other
    arrow-keys you can shift). You can also zoom-in with$R 'i'$X and zoom-out
    with$R 'o'$X. And with$R 'b'$X you can go back to the$B bbox$X view.
   (Note that with$B extract again$X or$R 'a'$X no extraction is done, but only
    the$B display.out$X file is read again.)
    Use the$R 'q'$X hotkey to$R quit$X the Xspace program.
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ Xspace -C3 -E space.def.t -P space.def.p ringosc"
Xspace -C3 -E space.def.t -P space.def.p ringosc
fi

cat <<!

$G*********************************************************************$B
STEP 16.$X
    To show the substrate noise on the$R sens$X pin, we can also
    extract substrate resistances.  We use the following commands:

    % space3d -Fv -Cl -B -E space.def.t -P space.def.p ringosc
    % simeye

    Note that the option$R -B$X is used for extraction of accurate
    substrate resistances.  It is using a 3D BEM method, therefor
    the$R space3d$X version of the extractor must be used in this case.
$G*********************************************************************$X
!
printf "Shall we extract?"; read ok
if test "$ok" != n
then
echo "+ space3d -Fv -Cl -B -E space.def.t -P space.def.p ringosc"
space3d -Fv -Cl -B -E space.def.t -P space.def.p ringosc
fi
printf "Shall we simulate again?"; read ok
if test "$ok" != n
then
echo "+ simeye"
simeye
fi

cat <<!

$G*********************************************************************$B
STEP 17.$X
    To show the substrate noise on the$R sens$X pin for the same
    layout with a ground shield.  We extract$R ringosc2$X and simulate
    again.  We use the following commands:

    % cgi ringosc2.gds
    % dali ringosc2
    % space3d -Fv -Cl -B -E space.def.t -P space.def.p ringosc2
    % simeye
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ cgi ringosc2.gds"
cgi ringosc2.gds
printf "View the layout?"; read ok
if test "$ok" != n
then
echo "+ dali ringosc2"
dali ringosc2
fi
printf "Shall we extract?"; read ok
if test "$ok" != n
then
echo "+ space3d -Fv -Cl -B -E space.def.t -P space.def.p ringosc2"
space3d -Fv -Cl -B -E space.def.t -P space.def.p ringosc2
fi
printf "Shall we simulate ringosc2?"; read ok
if test "$ok" != n
then
echo "+ simeye"
simeye
fi
fi

cat <<!

$G*********************************************************************$B
STEP 18.$X
    You can also use a 2D method to extract substrate resistances.
    Before you can do that, there must be added 2D substrate data
    to the technology file. This data can be generated with the$R
    subresgen$X program.  This program runs the$R space3d$X extractor
    a number of times to generate this 2D substrate res data. Type:

    % subresmkdir sub2d 10
    % cp space.def.s sub2d
    % cd sub2d
    % subresgen
    % cd ..
    % diff space.def.s sub2d/space.def.s
    % cp sub2d/space.def.s space.sub2d.s
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ subresmkdir sub2d 10"
subresmkdir sub2d 10
echo "+ cp space.def.s sub2d"
cp space.def.s sub2d
echo "+ cd sub2d"
cd sub2d
echo "+ subresgen"
subresgen
echo "+ cd .."
cd ..
echo "+ diff space.def.s sub2d/space.def.s"
diff space.def.s sub2d/space.def.s
echo "+ cp sub2d/space.def.s space.sub2d.s"
cp sub2d/space.def.s space.sub2d.s
fi

cat <<!

$G*********************************************************************$B
STEP 19.$X
    Now we go to extract simple substrate resistances using a 2D method.
    And after that we go to simulate again.  But first we must compile
    the new technology file.  We do the following commands:

    % tecc space.sub2d.s
    % space -Fv -Cl -b -E space.sub2d.t -P space.def.p ringosc
    % simeye
$G*********************************************************************$X
!
printf "Shall we compile?"; read ok
if test "$ok" != n
then
echo "+ tecc space.sub2d.s"
tecc space.sub2d.s
fi
printf "Shall we extract?"; read ok
if test "$ok" != n
then
echo "+ space -Fv -Cl -b -E space.sub2d.t -P space.def.p ringosc"
space -Fv -Cl -b -E space.sub2d.t -P space.def.p ringosc
fi
printf "Shall we simulate ringosc?"; read ok
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
echo "+ rm -f display.out exp_dat ringosc.spc space.def.t"
rm -f         display.out exp_dat ringosc.spc space.def.t
echo "+ rm -f sim.diag ringosc.ana ringosc2.ana ringosc2.spc"
rm -f sim.diag sim.diag2 ringosc.ana ringosc.out ringosc2.ana  ringosc2.spc
echo "+ rm -rf sub2d"
rm -rf sub2d
fi
