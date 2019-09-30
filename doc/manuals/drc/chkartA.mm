
.nr Hy 0
.SA 1
.nr Ej 1
.nr Fg 0
.H 1 "APPENDIX A: Implementation of Technology"
The checks that have to be performed on the artwork of
an integrated circuit vary from technology to technology.
This appendix deals with the way the design rules must be implemented in the
design rule checker.
.SP 0
In general we may distinguish between two items with respect to
the technology:
.AL
.LI
Data about the masks used in a certain technology,such
as mask_name, mask_type etc.
.LI
Data that are specific for the design rule checker,
such as minimum widths and gaps etc.
.LE
.P
The data mentioned under (1 are stored in a technology file for
use by all programs needing it.
In this paper we will not discuss this, but assume this file to
be present.
We will restrict ourselves to the data mentioned under (2.
.SP 0
This data is stored in four files for each technology present.
.AL
.LI
A file \fIbooldata\fP, used by the program \fInbool\fP, in which
the logical combinations of the combination masks
needed are described.
.LI
A file \fIdimcheckdata1\fP, used by the program \fIdimcheck\fP,
which specify the width_ and gap_checks that have to be
carried out if the single_layer checker \fIautocheck\fP
is used.
.LI
A file \fIdimcheckdata2\fP, used by the program \fIdimcheck\fP,
which specify the width_ and gap_checks
in one (combination)mask that have to be
carried out if the multy_layer checker \fIdimcheck\fP
is used.
.LI
A file \fIdubcheckdata\fP, used by the program \fIdubcheck\fP,
which specify the gap and overlap checks that have to be
carried out between two different (combination)layers.
.LE
.SP 0
For the format of these files see the next section.
.P
Our design rule checker can handle design rules of one of the following types:
.AL
.LI
Width checks.
.SP 0
To implement a width check on a (combination)mask,
the following steps must be taken:
.AL a
.LI
In the case of a combination mask,
the logical formula of that mask,
if not yet present,
must be included in the file \fIbooldata\fP.
For the format of this file see later on under the sub_section
on file formats.
.LI
The (combination)mask to be checked must be given
in the file \fIdimcheckdata1(2)\fP.
In this file the minimum width of the mask must be given too.
For the format of this input file see the sub_section on file
formats later on in this appendix.
.LE
.P
Examples of rules that can be tested this way are e.g. in the nmos process:
.SP 0
the width of the items of the diffusion mask,
the width of the active areas etc.
.LI
Gap checks between items in one (combination)mask.
.SP 0
To implement a gap check on a (combination)mask,
the following steps must be taken:
.AL a
.LI
In the case of a combination mask,
the logical formula of that mask,
if not yet present,
must be included in the file \fIbooldata\fP.
For the format of this file see later on under the sub_section
on file formats.
.LI
The (combination)mask to be checked must be given
in the file \fIdimcheckdata1(2)\fP.
In this file the minimum gap between areas of the mask must be given too.
The possibility also exists to specify in this input a reduced gap
in case the gaplength is smaller than a certain given value.
With an helpmask specified in \fIdimcheckdata2\fP one can
change the test so, that the gap is only tested at places
where the helpmask is not present.
One also can determine if one wants error_messages
from gap_errors within the same polygon or from polygons
with touching corners by specifying an integer kind in the
file \fIdimcheckdata1(2)\fP.
For the format of this file see the sub_section on file
formats later on in this appendix.
.LE
.P
Examples of rules that may be tested this way are in the nmos process e.g.:
the gap between unrelated diffusion areas,
the gap between unrelated metal areas etc.
.LI
Gap checks between two (combination)masks.
.SP 0
To implement a rule for the gapcheck between two masks,
the following steps must be taken:
.AL a
.LI
In the case that one or both masks are combination masks,
the logical formulas of these masks,
if not yet present,
must be added to the file booldata (for format see sub_section on
file formats).
.LI
The masks between which the check has to be carried out
must be added to the file \fIdubcheckdata\fP.
The minimum gap between unrelated areas of the masks must
be specified here too.
The possibility here also exist to specify a reduced
gap if the gaplength is smaller than a certain given value.
Furthermore with the integer kind one can
specify if gap errors between overlapping items
must be reported or not .
.LE
.P
Examples of rules that can be tested this way are e.g. in
the nmos process:
.SP 0
the gap between an undercrossing and unrelated diffusion 
and the gap between unrelated poly and diffusion.
.LI
Overlap checks of (combination)masks.
.SP 0
To implement a rule to test an overlap
of one combination(mask) over another one, the following steps must be taken:
.AL a
.LI
In the case that one or both masks are combination masks
the logical formulas for these masks,
if not yet present,
must be added to the file \fIbooldata\fP (for format see sub_section on
file formats).
.LI
The masks concerned must be given in the
file \fIdubcheckdata\fP.
Here also the value of the overlap must be given.
One also must specify what kind of overlap one wants to test,
by specifying an integer kind.
At present the possibilities are:
.BL
.LI
full overlap
.LI
overlap over two opposite sides
.LI
overlap only where a specified helpmask is not present
.LI
left_right and/or bottom_top overlap only if an internally
set array tells to do so.
This array is set by stating tests in \fIdubcheckdata\fP with
kind is 4 and kind is 5.
These tests also must be defined before this last kind of
overlap can be tested.
.LE
.LE
.P
Examples of rules that can be tested this way are e.g. in the
nmos technology:
.SP 0
overlap of metal over a connect_cut,
overlap of poly over an active area etc.
.LE
.P
.H 2 "file formats"
This section describes the file_formats of the files
used by the design_rule checker.
.AL
.LI
The file \fIbooldata\fP, read by the program \fInbool\fP,
contains the logical combination of masks to be made.
.P
Example:
.fS
  od_vln nw_vln sp_vln ps_vln con_vln cop_vln
  cps_vln cb_vln in_vln sn_vln               : filenames
  od_vln&!nw_vln                             :  0 OD.3.1
  od_vln&nw_vln                              :  1 OD.4.1.1
  od_vln&sp_vln&!nw_vln                      :  2 OD.3.2+SP/SN.3.3+4.3
  od_vln&ps_vln                              :  3 PS.3.1+PS.5.1
  sp_vln|sn_vln                              :  4 SP.3.1+SN.3.1
  od_vln&ps_vln&nw_vln                       :  5 SP.3.2+SP.4.2
  od_vln&!ps_vln                             :  6 OD.2.1
  od_vln&con_vln&!nw_vln|
  od_vln&cop_vln&nw_vln|od_vln&ps_vln        :  7 SP/SN.3.3+4.3
  od_vln&sn_vln&nw_vln                       :  8 SP/SN.3.3+4.3
  od_vln&ps_vln&!nw_vln                      :  9 SN.3.2+SN.4.2
  od_vln&con_vln                             : 12 CON.3.1+CON.3.2
  od_vln&con_vln&sn_vln&nw_vln               : 13 CON.3.3+CON.3.4
  od_vln&cop_vln                             : 14 COP.3.1+COP.3.2
.fE
.fS
  od_vln&sp_vln&cop_vln&!nw_vln              : 15 COP.3.3+COP.3.4
  od_vln&ps_vln&cps_vln                      : 16 CPS.4.1
  cps_vln&ps_vln                             : 17 CPS.4.2+CPS.4.3
  con_vln&!in_vln|cop_vln&!in_vln|cps_vln&!in_vln : 18 IN.3.1
  con_vln&in_vln|cop_vln&in_vln|cps_vln&in_vln : 19 IN.3.2
  cb_vln&in_vln                                : 20 CB.1.1
.fE
The first lines of the file until the first ':' contain the names of the
input files that are involved in the formulas to come.
After that each line of the file contains the logical
formula to make.
In this formula the logical AND operation is indicated by the
character '&', the logical OR operation by '|' and
the negation operation by the character '!'.
The precedence of the operators is !, &, |.
The end of the formula is indicated by the ':' on the line.
After this ':' a number is given indicating the name of
the file where the result has to be stored.
The name of the file becomes bool_nn,
where nn is the number just mentioned.
After this number a string is given indicating what
design rule is involved with the operation.
.LI
The files \fIdimcheckdata1\fP and \fIdimcheckdata2\fP,
read by the program \fIdimcheck\fP,
contain the names of the files to be checked and their
width and gap dimensions.
.P
Example:
.fS I
nw_vln   NOFILE  12 15  0 0 0  NW.1.1+NW.2.1
od_vln   NOFILE   6  6  0 0 2  OD.1.1+OD.1.2
ps_vln   NOFILE   6  6  0 0 2  PS.1+PS.2.1
sp_vln   NOFILE  12 12  0 0 0  SP.1.1+SP.2.1
sn_vln   NOFILE  12 12  0 0 0  SN.1.1+SN.2.1
con_vln  NOFILE   6  6 -1 6 2  CON.1.1+CON.2.1
cop_vln  NOFILE   6  6 -1 6 2  COP.1.1+COP.2.1
cps_vln  NOFILE   6  6 -1 6 2  CPS.1.1+CPS.2.1
in_vln   NOFILE   7  7  0 0 2  IN.1.1+IN.2.1
cb_vln   NOFILE 150 80  0 0 2  CB.3.1+CB.4.1
.fE
Each line of one of these files must contain
the following items in the order given:
.AL
.LI
The name of the file to be checked.
.LI
Eventually the name of an help_layer; if not needed 'NOFILE'
is coded here.
If a layer is specified errors will only be reported
in places where this layer is not present.
.LI
The minimum width of elements on the file.
If it is zero no check will be carried out.
.LI
The minimum gap between two elements on the file.
If it is zero no check will be carried out.
.LI
The minimum gap between elements on the file for short
lengths of the gap.
.SP 0
If a negative value is given here the program \fIdimcheck\fP
will interpret it as an maximum width check,
with the maximum value for the width given in the
next item.
.LI
The maximum length of the gap for which the reduced
gap may be applied,
or if the previous item is negative the maximum value
of the width permitted.
.LI
The value for kind. This variable may have one of the
following values:
. \".DL
.LB 4 0 2 2 1 0 2
.LI 0:
gap_errors between edges of the same polygon
and errors stemming from touching corners will not be
reported.
.LI 1:
errors stemming from touching corners will not be
reported, but gap_errors between edges of the same polygon
will be reported.
.LI 2:
gap_errors between edges of the same polygon will not be
reported, but errors stemming from touching corners will be.
.LI 3:
gap_errors between edges of the same polygon will be
reported as well as errors stemming from touching corners.
.LE
.LI
A string indicating the design rule(s) involved.
.LE
.SP
In this example only primary vln files are used.
However one may also use vln files made by \fInbool\fP,
so files bool_nn.
.LI
The file dubcheckdata, read by the program \fIdubcheck\fP,
contains the names of the files to be checked and
the gap and overlap dimensions.
.P
Example:
.fS
  bool_0  nw_vln NOFILE 0 20 0 0 0 OD.3.1 (OD - NW)
  bool_2  nw_vln NOFILE 0 20 0 0 0 OD.3.2 (p+OD - NW)
  bool_1  nw_vln NOFILE 10 0 0 0 0 OD.4.1.1 (ovlp NW - OD)
  bool_3  ps_vln od_vln  5 0 0 0 2 PS.3.1 (ovlp PS - gate)
  od_vln  ps_vln NOFILE  0 3 0 0 0 PS.4.1 (PS - OD)
  bool_3  od_vln ps_vln  6 0 0 0 2 PS.5.1 (ovlp OD - gate)
  bool_1  bool_4 NOFILE  6 0 0 0 0 SP.3.1 (ovlp SP - OD)
  bool_5  sp_vln NOFILE  6 0 0 0 1 SP.3.2 (ovlp SP - p_chan_gate)
  bool_8  bool_7 od_vln  0 0 0 0 4 SP.3.3+SN.4.3 (det_hor_connection)
  bool_8  bool_7 od_vln  0 0 0 0 5 SP.3.3+SN.4.3 (det_ver_connection)
  bool_8  bool_6 NOFILE 12 0 0 0 3 SP.3.3+SN.4.3 (ovlp OD - nwell_cont)
  od_vln  sp_vln NOFILE  0 6 0 0 1 SP.4.1 (SP - OD)
  sp_vln  bool_5 NOFILE  0 6 0 0 0 SP.4.2 (SP - n_chan_gate)
  bool_0  bool_4 NOFILE  6 0 0 0 0 SN.3.1 (ovlp SN - OD)
  bool_9  sn_vln NOFILE  6 0 0 0 0 SN.3.2 (ovlp SN - n_chan_gate)
  bool_2  bool_7 od_vln  0 0 0 0 4 SN.3.3+SP.4.3 (det_hor_connection)
  bool_2  bool_7 od_vln  0 0 0 0 5 SN.3.3+SP.4.3 (det_ver_connection)
  bool_2  bool_6 NOFILE 12 0 0 0 3 SN.3.3+SP.4.3 (ovlp OD - substr_cont)
  od_vln  sn_vln NOFILE  0 6 0 0 1 SN.4.1 (SN - OD)
  od_vln  bool_9 NOFILE  0 6 0 0 0 SN.4.2 (SN - p_chan_gate)
  bool_12 od_vln NOFILE  5 0 0 0 0 CON.3.1 (ovlp OD - CON)
  bool_12 ps_vln NOFILE  0 5 0 0 0 CON.3.2 (CON - PS)
  bool_13 sn_vln NOFILE  3 0 0 0 0 CON.3.3 (ovlp CON - SN)
  sp_vln bool_13 NOFILE  0 3 0 0 0 CON.3.4 (CON - SP)
  bool_14 od_vln NOFILE  5 0 0 0 0 COP.3.1 (ovlp OD - COP)
  bool_14 ps_vln NOFILE  0 5 0 0 0 COP.3.2 (COP - PS)
.fE
.fS
  bool_15 sp_vln NOFILE  3 0 0 0 0 COP.3.3 (ovlp COP - SP)
  sn_vln bool_15 NOFILE  0 3 0 0 0 COP.3.4 (COP - SN)
  bool_17 ps_vln NOFILE  4 0 0 0 0 CPS.4.2 (ovlp CPS - PS)
  bool_17 od_vln NOFILE  0 5 0 0 0 CPS.4.3 (CPS - OD)
  bool_19 in_vln NOFILE  3 0 0 0 0 IN.3.2 (ovlp CO - IN)
  bool_20 in_vln NOFILE 10 0 0 0 0 CB.1.1 (ovlp CB - IN)
.fE
Each line of this file must contain the following items in the order given:
.AL
.LI
The first file involved with the operation.
In case of overlap check this is the file of whose
elements have to be overlapped.
.LI
The second file involved with the operation.
In case of overlap check this is the file whose elements have
to overlap the elements of the first file.
.LI
A helpfile involved in the operation.
This file is used for checks with a certain kind.
If not needed, 'NOFILE' is coded.
.LI
The overlap the second file must have over the first file.
If it is zero, no overlap check will be carried out.
.LI
The minimal gap between non overlapping elements of the
first and second file.
If it is zero no check will be carried out.
.LI
The minimal gap that must be maintained if the length of
the gap is only small.
.LI
The maximum gaplength for which the reduced gap value may be
applied.
.LI
The value of the variable kind.
For gap checks the value of kind means:
.\".VL 5
.LB 4 0 2 2 1 0 2
.LI 0:
do not suppress gap errors of overlapping items.
.LI 1:
suppress gap errors of overlapping items.
.LE
.P
For overlap checks the value of kind means:
.\".VL 5
.LB 4 0 2 2 1 0 2
.LI 0:
check for a total overlap.
.LI 1:
check for overlap over two opposite sides.
.LI 2:
only check the overlap for places where the helplay is
not present.
.LI 3:
check only at the sides indicated by the conn_dir array.
This array will be filled using checks with kind = 4 and kind = 5.
.LI 4:
sets the conn_dir array to 'check bottom and top overlap' if
in the same polygon of the helplayer
there is one area of the second layer present  to the left
and one to the right of an area of the first layer.
.LI 5:
sets the conn_dir array to 'check left and right overlap' if
in the same polygon of the helplayer
there is one area of the second layer present  to the bottom
and one to the top of an area of the first layer.
.LE
.LI
A string indicating which design rules are involved.
.LE
