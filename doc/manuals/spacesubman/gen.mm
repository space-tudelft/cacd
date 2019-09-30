.H 1 "General Parameters"
.sY %SE_GENPAR%  \n(H1
There are some parameters for substrate resistance
extraction that can be used both with
the boundary-element method and the interpolation method.
These parameters are described below.
.DS I
elim_sub_node  \fIboolean\fP    (default: off)
.DE
If this parameter is set, the substrate node "SUBSTR"
will be eliminated after substrate resistance extraction.
This option can not be used when extracting capacitances.
.DS I
elim_sub_term_node  \fIboolean\fP    (default: off)
.DE
If this parameter is set, nodes corresponding to substrate
terminals will be eliminated,
unless they are retained because of another reason,
like being connection of a transistor.
.P
The following two parameters are useful when using
.P= Xspace
/
.P= helios.
.DS I
disp.save_prepass_image  \fIboolean\fP    (default: off)
.DE
During substrate resistance extraction,
a preprocessing step is executed.
If this parameter is set, the 
image that is generated by
.P= Xspace
/
.P= helios
during the first pass
will not be erased but will also be shown
during subsequent passes.
.DS I
disp.fill_sub_term  \fIboolean\fP    (default: off)
.DE
When the above parameter is on,
substrate terminals are drawn with their tile (mask) color.
When it is off, only the border of the substrate terminals
is drawn.