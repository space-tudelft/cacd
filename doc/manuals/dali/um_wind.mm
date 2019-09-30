.LI
BBOX: Set the window to the bounding box of the edited cell.
.P
The new window is the smallest window in which the bounding box
of the design, together with a small margin, fits.
.LI
PREV: Set the window to the previous window.
.P
The new window is the window that was displayed
just before the current one.
.LI
CENTER: Set the window with a new center but at the same scale.
.P
By fixing one point in the PICT viewport,
the new center of the window is specified.
The picture is redrawn at the same scale but with the specified
point as the new center.
Thus, using the \fIcenter\fP command
one can \fIpan\fP with variable step and direction.
This command is self-repeating.
.LI
ZOOM: Zoom in with the help of a rubber-box cursor.
.P
By fixing two points with the help of a rubber-box cursor
a new, smaller, window can be specified.
The new window is the smallest window of a given ratio,
in which the specified part of the design fits.
This command is self-repeating.
.LI
DEZOOM: Fit the current window in the cursor area.
.P
An area has to be specified with the help of a rubber-box cursor.
The window is then set in such a way, that the current window fits in
the specified area.
As a consequence, a larger part of the edited cell becomes visible.
This command is self-repeating.
