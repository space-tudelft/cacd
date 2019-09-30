.H 1 "The TERM_MENU"
This menu contains the commands for the manipulation of the terminals
of the edited cell.
.AL
.LI
RETURN: Return to the main menu.
.P
After selection of this command the term_menu will be left and
the main menu will be shown again.
.so ../dali/um_grid.mm
.so ../dali/um_wind.mm
.LI
ADD_TERM: Add a terminal, using a rubber-box cursor.
.P
This command operates in the same way as \fIadd_box\fP does,
but now dali also asks for a terminal name,
which has to be entered via the keyboard.
Only names that have not yet been used for other terminals in the edited
cell are allowed.
This command operates only on the activated \fIinterconnection\fP layers.
For each of these layers dali prompts for a name and adds a terminal.
The \fIadd_term\fP command is self-repeating.
.LI
DEL_TERM: Delete a terminal.
.P
The terminal that has to be deleted must be identified explicitly
by pointing at it in the PICT viewport.
This terminal selection procedure is described in more detail in section 1.6.
After a unique terminal has been identified it is deleted.
The name of the deleted terminal is reported in the TEXT viewport.
The \fIdel_term\fP command is self-repeating.
.LI
DEL_AREA: Delete all terminals within a specified area.
.P
After the selection of this command a rectangular area has to be specified
with the help of a rubber-box cursor.
All terminals (arrays of terminals) of editable layers that lie completely
within this area are deleted.
That is, a terminal that is partly situated outside
the specified area is left in its present state.
The \fIdel_area\fP command does not take
the activated layers into account.
.P
All deleted terminals are saved in the terminal buffer.
Thus, one can \fIundo\fP the deletion by
placing this buffer at the old position or one can \fImove\fP the terminals to
another position by means of
the command sequence \fIdel_area\fP and \fIput_buf\fP.
.LI
YANK_AREA: Yank all terminals within a specified area.
.P
This command operates in the same way as \fIdel_area\fP does,
but in place of deleting,
the terminals are copied to the terminal buffer.
.LI
PUT_BUF: Add the contents of the terminal buffer to the edited cell.
.P
One can position the terminal buffer contents by specifying a position
for the lower left corner of its bounding box,
just as with the \fIput\fP command in the box_menu.
For the operation of this command see the \fIput\fP command.
Note that the \fIput_buf\fP command only works if the terminals
are visible and not takes in account the visibility of the layers.
.LI
MOV_TERM: Move a terminal to a new position.
.P
The terminal that has to be moved must be selected explicitly
in the PICT viewport.
If a terminal is selected, then a new position may be entered
by pointing at a certain position in the PICT viewport.
This specifies the new position for the lower left corner of the terminal.
If the terminal has been \fIarrayed\fP, the lower left corner of
the (0,0) occurrence is taken as the reference point.
In this case the complete array is moved.
The \fImov_term\fP command is self-repeating.
.LI
ARRAY_TERM: Change the array parameters of a selected terminal.
.P
The array parameters are the number of repetitions in both directions:
\fInx\fP and \fIny\fP,
and the repetition spacing in both directions: \fIdx\fP and \fIdy\fP.
The default value for nx and ny is 0.
The repetition spacing in the x-direction, dx, is defined as the distance
between the left side of the (0,k) occurrence and the left
side of the (1,k) occurrence.
In the same way, dy is defined as the distance between the bottom side
of the (k,0) occurrence and the bottom side of the (k,1) occurrence.
Default values for dx and dy are the width and height of the individual
occurrence: abutment.
.P
A sub-menu is presented for the selection of one of the array parameters.
A cancel possibility is included as the bottom menu item.
If nx or ny is selected, a new value can be entered via the keyboard.
A negative nx or ny will cancel the command.
If dx or dy is selected,
a new spacing can be specified by pointing at a position in the PICT viewport.
In case of dx, the x-coordinate of the specified point is taken
as the new left side of the (1,k) occurrence(s).
In case of dy, the y-coordinate is taken
as the new bottom side of the (k,1) occurrence(s).
.LI
LIFT_TMS: Create terminals on top of the terminals of an instance.
.P
First the user has to select an instance by pointing at it
in the PICT viewport.
See also section 1.7.2.
Then he has to specify at which side(s) of the
bounding box of this instance he would like to create terminals.
This can be done
by toggling the corresponding side-specifiers (\fItop, bottom, right, left\fP)
in a sub-menu until the \fIready\fP menu item is selected.
After that, another sub-menu is presented from which the user has to choose
how he would like to assign the names to the new terminals:
use the names of the underlaying terminals (\fIold name\fP),
give an extension to the old names (\fIextension\fP)
or give totally new names (\fInew name\fP).
A cancel possibility is included as the bottom menu item.
.P
When all the necessary information has been supplied, dali starts with
the creation of the terminals.
Only the terminals of the selected instance that lie exactly on the
specified side(s) of the bounding box are lifted.
This may be interrupted if the name that has been generated for a new terminal
is already associated with an existing terminal
(which the \fIlift_tms\fP command itself might just have created).
On this occasion dali prompts for a new name, which has to be entered
via the keyboard.
After the complete process has been finished the generated terminals become
visible (note: if the terminals are visible).
Note that the terminals of non-editable layers are skipped by
the \fIlift_tms\fP command.
.LE
