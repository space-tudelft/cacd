#!/bin/sh
# directory: demo/invert

R="[1;31m"
G="[1;32m"
B="[1;34m"
X="[0m"

unset CWD
clear
cat <<!
$G***********************************************************************
**$R Demonstration of extraction of an inverter with transistor bulk  $G **
**$R connections and drain/source regions. The latter are extracted as$G **
**$R either non-linear junction capacitances, or drain/source area and$G **
**$R perimeter information attached to the MOS transistors.           $G **
**$B See the "Space Tutorial" sections "3.8", "4" and "5".            $G **
***********************************************************************$X

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
    We shall use the$R scmos_n$X technology and a lambda of$R 0.2$X micron.
    We change the current working directory $R'.'$X into a project directory
    with the following command:

    % mkpr -p scmos_n -l 0.2 .
$G*********************************************************************$X
!
printf "Shall we do it? (type$R Enter$X)"; read ok
if test "$ok" == n
then
echo "Ok, see you a next time!"
exit
fi

echo "+ mkpr -p scmos_n -l 0.2 ."
mkpr -p scmos_n -l 0.2 .

cat <<!

$G*********************************************************************$B
STEP 2.$X
    The layout of the$R inverter$X is available in a gds2 file.
    We use$R cgi$X (convert-gds-internal) to put the layout into the
    project database with the following command:

    % cgi invert.gds
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ cgi invert.gds"
cgi invert.gds
fi
fi

cat <<!

$G*********************************************************************$B
2b.$X Now, we can inspect the layout with the layout editor/viewer$R dali$X.
    Because the inverter has cell name $R'invert'$X, we use command:

    % dali invert

    To exit dali, click on $R'-quit-'$X and $R'yes'$X.
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ dali invert"
dali invert
fi

cat <<!

$G*********************************************************************$B
3.$X  Perform an extraction of the$B invert$X cell. Use option $R-C$X to
    include coupling capacitances. The drain/source regions are extracted
    as non-linear junction capacitances that have parameters $R'area'$X and
    $R'perimeter'$X (using option $R-S$X). The following command is used:

    % space -C -Sjun_caps=area-perimeter invert
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ space -C -Sjun_caps=area-perimeter invert"
space -C -Sjun_caps=area-perimeter invert
cat <<!

$G*********************************************************************$X
  If you see a warning message of the$R makegln$X program, then this is
  no problem. In that case, no$R compress$X tools are installed.
  Note that$R space$X gives a warning about the join of two different
  active areas (p+/n+). In this layout, an active area contact with
  first metal and a substrate contact lay close to each other.
  Accidental active area mask "caa" is used for the substrate contact.
$G*********************************************************************$X
!
printf "Shall we go on?"; read ok
if test "$ok" == n
then
echo "Ok, then we stop!"
exit
fi
fi

cat <<!

$G*********************************************************************$B
4.$X  Look in the file $R"xspicerc"$X to see how the junction capacitances are
    printed in the netlist output as diodes with parameters $R'area'$X and $R'pj'$X:

    % cat xspicerc
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ cat xspicerc"
cat xspicerc
fi

cat <<!

$G*********************************************************************$B
4b.$X And next get the$R SPICE$X circuit description. We use the$R xspice$X
    command with options $R-a$X and $R-u$X to eXstract SPICE from the database:

    % xspice -au invert                    (see:$R icdman$X xspice)
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ xspice -au invert"
xspice -au invert
fi

cat <<!

$G*********************************************************************$B
5.$X  An alternative is to extract the drain/source regions as $R'area'$X and
    $R'perimeter'$X information that is attached to the MOS transistors.
    This is achieved by modifying the transistor definitions in the element
    definition file.  Therefore, first copy the element definition file
    from the process directory to the local file $R"elem.s"$X:

    % cp \$ICDPATH/share/lib/process/scmos_n/space.def.s elem.s
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
    if test "$ICDPATH" = ""
    then
	ICDPATH=`which mkpr | sed -e 's:/cacd/.*:/cacd:'`
    fi
    if test ! -f $ICDPATH/share/lib/process/scmos_n/space.def.s
    then
	echo "Sorry, cannot copy the element definition file!"
	echo "Please, set your ICDPATH first!"
	exit
    fi
echo "+ cp \$ICDPATH/share/lib/process/scmos_n/space.def.s elem.s"
cp $ICDPATH/share/lib/process/scmos_n/space.def.s elem.s
fi

cat <<!

$G*********************************************************************$B
5b.$X To change the lines, edit the $R"elem.s"$X file:

    % gedit elem.s
*********************************************************************
$B(1)$X Change the lines (in conductors definitions)
       cond_pa : caa !cpg !csn : caa  : 70  : p    # p+ active area
       cond_na : caa !cpg  csn : caa  : 50  : n    # n+ active area
    into
       cond_pa : caa !cpg !csn : caa  : 0   : p    # p+ active area
       cond_na : caa !cpg  csn : caa  : 0   : n    # n+ active area
    to avoid complaints by tecc about the fact the resistance for drain/
    source regions should be zero (that resistance is already modeled in
    the transistor model).

$B(2)$X Change the lines (in fets definitions)
       nenh : cpg caa  csn : cpg caa : @sub  # nenh MOS
       penh : cpg caa !csn : cpg caa : cwn   # penh MOS
    into
       nenh : cpg caa  csn : cpg caa (!cpg caa csn)  : @sub # nenh MOS
       penh : cpg caa !csn : cpg caa (!cpg caa !csn) : cwn  # penh MOS
    to describe the drain/source regions.

$B(3)$X And remove the 'ndif' and 'pdif' junction capacitance lists.
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ gedit elem.s"
chmod +w elem.s
gedit elem.s
fi

cat <<!

$G*********************************************************************$B
5c.$X Next, run$R tecc$X to compile the new element definition file:

    % tecc elem.s
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
5d.$X Now, run$R space$X again with the new element definition file:

    % space -C -Sjun_caps=area-perimeter -E elem.t invert
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ space -C -Sjun_caps=area-perimeter -E elem.t invert"
space -C -Sjun_caps=area-perimeter -E elem.t invert
fi

cat <<!

$G*********************************************************************$B
6.$X  And watch the SPICE output again:

    % xspice -au invert
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ xspice -au invert"
xspice -au invert
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
echo "+ rm -f exp_dat elem.s elem.t"
rm -f exp_dat elem.s elem.t
fi
