#!/bin/sh
# directory: demo/switchbox

R="[1;31m"
G="[1;32m"
B="[1;34m"
X="[0m"

unset CWD
clear
cat <<!
$G*********************************************************************
** $R   Demonstration of the switchbox$B nan3$R example.               $G  **
** $R   Which demonstrates$B net naming$R using terminals.             $G  **
*********************************************************************$X

*********************************************************************
Note: Type$R Enter$X on the keyboard, each time, to do a step.
      If you want to skip a step, type first a $R'n'$X.
      If you want to stop the demo, type$R Ctrl-C$X.
*********************************************************************

!

if test -f .dmrc
then
  echo "The current working directory seems to be a project directory."
  echo "This script first tries to remove the old project data."
  printf "Shall we$B restart$X this demo and do it? (type$R Enter$X or$R n$X)"; read ok
  if test "$ok" != n
  then
    echo "+ rmpr -fs ."
    rmpr -fs .
    rm -f exp_dat .dblist_done
  fi
else
  printf "Shall we start this demo? (type$R Enter$X for yes or$R n$X for no)"; read ok
  if test "$ok" == n; then echo "Bye."; exit 1; fi
fi

if test "$ok" != n
then
cat <<!

$G*********************************************************************$B
STEP A.$X
    First, we need a new project directory to work with.
    We shall use the$R scmos_n$X technology and a lambda of$R 1$X micron.
    We change the current working directory $R'.'$X into a project directory
    with the following command (see$B icdman mkpr$X):

    % mkpr -p scmos_n -l 1 .
$G*********************************************************************$X
!
printf "Shall we do it? (type$R Enter$X)"; read ok
if test "$ok" == n; then echo "Bye."; exit 1; fi
echo "+ mkpr -p scmos_n -l 1 ."
mkpr -p scmos_n -l 1 .
fi

if test ! -d layout/switchbox4
then
cat <<!

$G*********************************************************************$B
STEP B.$X
    The layout of cell$B nan3$X is available in a$B GDS2$X file.
    We use$R cgi$X (convert-gds-internal) to put the layout into the
    project database with the following command:

    % cgi ../switchbox/switchbox4.gds
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" == n; then echo "Bye."; exit 1; fi
echo "+ cgi ../switchbox/switchbox4.gds"
cgi ../switchbox/switchbox4.gds
fi

if test ! -f .dblist_done
then
cat <<!

$G*********************************************************************$B
STEP C.$X
    You can list the contents of the project and show the hierarchical
    layout tree with the$R dblist$X command:

    % dblist -h
$G*********************************************************************$X
!
touch .dblist_done
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ dblist -h"
dblist -h
cat <<!
You see$R 3$X hierarchical levels. The top cell$B switchbox4$X has$R 4$X sub-cells.
This$B dec1of4$X cell has also a number of sub-cells (leaf cells).
!
fi
fi

cat <<!

$G*********************************************************************$B
1.$X  Do you want to show the layout of the leaf cell$B nan3$X with the
    layout editor/viewer$R dali$X?  To do so, type:

    % dali nan3

    To exit the program, click on $R'-quit-'$X and click on $R'yes'$X.
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ dali nan3"
dali nan3
fi

cat <<!

$G*********************************************************************$B
2.$X  Perform an extraction of the layout of cell$B nan3$X.
    We use the verbose mode (option $R-v$X) to see what the program is doing.
    We use the following command to extract the citcuit from the layout:

    % space -v nan3
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ space -v nan3"
space -v nan3

cat <<!

$G*********************************************************************$X
    You see that the extraction result contains$B 8 nodes$X and$B 6 mos
    transistors$X.
    There are 3 p-enhancement (penh) and 3 n-enhancement (nenh) tors.
$G*********************************************************************$B
2b.$X We shall list the contents of the project database, using:

    % dblist -dh
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ dblist -dh"
dblist -dh
fi

cat <<!

$G*********************************************************************$B
2c.$X We shall now inspect the extracted circuit. To see a $B
    SPICE$X description (or netlist), we use the$R xspice$X command with
    the following options to eXstract SPICE from the database:

    % xspice -aou nan3               (see:$R icdman$X xspice)
*********************************************************************
    Note: With option $R-a$X you get node names and terminal names.
    With option $R-u$X you do not get automatically addition
    of terminals for bulk connections (with names:$B pbulk$X and$B nbulk$X).
    With option $R-o$X you suppress the output of SPICE models.
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ xspice -aou nan3"
xspice -aou nan3
fi

cat <<!

$G*********************************************************************$B
2d.$X You can also eXstract the netlist as a$B SLS$X description, type:

    % xsls nan3
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ xsls nan3"
xsls nan3
fi

cat <<!

$G*********************************************************************$B
2e.$X If you wish, you can also list the internal database streams
    of the$B nan3$X circuit view, type:

    % dbcat -c nan3
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ dbcat -c nan3"
dbcat -c nan3
fi
fi

cat <<!

$G*********************************************************************$B
3.$X  We can also extract a circuit with$B resistances$X, type:

    % space -rv nan3
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ space -rv nan3"
space -rv nan3

cat <<!

$G*********************************************************************$X
    You see that the extraction result contains$B 14 resistances$X and$B
    19 nodes$X and$B 6 mos transistors$X.  You see, because of the
    resistances, that there are now more local nodes.
$G*********************************************************************$B
3b.$X To see the$B SLS$X description of the result, type:

    % xsls nan3
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ xsls nan3"
xsls nan3
fi

cat <<!

$G*********************************************************************$X
    You see that the$B local nodes$X do not have$R names$X, but node$R numbers$X.
$G*********************************************************************$B
3c.$X You can also inspect the internal$B net$X stream, type:

    % dbcat -cs net nan3
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ dbcat -cs net nan3"
dbcat -cs net nan3
fi
fi

cat <<!

$G*********************************************************************$B
4.$X  If you are not satisfied with local node numbers, when extracting
    resistances with the$R space$X layout to circuit extractor. Then you
    must know that$R space$X has the possibility to extract node names
    based on terminals in the same conductor group. To do so, you
    must use the parameter$B "term_is_netname"$X, type:

    % space -rv -Sterm_is_netname nan3
*********************************************************************
    Note: You can also specify option$R -S$X as follows:
    % space -rv -Sterm_is_netname=on nan3
    But you can left out$B '=on'$X, because it is the default.
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ space -rv -Sterm_is_netname nan3"
space -rv -Sterm_is_netname nan3
cat <<!

$G*********************************************************************$X
    Note that in the result, terminal$B t_in_0$X is chosen in stead of
    terminal$B b_in_0$X. This because of the x-position of the terminals.
    Note also, that$R not all local nets get net names$X, this because$B
    two nets$X are only between the nenh transistors.
$G*********************************************************************$B
4b.$X To see the resulting$B SLS$X description, type:

    % xsls nan3
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ xsls nan3"
xsls nan3
fi
cat <<!

$G*********************************************************************$B
4c.$X To see the internal$B net$X stream, type:

    % dbcat -cs net nan3
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ dbcat -cs net nan3"
dbcat -cs net nan3
fi
fi

cat <<!

$G*********************************************************************$X
 GENERAL REMARK: Actually, default$R space$X is using labels for the
 net naming. And because there are no labels in the layout, you must
 use parameter $B"term_is_netname"$X, to make them equally to labels
 to be used for the net naming.  However, note that always the first
 label/terminal of the cell found (x,y-position) is used.
 Note: If you do not want to use the labels, you can specify
       parameter $B"no_labels"$X.
$G*********************************************************************$R
    Congratulations, you have now finished this demo!
$G*********************************************************************$X
!
printf "Shall we clean the demo directory and exit?"; read ok
if test "$ok" != n
then
echo "+ rmpr -fs ."
rmpr -fs .
rm -f exp_dat .dblist_done
fi
