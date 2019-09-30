.H 1 "The INST_MENU"
This menu contains commands for manipulation of instances.
.AL
.LI
RETURN: Return to the main menu.
.P
After selection of this command the inst_menu will be left and
the main menu will be shown again.
.so ../dali/um_grid.mm
.so ../dali/um_wind.mm
.LI
ADD_INST: Add instance of local / imported cell to the edited cell.
.P
After selecting this command one must first make a choice between
the \fIlocal\fP and the \fIimported\fP cells.
.P
After the choice, if present,
dali displays this list of cells in the MENU viewport.
If this is a long list only part of it is presented at a time, and
\fI-prev-\fP and \fI-next-\fP menu items are included to allow the user
to move back and forth through the list.
One can either select a cell_name or cancel the command
by selecting the last menu item.
A selected cell can be placed in the edited cell by fixing its lower
left corner:
by pointing at a certain position in the PICT viewport the placement of the
lower left corner of the selected component cell is specified and
its instance will appear there.
The expansion information of the new instance will be displayed
right away, up to the global expansion level (set by \fIall_exp\fP).
.P
With the \fIkeyboard\fP menu item the cells can interactively be selected.
It is also possible to search for a cell_name
(see the \fIread_cell\fP command in the DB_menu).
.LI
DEL_INST: Delete an instance.
.P
An instance has to be identified
by pointing at a position within its bounding box (repetitions included).
See also section 1.7.2.
The instance that is identified is deleted.
The name of the component cell of the deleted instance is reported
in the TEXT viewport.
Explicit specification is required in this case to prevent annoying mistakes.
The \fIdel_inst\fP command is self-repeating; multiple instances can be
deleted without having to reselect the command.
.LI
MOV_INST: Move an instance.
.P
By pointing at a certain position in the PICT viewport
the new position for the lower left corner of the
selected instance is specified.
If the instance has been \fIarrayed\fP, the lower left corner of
the (0,0) occurrence is taken as the reference point.
In this case the complete array is moved.
The \fImov_inst\fP command is also self-repeating.
.LI
MIR_INST: Mirror an instance.
.P
If an instance is selected, then
dali presents a sub-menu by means of which the user has to specify
whether the mirroring should be performed around the
x-axis (\fIx\fP) or around the y-axis (\fIy\fP).
A cancel possibility is included as the bottom menu item.
If an axis is selected, the instance
is mirrored around this axis in such a way that its lower left corner
remains at its original position.
If the instance has been \fIarrayed\fP, all individual occurrences
will be mirrored.
.LI
ROT_INST: Rotate an instance.
.P
The selected instance
is instantly rotated 90 degrees counter-clockwise.
The lower left corner of the instance remains at its original position.
If the instance has been \fIarrayed\fP, all individual occurrences
will be rotated.
.LI
ARRAY_INST: Change the array parameters of an instance.
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
After the selecting an instance,
a sub-menu is presented for the selection of one of the array parameters.
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
NAME_INST: Name / rename an instance.
.P
The instance name of the selected instance of the component cell can be
set or renamed.
The current instance name and cell_name are reported
in the TEXT viewport and there is asked for a new name.
The new name must be entered via the keyboard.
This command is also self-repeating.
.LI
SHOW_INST: Show and set the name of an instance.
.P
The instance name and cell_name of the selected instance are reported
in the TEXT viewport.
The selected instance can also be used by some other commands
(\fIrot_inst\fP and \fImir_inst\fP for example).
This command is also self-repeating.
.LI
SHOW_TREE: Show complete cell tree at given position.
.P
The complete hierarchical tree of cell cq. instance names,
existing at the clicked pointer position,
is outputted to 'stdout'.
The coordinate position must lay in a cell instance bounding box.
This command is also self-repeating.
.LI
UPD_BBOX: Update the bounding box of an instance.
.P
If one has edited a cell that is instantiated in the edited cell,
dali may no longer display a correct bounding box for the instance(s) of
the modified component cell.
By selecting such an instance and performing the
\fIupd_bbox\fP command, the bounding box can be corrected.
.P
In order to maintain data consistency with respect to
the bounding boxes one has to apply this command
to all the 'father' cells, if a 'son' cell has been edited
in such a way that its bounding box has changed.
.LE
