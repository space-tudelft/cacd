#!/bin/sh
# directory: demo/FreePDK45/NangateOpenCellLibrary

R="[1;31m"
G="[1;32m"
B="[1;34m"
X="[0m"

unset CWD
clear
cat <<!
$G*********************************************************************
****** $R     Welcome to the Nangate Open Cell Library!        $G  ******
** $R The Nangate 45nm Open Cell Library is an open-source,         $G **
** $R standard-cell library provided for the purposes of testing    $G **
** $R and exploring EDA flows.  The library was generated using     $G **
** $R Nangate's Library Creator(TM) and the 45nm FreePDK Base Kit   $G **
** $R from North Carolina State University (NCSU).                  $G **
*********************************************************************$X

Note: Type$R Enter$X on the keyboard, each time, to do a step.
      If you want to skip a step, type first a $R'n'$X.
      If you want to stop this visiting tour, type$R Ctrl-C$X.
*********************************************************************
!
printf "Shall we start the visiting tour? (type$R Enter$X)"; read ok
if test "$ok" = n
then
echo "ok, see you another time!"
exit
fi
if ! test -f .dmrc
then
echo "sorry, the NangateOpenCellLibrary is not yet installed"
echo "type \"./install.sh\" or \"sh install.sh\" to install the library"
exit
fi

cat <<!

$G*********************************************************************$B
STEP 1.$X
    We can inspect the layout of a basic cell with the layout editor$R dali$X.
    Start the layout editor with the following command:

    % dali                         (see:$R icdman$X dali)

    Click on the "DB_menu" and click on "read_cell" and click on a cell
    name.  Click a second time, if you are sure to read this basic cell.

    To exit the program, click on $R'-return-'$X and on $R'-quit-'$X
    and click on $R'yes'$X.
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
STEP 2.$X
    We can inspect the extraction result (the extracted circuit netlist)
    of a basic cell.  To see a$R SPICE$X netlist of the INV_X1 cell, we
    use the$R xspice$X command to eXstract SPICE from the database:

    % xspice -aou INV_X1           (see:$R icdman$X xspice)
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ xspice -aou INV_X1"
xspice -aou INV_X1
fi

cat <<!

$G*********************************************************************$B
STEP 3.$X
    We can also generate a$R SLS$X netlist with the following command:

    % xsls INV_X1                  (see:$R icdman$X xsls)
$G*********************************************************************$X
!
printf "Shall we do it?"; read ok
if test "$ok" != n
then
echo "+ xsls INV_X1"
xsls INV_X1
fi

cat <<!

$G*********************************************************************$R
    Thanks for visiting this short tour, see you another time!
$G*********************************************************************$X
!
