.H 1 "The INFO_MENU"
This menu contains the following information commands
.AL
.LI
RETURN: Return to the main menu.
.P
After selection of this command the info_menu will be left and the
main menu will be shown again.
.LI
WINDOW: Present information about the current window.
.P
The current window settings are reported in the TEXT viewport.
This includes the coordinates of the piece of layout that is currently
being displayed in the PICT viewport.
.LI
PROCESS: Present information about the process of the current project.
.P
Some information about the process that corresponds to the project
in which dali is running, is reported in the TEXT viewport.
This includes the name of the process and the type process and
the value of lambda in microns (micrometer) that has been set for this project.
.LI
CELL: Present information about the edited cell.
.P
Some information about the edited cell in the workspace
is reported in the TEXT viewport.
This includes the name of the cell and its bounding box and
its current level of expansion.
.LI
S-O-GATES: Present information about Sea-of-Gates items.
.P
There is information about the following Sea-of-Gates items:
.DL
.LI
\fIimage_name\fP:
Displays the instance name which is used for the Sea-of-Gates 'image' cell.
.LI
\fIvia_name\fP:
Displays the first three characters of instance names which are used for the Sea-of-Gates 'via' cells.
.LI
\fImaxdraw\fP:
Displays the value (default: 120) of the maximum number of repetitions
of the Sea-of-Gates 'image' which are drawn.
.LE
.H 1 "The SETTINGS MENU"
This menu contains the following settings commands
.AL
.LI
RETURN: Return to the main menu.
.P
After selection of this command the settings menu will be left and the
main menu will be shown again.
.so ../dali/um_grid.mm
.LI
DISP_GRID: Control display grid settings
.P
A sub-menu is presented from which the settings for the display grid
can be controlled.
The keyboard commands (see section 2.3) operates also in this menu.
The visibility of the grid can be switched on and off
via the \fIvisible\fP menu item.
The grid spacing can be controlled either by explicitly selecting
a value from the possible grid spacing values displayed in the menu,
or by setting the display grid to \fIauto-adjust\fP.
In the latter case dali will select an appropriate grid spacing,
based on the size of the PICT viewport in terms of lambda's
(size of the window).
Note that in \fIauto-adjust\fP mode the user can not change the
grid spacing values.
.LI
SNAP_GRID: Control snap grid settings
.P
A sub-menu is presented from which the settings for the snap grid
can be controlled.
The keyboard commands (see section 2.3) operates also in this menu.
The visibility of the snap grid can be switched on and off
via the \fIvisible\fP menu item.
The snap grid spacing can be controlled by explicitly selecting
a value from the possible grid spacing values displayed in the menu.
An offset with respect to the origin can be specified interactively
via the \fIoffset\fP menu item.
By fixing a point in the PICT viewport, the snap grid is positioned
such that the newly entered point is one of the snap grid points.
Of actual snapping is performed, i.e. snap grid is active,
depends on the setting of the \fIactive\fP menu item.
.LI
ZOOM MODE: Set zooming mode area / point / fixed.
.P
The zooming mode can be set to area (default), point or fixed.
With area mode one must specify the zooming area,
with point mode only a point must be specified and
with fixed mode nothing need to be specified with the cursor
(centre of the PICT viewport is used).
By fixed and point mode the zooming factor is always two.
The zoom mode can also be set via the ".dalirc" setup file (section 1.12).
.LI
NEW MODE: Set use of old / new cell_name after write.
.P
Normally after the \fIwrite_cell\fP command (in the DB_menu),
if you write the cell under a new name,
the old name remains in use for the cell in the workspace which can
be edit again.
In new mode however the new name is used for the edited cell.
The new mode can also be set via the ".dalirc" setup file (section 1.12).
.LI
HASH MODE: Set drawing mode for instances normal / hashed.
.P
In hashed mode the layers of the cell instances are drawn hashed,
with lower intensity.
Thus one can good see the difference between the layers of the edited cell
and the layers of the instances of other cells in the edited cell.
The hash mode can also be set via the ".dalirc" setup file (section 1.12).
.LI
SET_FILLST: Set fill style for layers.
.P
Select one layer to change the fill style.
Click on the menu items to (de)select a fill style item.
Hashed and Stipple styles can also be shifted.
Select another layer or click return to leave the menu.
.LI
SET_COLOR: Set color for layers.
.P
Select one layer to change the color.
Click on one of the menu items to select a new color.
Click on "black_lay" to (de)select the auto dominant feature
for the layer.
Select another layer or click return to leave the menu.
.LI
UNSET_ORDER: Unset drawing order for layer.
.P
This command puts the PICT and LAYER viewport in dominant mode.
Click on a layer to unset its dominant drawing order position.
Only the layers left of the red line in the LAYER viewport can
be unset (if they are not black_lays).
.LI
SET_ORDER: Set drawing order for layer.
.P
This command puts the PICT and LAYER viewport in dominant mode.
Click on a layer and click again to set it in dominant drawing order position.
.LI
DRAW MODE: Set drawing mode for layers transparent / dominant.
.P
The drawing mode can be set to either transparent or dominant.
The command has only effect if a drawing order for the layers is specified.
In transparent mode mix-colors are displayed if mask geometries
of different layers are stacked on top of each other.
In dominant mode 'top' layers hide 'lower' layers.
The order of the layers in the LAYER viewport are also changed,
to show from left to right which layers are most dominant.
In transparent mode,
the order of the layers is the order from the "maskdata" file.
In transparent mode, there can still be some dominant layers.
These are the ``black'' lays (normally contact hole layers).
The drawing order of the layers can also be controlled
via the ".dalirc" setup file (section 1.12).
.LI
DEF_LEVEL: Set default expansion level.
.P
Normally the default expansion level after the \fIread_cell\fP command
(in the DB_menu) is one,
however another level can be selected.
If a default level of 'no' is selected,
then dali is asking for a level after each \fIread_cell\fP command.
The default level can also be set via the ".dalirc" setup file (section 1.12).
.LI
ASK_INST: Set query for instance name on / off.
.P
By the placing of instances of cells,
the \fIadd_inst\fP command (in the inst_menu),
it is possible directly to ask for the instance names (if set).
.LI
TRACKER: Set tracker mode.
.P
The tracker which displays coordinate information in the rightmost
part of the TEXT viewport can be switched between on, off or automatic.
In automatic mode, the tracker is only on when the cursor is switched
to rubber-box or rubber-line cursor.
.LI
LOAD: Load settings from .dalisave file.
The saved settings can be loaded again.
The result is directly visible in the PICT and LAYER viewport.
.P
.LI
SAVE: Save settings to .dalisave file.
The settings current in use can be saved to a ".dalisave" file
in the current working directory.
They can be easy loaded again.
.P
.LE
