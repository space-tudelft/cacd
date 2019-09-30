.nr Ej 1
.H 1 "INTRODUCTION"
.H 2 "Running Dali"
Dali (the Delft Advanced Layout Interface) is an interactive front-end
to the layout database of the Nelsis IC Design System,
to be used for the creation and manipulation of layouts of integrated
circuits.
It has been written in the programming language C and
runs under the Unix/Linux Operating System.
Dali is to be run in an X Window System graphics environment.
.P
.SP 0
Dali must be started in a \fIproject\fP directory.
See also \fImkpr\fP (1ICD).
It is invoked as follows:
.DS 1
\fBdali\fP [-\fBA\fP] [-\fBC\fP] [-\fBf\fP] [-\fBo\fP] [-\fBh\fP host:d#[.s#]] [=geo] [cell_name]
.DE
The meaning of the options is:
.tr ~
.VL 13
.LI -\fBf\fP
With this option one may specify an alternate dali design-rule file "dali_drc",
which must reside in the current working directory (project directory).
.P
.LI -\fBo\fP
When this option is selected, dali writes messages to an "animate" file in
some occasions when reading the ".dalirc" setup file.
.P
.LI -\fBA\fP
Sets an alternative plane mask for the Text Graphics Context.
.P
.LI -\fBC\fP
With this option the use of the standard CACD colormap can be
switched off.
See also \fIsetcmap\fP (1ICD).
.P
.LI -\fBh\fP~host:d#[.s#]
With this option one may specify which display is to be used by dali.
The display is identified by a hostname 'host' and a display number 'd#',
separated by a colon.
If this does not completely identify a unique screen, an additional
screen number '.s#' has to be specified.
For example: "\fCdutentn:0.0\fP".
.P
If this option is not used,
the environment variable DISPLAY is consulted to identify the display.
.P
.LI =geo
The window geometry 'geo' specifies the initial window size and placement
according to the standard X Windows geometry format
<width>x<height>{+-}<xoffset>{+-}<yoffset>.
For example, =+0 gives a default window at xoffset 0, i.e. in the upper left
corner of the screen.
The window geometry =1000x800+0 gives a rather big window in the upper
left corner, and =1000x800-0+0 gives a window of the same size in the upper
right corner.
.P
.LI cell_name
Name of the cell that is to be read at start-up.
.LE
.SP
Note that some X Window System parameters can be set via
the ".Xdefaults" setup file.
This are the \fIBorderWidth\fP (default: 1),
the \fIFontName\fP (default: "fixed"),
and the window \fIGeoMetry\fP (default: --).
Note that the window geometry can also be specified via the command-line.
Example of ".Xdefaults" entries:
.P
.DS
.in 1i
\fCdali.BorderWidth: 2
dali.GeoMetry: 850x700+20+20\fP
.DE
.H 2 "The Screen Layout"
Dali divides the screen area of the graphics terminal into four parts,
called viewports:
.AL
.LI
MENU viewport:  the area on the left side of the screen.
.SP 0
In this viewport the \fIcommands\fP available are displayed.
One may select a command by pointing at it (section 1.3).
The corresponding area is highlighted until the operation is finished
or another command is selected.
Certain commands require additional information to be
specified during their execution.
On these occasions dali can use this viewport to present alternatives
from which the user has to choose.
Selection proceeds in the same way as command selection.
.LI
LAYER viewport:  the area on the right side of the screen.
.SP 0
In this viewport the \fIlayers\fP of the \fIprocess\fP
of the current project are displayed,
their colors and fill styles (section 1.8).
One may (de)select a layer by pointing at
the rectangle with the name of the layer in it.
If a layer is selected (activated) a small box before the
layer is painted yellow.
The active layers may be used in subsequent editing operations.
.br
Via the ".dalirc" setup file (\fIdisabled_layers\fP command)
layers may be declared to be non-editable.
Non-editable layers can not be selected or activated.
.br
Via the visible menu or the ".dalirc" setup file
layers may be declared to be non-visible.
The color and fill style information of non-visible layers are not shown.
Non-visible layers can also not be selected.
.P
Note that the order of the layers in the viewport depends on the setting
of the dominant/transparent drawing mode.
.LI
TEXT viewport:  the strip at the top of the screen area.
.SP 0
In this viewport dali displays (error) messages,
or asks the user to enter some alphanumerical information such as a cell name.
In the latter case dali also uses this viewport to echo
the characters that are entered via the keyboard.
.P
Note that the rightmost part of TEXT viewport is used also
by the tracker to display cursor coordinates and displacements.
.LI
PICT viewport:  the large center area.
.SP 0
In this viewport the picture of the layout one is working on is displayed
and points can be entered if required by one of the commands.
.LE
.P
The different viewports can be recognized in Figure %APPEARANCE%,
which is the real-life appearance of dali.
.P
.F+
.PSPIC "../dali/dali_fig2.ps"
.F-
.fG "Appearance of dali with a layout being displayed" %APPEARANCE%
.H 2 "Pointing and Cursors"
Dali accepts input from the keyboard of the
terminal and from a mouse attached to it.
Whenever a point has to be entered, dali displays a \fIcursor\fP on the
screen, which can be moved around with the aid of the \fImouse\fP.
Upon pushing the button of the mouse the point
is selected.
In the sequel of this manual we will refer to this activity as \fIpointing\fP.
.P
Dali uses several types of cursors.
For most operations, e.g. command and layer selection,
a small \fIcrosshair\fP is used.
When rectangular areas, polygons or center-lines of wires have to be entered,
dali switches to \fIrubber-box\fP / \fIrubber-line\fP cursors
after the first point has been fixed.
.P
On occasions where dali expects the user to specify a point
in the PICT viewport, it is often allowed to point at a command
or layer instead.
This gives the user the opportunity to cancel the command in progress
by selecting the same command again or another command,
or to (de)select the layers on which the command will operate.
Examples are the \fIadd_box\fP and \fIadd_wire\fP commands.
.P
Dali
provides a tracker which displays coordinate information of the cursor
in the rightmost part of the TEXT viewport as the cursor is moved around
in the PICT viewport.
For rubber-box / rubber-line cursors the tracker displays the current
cursor position as well as the displacement from the start(last end) point.
Via the settings menu the tracker mode can be set "auto", "on" or "off"
with the \fItracker\fP command.
The default tracker mode ("off") can also be changed via the ".dalirc" setup file.
In "auto" mode the tracker information is only visible if a start point
is specified and the cursor is switched to rubber-box / rubber-line cursor.
.H 2 "Cell Editing"
Dali must be started in a \fIproject\fP directory.
It then interacts with the layout database of this project to allow
the designer to create or modify layout \fIcells\fP.
These cells are the entities on which dali operates; they are
retrieved / stored as a whole from / into the database.
.P
While operating on a cell, all modifications are performed on a copy
which dali maintains internally in its \fIprivate workspace\fP.
Existing cells can be \fIread\fP from the database into this workspace
with the \fIread_cell\fP command.
Changes that have been made on the workspace copy are effectuated only
when the cell is \fIwritten\fP explicitly to the database 
with the \fIwrite_cell\fP command.
Thus, one should take care to \fIsave\fP the workspace copy whenever
a considerable amount of valuable changes has been made to it.
The workspace can be \fIerased\fP with the \fIerase_all\fP command.
This will \fInot\fP affect an original or previously saved cell description
that is already present in the database.
.P
Cells may be created from scratch or by modifying cells that were already
present in the database.
In the former case the newly created cell can be stored in the database
under a new name.
In the latter case the cell can either be stored under its present name,
thereby overwriting the old cell description,
or it can be stored under a new name.
There is a setup option \fIuse_new_name\fP,
whenever this option is activated the new name is used in place of the old
name for the edited cell in the workspace after the \fIwrite_cell\fP command.
In the sequel of this manual we will use the term "edited cell" to refer
to the piece of design that is present in the private workspace of dali,
irrespective of its origin.
.P
.ne 6
A layout cell consists of three types of data:
.BL 5
.LI
\fIprimitive mask geometries\fP (representation data).
.LI
\fIterminals\fP (interface data).
.LI
\fIinstances\fP: references to component cells (hierarchical data).
.LE
.P
Dali provides support for the manipulation of all three types of data.
For each type a set of commands is available in a separate menu.
In the following three sections we describe dali's capabilities in handling
these types of data.
.H 2 "(Non-) Orthogonal Mask Geometries"
The primitive mask geometries of the edited cell can be manipulated
by adding / deleting \fIboxes\fP or \fIpolygons\fP or by adding \fIwires\fP
for the selected layer(s).
As opposed to several other layout editors, manipulation of the primitive
mask geometries with dali is \fInot\fP element-based.
Instead, a \fIcut-and-paste\fP technique is supported.
This implies, for instance, that a rectangular mask geometry that was once
added as a box does not have to be deleted as a whole; parts of it can simply
be cut away.
.P
By means of \fIbuffer\fP operations, chunks of mask geometries
can be moved or copied.
Deleted geometries are saved in the buffer, allowing the designer to undo
the delete operation by \fIputting\fP (adding) the buffer at the same spot,
or \fImove\fP the geometries by putting the buffer somewhere else.
The buffer can also be filled explicitly, without deleting geometries,
thereby providing a \fIcopy\fP facility.
Care should however be taken, as only \fIone\fP geometry buffer is
available;
whenever it is filled, the old contents are destroyed.
.P
Dali supports orthogonal (Manhattan) and 45 degree geometries only.
The box (or rectangle) is the most frequently used type of geometry.
Boxes can be added / deleted
with the help of a rubber-box cursor.
The corner points are automatically rounded
to the \fIlambda grid\fP or a multi-lambda \fIsnap grid\fP,
if active (section 1.10).
.P
Polygons having edges at 45 degree angles can be added / deleted with the
help of a rubber-line cursor to specify the contour of the polygon.
Dali can handle self-intersecting polygons.
Corner points may be entered on grid points and half grid points.
(As intersection points of 45 degree edges may now occur at quarter grid
points, dali internally maintains a higher precision than the \fIlambda\fP
grid presented to the user.)
This capability is offered to allow the user to create wires containing
45 degree parts with a width that only slightly deviates from the width
of the orthogonal parts.
The snap grid does not apply to polygon editing.
.P
A wire facility is included to permit convenient specification
of interconnection patterns.
After a width has been fixed, the center-line of the wire can be entered with
the help of a rubber-line cursor.
Both orthogonal wires and wires containing 45 degree parts
can be entered this way.
The center-line is automatically fixed on
the \fIlambda grid\fP or a multi-lambda \fIsnap grid\fP,
if active (section 1.10).
If the snap grid is active only even widths are permitted.
.P
As dali is capable of handling Manhattan and 45 degree geometries only,
one should take care when reading cell descriptions from the database
that have been generated by other programs and contain arbitrary geometries
(polygons with non-45 degree edges, circles).
Dali checks all layout elements when reading from the database.
All edges are checked to be proper orthogonal or 45 degree edges.
Whenever a check fails the element is skipped and a warning is generated.
.H 2 "Terminals"
A terminal is a named box, which may be arrayed.
Terminals play an important role in the layout verification strategy
of the Nelsis IC Design System.
The terminals of a cell specify at which points the cell may be connected
to its surroundings upon instantiation (section 1.7).
New:
Now it is also possible to add point terminals (width and height = 0).
.P
Dali offers commands to add or delete terminals.
When the terminals in a specified area are deleted (\fIdel_area\fP),
they are saved in a terminal \fIbuffer\fP
(to be distinguished from the geometry buffer),
which can be placed again at a later time (\fIput_buf\fP command).
Note that the terminal buffer is overwritten after each
successful \fIdel_area\fP or \fIyank_area\fP operation.
.P
Other commands allow terminals to be \fImoved\fP or \fIarrayed\fP.
These commands operate on the terminal,
which is selected with the cursor.
As an example of a higher level assembly function, dali has the ability
to generate terminals "on top of" the terminals of the cell's instances,
thereby making these spots available for interconnection
at the next higher hierarchical level.
This is possible with the lift terminals (\fIlift_tms\fP) command.
.P
On various occasions one has to identify the terminal
on which a command should operate (\fIarray_term, del_term,\fP etc.).
A terminal can be selected by just pointing at it in the PICT viewport.
In case the terminal has been arrayed,
one can identify it by pointing at one of its occurrences.
If the point is ambiguous because it identifies more than one terminal,
dali asks for a further specification by presenting
a sub-menu with the names of the terminals that were identified by
the specified point.
In this case, one of the names has to be selected from the sub-menu.
The name of the selected terminal is reported in the TEXT viewport.
.H 2 "The Cell Hierarchy"
.H 3 "Instances and Instance Manipulation"
.br
Dali supports hierarchical design, meaning that cells can be constructed
using other (smaller) cells as their components.
The definition of these component cells must be present in the database
before they can be \fIinstantiated\fP in the edited cell.
An \fIinstance\fP is a \fIreference\fP to a component cell,
accompanied by a geometrical transformation (translation and orientation)
and repetition information.
Dali offers commands to add or delete instances to / from the edited cell,
thereby changing its hierarchical composition.
Normally this are \fIlocal\fP cells,
but these may also be instances of cells
that have previously been \fIimported\fP from other projects.
See also \fIimpcell\fP (1ICD).
.P
The translation and orientation of the instances in the edited cell can be
manipulated by \fImove, rotate\fP and \fImirror\fP commands.
\fIArrays\fP of instances can be created / manipulated by commands that
allow the user to specify repetition numbers and distances.
These instance manipulation commands operate on the instance,
which is selected with the cursor.
.P
Dali assumes the component cells to be fixed; it does not allow the
definition of a component cell
to be changed from \fIwithin\fP the edited cell.
Only the transformation and repetition information of the reference
can be manipulated.
Whenever changes have to be made to the component cell itself,
its definition must be edited separately.
.P
By default, dali displays only the bounding box of an instance,
with the name of the instance and the name of the component cell
displayed in the lower left corner.
Note that the name of the cell is displayed between '(...)' and
that the instance name is displayed above the cell name.
This instance name is a dot '.',
if no instance name is given to this cell occurrence.
Use the name instance (\fIname_inst\fP) command to give this instances
a instance name.
Set \fIask_inst\fP (in the settings menu) to do it directly by
the \fIadd_inst\fP command.
In case of an array of instances, dali shows the bounding boxes of all
the occurrences, with the name of the instance and the name of the
component cell in the (0, 0) occurrence,
together with the [n \(mu m] array dimensions.
When a command for the manipulation of the translation or orientation
of an instance is executed on an array of instances, this will affect
all individual occurrences. Dali handles the array as one entity.
.H 3 "Selection of Instances"
.br
On various occasions one has to identify the instance
on which a command should operate (\fIname_inst, del_inst, indiv_exp, lift_tms\fP).
An instance can be selected by just pointing at a position
within its bounding box (repetitions included) in the PICT viewport.
If the point is ambiguous because it lies inside the bounding box of more
than one instance, dali asks for a further specification by presenting
a sub-menu with the names of the component cells of these instances.
In this case, one of the names has to be selected from the sub-menu.
The name of the component cell of the selected instance (and its instance name)
is reported in the TEXT viewport.
If this is the correct one, then this cell must be selected again,
else another cell must be selected.
.H 3 "Expansion of Instances"
.br
Dali offers commands that enable the user to make the contents of the
instance(s) visible up to a certain \fIexpansion level\fP,
either by setting a \fIglobal\fP expansion level for \fIall\fP instances
or by setting the level on an \fIindividual\fP basis (\fIindiv_exp\fP command).
The default situation,
where only the bounding box of an instance is displayed,
is called level 1.
If an instance is \fIexpanded\fP to level 2, the primitive mask geometries
of the component cell are displayed, together with bounding boxes for the
instances that are in turn part of this component cell.
As the expansion level is further increased, say to level n, dali displays
the mask geometries of cells from deeper levels in the hierarchy,
up to level n.
Dali always displays the bounding boxes of the instances of the level 2 cells
in the edited cell (edited cell = root: level 1), as these
instances are the objects that can be manipulated.
Dali also displays the bounding boxes of the level n+1 cells, which are
still unexposed.
Note that dali reads cells (\fIread_cell\fP command) normally at level 1,
but it is possible to specify another default expansion level
(see settings menu or ".dalirc" setup file section).
.P
It should be emphasized that the expansion facility is provided only for
display purposes:
the contents of the instances does not really become part of the edited cell
(in the sense of primitive mask geometries).
Only the reference to the component cell is part of the edited cell.
.H 3 "Hierarchical Consistency"
.br
Care must be taken when a cell has been edited,
as other cells higher up in the hierarchy may have to be edited too.
That is, the correctness of cells containing the edited cell may be
affected by changes in the edited cell, such as a change
of its bounding box
(see also the command \fIupd_bbox\fP in the \fIinst_menu\fP).
See also \fIinstcell\fP (1ICD).
.H 2 "Layers"
The layers or "colors" that are available for the construction of primitive
mask geometries and terminals are determined by the \fIprocess\fP the user is using.
For each project a corresponding process has been specified;
see \fImkpr\fP (1ICD).
.P
At startup, dali retrieves the layer information from the project environment.
From this information it learns e.g. which layers to use (names),
which colors and fill styles to use for their representation,
and how these colors should mix when several layers are put on top of
each other (using dominant or transparent drawing mode).
From this information dali also learns which layers are the interconnection
layers, to be used for terminals.
See also 
\fIgetproc\fP (1ICD) and 
\fImaskdata\fP (4ICD).
In this way, dali can be used for any technology, provided that a proper
layer description is present.
Note that the colors and fill styles can be changed via the ".dalirc" setup file.
Also the dominant drawing order and mode can be choosen.
Note that now the \fIsettings\fP sub-menu of dali can also change this setup.
.P
The layers are displayed in the LAYER viewport.
Their names are always shown.
And also the colors and fill styles of visible layers are always shown.
The order of the layers (bottom to top) depends on the dominant/transparent
drawing mode selection.
Note that ``black'' layers are always (most) dominant,
and that black colored layers are drawn against a white background.
In the LAYER viewport area the user can select (activate) the layers
that must be involved in certain
operations (e.g. \fIadd_box, add_wire, add_term\fP)
by toggling the corresponding rectangle.
Toggling puts the yellow lamp (a small box before the layer name) "on" or "off".
.P
Via the ".dalirc" setup file (section 1.12) layers may be declared
to be \fInon-editable\fP.
Non-editable layers can not be selected for manipulation of mask
geometries or terminals.
.P
At startup, dali assigns colors to the bitplanes of the graphics terminal.
These are not only layer colors and their mixtures, but also colors for
grid, menu, text, etc.
For graphics terminals having only a small number of bitplanes (less than 8),
dali has to make a less optimal color assignment, implying that screen updates
are sometimes handled less efficiently.
Note that the program \fIsetcmap\fP (1ICD) tries to install a CACD colormap,
such that transparent mode shows nice mixture colors.
If only a read-only colormap is available,
this color mixture may be wrong.
In that case it is better to use the dominant drawing mode and to select
nice fill styles (or stipples).
.P
Note that if the process contains many layers, maybe not all layers are shown.
You can resize (enlarge) the dali screen to the bottom to show more layers.
You can also click on "next" or "prev" at the top of this viewport to see
other layers.
.H 2 "Panning and Zooming"
Dali offers various commands to control which part of the edited cell
is displayed in the PICT viewport, i.e. the \fIwindow\fP on the layout.
When a cell is read in from the database, it is fit in the viewport,
together with a small margin.
Zooming-in (\fIzoom\fP) and -out (\fIdezoom\fP) as well as
panning (\fIcenter\fP) can be done with the mouse and cursor:
simply point at a new area or new center-point.
This "area" zooming mode is default case,
but can be changed (see settings menu or ".dalirc" setup file section)
to "point" or "fixed" mode.
Other commands allow the user to go back to the previous window (\fIprev\fP)
or to the window that comprises the complete layout (\fIbbox\fP).
This "\fIwindow\fP"-manipulation commands have '[...]' around them,
to distinguish them from other menu commands.
.H 2 "The Grid"
Dali is a \fIgrid\fP based editor; all coordinates (except for polygons)
are rounded to integer coordinates on the \fIlambda\fP grid.
Absolute coordinate values can be retrieved by means of the \fIx,y,masks\fP command.
This command displays also the layer stack on that point.
.P
Dali provides two user-controlled grids: a \fIdisplay grid\fP and
a \fIsnap grid\fP.
The display grid is used for display purposes only, as a reference
of scale for the designer.
With the \fIdisp_grid\fP command in the \fIsettings\fP menu the
display grid can be switched on or off (\fIvisible\fP command, default "on"),
and a grid spacing can be selected explicitly.
The display grid can also be set to \fIauto-adjust\fP (default "on"),
then dali will select an appropriate grid spacing
based on the size of the PICT viewport
in terms of lambda's (size of the window).
Note that if \fIauto-adjust\fP is "on",
the user can't select any other grid spacing value.
The grid-points are always positioned at coordinates that
are 'modulo grid spacing' values.
By default,
the possible grid spacing values are
1, 2, 4, 8, 10, 20, 50, 100, 1000 and 10000 lambda.
The set of possible grid spacing values can be redefined
via the ".dalirc" setup file (section 1.12).
.P
The snap grid is used to round coordinates entered by the designer
to a multi-lambda grid.
The snap grid points to which rounding is performed are displayed as small
plus-signs.
Note that snapping (rounding to snap grid)
is only performed as the snap grid is set \fIactive\fP,
no matter of the snap grid is displayed or not.
With the \fIsnap_grid\fP command in the \fIsettings\fP menu the snap
grid can be switched on or off (\fIvisible\fP command, default "off").
The snap grid spacing can be selected explicitly and an offset
(\fIoffset\fP command)
for the snap grid with respect to the origin can be specified interactively.
By default, the snap grid spacing is 1 lambda and the offset is (0, 0).
By default,
the possible snap grid spacing values are
1, 2, 4, 8, 10, 20, 50, 100, 1000 and 10000 lambda.
The set of possible snap grid spacing values can be redefined
via the ".dalirc" setup file (section 1.12).
.P
Note that the display of both grids is controlled by the \fIgrid\fP command,
and that the display grid and/or snap grid are
only visible if they are set \fIvisible\fP.
Note that besides these grids
dali always displays the axes of its coordinate system (if possible).
.H 2 "Design Rule Checking"
Dali offers facilities to perform on-line design-rule checks.
First of all, dali has been interfaced to the \fIdimcheck\fP
(1ICD) package, to perform complete (multi-layer) checks (\fIDRC_menu\fP).
When a check has to be performed, dali writes the edited cell
from its private workspace to a scratch
cell in the database.
The dimcheck package performs the actual check
on this scratch cell,
and dali retrieves the results and removes the scratch cell.
It must be noted that by default a \fIhierarchical\fP check is performed.
That is, the primitives of the edited cell are checked as well as
the interaction with and among the instances,
but not the complete component cells.
This is achieved by running \fIexp\fP (1ICD) with the "-h" option
before running dimcheck.
The default hierarchical expansion mode can be overruled from
the ".dalirc" setup file (section 1.12),
to have a linear or flat expansion performed for a complete check.
Dali offers several commands that can help the user in localizing / displaying
the errors that were found (See also chapter 7: DRC_menu).
.P
The second check facility (\fIcheck\fP command in the \fIbox_menu\fP)
is a fast integrated design-rule checker, performing \fIsingle-layer\fP checks:
it checks only for design-rule violations caused by mask geometries
of the same layer (compare to \fIautocheck\fP (1ICD)).
The checks are performed only for the area that is specified by the user.
All \fIvisible\fP layout geometries (i.e. primitive mask geometries, terminals,
expansion information and sub-terminals) within this area are checked:
\fIWhat You See Is What You Check\fP.
Errors that are found are displayed automatically.
.P
The integrated checker tries to retrieve its design-rules from a file
\fIdali_drc\fP (4ICD) that must be present in the process directory
of the current process in the standard process library.
This file specifies for each layer (field 1) the minimal width (2),
the minimal gap (3) for a pair of edges that are both longer
than a critical length (5) and the minimal gap (4) if one of the edges is
shorter than or equal to this length (5).
If this file is not present for the current process a message will be
generated.
.H 2 "The .dalirc Setup File"
.de Lc
.LI
.ft C
\\$1 \\$2 \\$3 \\$4 \\$5 \\$6 \\$7 \\$8 \\$9
.ft R
.br
..
Dali provides the user with the capability to specify setup data or
to execute statements at startup time.
To that end dali tries to read commands from a setup file.
It first looks for a file named ".dalirc" in the current working directory.
If the file does not exist or cannot be opened for reading,
dali looks for a file named ".dalirc" in the user's home directory.
If that file does not exist or cannot be opened for reading either,
dali finally tries for a file named "dalirc" in the process directory.
If a setup file is found present and readable, dali executes the
commands found in the file.
.P
The commands that can be specified in the setup file
are divided into two groups.
One group is for specifying setup data,
such as the possible snap grid spacing values,
the other group contains editing commands that are to be executed by dali,
and can be used for purposes of animation.
.P
The syntax and semantics for these commands is given below.
A complete command or statement consists of a single line and starts
with a (literal) keyword followed by a number of arguments.
Other literals are delimited by '"' and '"'.
Arguments delimited by '<' and '>' are obligatory
(not literal, their value has to be filled in),
arguments delimited by '[' and ']' are optional,
arguments separated by a '/' means a choice of either one of them.
\&'....' means a variable number of arguments.
.H 3 "Specifying Setup Data"
.BL
.Lc default_expan[sion] <level>
Set default expansion level for the \fIread_cell\fP command.
Value must lay between 1 to 9.
If "no" is specified,
then there is always asked for a level.
.Lc disabled_layers [<v1> <v2> .... <vn>]
\&\fCv1\fR, \fCv2\fR, ...., \fCvn\fR are the names of the layers
that are declared to be non-editable.
If no layers are specified,
then all layers are editable.
By default all layers of the given process can be edited.
.Lc display_grid_values <v1> <v2> .... <vn>
\&\fCv1\fR, \fCv2\fR, ...., \fCvn\fR are the possible values that will be
displayed in the display-grid menu.
If not specified, a set of default values will be displayed.
.Lc display_grid_width <aWidth>
The program initializes the display grid spacing to \fCaWidth\fR (>= 1).
This command implies the disabling of the automatic grid adjustment
(see section 1.10).
.Lc dominant ["on"/"off"]
The drawing mode can be set to dominant ("on") or transparent ("off"),
which is the default mode.
No second argument implies "on".
In transparent mode mix colors are produced if layers are stacked on top
of each other.
In dominant mode one layer may hide another layer underneath.
.Lc drawing_order [<v1> <v2> .... <vn>]
This command specifies the drawing order for the layers in case
the dominant mode was set.
\fCv1\fR, \fCv2\fR, ...., \fCvn\fR are the names
of the layers.
The order is such that \fCv1\fR is the 'top' layer, and hence is drawn last.
A new drawing_order command specifies a new drawing order.
If no layers are specified,
then the default situation is used.
.Lc drc_options [<options>]
Set command-line options for the dimcheck program.
.Lc exp_options [<options>]
Set command-line options for the exp program (see also \fCflat_expansion\fP).
.Lc fill_style <layer> <style#>["+"/"-"<shift>]
Set fill style \fCstyle#\fP for certain layer.
The following fill styles can be defined: 0=hashed, 1=solid, 2=hollow,
3-5=12.5,25,50% hashed+outline, 6-8=12.5,25,50% hashed (no outline).
Note that fill styles 0 and 6 are identical.
By the hashed fill styles it is possible to specify a \fCshift\fP
in the range 1 to 7 (fill lines are shifted that number of pixels).
Fill style numbers 10-18 are for cross(=10) and stipple1 to stipple8.
Use style numbers 20-28 to get also an outline.
.Lc flat_expansion ["on"/"off"]
The expansion mode for the \fIdo_check\fP command in the DRC_menu
can be set to linear / flat expansion ("on") or
to hierarchical expansion ("off"), which is the default mode.
No second argument implies "on".
.Lc hashed ["on"/"off"]
Set the drawing mode for the layers of instances to hashed fill style.
No second argument implies "on".
.Lc image[name] [<image_name>]
Define or undefine the instance name for the Sea-of-Gates image.
For this instance no bounding boxes, no instance names, no terminal names
are displayed.
On level 1 the terminals of the image are displayed in hashed style.
.Lc imagemode ["on"/"off"]
If there is an image grid and image mode is "on",
instances during move or add commands are automatic snapped
to this image grid.
.Lc init_window <xl> <yb> <dx>
Specifies that this initial window must be used
after reading a cell or after the \fIerase_all\fP command.
The values must be specified in quarter lambda design units.
Values <xl> and <yb> specify the lower-left bottom and <dx> the delta
(window size) in the x-direction (must be >= 40).
.Lc load <savefile>
Load this dali <savefile> (see the dali \fC[load]\fP and \fC[save]\fP commands
in the \fIsettings\fP menu) and redraw the picture.
One can change the colors, fill styles, dominant order and visibility
of the layers with this command and also change other menu settings.
.Lc maxdraw <value>
Specifies the maximum number of cell instance repetitions which may be drawn for
the Sea-of-Gates image.
Default value is 120.
.Lc no_grid_adjust
This command disables the automatic adjustment of the display grid to the
size of the window (see section 1.10).
.Lc set_colornr <colornr> <color>
With this command other colors can be assigned to the internal dali
colortable.
However, colornumbers 0 to 7 can not be changed by the user,
they are already in use (colors: black, red, green, yellow, blue,
magenta, cyan and white).
Only colornumbers 8 to 15 can be assigned a new \fCcolor\fP (X Windows
colorname or #RGB-value).
This command must be at the beginning of the ".dalirc" file.
.br
See also the \fCset_maskcolornr\fP command.
.Lc set_maskcolornr <mask> <colornr>
With this command other colors (see \fCset_colornr\fP)
can be assigned to the masks (layers) of the process in use.
This overrules the specifications in the process \fImaskdata\fP file.
Note that the colornumbers 0 and 15 are in use by the contact hole layers
(which are displayed dominant)
and that colornumber 15 is default assigned the color 'grey'.
This command must be at the beginning of the ".dalirc" file.
.Lc snap_grid_offset <x_offset> <y_offset>
The initial offset of the snap grid with respect to the origin is
defined with this command.
Default snap grid offset is (0, 0).
This command implies the \fIvisible\fP and \fIactive\fP setting
(see section 1.10).
.Lc snap_grid_values [v1] [v2] .... [vn]
\&\fCv1\fR, \fCv2\fR, ...., \fCvn\fR are the possible snap grid values
that will be displayed in the snap-grid menu.
If not specified, a set of default values will be displayed.
Initial grid_value is \fCv1\fR.
.Lc snap_grid_width <aWidth>
The program initializes the snap grid spacing to \fCaWidth\fR (>= 1).
Default value is 1 lambda.
This command implies the \fIvisible\fP and \fIactive\fP setting
(see section 1.10).
.Lc stipple<nr> <width>x<height> { <bits>... }
Use this bitmap <bits> for stipple<nr> 1 to 8.
The <width> and <height> of the bitmap must be <= 8.
The <bits> must be in hexa-decimal notation (i.e. 0x2f).
Use the \fIbitmap\fP editor program to make bitmaps.
.Lc tracker [<mode>]
Set tracker mode to "auto", "on" or "off" (see \fIsettings\fP menu).
No second argument implies "on".
.Lc use_new_name ["on"/"off"]
Set the usage of the new_name in place of old_name
after the \fIwrite_cell\fP command.
No second argument implies "on".
.Lc via[name]   [<via_name>]
Define or undefine the first three characters of the instance names
for the Sea-of-Gates via cells.
Note that this instances are displayed
without instance names and terminal names to make them more clear.
.Lc wire_extension ["on"/"off"]
Switch wire extension on or off.
No second argument implies "on".
See the ADD_WIRE command in the BOX_MENU (section 4.13).
.Lc wire_width <aWidth>
The program initializes the wire-width to \fCaWidth\fR (>= 1).
If the wire-width is not set from the setup file,
dali will ask for a wire-width the
first time the \fIadd_wire\fP command is invoked.
.Lc wire_width_values <v1> <v2> .... <vn>
\&\fCv1\fR, \fCv2\fR, ...., \fCvn\fR are the values that will be
displayed in the wire-width menu.
Given values must be >= 1.
If not specified, a set of default values will be displayed.
.Lc zoom_mode <mode>
The zooming mode for the menu commands \fIdezoom\fP and \fIzoom\fP
can be set to "area", "fixed" or "point" (see \fIsettings\fP menu).
.LE
.P
.H 3 "Layout Editing Commands / Animation"
.BL
.Lc add_val <value>
Add integer \fCvalue\fP to internal value.
.Lc append <xl> <xr> <yb> <yt>
Append a rectangle with the specified coordinates
to the layout in the selected layers.
.Lc beep [<volume>]
Ring a bell.
The \fCvolume\fP percentage can be +/-100 (default: 0).
.Lc center <cx> <cy>
The new window is centered on (\fCcx\fR, \fCcy\fR),
where \fCcx\fR and \fCcy\fR
are relative to the contents of the dali workspace:
(0.0, 0.0) is the lower left corner of the dali workspace
and (1.0, 1.0) is the upper right corner of the dali workspace.
.Lc delete <xl> <xr> <yb> <yt>
Delete the layout of the selected layers found in the specified coordinate area.
.Lc expand <level>
The hierarchical expansion level is set to \fClevel\fR (>= 1 and <= 100).
The edited cell is expanded with this \fClevel\fP.
.Lc grid ["on"/"off"]
Turn the grid "on" or "off".
.Lc goto <name>
Jump to the position in the setup file that is labeled \fCname\fR.
If \fCname\fR can not be found in the setup file,
then remainder of the ".dalirc" file is skipped.
.Lc if_val <value> <name>
If value is equal to \fCvalue\fP goto position
in the setup file labeled \fCname\fP.
.Lc ifnot_val <value> <name>
If value is not equal to \fCvalue\fP goto position
in the setup file labeled \fCname\fP.
.Lc label <name>
Position in the setup file to which can be jumped through a \fCgoto\fR command.
.Lc layer <layer> ["on"/"off"]
The layer \fClayer\fR is either selected ("on") or de-selected ("off")
or toggled (no second argument) for editing.
Note that \fCdisabled_layers\fP are always "off".
.Lc print [<someText>]
Print \fCsomeText\fR in the TEXT viewport or \fCerase\fP the TEXT viewport.
.Lc quit
Exit immediately the dali program.
.Lc read <cell_name>
Read the specified cell into the dali workspace
(with default expansion level).
.Lc redraw
Redraw the dali display.
Works only after changing 'visible' or 'dominant' mode.
.Lc set_val <value>
Set internal value to integer \fCvalue\fP,
which is used by the \fCif_val\fP or \fCifnot_val\fP command.
.Lc sleep <numsec>
Wait \fCnumsec\fR seconds before continuing with the next command.
.Lc visible <layer> ["on"/"off"]
The layer \fClayer\fR is either displayed ("on") or not
displayed ("off") or toggled (no second argument) for display.
Note that \fClayer\fP can also be a name like
"\fIbboxes\fP",
"\fIdisp_grid\fP",
"\fIinstances\fP",
"\fIsub_terms\fP",
"\fIterminals\fP",
etc.
(see \fIvisible\fP menu).
.Lc wdw_bbx
Redraw the dali display with the full contents of the dali workspace
in view.
.Lc zoom <cx> <cy> <fraction>
The size of the new window on the dali workspace
is equal to \fCfraction\fR times
the size of the contents of the dali workspace,
and is centered on (\fCcx\fR, \fCcy\fR), where \fCcx\fR and \fCcy\fR
are relative to the contents of the dali workspace:
(0.0, 0.0) is the lower left corner of the dali workspace
and (1.0, 1.0) is the upper right corner of the dali workspace.
.LE
