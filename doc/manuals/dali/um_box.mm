.H 1 "The BOX_MENU"
This menu contains the commands
for the manipulation of the primitive mask geometries.
.AL
.LI
RETURN: Return to the main menu.
.P
After selection of this command the box_menu will be left and
the main menu will be shown again.
.so ../dali/um_grid.mm
.so ../dali/um_wind.mm
.LI
X,Y,MSKTOP: Show the coordinates and layer stack of a certain point.
.P
The old X,Y,MASKS command.
After the selection of this command one may point at a certain position in
the PICT viewport.
A small cross subsequently appears at the closest grid point
and the coordinates corresponding to this grid point
are reported in the TEXT viewport.
This command is self-repeating; the coordinates of multiple grid points
can be retrieved without interruption.
Also the layer stack of the top level is displayed in the LAYER viewport after pointing
in the PICT viewport.
It shows the layers which are laying on that point.
Note that you can use the keyboard-keys to change the picture
while using the \fIx,y,masks\fP command,
for example zooming-in (i) or toggling dominant/transparent mode (D).
.LI
X,Y,MSKEXP: Show the coordinates and expanded layer stack of a certain point.
.P
See the above command description.
This command shows also the layers of the sub-cells (till the expanded level).
Note that you can use the number keys to choice another expansion level.
.LI
ADD_BOX: Add a box.
.P
After the selection of this command
one can add boxes by just fixing two points.
After the first point has been fixed,
a rubber-box cursor supports the positioning
of the second point in an easy way. 
The command operates only on the activated layers.
Before a second point is given any layer may be (de)activated.
Note that this is only true for visible and editable layers.
.P
The \fIadd_box\fP command is self-repeating;
several boxes can be added without interruption
until one selects another command or de-selects the command.
.LI
DEL_BOX: Delete a box.
.P
This command operates in the same way as \fIadd_box\fP does,
except that now the mask geometries for
the activated layer(s) are \fIdeleted\fP inside the specified area (box).
.P
After the execution of the command
the deleted primitive layout will still be present in the geometry buffer.
This implies that one can recover from non-desirable results
using the \fIput\fP command.
Also a \fImove\fP can be made by deleting mask geometries at one spot
and \fIput\fP them somewhere else.
.LI
ADD_POLY: Add a polygon.
.P
After the selection of this command a menu is presented from which
one can add polygons by subsequently entering the corner points
of the contour polygon.
If the 45 degree mode has been switched on (the default),
non-orthogonal mask geometries can also be added.
After the first point has been fixed
a rubber-line cursor assists the positioning of the other corner points.
Upon entering such a point
the closest orthogonal or 45 degree line segment is generated
by either adjusting the x- or y-coordinate of the point that has been entered.
Note that with the \fIuse_big\fP setting the opposite adjustment takes place.
With the \fIset_cross\fP command a big cross is displayed on the
position of the first point.
A polygon may consist of up to 127 line segments.
Incorrect line segments can be corrected by just \fIwalking back\fP
over the parts that have already been entered or
by the use of the \fIwalk_back\fP command.
A polygon may be self-intersecting.
Upon closing the polygon (last point = first point), the corresponding
mask geometries are added to the internal workspace and painted on the screen.
Closing of the polygon can be forced with the \fInext\fP command.
The polygon can be cancelled (if not yet closed) with the \fIcancel\fP command.
With the \fIreturn\fP command the ADD_POLY menu is exited.
The \fIadd_poly\fP command is also self-repeating
like the \fIadd_box\fP command.
.LI
DEL_POLY: Delete a polygon.
.P
This command operates in the same way as \fIadd_poly\fP does,
except that now the mask geometries for the activated layer(s)
are \fIdeleted\fP inside the specified contour polygon.
.P
After the execution of the command
the deleted primitive layout will still be present in the geometry buffer.
Like the \fIdel_box\fP command.
.LI
ADD_WIRE: Add a wire, using a rubber-line cursor.
.P
A menu is presented from which the wiring process can be controlled.
It includes commands to specify a wire width, to manipulate the display
window, and to start and finish wire-editing.
.P
If no wire width has been specified previously or from the ".dalirc" setup file,
dali first asks for a wire width.
See the command \fIset_width\fP below.
A wire is entered by specifying its center-line in the PICT viewport.
After the first point has been fixed,
a rubber-line cursor assists the positioning of the other points.
Upon entering such a point
the closest orthogonal or 45 degree line segment is fixed
by either adjusting the x- or y-coordinate of the point that has been entered.
45 degree segments can be entered only
if the \fI45 degree\fP mode has previously been switched on.
The center-line may consist of up to 127 line segments.
Incorrect line segments can be corrected by just \fIwalking back\fP
over the parts that have already been entered.
A wire of the specified width is generated for the selected layers
if the \fInext\fP or \fIready\fP command is selected,
or if the same wire-point is entered twice.
The wire is generated either with or without an extension of half
the wire width for the first
and last wire segment, as controlled by the \fIextension\fP command.
Default is no extension.
.P
The wire-menu contains the following commands:
.DL
.LI
\fIready\fP:
A wire is generated for the center-line that has been entered so far.
The corresponding mask geometries are added to the edited cell
and appear on the screen.
This finishes the \fIadd_wire\fP command and a return is made to the box_menu.
.LI
\fInext\fP:
A wire is generated for the center-line that has been entered so far.
The corresponding mask geometries are added to the edited cell
and appear on the screen.
As opposed to \fIready\fP this does not finish the \fIadd_wire\fP command.
A new center-line for the \fInext\fP wire, having the same width,
can be entered right away.
.LI
\fIcancel\fP:
The center-line that has been entered so far is erased
and one may start again to enter a new center-line.
No changes are made to the edited cell.
.LI
\fIgrid\fP:
Toggle grids.
See description of this command above.
.LI
\fIbbox\fP:
Set the window to the bounding box of the edited cell.
See description of the \fIbbox\fP command above.
This command can be intermixed with the specification of center-line segments,
thereby allowing
the user to view the complete layout while specifying the wire.
.LI
\fIprev\fP:
Set the window to the previous window.
See description of the \fIprev\fP command above.
This command can be intermixed with the specification of center-line segments.
.LI
\fIcenter\fP:
After this menu item has been selected, one can enter a new center
for the window, just as with the \fIcenter\fP command described above.
As this can be intermixed with the specification of center-line segments,
it allows the user to move over the layout while specifying the wire.
This increased flexibility permits longer wires to be added, while
viewing the layout at a reasonable scale.
.LI
\fIzoom\fP:
After this menu item has been selected, one can specify a smaller window,
just as with the \fIzoom\fP command described above.
As this can be intermixed with the specification of center-line segments,
it allows the user to focus on a small part of the layout when fixing
e.g. a start- or end-point of the wire.
.LI
\fIdezoom\fP:
After this menu item has been selected, one can specify a larger window,
just as with the \fIdezoom\fP command described above.
As this can be intermixed with the specification of center-line segments,
it allows the user to display a larger part of the layout, e.g.
after fixing a point at a more detailed level.
By subsequently zooming-in on another part of the layout long wires
can comfortably be entered with reasonable accuracy.
.LI
\fIset_width\fP:
As shown in a sub-menu,
a wire width can be specified either by selecting a predefined value,
by entering a width via the keyboard (\fIkeyboard\fP command)
or by interactively pointing at two points in the PICT viewport
(\fIinteract\fP command).
In the last case, the selected width is the maximum distance in either
the x-direction or the y-direction between the two
points that have been entered.
A wire width can not be specified while a center-line is being entered.
If the snap grid is on only even wire widths are permitted.
.LI
\fIextension\fP:
As can be controlled with this command,
a wire can be generated either with or without an extension
for the first and last wire segment.
If extension is switched off the generated wires will start and stop
at the first and last point of the center-line that has been entered
(with rounding to grid for odd wire widths).
If extension is switched on an overlap of half the wire width
will be generated around the first and last point of the center-line.
This is particularly convenient when editing wires on a course snap grid
to connect, for instance, contacts positioned on this snap grid.
Default is no extension, but extension can be switched on
via the ".dalirc" setup file (section 1.12).
.LI
\fI45 degree\fP:
With this command the 45 degree mode can be switched on or off.
This mode can only be switched,
before the first point of the center-line has been entered.
.LE
.LI
YANK: Fill the mask geometry buffer, using a rubber-box cursor.
.P
A rectangular area has to be specified with the help of a rubber-box cursor.
Primitive mask geometries of the edited cell that lie
within the specified area are copied into the geometry buffer.
The \fIyank\fP command operates only on the activated layers.
.LI
PUT: Add the contents of the geometry buffer to the edited cell.
.P
After the selection of this command one can position
the buffer contents by specifying a position for the lower left corner
of its bounding box in the PICT viewport.
While an impression of the buffer contents is being displayed
new positions can repeatedly be specified.
The \fIput\fP command can be finished by selecting the \fIreturn\fP menu item
in the sub-menu that is presented.
By selecting the \fIput_buf\fP menu item on the other hand,
the contents of the geometry buffer are added to the edited cell
at the specified position.
This is also performed if the same cursor position is entered twice.
Note that only the buffer contents for the visible layers is put back.
The cursor position related to the contents of the buffer can be changed
with the \fIset_cursor\fP menu item.
In the PICT viewport this position is marked with a cross.
At last,
the buffer contents can also be mirrored and rotated around
this cross position.
.LI
CHECK: Perform a single-layer design-rule check.
.P
A rectangular area has to be specified with the help of a rubber-box cursor,
which is subsequently checked for design rule violations.
All \fIvisible\fP layout geometries (i.e. primitive mask geometries, terminals,
expansion information and sub-terminals) within the specified area are checked:
What You See Is What You Check.
Only \fIsingle-layer\fP checks are performed (width and gap).
That is, it doesn't check for design-rule violations caused by a combination
of mask geometries from two or more different layers
(compare to \fIautocheck\fP (1ICD)).
The violations that are found are displayed on the screen.
No check is performed if dali has not been able to retrieve
the design-rules at the start of the session (section 1.11).
In this case, a message is generated.
.LE
