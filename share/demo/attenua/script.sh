#!/bin/sh
# directory: demo/attenua

R="[1;31m"
G="[1;32m"
B="[1;34m"
X="[0m"

unset CWD
clear
cat <<!
$G**********************************************************************
**$R Demonstration of attenuator circuit in bipolar DIMES-01 process.$G **
**$R See also,                                                       $G **
**$R   "Low-voltage Low-power Controlled Attenuator for Hearing Aids"$G **
**$R   A. van Staveren and A.H.M. van Roermund, Electronic Letters,  $G **
**$R   22nd. July 1993, Vol. 29, No. 15, pp. 1355-1356.              $G **
**********************************************************************$X
After extraction, the circuit can be simulated using PSPICE.

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
    We shall use the$R dimes01$X technology and a lambda of$R 0.1$X micron.
    We change the current working directory $R'.'$X into a project directory
    with the following command:

    % mkpr -p dimes01 -l 0.1 .
$G*********************************************************************$X
!
printf "Shall we do it? (type$R Enter$X)"; read ok
if test "$ok" == n
then
echo "Ok, see you a next time!"
exit
fi

echo "+ mkpr -p dimes01 -l 0.1 ."
mkpr -p dimes01 -l 0.1 .

cat <<!

$G*********************************************************************$B
STEP 2.$X
    The layout of the$R attenuator$X is available in a gds2 file.
    We use$R cgi$X (convert-gds-internal) to put the layout into the
    project database with the following command:

    % cgi attenua.gds
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ cgi attenua.gds"
cgi attenua.gds
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
You see$R 3$X hierarchical levels. The top cell$B attenua$X has$R 84$X sub-cells.
The$B pnp33$X sub-cell has$R 3$X sub-cells.
!
fi

cat <<!

$G*********************************************************************$B
2c.$X Now, we can inspect the layout with the layout editor/viewer$R dali$X.

    % dali

    Use the command $R'read_cell'$X in the $B"DB_menu"$X to read the
    different cells.  To see more levels, use hotkey $R'2'$X or $R'3'$X.
    To exit the program, use hotkey $R'q'$X and click on $R'yes'$X.
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ dali"
dali
fi

cat <<!

$G*********************************************************************$B
3.$X  Perform now a flat extraction of cell$B attenua$X.
    We use verbose mode (option $R-v$X) to see what the program is
    doing and option $R-F$X for flat extraction.  The command is:

    % space -vF attenua
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ space -vF attenua"
space -vF attenua
fi

cat <<!

$G*********************************************************************$B
3b.$X Shall we list the contents of the project again, using:

    % dblist -h
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ dblist -h"
dblist -h
fi

cat <<!

$G*********************************************************************$B
4.$X  Now, inspect the extracted$B attenua$X circuit. To see
    a SPICE$X description (or netlist), the$R xspice$X command with
    option $R-a$X is used to eXstract SPICE from the database:

    % xspice -a attenua                   (see:$R icdman$X xspice)
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ xspice -a attenua"
xspice -a attenua
fi

cat <<!

$G*********************************************************************$B
4b.$X Try out the options $R-c$X, $R-C$X, $R-r$X and $R-z$X for capacitance and
    resistance extraction.

    % space -vFc attenua
    % xspice -a attenua
    ...
$G*********************************************************************$X
!
printf "Shall we do $R-c$X?"; read ok
if test "$ok" != n
then
echo "+ space -vFc attenua"
space -vFc attenua
printf "Inspect result?"; read ok
echo "+ xspice -a attenua"
xspice -a attenua
fi
echo ""
printf "Shall we do $R-C$X?"; read ok
if test "$ok" != n
then
echo "+ space -vFC attenua"
space -vFC attenua
printf "Inspect result?"; read ok
echo "+ xspice -a attenua"
xspice -a attenua
fi
echo ""
printf "Shall we do $R-r$X?"; read ok
if test "$ok" != n
then
echo "+ space -vFr attenua"
space -vFr attenua
printf "Inspect result?"; read ok
echo "+ xspice -a attenua"
xspice -a attenua
fi
echo ""
printf "Shall we do $R-z$X?"; read ok
if test "$ok" != n
then
echo "+ space -vFz attenua"
space -vFz attenua
printf "Inspect result?"; read ok
echo "+ xspice -a attenua"
xspice -a attenua
fi

cat <<!

$G*********************************************************************$B
5.$X  If you have$R PSPICE$X, you can perform a circuit simulation (after
    customizing the script$R nspice$X) as follows:

    % nspice attenua attenua.cmd
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ nspice attenua attenua.cmd"
nspice attenua attenua.cmd
fi

cat <<!

$G*********************************************************************
*$B You can compare the extracted circuit against a reference circuit $G*
*$B with the circuit comparison program$R match$B.                        $G*
*********************************************************************$X
!
printf "Shall we go on?"; read ok

cat <<!

$G*********************************************************************$B
6.$X  First add device descriptions for the bipolar transistors to the
    database so that the reference circuit can be stored into the
    database using the program$R cspice$X.

    % putdevmod pnpWP.dev npnBW.dev
    % xcontrol -device pnpWP npnBW
    % cspice att_ref.spc
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ putdevmod pnpWP.dev npnBW.dev"
putdevmod pnpWP.dev npnBW.dev
echo "+ xcontrol -device pnpWP npnBW"
xcontrol -device pnpWP npnBW
echo "+ cspice att_ref.spc"
cspice att_ref.spc
fi

cat <<!

$G*********************************************************************$B
7.$X  Extract cell$B attenua$X, possibly after an error has been added to
    the layout.

    % space -vF attenua
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ space -vF attenua"
space -vF attenua
fi

cat <<!

$G*********************************************************************$B
8.$X  Use the program$R match$X to compare the extracted circuit against
    the reference circuit.

    % match -bindings att_ref attenua
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ match -bindings att_ref attenua"
match -bindings att_ref attenua
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
echo "+ rm -f exp_dat attenua.ana attenua.axa attenua.spc sim.diag"
rm -f exp_dat attenua.ana attenua.axa attenua.spc sim.diag
fi
