.H 1 "The VISIBLE MENU"
As opposed to the other sub-menu's presented in the previous chapters,
the \fIvisible\fP menu actually is a dedicated menu for \fIone\fP command:
the \fIvisible\fP command.
Upon entering this command,
the LAYER viewport is used to control the visibility setting
of the layers of the current process,
and a menu is presented containing some menu items which are control commands,
together with some menu items corresponding to
certain classes of design data / graphical features:
.P
.in +4
.TS
tab(@);
l l.
\fIdisp_grid\fP @ (display grid)
\fIsnap_grid\fP @ (snap grid)
\fIterminals\fP @ (terminals)
\fIsub_terms\fP @ (sub-terminals)
\fIinstances\fP @ (instances)
\fIbboxes\fP @ (bounding boxes)
\fIterm_name\fP @ (terminal names)
\fIsubt_name\fP @ (sub-terminal names)
\fIinst_name\fP @ (instance names)
\fIlabels\fP @ (see annotate menu)
\fIcomments\fP @ (see annotate menu)
.TE
.in -4
.P
Each of the menu items can be \fIset\fP / \fIreset\fP by toggling
the corresponding area.
The menu items that are set (visible) are highlighted.
Initially, almost all items are set (except \fIsub_terms\fP and \fIsnap_grid\fP).
Pointing at an item causes it to be reset.
Pointing at items that have previously been reset will set it.
The newly chosen visibility setting will become directly effective.
Upon selection of the \fIreturn\fP menu item the visible menu is exited.
.P
The other menu commands are:
.tr ~
.VL 10n
.LI \fI\(em~restore\fP:
Restores the last saved visibility setting.
If no visibility setting was saved before,
the initial dali defaults are restored.
Note that \fIrestore\fP also saves the current visibility setting.
.LI \fI\(em~save\fP:
Saves the current visibility setting for later use.
.LI \fI\(em~all_on\fP:
Puts all "visibles" on.
.LI \fI\(em~all_off\fP:
Puts all "visibles" off.
.LE
.P
Note that the layers and the graphical features can also be set with the
\fIvisible\fP command via the ".dalirc" setup file.
.H 1 "The ANNOTATE MENU"
With the annotate sub-menu you can add and delete comments and labels.
Comments can be lines with or without arrows and text strings.
Labels are text strings (like terminals) used to specify names to
(interconnection) layers.
This is for back-annotation and specification of net-names.
.br
Besides the standard menu commands,
the following menu commands exists:
.AL
.LI
---------- :  Add line (comment).
.P
Enter begin and end points to add a line.
.LI
---------> :  Add line with right-arrow (comment).
.P
Enter begin and end points to add a line.
.LI
<--------- :  Add line with left-arrow (comment).
.P
Enter begin and end points to add a line.
.LI
<--------> :  Add line with double-arrow (comment).
.P
Enter begin and end points to add a line.
.LI
\&. . . .         :  Add right-adjusted text (comment).
.P
Enter the position to add adjusted text and
type the text in the text input area.
.LI
        . . . . :  Add left-adjusted text (comment).
.P
Enter the position to add adjusted text and
type the text in the text input area.
.LI
    . . . .     :  Add center-adjusted text (comment).
.P
Enter the position to add adjusted text and
type the text in the text input area.
.LI
LABEL :  Add a label.
.P
Enter the position to add the label and
type the label string in the text input area.
You can specify a layer in the layer area to add the label for.
The label string contains the label name and may be followed
by the label class name and a layer code (#number) or name.
Each separated from each other by a colon (:).
The class name may be empty.
.LI
DELETE :  Delete comments and/or labels.
.P
Click with the mouse on the position (begin or end point)
to delete the comment (line or text) or the label.
.LE
.P
Note that all these commands are self-repeating.
.TC
