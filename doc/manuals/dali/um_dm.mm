.nr Ej 0
.ne 40 \" I really like troff; all these mysteries.
.H 1 "The DB_MENU"
.nr Ej 1
This menu contains the following commands to interact with the
data manager (database),
as well as some related commands:
.AL
.LI
RETURN: Return to the main menu.
.P
After selection of this command the DB_menu will be left
and the \fImain\fP menu will be shown again.
.LI
READ_CELL: Read a cell from the database.
.P
Dali presents the list of local cells
from which one cell can be selected.
Note that only the local cells are presented and
not also the alias names of imported remote cells.
If this is a long list only part of it is presented at a time,
and \fI-prev-\fP, \fI-next-\fP and \fI-keyboard-\fP menu items are included
to allow the user to move back and forth through the list.
The \fI-keyboard-\fP menu item can be used to give interactively the cell_name,
but it can also be used to search for a cell_name beginning
with certain character(s) (and it sets a new list position).
Or a cell_name containing certain character(s).
In the latter case, the asterisk '*' wildcard must be used.
For example, the following searches are valid:
"a*4" (cell_name must start with "a" and end with "4"),
"a*4*" (cell_name must start with "a" and contain "4"),
"*14" (cell_name must end with "14"),
"*14*" (cell_name must contain "14"),
"via" or "via*" (cell_name must start with "via").
A cancel possibility is included as the last menu item.
Upon selection of a cell dali asks for a confirmation
if there is already a cell present in the workspace.
If confirmed, the workspace is cleared.
The layout data of the selected cell is
loaded into the workspace and the picture appears.
The objects that were present in the geometry and terminal buffer
will still exist after the execution of this command.
.LI
WRITE_CELL: Write a cell to the database.
.P
With this command
all the information pertaining to the edited cell that is
present in the workspace,
can be stored in the database.
One has the option to give the cell
a new name or to store it with its present name if it already has one.
A cancel possibility is included as the bottom sub-menu item.
Note that by \fInew name\fP mode the cell CREATE-mode is used,
otherwise the cell UPDATE-mode.
If the edited cell is stored under a new name while it already has one,
then the old name will still be related to the workspace copy that
remains after the \fIwrite_cell\fP command has finished.
This permits a quick \fIsave\fP of an intermediate result under a different
name.
However,
if the \fInew mode\fP setting (in settings menu) or the \fIuse_new_name\fP
command (".dalirc" setup file) is activated,
then the new name is used instead of the old name for the edited cell.
If the edited cell has been built from scratch and has not been saved before,
i.e. no name is yet related to the workspace copy,
then the new name will be related to the workspace copy after the
\fIwrite_cell\fP command has finished.
.P
If a design rule check has been performed
with the \fIdo_check\fP command (DRC_menu)
on the cell that is being saved,
the results of this check are present in the file "chk_mod.ck" in the
project directory.
\fIWrite_cell\fP changes the name of this file to "cell_name.ck",
where cell_name stands for the name of the cell that is written
to the database.
This file can then be used during a next session with the same cell
(see also chapter 7 on the DRC_menu).
.LI
ERASE_ALL: Erase the workspace.
.P
After a confirmation has been given, the workspace is cleared.
The geometry and terminal buffer will remain intact.
.LI
INDIV_EXP: Expand an individual instance.
.P
Upon the selection of this command 
one first has to identify the instance one wants to expand.
This can be done by just pointing at a position within the bounding box
of the instance (repetitions included).
See also section 1.7.2.
After a unique instance has been identified
a sub-menu appears from which one may choose the level of the expansion.
The current level of the selected instance is initially highlighted.
If the selected level is the same as the current level,
the command is canceled.
Otherwise,
the selected instance is (un)expanded to the selected level.
Thus, if the level is increased, additional detail will be shown on the
screen for this instance.
If the level is decreased, the instance will be erased and drawn again
with less detail in it.
.LI
ALL_EXP: Expand all instances.
.P
Upon the selection of this command 
a sub-menu appears from which one may choose the level of the expansion.
The current global expansion level is initially highlighted.
Note that this current level can be the level,
which is set with the \fIexpand\fP command from the ".dalirc" setup file.
If the selected level is the same as the current level,
the command is canceled.
Otherwise,
all instances are (un)expanded to the selected level.
That is, if the level has to be increased for a certain instance
because the new global level is larger than its current level,
additional detail will be shown on the screen for this instance.
If the level is decreased, the instance will be erased and drawn again
with less detail in it.
Note that level 10 can be any selected level in the range 10 to 99.
The maximum possible level (= 100) can directly be chosen with
the \fImaximum\fP command.
At last,
with the \fIkeyboard\fP command can interactively every level be chosen
in the range 1 to 100.
.LE
