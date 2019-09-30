.H 1 "THE MENU STRUCTURE"
.H 2 "The Command Classes"
The dali commands can be divided into several classes.
.BL
.LI
Basic editing commands to make changes to the edited cell.
This class can be divided into three sub-classes.
.AL
.LI
Commands to handle the primitive mask geometries
(representation data).
.LI
Commands to add, delete or manipulate terminals
(interface data).
.LI
Commands to add or delete instances or to change their
parameters (hierarchical data).
.LE
.LI
Database interaction / workspace commands which allow the user to read
cells from the database into the private workspace of dali,
store them again into the database when he thinks he has made
some valuable changes,
and a command to erase the workspace.
.LI
Additional support commands which allow the user to change his window, set
the visibility of some sort of objects or to put a grid over the design.
.LI
Commands to check if the layout that has been generated obeys the
design-rules posed upon them.
.LE
.H 2 "The Main Menu"
Dali has a hierarchical menu structure, based on the command classes
recognized above.
Upon starting, the \fImain\fP menu is shown on the screen, from which
the user may select one of the sub-menu's (command clusters).
Upon doing so, the set of commands belonging to the
cluster is shown, and the user may select one of them.
The bottom command in each sub-menu allows the user to return to the main menu.
The sub-menu's and commands in the main menu are:
.BL
.LI
.B quit
.br
With this command one can quit from the editor.
After a confirmation has been given the edit session is terminated.
The contents of the workspace will \fInot\fP be saved on this occasion.
If a valuable layout description is present in the workspace,
it should be saved explicitly in the database by means of the \fIwrite_cell\fP
command, before terminating the edit session.
.LI
.B visible
.br
In this sub-menu the user can set layers as well as other graphical information
visible / not-visible on the screen.
This menu is explained in more detail in chapter 10.
.LI
.B DB_menu
.br
Commands to interact with the \fIdata manager\fP (database).
Also commands to expand the instance(s) for display,
as well as commands to erase the workspace.
The commands in this menu are explained in more detail in chapter 3.
.LI
.B box_menu
.br
Commands to manipulate the \fIprimitive
mask geometries\fP of the cell being edited.
This includes the manipulation of polygons containing 45 degree edges,
buffer operations, wire editing and a fast integrated design-rule
checker performing single-layer checks.
The commands in this menu are explained in more detail in chapter 4.
.LI
.B term_menu
.br
Commands to manipulate the \fIterminals\fP, including the manipulation
of a terminal buffer as well as the possibility to "lift" terminals
of instances.
The commands in this menu are explained in more detail in chapter 5.
.LI
.B inst_menu
.br
Commands to manipulate \fIinstances\fP, including the manipulation
of arrays of instances
and the possibility to add instances of cells that have
previously been imported from other projects.
The commands in this menu are explained in more detail in chapter 6.
.LI
.B DRC_menu
.br
Contains the commands to do \fIdesign-rule checking\fP
and to show the results on the screen.
Dali contacts the \fIdimcheck\fP (1ICD) package
to perform the actual check.
The commands in this menu are explained in more detail in chapter 7.
.LI
.B info_menu
.br
In this sub-menu some \fIinformation\fP commands are gathered.
For instance, commands to get information about the active cell and
the process which is used.
The commands in this menu are explained in more detail in chapter 8.
.LI
.B settings
.br
This sub-menu contains the commands for setting \fIdali\fP working modes.
For instance, commands to control the display grid and snap grid
and to set the tracker mode.
The commands in this menu are explained in more detail in chapter 9.
.LE
.P
Some of the support commands appear in more than one sub-menu.
In particular the window operations as well as the \fIgrid\fP and
\fIx,y,masks\fP command are present in several sub-menu's
for convenience' sake.
New:
The
.B annotate
sub-menu (see chapter 11).
.H 2 "Command Selection / Operation"
Whenever a command is selected the corresponding area is highlighted
and remains highlighted until the operation has been finished
or another command has been selected.
Certain commands are \fIself-repeating\fP;
when they finish they are automatically re-selected.
These menu items remain highlighted, until another command is selected.
Thus you can apply these commands
repeatedly without explicit re-selection.
Examples are the \fIzoom\fP, \fIx,y,masks\fP and \fIadd_box\fP commands.
.P
Certain commands require additional information to be
specified during their execution.
For instance, coordinates, names, terminals, instances or modes
of operation may have to be specified.
When alphanumerical information has to be entered dali prompts
for this in the TEXT viewport.
The 'Return', 'Enter' or 'Esc' key finishes the keyboard input.
.br
The 'Delete' or 'BackSpace' key can be used to correct typing mistakes.
.P
When the user has to select an item from a set of alternatives
dali may present a menu for this in the MENU viewport,
from which the user has to make a choice.
When coordinates have to be specified in the PICT viewport,
it is often allowed to point at a command or layer instead.
As we already mentioned in section 1.3,
this gives the user the opportunity to cancel the command in progress
by selecting another command, or to (de)select the layers on which
the command will operate.
.H 3 "Keyboard Commands"
The following commands can be activated with keyboard keys:
.P
.nf
.ta 1.5c 2c 3c 4.5c
	Key:			Function:
.P
	0,1,...9			expand cell with this level (0 = maximum)
	e			expand cell with default level
	E			individual expansion
	N			new edit session (erase window)
	R			read (edit) another cell
	U			reread (update) current cell
	W			write (save) the current cell
	b	[Home]		bounding box window
	c	[Select]		center window at cursor position
	d			hashed drawing style on/off
	D			dominant drawing style on/off
	g			grid(s) on/off
	h	[\(<-]	(H)	pan left
	j	[\(da]	(J)	pan down
	k	[\(ua]	(K)	pan up
	l	[\(->]	(L)	pan right
	n			no confirmation to an asked question
	i		(+/=)	zoom in at cursor position (current window)
	o		(-)	zoom out at cursor position (current window)
	p	[Prev]		previous window
	q			quit (exit) the program
	r	[Next]	(^L)	redraw screen
	s			visible sub-terminals on/off
	t			tracker (cursor position display) on/off
	v			visible setup menu
	x			give coordinate (in lambda) to center window
	y			yes confirmation to an asked question
	Esc			escape from current menu or enter string
.fi
