.H 1 "The DRC_MENU"
In this menu an interactive interface to the \fIdimcheck\fP (1ICD)
package is provided.
It contains the following commands to perform design rule checks
and show their results:
.AL
.LI
RETURN: Return to the main menu.
.P
After selection of this command the DRC_menu will be left and
the main menu will be shown again.
.so ../dali/um_grid.mm
.so ../dali/um_wind.mm
.so ../dali/um_coor.mm
.LI
IND_ERR: Show error-data of an error pointed at.
.P
Upon selection of this command one may point at an error in the
PICT viewport.
The errors can be displayed as white rectangles
by e.g. \fIerror\fP, \fIchk_file\fP or \fIdo_check\fP.
The details of the error pointed at are displayed
in the TEXT viewport.
Information displayed is e.g. the design rule that was not obeyed and the
coordinates where the error occurred.
This command is self-repeating; one may point at the next error without
interruption.
.LI
NXT_IN_W: Display the next error from the error list inside the
current window.
.P
Using the \fInxt_in_w\fP command, one may walk along the internal list of
design rule errors
that has been built with the \fIchk_file\fP or \fIdo_check\fP command.
Upon selection of this command the next error from the list
that lies \fIinside\fP the current window
is displayed as a white rectangle in the PICT viewport.
More detailed textual information about the error
is presented in the TEXT_window.
The list is traversed in a wrap-around fashion; when the end of the
list is reached, dali continues with the first list item.
.LI
NXT_ERR: Display the next error from the error list.
.P
Using the \fInxt_err\fP command, one may walk along the internal list of
design rule errors
that has been built with the \fIchk_file\fP or \fIdo_check\fP command.
Upon selection of this command the next error from the list
is displayed as a white rectangle in the PICT viewport
and more detailed textual information is presented in the TEXT_window.
If the next error does not fall within the current window,
dali automatically moves the window at the same scale (\fIpanning\fP,
compare \fIcenter\fP) such that the error will be located in the
center of the window.
The list is traversed in a wrap-around fashion; when the end of the
list is reached, dali continues with the first list item.
.LI
ERROR: Toggle the visibility of the current error list.
.P
By subsequently selecting this command the visibility of the errors
from the list of design rule errors,
which may be built with the \fIchk_file\fP or \fIdo_check\fP command,
is alternately turned on and off.
If the visibility is turned on, all errors from the list
are displayed in the PICT viewport as white rectangles.
If either all errors or a single error were being displayed,
selection of the \fIerror\fP command will turn off their visibility.
.LI
CHK_FILE: Load the errors from a previously generated error file.
.P
Upon the selection of this command dali tries to
read a file named "cell_name.ck"
from the project directory,
where cell_name stands for the name of the cell one is editing.
This file contains the results of a previously performed
dimcheck upon the cell,
before it was called for the edit session.
The errors are placed in the internal error list
and are displayed in the PICT viewport.
.LI
DO_CHECK: Perform a design rule check on the edited cell.
.P
A choice can be made out of the following sub-commands:
.DL
.LI
\fIcheck\fP:
This command starts the requested design rule check.
To do this,
the edited cell that is present in the workspace of dali
is saved in the database as
a temporary cell with the name "chk_mod".
This cell is subsequently expanded by the program \fIexp\fP (1ICD)
and checked by the program \fIdimcheck\fP (1ICD).
By default, expansion is performed \fIhierarchically\fP
(\fIexp\fP is run with the option "-h").
Via the ".dalirc" setup file (section 1.12) this can be switched
to linear / flat expansion.
The results of \fIdimcheck\fP
are placed in the file "chk_mod.ck" in the project directory.
This file is subsequently read by dali,
an internal error list is built
and the errors are displayed in the PICT viewport.
The temporary cell "chk_mod" is removed from the database,
after the check has been performed.
.LI
\fIcancel\fP:
This sub-command cancels the DO_CHECK command.
.LI
\fIlinear\fP:
Selects linear expansion mode in place of hierarchical.
.LI
\fIhierarch\fP:
Selects hierarchical expansion mode in place of linear.
.LI
\fIset_opt\fP:
This sub-command makes it possible to set command-line options
for either the program \fIdimcheck\fP or \fIexp\fP.
.LI
\fIshow_opt\fP:
With this sub-command it is possible to show the used command-line options
for either the program \fIdimcheck\fP or \fIexp\fP.
.LE
