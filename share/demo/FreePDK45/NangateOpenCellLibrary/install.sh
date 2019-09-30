#!/bin/sh
# directory: demo/FreePDK45/NangateOpenCellLibrary

if test -f .dmrc
then
echo "NangateOpenCellLibrary already installed"
echo "use \"rmpr -f .\" to remove the project data"
exit
fi

# turn cwd '.' into a project
echo "+ mkpr -p FreePDK45 ."
mkpr -p FreePDK45 .

# install the layouts
echo "+ cgi NangateOpenCellLibrary.gds"
cgi NangateOpenCellLibrary.gds

# foreach cell -> extract the netlist
# and set the library status (not for the FILLCELL's)
# metal1 must be a free-mask, else the basic cell
# terminals can not connect to the above level
for c in `dblist -l`
do
echo "+ space -F $c"
space -Scompression=off -F $c
echo "+ sls_exp $c"
sls_exp $c
case $c in
FILLCELL*) echo "skip xcontrol for $c";;
*)
echo "+ xcontrol -library -freemasks:metal1 $c"
xcontrol -library -freemasks:metal1 $c
esac
done

# remove extract data from the layouts
echo "+ dbclean -l -a"
dbclean -l -a

