.nr Hy 0
.SA 1
.nr Ej 1
.nr Fg 0
.H 1 "Design Rule Checking"
Design rule checking is needed to see if all rules
concerning the dimensions of primitives of a designed integrated circuit
obey the rules imposed upon it by the technology the
integrated circuit has to be made in.
The checks may involve gap or width checks on a
combination of masks present or a gap or overlap check between
two combinations of masks present.
Every technology has its own set of design rules that has to be obeyed.
To make the design rule checker as much independent of a special
process as possible,
it is based upon combinations of masks one may specify for a
certain technology.
.br
Generally we may split the problem of design rule checking
into two parts:
.BL
.LI
A part that for a certain rule selects the items involved.
.LI
A part that does the actual checking upon these items.
.LE
.P
For a hierarchical design rule checker the first again
may be split in two parts:
.BL
.LI
A part that picks from the total amount of data of the
artwork of the integrated circuit the minimum part that is needed for
the checking of a certain part of the cell.
This part has been described in the previous sections.
.LI
A part that selects the data needed
for a check of a certain design rule
from this data gathered.
.LE
.P
This second part is carried out by the program \fInbool\fP,
which performs all and, or and negate operations needed
to select mask combinations needed for the checker.
It does so in one pass, with a linear complexity.
.P
As stated the checks to be performed are width, gap  and overlap checks.
The width check always concerns one (combination)mask and gap checks
may concern one or two (combination)masks.
Overlap checks always concerns two (combination)masks.
Therefore the checker itself is also divided into two parts:
.AL
.LI
A part that does width and gap checks concerning
one (combination)mask.
.LI
A part that does overlap checks and gap checks concerning
.br
two (combination)masks.
.LE
.P
So the design rule check is done in three steps:
.AL
.LI
First all the combination masks needed in the check are made
by the program \fInbool\fP.
The files that have to be made must be given in a file booldata,
the format of which is given in appendix A.
.LI
Then the checks concerning one (combination)mask are carried out
by the program \fIdimcheck\fP.
The checks that have to be carried out must be given in a
file \fIdimcheckdata2\fP, the format of which is also given in appendix A.
.LI
At last the checks concerning two (combination)masks are carried out
by the program \fIdubcheck\fP.
The checks that have to be carried out must be given in a
file \fIdubcheckdata\fP, the format of which is also given in appendix A.
.LE
.P
Two design rule checkers are in use at the moment:
.AL
.LI
A simple checker \fIautocheck\fP which only checks for width and
gap errors per mask; so errors stemming from
combinations of masks will not be looked for.
Autocheck is implemented as a shell_script,
which calls the program \fIdimcheck\fP with the
correct options.
.SP 0
How \fIautocheck\fP is called and what options are possible
is given in appendix E.
.LI
The check program \fIdimcheck\fP,
which preforms all the design rule checks, so checks in the
same mask as well as checks between (combinations of) masks.
.SP 0
Dimcheck is also implemented as a shell_script.
It first calls the program \fInbool\fP which determines the
vln_files of the combination_masks needed for the checks.
After that the programs \fIdimcheck\fP and \fIdubcheck\fP are called,
which use these files to perform the checks,
together with the cell_data of the cell(s) to test and
the technology files \fIdimcheckdata2\fP and \fIdubcheckdata\fP
Dimcheck thereby checks for width and gap errors in
the same (combination)mask and \fIdubcheck\fP checks for
gap and overlap errors between two different (combination)masks.
.SP 0
Schematically this is shown in the next figure:
.DS
.PS < ../drc/fig8.pic
.PE
.FG "The checker dimcheck" \n(H1.
.DE
How \fIdimcheck\fP is called and what options are possible
is given in appendix E.
.LE

A short description of programs mentioned will be given in the
next sub_sections.
.P
.H 2 "The program nbool"
Nbool is a program to generate logical combination masks,
which are a combination of input masks in
vertical line segment format (vln format).
.SP 0
As input it uses a file with the description of
the logical formulas of the masks to be made.
For a description of this file see appendix A.
Also the vln files mentioned in these formulas and a file
with the name of the cell(s) of which the files have to be made
is needed.
It uses the stateruler algorithm in a similar way as described
in section 3.
Globally the program works as follows:
.fS
read 'logical' file and set up an internal logical structure.
for each cell do {
    while not all segments inserted {
        read segments and select segment to insert.
        if ( x_segment != stateruler position) {
            check stateruler for hierarchical errors of
                  the cell.
            determine using the internal logical structure
                  and the layers present in the stateruler
                  fields what new segments have to be made
                  to what output_mask.
            update stateruler
        }
        insert new segment
    }
    check stateruler for hier. errors
    output segments from the last stateruler position

    add group_numbers to all segments made.
}
.fE
First the file which contains the logical combinations to make
is read and a structure is made to indicate what layers must
be present for a segment to belong to a certain logical formula,
and what layers must not be present.
This structure is used by the analysis of the stateruler.
.P
The stateruler fields in this case must contain as its state two vectors, 
one indicating the masks present in the past(i.e. left of the stateruler)
and one indicating the masks that will be present in the future(i.e. 
right of the stateruler).
Also the check types of the edges must be stored in it.
.P
In the stateruler process of making and analyzing stateruler profiles
events are inserted which are
read from the input vln files.
The selection criteria are the x_value of the segment
and its bottom y_value.
Whenever during the analysis of
a stateruler a change of layer combinations occurs,
which is indicated by the fact that that the past_vector is
different from the future_vector,
the past_ and future_vectors are compared with the
logical structure built to see if the field gives
rise to the generation of line segments in one or
more of the output masks.
.P
If in a field different checktypes occur this indicates
a possible violation of the hierarchical rules.
These errors are reported by \fInbool\fP in the following cases:
.BL
.LI
Overlap of interconnection layers without the presence of
a terminal in that layer.
.LI
Overlap of layers which do not interconnect.
.LI
Overlap of different layers belonging to different
cells indicating that one has possibly created an unwanted
element.
.LE
.P
For a more detailed description of the program \fInbool\fP,
see appendix B.
.H 2 "The program dimcheck"
The program \fIdimcheck\fP performs gap and width checks
on one mask.
.SP 0
As its input it uses a file containing the masks that have to be checked
and the widths and gaps they have to obey. Also a reduced gap
may be defined for gaplengths smaller then a certain value.
For a description of this file see appendix A.
Also the vln files mentioned in the file above and a file
containing the cell(s) to check must be present.
.SP 0
It uses the stateruler algorithm in a similar way as described
in section 3.
The global way the program works is:
.fS
for each cell do {
    for each vln_file do {
        while(not all segments read) {
            read segment from vln_file.
                if(x_segment != stateruler position) {
                    analyze the stateruler for presence of
                          width  and gap errors.
                    update stateruler
                }
                insert segment in the stateruler.
            }
        }
        analyze the last stateruler for width  or gap errors
    }
}
.fE
The events of the algorithm here are the segments read from
the vln file.
.SP 0
In the stateruler the following variables are recorded:
.BL
.LI
The x_position of the edge in the field previous to the one where
the stateruler is analyzed.
.LI
The status of the layer (PRESENT, NOT_PRESENT, CHG_TO_PRESENT or
CHG_TO_NOTPRESENT).
.LI
The group_number of the edge.
.LI
The group_number of the previous edge in the field.
.LI
The check_type of the edge, indicating from which cell the edge
is originating.
.LI
The check_type of the previous edge in the field.
.LI
The status of an help_layer
.LE
.SP
Depending on the status of the layer in a stateruler field
during analysis, width or gapchecks are performed in the x_ and
y_direction.
The group_numbers of the edges thereby can be used to suppress gap errors
that occur between edges of the same polygon.
The check_types are used to suppress errors that occur
between edges belonging to the same sub_cell,
as these already will be reported when this sub_cell is checked.
.SP 0
For a more detailed description of the program \fIdimcheck\fP,
see appendix C.
.H 2 "The program dubcheck"
The program \fIdubcheck\fP performs gapchecks between to different
masks and determines if a mask is overlapped by another mask
with a certain value.
As its input it uses a file containing the files that have
to be checked with respect to each other and the distance or
overlap they have to obey. Also an integer is given 
to indicate what kind of gap_check or overlap_check
has to be performed.
For a description of this file see appendix A.
Furthermore the vln files stated in the file mentioned
above must be present and a
file containing the cells to be checked.
.SP 0
It also uses the stateruler algorithm in a similar way as described
in section 3.
The global way the program works is:
.fS
for each cell do {
    for each line of check_file do {
        while (segment_files not empty) {
            read segment and select segment to insert
            if( x_segment != stateruler position)
                analyze the stateruler for the presence of
                      gap or overlap errors.
                update stateruler
            }
            insert segment in the stateruler
       }
       analyze last stateruler.
   }
}
.fE
.\".SP 0
The events of the algorithm here are the segments read from
the two vln files.
The selection criteria again are the x_value and the y_bottom
value of the segment.
.SP 0
In the stateruler the status (PRESENT, NOT_PRESENT, CHG_TO_PRESENT
or CHG_TO_NOTPRESENT) of both masks is recorded, together with the
groups and checktypes of the edges and
the presence of an helpmask.
.SP 0
The analysis of the stateruler is done in different
procedures: one for gap errors
and one for each kind of overlap.
.SP 0
At present the following gap and overlap checks
are implemented in \fIdubcheck\fP:
.BL
.LI
gap checks which only report errors for non_overlapping items.
.LI
gap checks which report errors for overlapping and
non_overlapping items
.LI
overlap checks for a full overlap
.LI
overlap checks for overlap over two opposite sides
.LI
overlap checks only in places where the helpmask is not present
.LI
overlap checks on sides set earlier by \fIdubcheck\fP
.LE
.P
.SP 0
For a more detailed description of the program \fIdubcheck\fP,
see appendix D.

