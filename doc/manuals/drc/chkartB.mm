.nr Hy 0
.SA 1
.nr Ej 1
.nr Fg 0
.H 1 "APPENDIX B: The program nbool"
.H 2 "Introduction"
The program \fInbool\fP is the program that performs the logical operations
between the masks of a cell. 
.SP 0
The program must be called as:
.fS I

nbool [-c|-n] [-f] [cell_name]

.fE
If \fInbool\fP is called with the option \fB-c\fP the program will
check the input for hierarchical errors;
if option \fB-n\fP is given it will not.
If \fInbool\fP has to operate on input files that
themselves are boolean combination files, the last option has to
be chosen, otherwise false error messages will occur, because
\fInbool\fP then does not know what terminal masks are involved with
the boolean masks.
Default hierarchical checks are carried out.
.SP 0
If \fInbool\fP is called with the option \fB-f\fP the 'current working directory'
will be searched for the presence of a file \fIbooldata\fP.
If found this file will be taken as the technology_file for \fInbool\fP
instead of the standard one for the technology one is
working in.
.SP 0
If a cell_name is given this given cell will be tested.
If no cell_name is specified \fInbool\fP looks for the file \fIexp_dat\fP
in which the cell(s) to test then must be given.
.SP 0
So as its input the program needs:
.BL
.LI
A file exp_dat containing the cell(s) the program
has to be applied to, or a cell_name specified as argument.
.LI
A file booldata containing the logical formulas the
program has to perform upon its input files.
This file is either taken from the library or from
the 'current working directory'.
.LI
The vln files (edge files) of the cell(s)
involved in the logical combinations.
.LE
.SP
As its output the program generates the vln files of
the combination masks of the formulas.
Furthermore the program generates error messages when
the rules about hierarchy are violated.
.SP
The main parts of the program are:
.BL
.LI
The part that decodes the logical formulas given in the file booldata
and makes a structure to check if a certain mask combination
belongs to the formula given.
This part will be described in the part about decoding of the
design rules.
.LI
The part that builds up and updates the stateruler.
This will be described in the part about the stateruler.
.LI
The part that analyses the stateruler for hierarchical errors.
This will be described in the part about hierarchy check.
.LI
The part that analyses the stateruler and determines from that
what edges have to be output.
This will be described in the part about extract_profile.
.LI
The part that adds the group_numbers to the vln files made.
This will be described in the part about add_groupnumbers.
.LE
.H 2 "Decoding the design rules"
Most of the design rules involve more than one mask.
To check these rules masks must be made containing logical
combinations of the masks needed for that particular check.
The formulas of the masks that must be made for all the checks
of a certain technology must be given on the file booldata,
which is described in appendix A.
This section will describe the way these formulas are
stored in memory to allow for an efficient way to produce
all output masks wanted in one pass of the algorithm.
In the program this is done in the routine mk_formstruct.
.SP 0
This routine starts by reading the first line of the file booldata
which names all masks involved in the formulas to be made.
.SP 0
Then the routine ini_heap is entered which makes an input structure.
For each file listed in the input line,
and if the hierarchy must be checked also for each terminal file,
a structure is set up containing:
.BL
.LI
The name of the mask involved.
.LI
The binary number of the mask.
If the name of the mask is known to the process, the corresponding
mask number is taken from it. If not it gets a number twice as high
as the previous unknown mask, starting at the mask_number twice as
high as the highest mask_number known in the process.
.LI
The mask_type of the mask.
If the mask is known to the process the mask_type is copied from it.
So terminal masks become 1, connection masks become 2 and the
others become 0.
For masks unknown to the process the mask_type is set to BOLEAN (=3).
.LI
The pointer to the vln file
.LI
The data of the first edge in the vln file.
So x_position,y_bottom, y_top, edge_type and check_type.
.LE
.SP
The latter data will be updated with a new line segment when the
program has inserted the line segment in the stateruler.
.SP
After ini_heap has made this structure the procedure mk_formstruct
starts reading the formulas, line by line.
For each line it sets up a c_structure 'form' which contains:
.BL
.LI
The name of the file,where the edges of the mask to form must be
written. This name is bool_xx, where xx is the form_number.
.LI
A number of buffers to temporarily store the edge data before
it is written to the output file.
.LI
A number curr_place indicating which buffer has been filled last.
Initially this variable is set to -1 to indicate that all buffers
are free.
.LI
A pointer to a list of min_term structures, which will be explained
later on
.LI
A vulnerability mask containing all masks present in the formula.
This variable is not strictly needed, but added for efficiency
reasons.
.LI
A pointer to the next form_struct, or if there is not any a NULL_pointer.
.LE
.SP
The formula read now is decoded to fill this structure with its information.
File names are detected from the file and
also the special characters ! (negotiation)
| (logical or) and & (logical and).
Two masks are kept for each term of the formula:
.BL
.LI
The masks that must be present for a mask combination to be
part of the term of that formula.
.LI
The masks that must NOT be present for a mask combination to be
part of the term of that formula.
.LE
.SP
These variables are updated in the procedure 'update_masks'
each time a '&' or '|' character is discovered.
If a term of the formula is finished (a | character discovered or
end of formula) these variables mask and not_mask are placed in
a structure min_term and this structure is added to the list
of min_term structures of the formula.
This is done in the procedure 'add_minterm'.
The variable vuln_mask of the form_structure there is updated too.
Upon leaving the procedure 'mk_formstruct'
we thus have created a structure like
the one shown in figure \n(H1.1
.DS
.PS < ../drc/fig4.pic
.PE
.FG "the formula structure" \n(H1.
.DE
.H 2 "The stateruler"
In this chapter the contents of the stateruler and the way it
is formed and updated will be described.
In this program the stateruler consists of fields
with the following variables:
.BL
.LI
yb: the bottom of the field.
.LI
yt: the top of the field.
.LI
chk_type: the check_type of the layers in the field.
If the layers have different check_types this value is set
to DIFF_CT (= -2).
.LI
p_check: A pointer to a structure in which the check_types are
stored per layer. If all layers have the same check_type there
is no need for such a structure and p_check is a NULL_pointer.
.LI
p_chg_ct: A pointer to a list of structures which contain
the old check_type and the mask a change of check_type occurred in.
If no change of check_type occurred this is a NULL_pointer.
.LI
mask_past: This variable bitwise contains the layers present in
the field before the edges at the present x_value are installed.
.LI
mask_fut:This variable bitwise contains the layers that are
present after the insertion of the edges at the present x_value.
.LI
ov_mask: This variable contains bitwise the layers in which an
overflow of layers of different check_type has occurred.
.LI
next: A pointer to the next stateruler field.
.LI
prev: A pointer to the previous stateruler field.
.LE
.SP
The stateruler is initialized to contain one field,
from yb = -MAXINT to yt = MAXINT, with no masks present,so
mask_past = mask_fut = ov_mask = 0 and chk_type set to
INITIAL (=-1). The pointer to the check_type structure is
set to NULL.
Through the procedure 'select_edge' the edges that have to be inserted
then are selected from the edge_heap in such a way,
that the edges with the lowest
x_coordinate come first and for edges with the same x_coordinate the
one with the lowest bottom value comes first.
.SP 0
The procedure 'insert_edge' then inserts the edge in the stateruler.
In this procedure the stateruler is scanned from the current field
until the new edge and a field in the stateruler have an overlap.
If this occurs, and the values of the bottom of the stateruler field
and the bottom of the new edge do not coincide,
the procedure 'split_field' is called,
which splits the field in two parts,
the split point being the bottom value of the new edge.
The bottom and
top values of the two created fields are updated and the other
values of the old field are copied into the new field.
The current stateruler pointer is set to the top field of the two fields
being created/updated.
As long as the top value of the next fields in the stateruler is not
greater then the top value of the new edge, the fields in the stateruler
are updated with information from the new edge.
This is done in the procedure 'update_fld'.
In this procedure the values of mask_fut, ov_mask and chk_type are
updated, according to the values of the edge.
If the top value of the stateruler field becomes smaller then the
top value of the new edge a split is carried out
with the procedure 'split_field' and the bottom
field of the two newly created/updated
fields is updated with the procedure 'update_fld'.
.SP 0
This process of selecting and inserting fields is continued until
a new x_value is found.
Then the stateruler (if this option is chosen) is checked for
hierarchy errors and after that analyzed to extract the edges for the
boolean files to be made.
Then the stateruler is updated.
This is done in the routine 'update_sr'.
In this routine first the value of the mask_past in the fields are set
to mask_fut, and
the check_types of the stateruler fields are updated.
After that fields containing the same values for the masks and check_types are
joined.
.SP 0
After being updated a new stateruler is built for the next
x_value until all edges have been read.
A schema of the operations is given in figure \n(H1.2
.DS
.PS < ../drc/fig5.pic
.PE
.FG "Stateruler main flow" \n(H1.
.DE
.H 2 "The hierarchy check"
The checking of the hierarchy rules is carried out in
the procedure 'check_hierarchy'.
This procedure contains a loop for checking all of the
fields in the stateruler for:
.BL
.LI
The ov_mask.
.SP 0
If a bit in this mask is set,
indicating that an overlap in the corresponding mask
has occurred,
the following is done:
.DL
.LI
If the mask in which the overlap occurred is a
connection mask,
a check is carried out to see if the
corresponding terminal mask is present.
If not an error massage is generated,
telling where the error occurred and in which mask.
.LI
If the layer is not a connection mask a warning 
massage is generated,
telling where the overlap took place and in which mask.
.LE
.SP
.LI
The check_types.
.SP 0
If the variable chk_type in the stateruler field is DIFF_CT,
indicating that in the field layers with different check_types
are present,
the following actions are taken:
.SP 0
A check is made to see if the difference is caused by
a check_type 0 in a connection mask with the presence of
a terminal in the same layer, indicating an overlap permitted.
In this case no messages are generated.
If the difference is not caused by the situation described
above,
a warning massage is generated telling the place where
different checktypes occur,
and the checktypes of the layers present.
.LI
Change of checktype.
.SP 0
If a change of checktype in the y_direction occurs the following
steps are taken:
.BL
.LI
If the change takes place in a connection layer a check
is carried out if a terminal is present there.
If not, an ERROR message is generated,
stating where the error occurred and in which layer.
.LI
If the change takes place in another layer a WARNING is
generated, stating the place of the change of checktype
and the layer it occurred in.
.LE
.LE
.H 2 "Analysis of the stateruler"
The analysis of the stateruler,
is carried out in the procedure 'extr_profile'.
It finds the edges that have to be output in the
vln file of the corresponding formulas.
In this procedure a loop is set up,
which examines each stateruler field.
If the values of mask_past and mask_fut differ,
indicating that one or more layers have changed state,
the procedure 'buff_edge',
which does the actual work is called.
The main flow of this procedure is given in figure \n(H1.3
.SP 0
The way the program checks if a certain mask combination belongs
to a formula is done using the structure made with mk_formstruct.
The mask combination is compared to the masks and not_masks of the
min_terms of the formula.
If all the masks present in the variable mask of the min_term
structure are also present in the mask combination to check,
and the masks set in the variable not_mask do not appear in the
mask combination to check,
the mask combination belongs to that min_term,
and hence to the formula.
.SP 0
According to the presence of mask_past and mask_fut in the formula
the pres_flag is set.
If mask_past belongs to the formula and mask_fut does not,
pres_flag is set to 1,
indicating a stop edge.
If mask_past does not belong to the formula and mask_fut does,
pres_flag is set to -1,
indicating a start edge.
If mask_past and mask_fut both belong to the formula,
or if they both do not belong to the formula,
pres_flag is set to 0,
indicating that no edge has to be output.
.DS
.PS < ../drc/fig6.pic
.PE
.FG "buff_edge main flow" \n(H1.
.DE
After the value of the pres_flag is established,
and the pres_flag = 0,
no further actions are taken.
If this value not equals zero,
two cases may occur:
.BL
.LI
The bottom value of the newly found edge and the top value
of the last buffered edge of the formula are the same and
so are their x_positions.
.SP 0
In this case the last buffered edge is updated,
i.e. its top value and its connection type are updated.
This is done in the procedure 'update_edge'.
.LI
If the values mentioned above do not coincide,
the edge is added to the next place in the buffer.
If all buffers of the formula have  been filled,
the buffer is appended to the file,
whose name is given in the f_name variable in the form_structure.
These actions are carried out in the procedure 'add_edge'.
.LE
.H 2 "The generation of the group_numbers(connectivity)"
The group_numbers of the edges indicate to which connected region
they belong. They are generated after \fInbool\fP has generated the edges.
This is done on temporary files,
which for efficiency reasons are in binary format.
They have a boolean name,
with bt1 added to it.
For example bool_2bt1.
.SP 0
Now the files generated are read one by one
and a pointer structure is set up in the same way as it
is done in i.e. the program \fImakevln\fP.
The pointers are added to the file and written on a
file with the addition of bt2 (e.g.bool_2bt2),
and the bt1_file is removed from the system.
The bt2 file then is read and the pointers are replaced
by their corresponding group_numbers.
Then the edges are written to the boolean file that
remains in existence and is used by the programs \fIdimcheck\fP and \fIdubcheck\fP.
These files (e.g bool_2) are in the known vln_format.
The bt2_files are also removed from the system.
