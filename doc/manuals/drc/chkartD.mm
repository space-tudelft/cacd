.nr Hy 0
.SA 1
.nr Ej 1
.H 1 "APPENDIX D: The program dubcheck"
.H 2 "Introduction"
The program \fIdubcheck\fP is the program that does the
checking for overlap and gap errors between two (combination)masks.
.SP 0
It is called as:
.fS I

dubcheck [-f][-t][cell_name]

.fE
The meaning of the options is:
.VL 10
.LI -f
The program looks for the file \fIdubcheckdata\fP in
the current working directory instead of taking
the standard one from the technology used.
.LI -t
This option has been added for debugging purposes.
In generates a lot of test_data.
.LI cell_name
The name of the cell to be tested.
If not specified \fIdubcheck\fP looks for a file \fIexp_dat\fP
in which the cell(s) to be tested must be given.
.LE
.SP
So as its input the program needs:
.BL
.LI
A file \fIexp_dat\fP containing the cell(s) \fIdubcheck\fP
has to be applied to, or a cell_name must be given in
the call of the program.
.LI
A file dubcheckdata containing the layers to check and the
overlaps and gaps permitted.
.LI
The vln files of the cell(s) to be tested.
.LE
.SP
As its output \fIdubcheck\fP generates error messages on
the terminal,
stating the rule that was violated and the place
where the error occurred.
.SP
The program may be divided into two mayor parts:
.BL
.LI
One part consisting of the building and updating of the stateruler.
.LI
A second part consisting of the analysis of the stateruler
and the generation of the error messages from it.
.LE
.SP
These two parts will be discussed in the chapters 'Making
and updating the stateruler'
and 'The analysis of the stateruler' respectively.
.H 2 "Making and updating the stateruler"
In this chapter the contents of the stateruler fields
and the way they are formed and updated will be described.
.SP 0
In \fIdubcheck\fP the fields of the stateruler hold the
following variables:
.BL
.LI
xstart[0]: The x_position of the previous edge of mask1 in the field.
.LI
xstart[1]: The x_position of the previous edge of mask2 in the field.
.LI
yb: The bottom of the field.
.LI
yt: The top of the field.
.LI
lay_status[0]: The status of mask1. This may be:
.DL
.LI
NOT_PRESENT: This means that the layer is not present at the
stateruler position.
.LI
CHG_TO_PRESENT: This means that the layer starts
at the stateruler position.
.LI
CHG_TO_NOTPRESENT: This means that the layer stops
at the stateruler position.
.LI
PRESENT: This means that the layer is present
at the stateruler position.
.LE
.SP
.LI
lay_status[1]: The status of mask2.
.LI
helplay_status: The status of the helplay.
.LI
group[0]: The group of mask1 in the stateruler field.
.LI
group[1]: The group of mask2 in the stateruler field.
.LI
chk_type[0]: The checktype of mask1 in the stateruler field.
.LI
chk_type[1]: The checktype of mask2 in the stateruler field.
.LI
next: A pointer to the next stateruler field.
.LI
prev: A pointer to the previous stateruler field.
.LE
.SP
Upon initiation the stateruler consists of one field,
reaching from -MAXINT to MAXINT,
with lay_status NOT_PRESENT ,
xstart = -MAXINT,
next and prev pointing to the field itself
and the other variables set to zero.
.SP 0
After the initiation a loop is started reading
edges from the vln files (procedure get_vln),
selecting the one with the smallest x_value and the
smallest value of y_bottom and inserting them
into the stateruler (procedure insert_edge).
The loop is continued until all edges with the same x_value have
been inserted.
The stateruler for that x_value then is completed,
and an analysis of the stateruler then will take
place (extr_*** procedures).
The stateruler is updated (procedure update_sr) and
new vln files are read and inserted to form the stateruler
for the next x_position.
This process is repeated until all edges have been read from
the vln file.
.SP 0
The process described above is carried out in the
procedure main_check.
.SP 0
As stated above the insertion of new edges in the stateruler
is done in the procedure insert_edge.
This procedure is similar to the one used in the program
\fIdimcheck\fP, with only a difference in the variables that
are present in the stateruler fields.
.SP
In the procedure 'update_sr' the stateruler is updated
after being analyzed.
This means :
.BL
.LI
The lay_status is updated:
CHG_TO_PRESENT becomes PRESENT and CHG_TO_NOTPRESENT
becomes NOT_PRESENT in lay_status[0] , lay_status[1]
and helplay_status.
.LI
The group_nbr, check_type and xstart are updated for both masks.
.LI
If possible stateruler fields are merged.i.e:
.SP 0
If two adjacent fields have:
.DL
.LI
the same checktype for both masks
.LI
the same group_nbr for both masks
.LI
the same lay_status for both masks
.LI
the same xstart or for both fields holds
stateruler position - xstart >= MAXINFLUENCE
for both masks
.LE
.SP 0
the two fields are merged.
.LE
.H 2 "The analysis of the stateruler"
The analysis of the stateruler to detect possible
design rule errors is done in the
procedures 'extr_profile' , 'extr_overlap' , 'extr_overlap1'
, 'extr_overlap2' and 'extr_overlap3'.
The first procedure is used to detect gap errors,
the last ones to detect overlap errors.
.H 3 "detection of gap errors"
In the procedure extr_profile all fields of the stateruler are checked for
possible gap errors.
According to the lay_status of the masks the following checks are
carried out:
.BL
.LI
lay_status[0] = CHG_TO_PRESENT.
.SP 0
This means that an area in mask1 is starting.
In this case the following checks are done:
.DL
.LI
A check is carried out to see if mask1 and mask2 do have an
overlap here. In this case the status of mask2 is PRESENT or CHG_TO_PRESENT.
No error exists then and a structure is set up,
to indicate that the group of the item in mask1 and the item in mask2
have an overlap. If other errors occur between these equivalent groups
of mask1 and mask2 they will be suppressed is this is wanted.
.LI
If the masks have no overlap the distance to the last recorded edge
of mask2 in the field is checked.
.LI
If lay_status[0] of the previous or next field in the
stateruler is NOT_PRESENT checks are carried out to see if no
error exists in the areas left under respectively left above the edge. 
.LE
.SP
.LI
lay_status[1] = CHG_TO_PRESENT.
.SP 0
This means that an area in mask2 is starting.
In this case the same checks are carried out
with respect to mask1, as in the previous case with respect to
mask2.
.LI
lay_status[0] = CHG_TO_NOTPRESENT
.SP 0
This means that an area in mask1 has stopped.
In this case the following checks are done:
.DL
.LI
A check is carried out to see if mask1 and mask2 do have an
overlap. An equivalence of groups then is set up again.
.LI
If no overlap occurs a check is carried out to see if no error
occurs at the bottom of the field and a check is carried out to
see if no error occurs at the top of the field.
.LE
.SP
.LI
lay_status[1] = CHG_TO_NOTPRESENT.
.SP 0
This means that an area in mask2 has stopped.
In this case the same checks are carried out
with respect to mask1, as in the previous case with respect to
mask2.
.LE
.SP
In the check routines no errors will be reported between
two edges if they have the same check_type,and this checktype
does not equal zero, indicating that the edges stem from
the same instance of a subcell. If the errors exist,they
will be reported when the subcell is checked.
If the variable kind is made zero, also no errors between
areas of mask1 and mask2 that have an overlap will be
reported. Else these errors will be reported.
.SP 0
The errors found in this case are not immediately shown,
but temporarily stored first.
In this way one can suppress purely geometric errors,
which turn out to be unimportant when connectivity is taken into account
(this is an important topic in hierarchical design,
because the design rule checker output often gets
clothered with unimportant 'faults'
obstructing the really important messages).
.SP
.H 3 "detection of overlap errors of kind 0"
In the procedure extr_overlap checks are carried out to
see if all areas of the first layer are fully overlapped
by a distance overlap by the areas of layer 2.
According to the lay_status of the masks in the stateruler fields
the following checks are carried out:
.BL
.LI
lay_status[0] = CHG_TO_PRESENT.
In this case the next checks are carried out:
.DL
.LI
A check to see if lay_status[1] is PRESENT.
If not an error is recorded.
.LI
If PRESENT a check to see if the stop of the last area in mask2
in the stateruler field is at least a distance of
overlap smaller then the position of the stateruler.
.LI
Checks to see if the areas left under the bottom
of the edge and left upper of the top of the edge
are covered by mask2.
.LE
.SP
.LI
lay_status[1] = CHG_TO_NOTPRESENT.
In this case the next checks are carried out:
.DL
.LI
A check to see if lay_status[1] is NOT_PRESENT.
If not an error is recorded.
.LI
A check to see if no area of mask1 is present over a distance
of overlap before the stop of mask2.
.LI
If not a check to see if mask1 is not present over a distance
of overlap under or above the edge.
.LI
A check to see if mask1 is not present left under the top of the edge
or left above the bottom of the edge.
.LE
.LE
.P
Errors are reported immediately in this case.
The check procedures are such that no errors are generated
if the area of mask1 in the stateruler field has a
checktype not equal zero, indicating it originates from a subcell.
In this case the error will already be detected when the subcell is checked.
.P
.H 3 "detection of overlap errors of kind 1"
In the procedure extr_overlap1 checks are carried out to
see if all areas of the first layer are overlapped
by a distance overlap by the areas of layer 2 in the x_ or y_direction.
According to the lay_status of the masks in the stateruler fields
the following checks are carried out:
.BL
.LI
lay_status[0] = CHG_TO_PRESENT.
In this case the next checks are carried out:
.DL
.LI
if lay_status[1] = PRESENT, a check is carried out to see if the overlapping
area started at least a distance overlap earlier.
.LI
if lay_status[1] = CHG_TO_PRESENT, checks are carried out to see if
the overlaps over the starting area of layer1 to the top and bottom
are great enough.
.LI
if lay_status[1] = NOT_PRESENT or CHG_TO_NOTPRESENT an error is
generated.
.LE
.SP
.LI
lay_status[1] = CHG_TO_NOTPRESENT.
In this case the next checks are carried out:
.DL
.LI
if lay_status[0] = NOT_PRESENT, a check is carried out if the stop edge
of the area to be overlapped has occurred at least a distance overlap before.
.LI
if in the previous stateruler field lay_status[1] = PRESENT and lay_status[0] =
NOT_PRESENT, a check is carried out to see if over a distance of
at least overlap under the stateruler field no area in mask1 is present.
.LI
if in the next stateruler field lay_status[1] = PRESENT and lay_status[0] =
NOT_PRESENT, a check is carried out to see if over a distance of
at least overlap over the stateruler field no area in mask1 is present.
.LE
.LE
.P
Errors are reported immediately in this case.
The check procedures are such that no errors are generated
if the area of mask1 in the stateruler field has a
checktype not equal zero, indicating it originates from a subcell.
In this case the error will already be detected when the subcell is checked.
.P
.H 3 "detection of overlap errors of kind 2"
In the procedure extr_overlap2 checks are carried out to
see if all areas of the first layer are overlapped
by a distance overlap by the areas of layer 2
at places where the helplayer is not present.
According to the lay_status of the masks in the stateruler fields
the following checks are carried out:
.BL
.LI
lay_status[0] = CHG_TO_PRESENT.
In this case the next checks are carried out:
.DL
.LI
if helplay_status != PRESENT a check is carried out to see
if the overlapping started at least a distance overlap earlier.
.LI
if in the previous stateruler field lay_status[0] = NOT_PRESENT
and the helplay_status is NOT_PRESENT of CHG_TO_NOTPRESENT here,
a check is carried out to see if the overlap to the bottom
is large enough.
.LI
if in the next stateruler field lay_status[0] = NOT_PRESENT
and the helplay_status is NOT_PRESENT of CHG_TO_NOTPRESENT here,
a check is carried out to see if the overlap to the top
is large enough.
.LE
.P
.LI
lay_status[1] = CHG_TO_NOTPRESENT
and helplay_status != PRESENT.
In this case the next checks are carried out:
.DL
.LI
A check to see if the mask to overlap does not exist
at least for a distance overlap before the x_position of
the overlapping edge.
.LI
If in the previous stateruler field lay_status[1] = PRESENT
a check is carried out to see if the overlap of layer[1] to the
bottom of layer[0] is large enough.
.LI
If in the next stateruler field lay_status[1] = PRESENT
a check is carried out to see if the overlap of layer[1] to the
top of layer[0] is large enough.
.LE
.LE
.P
Errors are reported immediately in this case.
The check procedures are such that no errors are generated
if the area of mask1 in the stateruler field has a
checktype not equal zero, indicating it originates from a subcell.
In this case the error will already be detected when the subcell is checked.
.P
.H 3 "detection of overlap errors of kind 3"
In the procedure extr_overlap3 overlap checks are carried out
in accordance to the
direction given in the con_dir array.
The latter is initialized if an overlap check with kind = 4
or kind = 5 is given.
According to the lay_status of the masks in the stateruler fields
the following checks are carried out:
.BL
.LI
lay_status[0] = CHG_TO_PRESENT.
In this case the next checks are carried out:
.DL
.LI
If the direction is (BOTTOM + TOP) a test is carried out
to see if the overlap to the left of the area to overlap
is large enough.
.LI
If the direction is (LEFT + RIGHT) tests is carried out to see
if the overlaps to the bottom and the top are large enough.
.LE
.P
.LI
lay_status[1] = CHG_TO_NOTPRESENT.
In this case the next checks are carried out:
.DL
.LI
If the direction is (BOTTOM + TOP) a test is carried out
to see if the overlap to the right of the area to overlap
is large enough.
.LI
If the direction is (LEFT + RIGHT) tests is carried out to see
if the overlaps to the bottom and the top are large enough.
.LE
.LE
.P
Errors are reported immediately in this case.
The check procedures are such that no errors are generated
if the area of mask1 in the stateruler field has a
checktype not equal zero, indicating it originates from a subcell.
In this case the error will already be detected when the subcell is checked.
.P
.H 3 "setting of the con_dir array"
The setting of the con_dir array needed for overlap
checks with kind = 3 is done in the procedures
det_conn_hor and det_con_ver.
.SP 0
Det_con_hor adds to the appropriate entry in the array conn_arr
the value LEFT if,
under presence of the same polygon of the helplay,
an item of the second given file is present to the left of
the item of the first given file under consideration.
It adds the value RIGHT if under the same conditions an item of
the second file is to the right of the item of the first file.
.SP 0
Det_con_ver adds to the appropriate entry in the array conn_arr
the value BOTTOM if,
under presence of the same polygon of the helplay,
an item of the second given file is present to the bottom of
the item of the first given file under consideration.
It adds the value TOP if under the same conditions an item of
the second file is to the top of the item of the first file.
.SP 0
The procedures det_con_hor and det_con_ver are performed
if a check with resp. kind = 4 and kind = 5 is
present in the file \fIdubcheckdata\fP.
To perform an overlap check of kind = 3,
the checks with kind = 4 and kind = 5 must be performed first.
