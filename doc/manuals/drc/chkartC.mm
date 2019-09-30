.nr Hy 0
.SA 1
.nr Ej 1
.nr Fg 0
.H 1 "APPENDIX C: The program dimcheck"
.H 2 "Introduction"
The program \fIdimcheck\fP checks a cell for the presence of
width or gap errors in a single (combination) layer.
.SP 0
The program is called as:
.fS I

dimcheck [-a|d][-d][-f][-t][-g][cell_name]

.fE
The meaning of the options is:
.VL 10
.LI -a
The program is used as a part of the single_layer
checker \fIautocheck\fP(see appendix E), and the file \fIdimcheckdata1\fP
is taken as design_rule input_file.
.LI -d
The program is used as a part of the multi_layer
checker \fIdimcheck\fP(see appendix E), and the file \fIdimcheckdata2\fP
is taken as design_rule input_file.
.LI -f
The program looks for the
file \fIdimcheckdata1\fP (or \fIdimcheckdata2\fP) in the
current working directory instead of
taking the standard one for the technology used.
.LI -t
This option has been added for debugging purposes.
It generates a lot of test_data.
.LI -g
With this option gap_errors within the
same polygon, which otherwise may be suppressed,
are always reported.
.LI cell_name
The name of the cell to be tested.
If not specified the program looks for a file \fIexp_dat\fP
in which the cell(s) to be tested must be given.
.LE
.SP
So as its input the program needs:
.BL
.LI
A file \fIexp_dat\fP containing the cell(s) \fIdimcheck\fP
has to be applied to, or a cell_name as argument
in the call of the program.
.LI
A file \fIdimcheckdata1\fP (or \fIdimcheckdata2\fP)
containing the layers to check and the
gaps and widths permitted.
.LI
The vln files of the cell(s) to be tested.
.LE
.SP
As its output \fIdimcheck\fP generates error messages on
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
These two parts will be discussed in the sections 'Making
and updating the stateruler'
and 'The analysis of the stateruler' respectively.
.H 2 "Making and updating the stateruler"
In this sections the contents of the stateruler fields
and the way they are formed and updated will be described.
.SP 0
In \fIdimcheck\fP the fields of the stateruler hold the
following variables:
.BL
.LI
xstart: The x_position in which the the field was started.
.LI
yb: The bottom of the field.
.LI
yt: The top of the field.
.LI
lay_status: The status of the layer. This may be:
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
helplay_status: The status of the helplayer if used.
.LI
group: The group in the layer the field belongs to.
.LI
group_old: The group of the edge before the last one.
.LI
chk_type: The checktype of the layer in the field.
.LI
chk_type_old: The checktype of the edge before the last one.
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
After the initiation a loop is started in which edges are read
from the vln file(s) and inserted
into the stateruler (procedure insert_edge).
The loop is continued until all edges with the same x_value have
been inserted.
The stateruler for that x_value then is completed,
and an analysis of the stateruler then will take place (procedure
extr_profile).
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
In this procedure the fields of the stateruler are scanned from
the current position to the topmost position to see if an
overlap with the edge to insert is present.
If this is the case and the bottom values of the field and the new
edge do not coincide,
the stateruler field is split into two and the variables are copied from the
old field.
As long as the top value of the edge is greater then the top value
of the stateruler fields, the latter are updated.
If the top value of the new edge becomes smaller as the top value of
the stateruler field, again a split is carried out and the bottom field of the
two newly created fields is updated.
The splitting of the fields is carried out in the procedure 'split_fld',
the updation of the fields is done in the procedure 'update_fld'.
.SP 0
An example is shown in figure \n(H1.1.
.DF
.PS < ../drc/fig7.pic
.PE
.FG "Building the stateruler" \n(H1.

.DE
.SP
In the procedure 'update_sr' the stateruler is updated
after being analyzed.
This means :
.DL
.LI
The lay_status is updated:
CHG_TO_PRESENT becomes PRESENT and CHG_TO_NOTPRESENT
becomes NOT_PRESENT.
.LI
The group_nbr, check_type and xstart are updated
.LI
If possible stateruler fields are merged.i.e:
.SP 0
If two adjacent fields have:
.DL
.LI
the same checktype
.LI
the same group_nbr
.LI
the same lay_status
.LI
the same xstart or for both fields holds stateruler
position - xstart >= MAXINFLUENCE
.LE
.P
the two fields are merged.
.LE
.H 2 "The analysis of the stateruler"
The analysis of the stateruler to detect possible
design rule errors is done in the procedure 'extr_profile'.
In this procedure all fields of the stateruler are checked for
possible design rule errors.
According to the lay_status the following checks are
carried out:
.BL
.LI
lay_status = PRESENT.
.SP 0
This means that no change of lay_status has taken place,
so nothing needs to be checked.
.LI
lay_status = CHG_TO_PRESENT.
.SP 0
This means that a new area has started.
In this case the following checks are carried out:
.DL
.LI
A check to see if
the distance between the previous edge and the new edge
is great enough (procedure 'check_xgap').
.LI
If in the previous stateruler field the layer is not present
a check to see if previous edges are not too close to the
bottom of the new edge (procedure 'check_g_circle').
.LI
If in the next stateruler field the layer is not present
a check to see if previous edges are not too close to the
top of the new edge (procedure 'check_g_circle').
.LI
If in the previous stateruler field the lay_status is not CHG_TO_PRESENT
and in the next stateruler field the lay_status is not PRESENT (in
which cases an error, if any, already has been reported),
a check of the y_width of the edge starting in the stateruler field is
carried out (procedure 'check_ywidth').
.LE
.SP 0
.LI
lay_status = CHG_TO_NOTPRESENT
.SP 0
This means that an area has stopped.
In this case the following checks are done:
.DL
.LI
A check to see if the area that stopped was not too small
in the x_direction (procedure 'check_xwidth').
.LI
If the previous stateruler field the lay_status is not CHG_TO_NOTPRESENT ( in
which case an error, if any, already has been reported)
and in the previous stateruler field the lay_status is PRESENT
a check is carried out to see if the area that
is left under the stop is not too small
in the y_direction (procedure 'check_ywidth').
.LI
Under these conditions also a check is done to see if the layer
is present in a circular area around the bottom of the stateruler field (
procedure 'check_w_circle').
.LI
If the layer is not present in the previous  stateruler field a gap check is 
done to see if the gap between the stopped area and the first area below it
is not too small (procedure 'check_ygap').
.LI
If the next stateruler field the lay_status is not CHG_TO_NOTPRESENT ( in
which case an error, if any, already has been reported)
and in the next stateruler field the lay_status is PRESENT
a check is carried out to see if the area that
is left above the stop is not too small
in the y_direction (procedure 'check_ywidth').
.LI
Under these conditions also a check is done to see if the layer
is present in a circular area around the top of the stateruler field (
procedure 'check_w_circle').
.LI
If the layer is not present in the next stateruler field a gap check is 
done to see if the gap between the stopped area and the first area over it
is not too small (procedure 'check_ygap').
.LE
.LI
lay_status = NOT_PRESENT.
.SP 0
This means that no change of lay_status has taken place,
so nothing needs to be checked.
.LE
.SP
The width_checks mentioned will only be
carried out if according to the file \fIdimcheckdata1(2)\fP the
width_flag is set.
The gap_checks are only carried out if the gap_flag is set
and if the helplay_status is NOT_PRESENT, if a helplayer
is specified.
.SP 0
In the check routines check_xwidth etc. gap and width errors
will not be generated if the edges have the same
checktype (except if it is zero).
This situation means that the error originates from a subcell and has
already been reported there.
