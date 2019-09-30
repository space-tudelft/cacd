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
** $R Demonstration of the switchbox example from the              $G  **
** $R      "Space Tutorial".                                       $G  **
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
    rm -f exp_dat .dblist_done .dali_done
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
    The layout of the$R switchbox4$X is available in a gds2 file.
    We use$R cgi$X (convert-gds-internal) to put the layout into the
    project database with the following command:

    % cgi switchbox4.gds
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" == n; then echo "Bye."; exit 1; fi
echo "+ cgi switchbox4.gds"
cgi switchbox4.gds
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

if test ! -f .dali_done
then
cat <<!

$G*********************************************************************$B
STEP D.$X
    Now, we can inspect the layout with the layout editor/viewer$R dali$X.

    % dali

    Use the command $R'read_cell'$X in the $B"DB_menu"$X to read the
    different cells.  To see more levels, use hotkey $R'2'$X or $R'3'$X.
    To exit the program, use hotkey $R'q'$X and click on $R'yes'$X.
$G*********************************************************************$X
!
touch .dali_done
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ dali"
dali
fi
fi

cat <<!

$G*********************************************************************$B
1.$X  Perform a hierarchical extraction of the layout of cell$B switchbox4$X.
    We use the verbose mode (option $R-v$X) to see what the program is doing.
    The following command shall be used:

    % space -v switchbox4
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ space -v switchbox4"
space -v switchbox4

cat <<!

$G*********************************************************************$B
1b.$X We shall list the contents of the project again, using:

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
1c.$X We shall now inspect the extracted circuits. To see a hierarchical$R
    SPICE$X description (or netlist), we use the$R xspice$X command with
    option $R-h$X to eXstract SPICE from the database:

    % xspice -ah switchbox4               (see:$R icdman$X xspice)
*********************************************************************
    Note: With option $R-a$X you get node names and terminal names.
    Note: Use option $R-u$X, if you do not want automatically addition
    of terminals for bulk connections (with names:$R pbulk$X and$R nbulk$X).

    Note that hierarchical extraction works fine here because
    the terminals are rectangles that abut.  When the terminals
    are just points (e.g. when they originate from gds text
    structures and not from gds boundary elements with a property),
    they must have an overlap at a higher cell level in order to
    connect them to other parts of the circuit.
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ xspice -ah switchbox4"
xspice -ah switchbox4
fi
fi

cat <<!

$G*********************************************************************$B
2.$X  Perform a flat extraction of the layout of cell$B switchbox4$X.
    Use option$R -F$X to enable the flat mode and type:

    % space -vF switchbox4
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ space -vF switchbox4"
space -vF switchbox4

cat <<!

$G*********************************************************************$B
2b.$X List the contents of the project database again.
    You can use option$R -d$X to list the separate device calls.

    % dblist -hd
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ dblist -hd"
dblist -hd
fi

cat <<!

$G*********************************************************************$B
2c.$X To retrieve the$B switchbox4$X circuit SPICE netlist, type:

    % xspice -aou switchbox4
*********************************************************************
    Note that option $R-o$X is used to omit model definitions for devices
    and that option $R-u$X is used to omit the bulk terminals.
    Option $R-h$X is not more needed, because there are no sub-cells.
    However, there are 144 devices (mos transistors).
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ xspice -aou switchbox4"
xspice -aou switchbox4
fi
fi

cat <<!

$G*********************************************************************$B
3.$X  Perform flat extractions of the layout of cell$B switchbox4$X with
    a capacitance or a resistance extraction option.
    Try out:
    - option $R-c$X: capacitance extraction to GND
    - option $R-C$X: couple capacitance extraction
    - option $R-r$X: resistance extraction
    - option $R-z$X: resistance extraction with mesh refinement

    % space -vFc switchbox4
    % xspice -aou switchbox4
    ...
$G*********************************************************************$X
!
printf "Shall we do these try outs?"; read ok
if test "$ok" != n
then
printf "Shall we do $R-c$X?"; read ok
if test "$ok" != n
then
echo "+ space -vFc switchbox4"
space -vFc switchbox4
printf "Ok?"; read ok
echo "+ xspice -aou switchbox4"
xspice -aou switchbox4
fi
echo ""
printf "Shall we do $R-C$X?"; read ok
if test "$ok" != n
then
echo "+ space -vFC switchbox4"
space -vFC switchbox4
printf "Ok?"; read ok
echo "+ xspice -aou switchbox4"
xspice -aou switchbox4
fi
echo ""
printf "Shall we do $R-r$X?"; read ok
if test "$ok" != n
then
echo "+ space -vFr switchbox4"
space -vFr switchbox4
printf "Ok?"; read ok
echo "+ xspice -aou switchbox4"
xspice -aou switchbox4
fi
echo ""
printf "Shall we do $R-z$X?"; read ok
if test "$ok" != n
then
echo "+ space -vFz switchbox4"
space -vFz switchbox4
printf "Ok?"; read ok
echo "+ xspice -aou switchbox4"
xspice -aou switchbox4
fi
fi

cat <<!

$G*********************************************************************$B
4.$X  Note that the layout of cell$B dec1of4$X contains a label$R test$X.
    Labels of the top cell are always used as net names, unless
    space parameter$R no_labels$X is specified.

    % space -v dec1of4
    % xspice -a dec1of4
*********************************************************************
    Note: Terminal names have precedence in SPICE listings. Thus, when
    a terminal is in the same net as the label, the terminal is shown.
    This is not the case in SLS listings (net statements are used).
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ space -v dec1of4"
space -v dec1of4
echo "+ xspice -a dec1of4"
xspice -a dec1of4
fi

cat <<!

$G*********************************************************************$B
5.$X  When performing a$R flat$X extraction for$B switchbox4$X, the label$R
    'test'$X may be inherited from each sub-cell$B dec1of4$X when setting
    the parameter$R hier_labels$X.   Note that the inherited label names
    use the sub-cell name, the original label name and the coordinates
    of the labels (because there are no cell instance names).
    Note that you can force this notation with parameter$R cell_pos_name$X.

    % space -vF -Shier_labels switchbox4
    % xspice -aou switchbox4
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ space -vF -Shier_labels switchbox4"
space -vF -Shier_labels switchbox4
echo "+ xspice -aou switchbox4"
xspice -aou switchbox4
fi

cat <<!

$G*********************************************************************$B
6.$X  In place of$R hier_labels$X you can also use$R hier_terminals$X.
    When both are used, the labels of terminals has precedence.
    Because the sub-cells have no instance names, the notation of
    space parameter$R cell_pos_name$X is used.

    % space -F -Shier_terminals switchbox4
    % xspice -aou switchbox4
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ space -F -Shier_terminals switchbox4"
space -F -Shier_terminals switchbox4
echo "+ xspice -aou switchbox4"
xspice -aou switchbox4
fi

cat <<!

$G*********************************************************************$B
7.$X  Note that you can also make a dump of the$R net$X file of the database.
    For more information, see the manual page with:$B icdman dbcat$X.

    % dbcat -cs net switchbox4 | more
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ dbcat -cs net switchbox4 | more"
dbcat -cs net switchbox4 | more
fi

cat <<!

$G*********************************************************************$B
8.$X  When you want to use space parameter$R leaf_terminals$X, you need to
    have leaf cells.  By a flat extraction that is normally not the
    case.  However, by a hierarchical extraction the sub-cells are
    also the leaf cells.  We shall see what happens with$B dec1of4$X.

    % space -Sleaf_terminals dec1of4
    % xspice -a dec1of4
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ space -Sleaf_terminals dec1of4"
space -Sleaf_terminals dec1of4
echo "+ xspice -a dec1of4"
xspice -a dec1of4
fi

cat <<!

$G*********************************************************************$B
9.$X  You can give the leaf cells instance names.  After that
    you can see the leaf_terminal names in another format.
    To add instance names, start$R dali$X and click on the$B inst_menu$X.
    Click on menu item$B name_inst$X and click in the bounding box of
    the sub-cell$B dubinv$X and type an instance name (for example$R di$X)
    and finish your input with the$B Enter$X key.
    Repeat this for the four$B nan3$X sub-cells (for example$R n0$X -$R n3$X)
    and for the$B nan4rout$X sub-cell (for example$R n4$X).
    When you are ready, write the modified cell back to the database.
    Click on$B -return-$X,$B DB_menu$X,$B write_cell$X and$B dec1of4$X.
    To leave the program, click on$B -return-$X,$B -quit-$X and$R yes$X.

    % dali dec1of4
    % space -Sleaf_terminals dec1of4
    % xspice -a dec1of4
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ dali dec1of4"
dali dec1of4
echo "+ space -Sleaf_terminals dec1of4"
space -Sleaf_terminals dec1of4
echo "+ xspice -a dec1of4"
xspice -a dec1of4
fi

cat <<!

$G*********************************************************************$B
10.$X To get$R leaf_terminals$X for a flat extraction.  You must specify
    which cells are leaf cells (often done in standard cell approach).
    Use the$R xcontrol$X program to declare that a number of sub-cells
    must be taken as device instances (and do a flat extraction).

    % xcontrol -device dubinv nan3 nan4rout
    % space -F -Sleaf_terminals dec1of4
    % xspice -a dec1of4
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ xcontrol -device dubinv nan3 nan4rout"
xcontrol -device dubinv nan3 nan4rout
echo "+ space -F -Sleaf_terminals dec1of4"
space -F -Sleaf_terminals dec1of4
echo "+ xspice -a dec1of4"
xspice -a dec1of4
fi

cat <<!

$G*********************************************************************$B
11.$X You can also use the$R xcontrol$X program to get a list of the status
    of each cell (for other options, see$B icdman xcontrol$X).  Type:

    % xcontrol -list
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ xcontrol -list"
xcontrol -list
fi

cat <<!

$G*********************************************************************$B
    The following demonstrates the use of
    the circuit comparison program$R match$X.
$G*********************************************************************$X
!
printf "Shall we go on?"; read ok
if test "$ok" != n
then
cat <<!

$G*********************************************************************$B
12.$X Add a reference circuit description for the$B switchbox4$X circuit
    to the database using the program$R cspice$X.

    % cspice swbox_ref.spc
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ cspice swbox_ref.spc"
cspice swbox_ref.spc
fi

cat <<!

$G*********************************************************************$B
13.$X Use$R cgi$X to put a flat version of the$B switchbox4$X layout into the
    database and extract the circuit.

    % cgi switchbox4_f.gds
    % space switchbox4_f
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ cgi switchbox4_f.gds"
cgi switchbox4_f.gds
echo "+ space switchbox4_f"
space switchbox4_f
fi

cat <<!

$G*********************************************************************$B
14.$X Use the program$R match$X to compare the extracted circuit against the
    reference circuit.

    % match swbox_ref switchbox4_f

    The result shows that the circuits are identical.
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ match swbox_ref switchbox4_f"
match swbox_ref switchbox4_f
fi

cat <<!

$G*********************************************************************$B
15.$X Make an error in the layout of$B switchbox4_f$X using$R dali$X (e.g. remove
    some metal) and run the programs$R space$X and$R match$X again.

    % dali switchbox4_f
*********************************************************************
    Click $B"box_menu"$X, click on the $R"cmf"$X mask, click $R'del_box'$X and
    click in the layout nearby a $R"cmf"$X part that you want to remove.
    (Note: To zoom-in, you can use hotkey $R'i'$X.)
    Move the pointer and click again. To write the cell, click $R'-return-'$X
    and click on $B"DB_menu"$X and $R'write_cell'$X and $R"switchbox4_f"$X.
    To quit the program use hotkey $R'q'$X and click $R'yes'$X.
*********************************************************************
    % space switchbox4_f
    % match swbox_ref switchbox4_f

    The result shows that the circuits are not identical.
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ dali switchbox4_f"
dali switchbox4_f
echo "+ space switchbox4_f"
space switchbox4_f
echo "+ match swbox_ref switchbox4_f"
match swbox_ref switchbox4_f
fi

cat <<!

$G*********************************************************************$B
16.$X Use the option $R-fullbindings$X to see which network parts are matched.

    % match -fullbindings swbox_ref switchbox4_f
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ match -fullbindings swbox_ref switchbox4_f"
match -fullbindings swbox_ref switchbox4_f
fi

cat <<!

$G*********************************************************************$B
The following demonstrates the use of the back-annotation program$R
highlay$B in combination with the circuit comparison program$R match$B.
Assume that the reference circuit "swbox_ref" has been added to the
database and assume that that an error is present in the layout of
"switchbox4_f" (see the steps above).
$G*********************************************************************$X
!
printf "Shall we go on?"; read ok

cat <<!

$G*********************************************************************$B
17.$X Run the program space with parameter $R-Sbackannotation=2$X and
    run the program match with the options $R-edif$X and $R-fullbindings$X.
    The last two options cause match to write a binding table into
    the database that can be used as input for highlay.

    % space -Sbackannotation=2 switchbox4_f
    % match -edif -fullbindings swbox_ref switchbox4_f
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ space -Sbackannotation=2 switchbox4_f"
space -Sbackannotation=2 switchbox4_f
echo "+ match -edif -fullbindings swbox_ref switchbox4_f"
match -edif -fullbindings swbox_ref switchbox4_f
fi

cat <<!

$G*********************************************************************$B
18.$X The nets that have not been matched (the deficient nets) are selected
    for high-lighting by running the program$R highlay$X as follows:

    % highlay -d -n -v switchbox4_f
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ highlay -d -n -v switchbox4_f"
highlay -d -n -v switchbox4_f
fi

cat <<!

$G*********************************************************************$B
19.$X Next, use$R dali$X to read in the cell$B HIGH_OUT$X that has been
    generated by$R highlay$X and inspect the unmatched conductors.

    % dali HIGH_OUT

    In the $B"settings"$X menu, set $R'hash mode'$X to $R'hashed'$X (you can also
    use the mode toggle key $R'd'$X).  To see the sub-cells, higher the
    expand level with key $R'2'$X.  To quit use $R'q'$X and click $R'yes'$X.
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ dali HIGH_OUT"
dali HIGH_OUT
fi

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
rm -f exp_dat .dblist_done .dali_done
fi
